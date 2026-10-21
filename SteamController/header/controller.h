#pragma once
#include <iostream>
#include <unordered_set>
#include <SDL3/SDL.h>

enum ButtonBindings
{
	LStickCapacitiveTouch = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC3,
	RStickCapacitiveTouch = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC4,
	LTrackpadClick = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_TOUCHPAD,
	RTrackpadClick = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC2,
	R4Pressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,
	L4Pressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,
	R5Pressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,
	L5Pressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,
	LGripsense = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC5,
	RGripsense = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC6,
	QuickAccessMenu = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC1,
	APressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_SOUTH,
	BPressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_EAST,
	XPressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_WEST,
	YPressed = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_NORTH,
	DPadUp = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_UP,
	DPadRight = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
	DPadDown = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_DOWN,
	DPadLeft = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_LEFT,
	LShoulder = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
	RShoulder = SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER
};

struct Vec3
{
	float x = 0;
	float y = 0;
	float z = 0;

	Vec3() { x = 0; y = 0; z = 0; };
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {};
};

struct Vec2
{
	float x = 0;
	float y = 0;

	Vec2() { x = 0; y = 0; };
	Vec2(float x, float y) : x(x), y(y) {};
};

class Controller
{
public:
	Controller();
	~Controller();

	SDL_AppResult Initialize(SDL_JoystickID connectedJoystickId = 0);
	SDL_Gamepad*& GetGamepad() { return gamepad; };
	SDL_JoystickID GetJoystickID() const { return joystickID; };

	const char* GetGamepadName() { return gamepadName; };

	void DrawDebugMenu();
	void SetButtonDownState(int buttonID, bool state);

	bool GetButtonPressed(ButtonBindings button);
	const char* GetButtonNameFromEnum(int buttonID);

	const Vec3 GetGyro();
	const Vec2 GetJoystick(bool left);

private:
	void ApplyDeadzone(float& value) const;

	const char* gamepadName = "Steam Controller";
	std::unordered_set<int> pressedButtonIds;

	SDL_Gamepad* gamepad;
	SDL_JoystickID joystickID;

	const float deadzone = .25f;
};