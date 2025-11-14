#pragma once
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			const ColorRGB rho{ cd * kd };
			return rho / PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			const ColorRGB rho{ cd * kd };
			return rho / PI;
		}

		/**
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//light direction is inverted because Phong shading requires vector to point toward surface
			const Vector3 reflect{ -l - 2.f * Vector3::Dot(-l,n) * n };
			float cosAlfa{ Vector3::Dot(reflect, v) };
			cosAlfa = std::max(cosAlfa, 0.f);

			const float specularReflection{ ks * powf(cosAlfa,exp) };
			return ColorRGB{ specularReflection,specularReflection,specularReflection };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			const float hDotv{ std::max(Vector3::Dot(h,v),0.f) };
			constexpr ColorRGB one{ 1.f,1.f,1.f };
			return f0 + ( one - f0) * powf((1.f - hDotv), 5.f);
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//Using UE definition for roughness, alpha is roughness squared
			const float alpha{ Square(roughness) };
			const float alphaSqrd{ Square(alpha) };

			//Calculate formula denominator
			const float nDothSqr{ Square(std::max(Vector3::Dot(n,h), 0.f)) };
			const float denominator{ PI * powf(nDothSqr * (alphaSqrd - 1) + 1, 2.f) };

			return alphaSqrd / denominator;
		}

		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//Using UE definition for roughness, alpha is roughness squared
			const float alpha{ Square(roughness) };
			const float k{ powf(alpha + 1, 2.f) / 8.f }; //Direct lighting

			const float nDotv{ std::max(Vector3::Dot(n,v), 0.f) };
			const float denominator{ nDotv * (1 - k) + k };

			return nDotv / denominator;
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			return GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
		}

	}
}