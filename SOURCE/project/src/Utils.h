#pragma once
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			
			//Calculate Vector between origins of ray and sphere
			const Vector3 rayToSphereOrigin{ sphere.origin - ray.origin };

			//Calculate t value for middle of chord made by the ray
			const float tAdjacent{ Vector3::Dot(rayToSphereOrigin, ray.direction)};

			//Calculate distance of tAdjacent and origin of sphere
			const float oppositeSideSqrd{ rayToSphereOrigin.SqrMagnitude() - Square(tAdjacent) };

			//Calculate difference (in t) between tAdjacent and border of sphere
			const float tDelta{ sqrt(Square(sphere.radius) - oppositeSideSqrd) };

			//t value of intersection between ray and sphere
			const float t0{ tAdjacent - tDelta };
			const float t1{ tAdjacent + tDelta };

			float t;
			if (t0 > ray.min && t0 < ray.max)
			{
				//If both t0 and t1 are in range then t0 should be the closest hit
				//So we check t0 first
				t = t0;
			}
			else
			{
				if (t1 < ray.min || t0 > ray.max)
				{
					//Both t values are out of range -> no hit
					return false;
				}

				t = t1;
			}


			if(t > ray.min && t < ray.max && t < hitRecord.t)
			{
				if (not ignoreHitRecord)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
				}
				return true;
			}
			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//Calculate Vector between origins of ray and plane
			const Vector3 rayToPlaneOrigin{ plane.origin - ray.origin };

			//calculate t value
			const float t{ Vector3::Dot(rayToPlaneOrigin, plane.normal) / Vector3::Dot(ray.direction, plane.normal) };

			if (t > ray.min && t < ray.max && t < hitRecord.t)
			{
				if (not ignoreHitRecord)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = plane.normal;
				}
				return true;
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//Use temporary hitRecord so we don't override previous data if no hit occurs
			HitRecord temp{};

			//Check if ray is parallel to triangle
			if (AreEqual(Vector3::Dot(triangle.normal, ray.direction), 0.f))
				return false;

			//Cull Mode Check
			TriangleCullMode cullMode{ triangle.cullMode };
			if (ignoreHitRecord && cullMode != TriangleCullMode::NoCulling)
			{
				//We assume 'ignoreHitRecord == true' means we are performing a shadow hittest
				//When performing shadow hittest, culling mode must be inverted
				cullMode = (cullMode == TriangleCullMode::FrontFaceCulling)
					           ? TriangleCullMode::BackFaceCulling
					           : TriangleCullMode::FrontFaceCulling;

			}
			switch (cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (Vector3::Dot(triangle.normal, ray.direction) < 0.f) return false;
				break;
			case TriangleCullMode::BackFaceCulling:
				if (Vector3::Dot(triangle.normal, ray.direction) > 0.f) return false;
				break;
			case TriangleCullMode::NoCulling:
				break;
			}

			//Check if ray hits plane of triangle
			if (not HitTest_Plane(Plane{ triangle.v0, triangle.normal, triangle.materialIndex }, ray, temp))
				return false;
			//Check if hit is not closer than previous hit in hitRecord
			if (temp.t > hitRecord.t)
				return false;


			//INSIDE OUTSIDE TEST
			//Is intersection point to the 'right side' of each edge
			Vector3 edge{}, pointToVertex{}, cross{};
			const Vector3 intersectionPoint{ temp.origin }; //intersection with plane

			//Edge v0 -> v1
			edge = triangle.v1 - triangle.v0;
			pointToVertex = intersectionPoint - triangle.v0;
			cross = Vector3::Cross(edge, pointToVertex); //no need to normalize, we only check if < 0
			if (Vector3::Dot(cross, triangle.normal) < 0.f)
				//Point is on the left side of edge -> outside triangle
				return false;

			//Edge v1 -> v2
			edge = triangle.v2 - triangle.v1;
			pointToVertex = intersectionPoint - triangle.v1;
			cross = Vector3::Cross(edge, pointToVertex); //no need to normalize, we only check if < 0
			if (Vector3::Dot(cross, triangle.normal) < 0.f)
				//Point is on the left side of edge -> outside triangle
				return false;

			//Edge v2 -> v0
			edge = triangle.v0 - triangle.v2;
			pointToVertex = intersectionPoint - triangle.v2;
			cross = Vector3::Cross(edge, pointToVertex); //no need to normalize, we only check if < 0
			if (Vector3::Dot(cross, triangle.normal) < 0.f)
				//Point is on the left side of edge -> outside triangle
				return false;
			//INSIDE OUTSIDE TEST was successful


			//Flip normal when hitting a back facing triangle, so lighting is correct
			const bool isBackFace{ Vector3::Dot(temp.normal, ray.direction) > 0.f };
			if (isBackFace) 
				temp.normal = -temp.normal;

			//Point is inside triangle, use info from temp HitRecord
			hitRecord = temp;
			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangleMesh SlabTest
		//Slab test to check if ray intersects with bounding box of mesh
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			const float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			const float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			const float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			const float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			const float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			const float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//slabTest
			if (!SlabTest_TriangleMesh(mesh, ray)) return false;

			//Use temporary hitRecord to avoid false positive from previous hitTest
			HitRecord temp{};

			for (size_t normalIndex{}; normalIndex < mesh.transformedNormals.size(); ++normalIndex)
			{
				const size_t tripletIndex{ normalIndex * 3 }; //3 indices for every normal/triangle
				const Vector3& v0{ mesh.transformedPositions[mesh.indices[tripletIndex]] },
					& v1{ mesh.transformedPositions[mesh.indices[tripletIndex + 1]] },
					& v2{ mesh.transformedPositions[mesh.indices[tripletIndex + 2]] };

				Triangle triangle{ v0,v1,v2, mesh.transformedNormals[normalIndex] };
				triangle.cullMode = mesh.cullMode;
				//Material index does not need to be set, will be done at the end if a hit is found
				HitTest_Triangle(triangle, ray, temp, ignoreHitRecord);
			}

			if (temp.didHit && temp.t < hitRecord.t)
			{
				if (not ignoreHitRecord)
				{
					hitRecord = temp;
					hitRecord.materialIndex = mesh.materialIndex;
					//Update material if mesh is the closest hit, instead of setting for every triangle
				}
				return true;
			}
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion

	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			switch (light.type)
			{
			case(LightType::Point):
				return Vector3{ origin, light.origin };
			case(LightType::Directional):
				return -origin;
			default:
				return Vector3{};
			}
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			switch (light.type)
			{
			case(LightType::Point):
				{
					const float radius{(light.origin - target).Magnitude() };
					return light.color * light.intensity / Square(radius);
				}
			case(LightType::Directional):
				return light.color * light.intensity;
			default:
				return ColorRGB{};
			}
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}