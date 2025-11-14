#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		for (const auto& sphere : m_SphereGeometries)
		{
			GeometryUtils::HitTest_Sphere(sphere, ray, closestHit);
		}
		for (const auto& plane : m_PlaneGeometries)
		{
			GeometryUtils::HitTest_Plane(plane, ray, closestHit);
		}
		for (const auto& mesh: m_TriangleMeshGeometries)
		{
			GeometryUtils::HitTest_TriangleMesh(mesh, ray, closestHit);
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		for (const auto& sphere : m_SphereGeometries)
		{
			if (GeometryUtils::HitTest_Sphere(sphere, ray))
				return true;
		}
		for (const auto& plane : m_PlaneGeometries)
		{
			if (GeometryUtils::HitTest_Plane(plane, ray))
				return true;
		}
		for (const auto& mesh: m_TriangleMeshGeometries)
		{
			if (GeometryUtils::HitTest_TriangleMesh(mesh, ray))
				return true;
		}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion
#pragma region SCENE_REFERENCE
	void Scene_Reference::Initialize()
	{
		sceneName = "Reference Scene";
		m_Camera.origin = { 0.f,3.f,-9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//Planes
		AddPlane(Vector3{ 0.f,0.f,10.f }, Vector3{ 0.f,0.f,-1.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 0.f,10.f,0.f }, Vector3{ 0.f,-1.f,0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 5.f,0.f,0.f }, Vector3{ -1.f,0.f,0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ -5.f,0.f,0.f }, Vector3{ 1.f,0.f,0.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere(Vector3{ -1.75f,1.f,0.f }, 0.75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f,1.f,0.f }, 0.75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f,1.f,0.f }, 0.75f, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75f,3.f,0.f }, 0.75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f,3.f,0.f }, 0.75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f,3.f,0.f }, 0.75f, matCT_GraySmoothPlastic);



		//CW Winding Order!
		const Triangle baseTriangle = { Vector3(-0.75f,1.5f,0.f), Vector3(0.75f,0.f,0.f), Vector3(-0.75f,0.f,0.f) };

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->Translate({ -1.75f,4.5f,0.f });
		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->Translate({ 0.f,4.5f,0.f });
		m_Meshes[1]->UpdateAABB();
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->Translate({ 1.75f,4.5f,0.f });
		m_Meshes[2]->UpdateAABB();
		m_Meshes[2]->UpdateTransforms();


		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, 0.61f, 0.45f }); // Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, 0.8f, 0.45f }); // Frontlight
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ 0.34f, 0.47f, 0.68f });
	}

	void Scene_Reference::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const auto yawAngle = (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;
		for (const auto m : m_Meshes)
		{
			m->RotateY(yawAngle);
			m->UpdateTransforms();
		}
	}
#pragma endregion
#pragma region SCENE_BUNNY
	void Scene_Bunny::Initialize()
	{
		sceneName = "Bunny Scene";
		m_Camera.origin = { 0.f,3.f,-9.f };
		m_Camera.fovAngle = 45.f;

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//Planes
		AddPlane(Vector3{ 0.f,0.f,10.f }, Vector3{ 0.f,0.f,-1.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 0.f,10.f,0.f }, Vector3{ 0.f,-1.f,0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 5.f,0.f,0.f }, Vector3{ -1.f,0.f,0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ -5.f,0.f,0.f }, Vector3{ 1.f,0.f,0.f }, matLambert_GrayBlue);

		//Bunny obj
		m_pBunnyMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny.obj",
			m_pBunnyMesh->positions,
			m_pBunnyMesh->normals,
			m_pBunnyMesh->indices);
		m_pBunnyMesh->Scale({ 2.f,2.f,2.f });
		m_pBunnyMesh->RotateY(PI);
		m_pBunnyMesh->UpdateAABB();
		m_pBunnyMesh->UpdateTransforms();

		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, 0.61f, 0.45f }); // Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, 0.8f, 0.45f }); // Frontlight
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ 0.34f, 0.47f, 0.68f });

	}

	void Scene_Bunny::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const auto yawAngle = (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;
		m_pBunnyMesh->RotateY(yawAngle);
		m_pBunnyMesh->UpdateTransforms();
	}
#pragma endregion
}
