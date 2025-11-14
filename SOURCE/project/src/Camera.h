#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include "Math.h"
#include "Timer.h"

#include <iostream>

namespace dae
{
	struct Camera final
	{
		Camera() = default;
		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{}

		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f }; //upward/downward
		float totalYaw{ 0.f }; //left/right

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{
			forward.Normalize();
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			cameraToWorld = Matrix(right, up, forward, origin);
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const auto forwardMove{ pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S] };
			const auto rightMove{ pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A] };
			if (forwardMove || rightMove)
			{
				constexpr float keyboardMovementSpeed{ 10.f };

				origin += static_cast<float>(forwardMove) * forward * keyboardMovementSpeed * deltaTime;
				origin += static_cast<float>(rightMove) * right * keyboardMovementSpeed * deltaTime;
			}


			//Mouse Input
			constexpr float mouseMovementSpeed{ 1.f };
			constexpr float rotationSpeed{ .1f };

			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			{
				//Moves up and down
				origin -= static_cast<float>(mouseY) * up * mouseMovementSpeed * deltaTime;
			}
			else if (mouseState == SDL_BUTTON_LMASK)
			{
				//Moves the camera forward & backward and rotates left & right
				origin -= static_cast<float>(mouseY) * forward * mouseMovementSpeed * deltaTime;
				totalYaw += static_cast<float>(mouseX) * rotationSpeed * deltaTime;
			}
			else if (mouseState == SDL_BUTTON_RMASK)
			{
				//Rotate the viewport camera
				totalPitch -= static_cast<float>(mouseY) * rotationSpeed * deltaTime;
				totalYaw += static_cast<float>(mouseX) * rotationSpeed * deltaTime;
			}

			//Create and apply rotation matrix
			const Matrix finalRotation{ Matrix::CreateRotation(totalPitch, totalYaw, 0.f) };
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
		}
	};
}
