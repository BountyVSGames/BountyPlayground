#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "header/controller.h"

constexpr const char* WINDOW_TITLE = "Controller Visualizer";
constexpr int WIDTH = 1280;
constexpr int HEIGHT = 800;

static SDL_AppResult InitializeSDL(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font)
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		std::cout << "Video and Gamepad could not be initialized: " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}
	else
	{
		std::cout << "SDL video and gamepad are ready to go" << std::endl;
	}

	SDL_CreateWindowAndRenderer(WINDOW_TITLE, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer);

	if (!renderer || !window)
	{
		std::cout << "SDL could not create window/renderer: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return SDL_APP_FAILURE;
	}
	else
	{
		std::cout << "Renderer and Window were created succesfully" << std::endl;
	}

	if (!TTF_Init())
	{
		std::cout << "SDL_TTF could not be initialized: " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	font = TTF_OpenFont("arial.ttf", 24);

	if (!font)
	{
		std::cout << "Failed to open font: " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	return SDL_APP_SUCCESS;
}
static bool InitializeImGui(SDL_Window*& window, SDL_Renderer*& renderer)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
	{
		std::cout << "ImGui SDL3 Init failed" << std::endl;
		ImGui::DestroyContext();
		return false;
	}

	if (!ImGui_ImplSDLRenderer3_Init(renderer))
	{
		std::cout << "ImGui SDL Renderer Init Failed" << std::endl;
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
		return false;
	}

	std::cout << "ImGui Init Succes" << std::endl;

	return true;
}

int main()
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;
	SDL_Texture* fontTexture = nullptr;

	if (InitializeSDL(window, renderer, font) == SDL_APP_FAILURE)
	{
		std::cout << "Something went wrong while initializing SDL. Aborting..." << std::endl;
		return -1;
	}

	if (!InitializeImGui(window, renderer))
	{
		std::cout << "Something went wrong while initializing ImGui. Aborting..." << std::endl;
		return -1;
	}

	std::unique_ptr<Controller> controller = std::make_unique<Controller>();
	
	if (controller->Initialize() == SDL_APP_FAILURE)
	{
		std::cout << "No controller called " << controller->GetGamepadName() << " found" << std::endl;
		controller.reset();
	}

	bool isRunning = true;
	SDL_Event event;

	while (isRunning)
	{
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);

			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				isRunning = false;
				break;
			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
				if (controller != nullptr)
				{
					controller->SetButtonDownState(event.gbutton.button, event.gbutton.down);
				}
				break;
			case SDL_EVENT_GAMEPAD_ADDED:
				if (controller == nullptr)
				{
					std::unique_ptr<Controller> addedController = std::make_unique<Controller>();

					if (addedController->Initialize(event.gdevice.which) == SDL_APP_SUCCESS)
					{
						controller = std::move(addedController);
					}
				}
				break;
			case SDL_EVENT_GAMEPAD_REMOVED:
				if (controller != nullptr && event.gdevice.which == controller->GetJoystickID())
				{
					controller.reset();
				}
				break;
			}
		}

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		if (controller != nullptr)
		{
			controller->DrawDebugMenu();
		}

		ImGui::ShowDemoWindow();

		SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Hello Word", 0, SDL_Color{ 255, 255, 0, 255 });
		fontTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		SDL_DestroySurface(textSurface);

		ImGui::Render();

		SDL_SetRenderDrawColor(renderer, 0, 248, 255, 255);

		SDL_RenderClear(renderer);

		SDL_FRect textRect;
		textRect.x = 20;
		textRect.y = 80;
		textRect.w = 100;
		textRect.h = 20;

		SDL_RenderTexture(renderer, fontTexture, nullptr, &textRect);

		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

		SDL_RenderPresent(renderer);
	}

	//delete controller;

	TTF_CloseFont(font);
	TTF_Quit();

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}