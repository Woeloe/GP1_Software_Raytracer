//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <execution>
#define PARALLEL_EXECUTION

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	const float fov = tan(camera.fovAngle * TO_RADIANS / 2.f);

#if defined(PARALLEL_EXECUTION)
	//Parallel Logic
	const uint32_t amountOfPixels{ static_cast<uint32_t>(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};
	pixelIndices.reserve(amountOfPixels);
	for (uint32_t index{}; index < amountOfPixels; ++index) pixelIndices.emplace_back(index);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(),
		[&](const uint32_t i) {
		RenderPixel(pScene, i, fov, aspectRatio, cameraToWorld, camera.origin);
	});

#else
	//Synchronous logic (no threading)
	const uint32_t amountOfPixels{ static_cast<uint32_t>(m_Width * m_Height) };
	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; ++pixelIndex)
	{
		RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(const Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio,
	const Matrix& cameraToWorld, const Vector3& cameraOrigin) const
{
	const auto& materials = pScene->GetMaterials();
	const auto& lights = pScene->GetLights();

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	//Calculate NDC coordinates
	const float x = (2.f * ((static_cast<float>(px)+0.5f) / static_cast<float>(m_Width)) -1.f) * aspectRatio * fov;
	const float y = (1.f - 2.f * ((static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height))) * fov;

	Vector3 rayDirection{ x, y, 1.f };
	rayDirection = cameraToWorld.TransformVector(rayDirection.Normalized());

	//Ray we are casting from camera towards each pixel
	const Ray viewRay{ cameraOrigin, rayDirection };

	//Color to write to the color buffer (default = black)
	ColorRGB finalColor{};

	//HitRecord containing more information about a potential hit
	HitRecord closestHit{};
	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		for (const Light& light : lights)
		{
			const Vector3 hitOrigin{ closestHit.origin };
			Vector3 directionToLight{ LightUtils::GetDirectionToLight(light, hitOrigin) };
			const float rayMax{ directionToLight.Normalize() };

			if (m_ShadowsEnabled)
			{
				const Ray shadowRay{ hitOrigin, directionToLight, 0.001f, rayMax };
				if (pScene->DoesHit((shadowRay))) continue;
			}

			const ColorRGB radiance{ LightUtils::GetRadiance(light, closestHit.origin) };
			const float observedArea{ std::max(Vector3::Dot(closestHit.normal, directionToLight), 0.f) };
			const ColorRGB BRDF{ materials[closestHit.materialIndex]->Shade(closestHit, directionToLight, -rayDirection) }; //invert rayDirection

			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				finalColor += {observedArea, observedArea, observedArea};
				break;
			case LightingMode::Radiance:
				finalColor += radiance;
				break;
			case LightingMode::BRDF:
				finalColor += BRDF;
				break;
			case LightingMode::Combined:
				finalColor += radiance * BRDF * observedArea;
				break;
			}
		}

	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	int nextState = static_cast<int>(m_CurrentLightingMode) + 1;
	nextState %= 4;
	m_CurrentLightingMode = static_cast<LightingMode>(nextState);

	std::cout << "[LIGHTING MODE]:\t";
	switch (m_CurrentLightingMode)
	{
	case LightingMode::ObservedArea:
		std::cout << "OBSERVED AREA";
		break;
	case LightingMode::Radiance:
		std::cout << "RADIANCE";
		break;
	case LightingMode::BRDF:
		std::cout << "BRDF";
		break;
	case LightingMode::Combined:
		std::cout << "COMBINED";
		break;
	}
	std::cout << "\n";
}

void Renderer::ToggleShadows()
{
	m_ShadowsEnabled = not m_ShadowsEnabled;

	std::cout << "[SHADOWS]:\t";
	if (m_ShadowsEnabled) 
		std::cout << "ON\n";
	else 
		std::cout << "OFF\n";
}
