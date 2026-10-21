#include "../header/controller.h"
#include <imgui.h>
#include <format>


Controller::Controller() 
{
	gamepad = nullptr;
	joystickID = 0;
};
Controller::~Controller()
{
	if (gamepad != nullptr)
	{
		SDL_CloseGamepad(gamepad);
		gamepad = nullptr;
	}
}

SDL_AppResult Controller::Initialize(SDL_JoystickID connectedJoystickId)
{
	if (gamepad != nullptr)
	{
		std::cout << "Initialize has already run. Aborting.." << std::endl;
		return SDL_APP_FAILURE;
	}

	if (!SDL_HasGamepad)
	{
		std::cout << "No controllers connected." << std::endl;
		return SDL_APP_FAILURE;
	}

	if (connectedJoystickId == 0)
	{
		int controllerCount = 0;
		SDL_JoystickID* ids = SDL_GetGamepads(&controllerCount);

		for (size_t i = 0; i < controllerCount; i++)
		{
			SDL_Gamepad* foundGamepad = SDL_OpenGamepad(ids[i]);
			const char* foundGamepadName = SDL_GetGamepadName(foundGamepad);

			if (SDL_strcmp(foundGamepadName, gamepadName) == 0)
			{
				gamepad = foundGamepad;
				joystickID = ids[i];
				break;
			}
			else
			{
				SDL_CloseGamepad(foundGamepad);
			}
		}
	}
	else
	{
		SDL_Gamepad* foundGamepad = SDL_OpenGamepad(connectedJoystickId);
		const char* foundGamepadName = SDL_GetGamepadName(foundGamepad);

		if (SDL_strcmp(foundGamepadName, gamepadName) == 0)
		{
			gamepad = foundGamepad;
			joystickID = connectedJoystickId;
		}
	}

	if (!gamepad)
	{
		std::cout << "Failed to open gamepad: " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	if (!SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_GYRO, true))
	{
		std::cout << "Failed to enable gyro: " << SDL_GetError() << std::endl;
	}

	std::cout << "Gamepad has been found and initialized" << std::endl;
	return SDL_APP_SUCCESS;
}

void Controller::DrawDebugMenu()
{
	ImGuiWindowFlags windowFlags = 0;

	windowFlags |= ImGuiWindowFlags_NoCollapse;
	windowFlags |= ImGuiWindowFlags_NoResize;

	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 650, mainViewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 1280), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Controller Debug Menu", 0, windowFlags))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Controller Debug:");
	ImGui::Spacing();

	if (gamepad)
	{
		ImGui::Text("Controlller Name: %s", SDL_GetGamepadName(gamepad));
		ImGui::Spacing();

		ImGui::Text("Input");

		Vec3 gyro = GetGyro();
		ImGui::Text("Gyro: %f, %f, %f", gyro.x, gyro.y, gyro.z);

		Vec2 leftJoystick = GetJoystick(true);
		ImGui::Text("Left Stick: %f, %f", leftJoystick.x, leftJoystick.y);

		Vec2 rightJoystick = GetJoystick(false);
		ImGui::Text("Right Stick: %f, %f", rightJoystick.x, rightJoystick.y);

		std::string inputButtonLabel = std::format("Controller Buttons Pressed {}", pressedButtonIds.size());

		if (ImGui::TreeNode(inputButtonLabel.c_str()))
		{
			for (int buttonID : pressedButtonIds)
			{
				ImGui::BulletText("%s", GetButtonNameFromEnum(buttonID));
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

bool Controller::GetButtonPressed(ButtonBindings button)
{
	return SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)button);
}
const char* Controller::GetButtonNameFromEnum(int buttonID)
{
	switch ((ButtonBindings)buttonID)
	{
	case ButtonBindings::LStickCapacitiveTouch:
		return "LStickCapacitiveTouch";
	case ButtonBindings::RStickCapacitiveTouch:
		return "RStickCapacitiveTouch";
	case ButtonBindings::LTrackpadClick:
		return "LTrackpadClick";
	case ButtonBindings::RTrackpadClick:
		return "RTrackpadClick";
	case ButtonBindings::R4Pressed:
		return "R4Pressed";
	case ButtonBindings::L4Pressed:
		return "L4Pressed";
	case ButtonBindings::R5Pressed:
		return "R5Pressed";
	case ButtonBindings::L5Pressed:
		return "L5Pressed";
	case ButtonBindings::LGripsense:
		return "LGripsense";
	case ButtonBindings::RGripsense:
		return "RGripsense";
	case ButtonBindings::QuickAccessMenu:
		return "QuickAccessMenu";
	case ButtonBindings::APressed:
		return "APressed";
	case ButtonBindings::BPressed:
		return "BPressed";
	case ButtonBindings::XPressed:
		return "XPressed";
	case ButtonBindings::YPressed:
		return "YPressed";
	case ButtonBindings::DPadUp:
		return "DPadUp";
	case ButtonBindings::DPadRight:
		return "DPadRight";
	case ButtonBindings::DPadDown:
		return "DPadDown";
	case ButtonBindings::DPadLeft:
		return "DPadLeft";
	default:
		return "";
	}
}

const Vec3 Controller::GetGyro()
{
	//TODO: Add deadzone if need be

	if (SDL_GamepadHasSensor(gamepad, SDL_SENSOR_GYRO))
	{
		float data[3];

		if (SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_GYRO, data, 3))
		{
			return Vec3(data[0], data[1], data[2]);
		}
	}

	return Vec3();
}
const Vec2 Controller::GetJoystick(bool left)
{
	float x = 0;
	float y = 0;

	if (left)
	{
		x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
		y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
	}
	else
	{
		x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
		y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
	}

	x /= SDL_JOYSTICK_AXIS_MAX;
	y /= SDL_JOYSTICK_AXIS_MAX;

	ApplyDeadzone(x);
	ApplyDeadzone(y);

	return Vec2(x, y);
}
void Controller::ApplyDeadzone(float& value) const
{
	float absValue = std::abs(value);

	if (absValue <= deadzone)
	{
		value = 0.0f;
		return;
	}

	float sign = (value > .0f) ? 1.0f : -1.0f;

	value = (sign * (absValue - deadzone) / (1.0f - deadzone));
}

void Controller::SetButtonDownState(int buttonID, bool state)
{
	if (state)
	{
		pressedButtonIds.insert(buttonID);
	}
	else if (!state)
	{
		pressedButtonIds.erase(buttonID);
	}
}