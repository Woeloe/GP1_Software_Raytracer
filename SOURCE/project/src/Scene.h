#pragma once
#include <string>
#include <vector>
#include "Math.h"
#include "DataTypes.h"
#include "Camera.h"

namespace dae
{
	//Forward Declarations
	class Timer;
	class Material;
	struct Plane;
	struct Sphere;
	struct Light;

	//Scene Base Class
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		Camera& GetCamera() { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
		bool DoesHit(const Ray& ray) const;

		const std::vector<Plane>& GetPlaneGeometries() const { return m_PlaneGeometries; }
		const std::vector<Sphere>& GetSphereGeometries() const { return m_SphereGeometries; }
		const std::vector<Light>& GetLights() const { return m_Lights; }
		const std::vector<Material*> GetMaterials() const { return m_Materials; }

	protected:
		std::string	sceneName;

		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};
		std::vector<Light> m_Lights{};
		std::vector<Material*> m_Materials{};

		Camera m_Camera{};

		Sphere* AddSphere(const Vector3& origin, float radius, unsigned char materialIndex = 0);
		Plane* AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

		Light* AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color);
		Light* AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color);
		unsigned char AddMaterial(Material* pMaterial);
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//Reference Scene
	class Scene_Reference final : public Scene
	{
	public:
		Scene_Reference() = default;
		~Scene_Reference() override = default;

		Scene_Reference(const Scene_Reference&) = delete;
		Scene_Reference(Scene_Reference&&) noexcept = delete;
		Scene_Reference& operator=(const Scene_Reference&) = delete;
		Scene_Reference& operator=(Scene_Reference&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;

	private:
		TriangleMesh* m_Meshes[3]{};
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//Bunny Scene
	class Scene_Bunny final : public Scene
	{
	public:
		Scene_Bunny() = default;
		~Scene_Bunny() override = default;

		Scene_Bunny(const Scene_Bunny&) = delete;
		Scene_Bunny(Scene_Bunny&&) noexcept = delete;
		Scene_Bunny& operator=(const Scene_Bunny&) = delete;
		Scene_Bunny& operator=(Scene_Bunny&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;

	private:
		TriangleMesh* m_pBunnyMesh{nullptr};
	};
	
}
