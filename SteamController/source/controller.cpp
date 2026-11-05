#include "../header/controller.h"
#include <imgui.h>
#include <format>

#define AXIS_TRIGGER_MAX 32766

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

	if (!SDL_HasGamepad())
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
	ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 20, mainViewport->WorkPos.y + 20));
	ImGui::SetNextWindowSize(ImVec2(300, 400));

	if (!ImGui::Begin("Controller Debug Menu", 0, windowFlags))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Controller Debug");

	if (gamepad)
	{
		ImGui::Text("Controlller Name: %s", SDL_GetGamepadName(gamepad));

		int power = 0;
		std::string powerStateString = "";
		
		SDL_PowerState powerState = SDL_GetGamepadPowerInfo(gamepad, &power);
		
		switch (powerState)
		{
		case SDL_POWERSTATE_ON_BATTERY:
			powerStateString = "on battery";
			break;
		case SDL_POWERSTATE_CHARGING:
			powerStateString = "charging";
			break;
		case SDL_POWERSTATE_CHARGED:
			powerStateString = "full";
			break;
		case SDL_POWERSTATE_ERROR:
		case SDL_POWERSTATE_UNKNOWN:
		case SDL_POWERSTATE_NO_BATTERY:
		default:
			powerStateString = "unknown";
			break;
		}

		ImGui::Text("Battery Level: %i", power);
		ImGui::Text("Controller State: %s", powerStateString.c_str());
		ImGui::Spacing();

		ImGui::Text("Gyro:");

		Vec3 gyro = GetGyro();
		ImGui::Text("Gyro Rotation: %f, %f, %f", gyro.x, gyro.y, gyro.z);

		ImGui::Spacing();
		ImGui::Text("Joysticks:");

		Vec2 leftJoystickRaw = GetJoystickRaw(true);
		ImGui::Text("Raw Left Stick: %f, %f", leftJoystickRaw.x, leftJoystickRaw.y);

		Vec2 rightJoystickRaw = GetJoystickRaw(false);
		ImGui::Text("Raw Right Stick: %f, %f", rightJoystickRaw.x, rightJoystickRaw.y);

		Vec2 leftJoystickNormalized = GetJoystickNormalized(true, rightJoystickRaw);
		ImGui::Text("Raw Left Stick: %f, %f", leftJoystickNormalized.x, leftJoystickNormalized.y);

		Vec2 rightJoystickNormalized = GetJoystickNormalized(false, leftJoystickRaw);
		ImGui::Text("Raw Right Stick: %f, %f", rightJoystickNormalized.x, rightJoystickNormalized.y);
		ImGui::Spacing();

		ImGui::Text("Touchpad:");

		if (lTouchpadFinger)
		{
			ImGui::Text("Left Touchpad Pressed");
			ImGui::Text("Movement: X: %f, Y: %f", lTouchpadFinger->pos.x, lTouchpadFinger->pos.y);
			ImGui::Text("Pressure: %f", lTouchpadFinger->pressure);
		}
		else
		{
			ImGui::Text("Left Touchpad not pressed");
		}

		if (rTouchpadFinger)
		{
			ImGui::Text("Right Touchpad Pressed");
			ImGui::Text("Movement: X: %f, Y: %f", rTouchpadFinger->pos.x, rTouchpadFinger->pos.y);
			ImGui::Text("Pressure: %f", rTouchpadFinger->pressure);
		}
		else
		{
			ImGui::Text("Right Touchpad not pressed");
		}

		ImGui::Spacing();
		ImGui::Text("Triggers:");

		float leftTriggerRaw = GetTriggerAxisRaw(SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
		float leftTriggerNormalized = GetTriggerAxisNormalized(SDL_GAMEPAD_AXIS_LEFT_TRIGGER, leftTriggerRaw);

		ImGui::Text("Left Trigger Raw: %f", leftTriggerRaw);
		ImGui::Text("Left Trigger Normalized: %f", leftTriggerNormalized);

		float rightTriggerRaw = GetTriggerAxisRaw(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
		float rightTriggerNormalized = GetTriggerAxisNormalized(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, rightTriggerRaw);

		ImGui::Text("Right Trigger Raw: %f", rightTriggerRaw);
		ImGui::Text("Right Trigger Normalized: %f", rightTriggerNormalized);

		ImGui::Spacing();
		ImGui::Text("Buttons:");

		std::string inputButtonLabel = std::format("Controller Buttons Pressed: {}", pressedButtonIds.size());

		if (ImGui::TreeNode("Buttons", "%s", inputButtonLabel.c_str()))
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
		return "L Stick Capacitive Touch";
	case ButtonBindings::RStickCapacitiveTouch:
		return "R Stick Capacitive Touch";
	case ButtonBindings::LTrackpadClick:
		return "L Trackpad Click";
	case ButtonBindings::RTrackpadClick:
		return "R Trackpad Click";
	case ButtonBindings::R4Pressed:
		return "R4";
	case ButtonBindings::L4Pressed:
		return "L4";
	case ButtonBindings::R5Pressed:
		return "R5";
	case ButtonBindings::L5Pressed:
		return "L5";
	case ButtonBindings::LGripsense:
		return "L Gripsense";
	case ButtonBindings::RGripsense:
		return "R Gripsense";
	case ButtonBindings::QuickAccessMenu:
		return "Quick Access Menu";
	case ButtonBindings::APressed:
		return "A";
	case ButtonBindings::BPressed:
		return "B";
	case ButtonBindings::XPressed:
		return "X";
	case ButtonBindings::YPressed:
		return "Y Pressed";
	case ButtonBindings::DPadUp:
		return "D Pad Up";
	case ButtonBindings::DPadRight:
		return "D Pad Right";
	case ButtonBindings::DPadDown:
		return "D Pad Down";
	case ButtonBindings::DPadLeft:
		return "D Pad Left";
	case ButtonBindings::LShoulder:
		return "L Shoulder";
	case ButtonBindings::RShoulder:
		return "R Shoulder";
	case ButtonBindings::SelectPressed:
		return "Select";
	case ButtonBindings::SteamButtonPressed:
		return "Steam Button";
	case ButtonBindings::StartPressed:
		return "Start";
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

const float Controller::GetTriggerAxisRaw(SDL_GamepadAxis axis)
{
	if (axis != SDL_GAMEPAD_AXIS_LEFT_TRIGGER && axis != SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
	{
		printf("Controller::GetTriggerAxis: Left or Right Trigger axis expected. Instead got %i. Aborting call...", (int)axis);
		return -1.0f;
	}

	return SDL_GetGamepadAxis(gamepad, axis);	
}
const float Controller::GetTriggerAxisNormalized(SDL_GamepadAxis axis, float axisValue = -1.0f)
{
	if (axis != SDL_GAMEPAD_AXIS_LEFT_TRIGGER && axis != SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
	{
		printf("Controller::GetTriggerAxis: Left or Right Trigger axis expected. Instead got %i. Aborting call...", (int)axis);
		return -1.0f;
	}

	float value = axisValue == -1.0f ? GetTriggerAxisRaw(axis) : axisValue;

	value /= AXIS_TRIGGER_MAX;
	return value;
}

const Vec2 Controller::GetJoystickRaw(bool left)
{
	Vec2 value = Vec2();

	if (left)
	{
		value.x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
		value.y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
	}
	else
	{
		value.x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
		value.y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
	}

	return value;
}
const Vec2 Controller::GetJoystickNormalized(bool left, Vec2 joystickValue = Vec2(0, 0))
{
	Vec2 value = joystickValue.IsZero() ? GetJoystickRaw(left) :
		joystickValue;

	value.x /= SDL_JOYSTICK_AXIS_MAX;
	value.y /= SDL_JOYSTICK_AXIS_MAX;

	ApplyDeadzone(value.x);
	ApplyDeadzone(value.y);

	return value;
}

void Controller::RegisterFingerOnTouchpad(SDL_GamepadTouchpadEvent& event)
{
	bool fingerDown = event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN;
	bool leftFinger = event.touchpad == 0;

	if (fingerDown)
	{
		if (leftFinger)
		{
			lTouchpadFinger = TouchpadFinger(Vec2(event.x, event.y), event.pressure);
		}
		else
		{
			rTouchpadFinger = TouchpadFinger(Vec2(event.x, event.y), event.pressure);
		}
	}
	else
	{
		if (leftFinger)
		{
			lTouchpadFinger.reset();
		}
		else
		{
			rTouchpadFinger.reset();
		}
	}
}
void Controller::RegisterMovementOnTouchpad(SDL_GamepadTouchpadEvent& event)
{
	if (event.touchpad == 0)
	{
		if (lTouchpadFinger.has_value())
		{
			lTouchpadFinger->pos = Vec2(event.x, event.y);
			lTouchpadFinger->pressure = event.pressure;
		}
		else
		{
			printf("Error: Movement found on left touchpad, but finger was not registered");
			return;
		}
	}
	else
	{
		if (rTouchpadFinger.has_value())
		{
			rTouchpadFinger->pos = Vec2(event.x, event.y);
			rTouchpadFinger->pressure = event.pressure;
		}
		else
		{
			printf("Error: Movement found on right touchpad, but finger was not registered");
			return;
		}
	}
}

void Controller::ApplyDeadzone(float& value) const
{
	float absValue = std::abs(value);

	if (absValue <= joystickDeadzone)
	{
		value = 0.0f;
		return;
	}

	float sign = (value > .0f) ? 1.0f : -1.0f;

	value = (sign * (absValue - joystickDeadzone) / (1.0f - joystickDeadzone));
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