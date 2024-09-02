#pragma region C++ Includes
#include <iostream>
#include <chrono>
#include <vector>
#pragma endregion

#pragma region SDL Includes
//	SDL Core
#include <SDL.h>

//	SDL Modules
#include <SDL_ttf.h>
#pragma endregion

#pragma region Game Includes
//	Interfaces to hook into the main loop
#include "ILifecycle.h"
#include "IInteractable.h"
#include "IRenderable.h"

//	Game elements
#include "LockpickingGame.h"
#pragma endregion

#pragma region Emscripten Includes
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#pragma endregion

using namespace std;
using namespace std::chrono;

#pragma region Docs Links
/*
 * SDL Resources
 *		Lifecycle: https://wiki.libsdl.org/SDL2/CategoryInit
 *		Error Handling: https://wiki.libsdl.org/SDL2/CategoryError
 *		General API: https://wiki.libsdl.org/SDL2/APIByCategory
 *		TTF API: https://wiki.libsdl.org/SDL2_ttf/CategoryAPI
 */
#pragma endregion

#pragma region Constant Parameters
/*
 * Viewport should be Full-HD and full screen
 * in release builds, and windowed HD-Ready
 * in debug builds, to ease debug operations.
 * On webgl, instead, we set it to resizable
 * to match the HTML container.
 */
#ifdef _DEBUG
#define VIEWPORT_W 1280
#define VIEWPORT_H 720
#define VIEWPORT_MODE SDL_WINDOW_RESIZABLE
#else
#ifdef __EMSCRIPTEN__
#define VIEWPORT_W 800
#define VIEWPORT_H 480
#define VIEWPORT_MODE SDL_WINDOW_RESIZABLE
#else
#define VIEWPORT_W 1920
#define VIEWPORT_H 1080
#define VIEWPORT_MODE SDL_WINDOW_FULLSCREEN
#endif
#endif
#define SDL_INIT_MODE SDL_INIT_VIDEO

#ifdef __EMSCRIPTEN__
#define HTML_CANVAS_SELECTOR "#canvas"
#endif

//	The fixed time step we aim to
#define TARGET_FPS 60
#define TARGET_FRAME_TIME 1000 / TARGET_FPS

//	Color palette
#define COL_CLEAR 32, 32, 32, 255
#pragma endregion

#pragma region Exchange data
/*
 * Here we define a set of data structures that we'll
 * use to keep the application's state.
 * All toghether form the application's context, that
 * can be passed around to relevant parts of the
 * program or simply made global.
 */
typedef struct
{
	SDL_Window * window;
	SDL_Renderer * r;
} SystemData;
typedef struct
{
	bool closeRequested;
	vector<ILifecycle *> lifecycleQueue;
	vector<IInteractable *> interactionQueue;
	vector<IRenderable const *> renderQueue;
} EngineData;
typedef struct
{
	LockpickingGame lockpickingGame;
	SDL_Rect lockpickingGameArea;
} GameData;
typedef struct
{
	SystemData system;
	EngineData engine;
	GameData game;
} Context;
#pragma endregion

//	Forward declarations
void MainLoop();
int SystemSetup();
void SystemShutdown();

//	Prepare a global context for the main loop and the main function
Context ctx;

/*	ENTRY POINT	*/
int main(int argc, char * argv[])
{
#pragma region System Setup
	/*
	 * Here we're going to initialize and set up
	 * the main elements that will make our game
	 * work, such as SDL itself, font support, the
	 * game window and the renderer.
	 */
	int systemStatus = SystemSetup();
	if(systemStatus != 0)
		return systemStatus;
#pragma endregion

#pragma region Preparations
	//	Initialize game context
	ctx.game.lockpickingGameArea = {0, 0, VIEWPORT_W, VIEWPORT_H};
	ctx.game.lockpickingGame.SetViewportArea(ctx.game.lockpickingGameArea);

	ctx.engine.closeRequested = false;

	//	Fill lists for input, update and rendering
	ctx.engine.lifecycleQueue.push_back(&ctx.game.lockpickingGame);
	ctx.engine.interactionQueue.push_back(&ctx.game.lockpickingGame);
	ctx.engine.renderQueue.push_back(&ctx.game.lockpickingGame);
#pragma endregion

#pragma region Main Loop
	/*
	 * Just a couple of lines, here the program will
	 * spend 99% of its time.
	 * Here all game logic will hook and run, frame
	 * by frame.
	 */
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(MainLoop, 0, 1);
#else
	while(!ctx.engine.closeRequested)
		MainLoop();
#endif
#pragma endregion

#pragma region System Shutdown
	/*
	 * Not calling when building with emscripten,
	 * it follows a slightly different logic.
	 * The main loop started above with
	 *	emscripten_set_main_loop()
	 * schedules a call to the MainLoop function
	 * at intervals decided by the browser (but
	 * configured by us) but that function doesn't
	 * block execution so we don't want to shutdown
	 * the system before the first frame is
	 * processed.
	 * We'll call SystemShutdown() later, in the
	 * main loop.
	 */
#ifndef __EMSCRIPTEN__
	SystemShutdown();
#endif
#pragma endregion

	return 0;
}

int SystemSetup()
{
	/*
	 * At the end of this function all the core elements
	 * will be initialized and ready to be used.
	 * This funciton will return 0 if everything is ok,
	 * != 0 if something failed initializing.
	 */
	//	Initialize SDL (here we can selectively initialize different modules using SDL_INIT_* OR'd constants)
	if(SDL_Init(SDL_INIT_MODE) < 0)
	{
		cout << "Couldn't initialize SDL2: " << SDL_GetError() << endl;
		return 1;
	}

	//	Check canvas size when targetting webgl
	int windowWidth;
	int windowHeight;
#ifndef __EMSCRIPTEN__
#ifndef _DEBUG
	SDL_DisplayMode displayMode;
	if(SDL_GetCurrentDisplayMode(0, &displayMode) == 0)
	{
		windowWidth = displayMode.w;
		windowHeight = displayMode.h;
	}
	else
#endif
	{
		windowWidth = VIEWPORT_W;
		windowHeight = VIEWPORT_H;
	}
#else
	emscripten_get_canvas_element_size(HTML_CANVAS_SELECTOR, &windowWidth, &windowHeight);
#endif

	//	Create the game window
	ctx.system.window = SDL_CreateWindow(
		"SDL Keypad!",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		windowWidth, windowHeight,
		VIEWPORT_MODE
	);
	if(!ctx.system.window)
	{
		cout << "Couldn't create SDL window: " << SDL_GetError() << endl;
		return 1;
	}

	//	Get or create a rendeer for future render operations
	ctx.system.r = SDL_GetRenderer(ctx.system.window);
	if(!ctx.system.r)
	{
		cout << "Couldn't get SDL renderer from window: " << SDL_GetError() << endl;
		cout << "Trying to create a new renderer.." << endl;
		ctx.system.r = SDL_CreateRenderer(ctx.system.window, -1, SDL_RendererFlags::SDL_RENDERER_ACCELERATED);
		if(!ctx.system.r)
		{
			cout << "Couldn't create SDL renderer on window: " << SDL_GetError() << endl;
			return -1;
		}
	}

	//	Initialize the TTF module
	if(TTF_Init() != 0)
		cout << "Cannot initialize SDL_ttf: " << TTF_GetError() << endl;
#ifdef _DEBUG
	else
		cout << "SDL_ttf intialized succesfully!" << endl;
#endif

	return 0;
}

void MainLoop()
{
	/*
	 * The main loop doesn't assume anything about
	 * game-specific implementations.
	 * All we do here is to run a set of operations
	 * in a given order: this loop represents a frame,
	 * at the end of each iteration an image will be
	 * presented on screen.
	 * This loop's main goal is to handle "engine"
	 * operations (intended as a cyclic,
	 * implementation-agnostic operations) and to
	 * expose hooks for specific implementations
	 * to interact with that loop.
	 * There are 3 main categories of hooks:
	 * - Interaction Hooks: bound to the input loop
	 * - Render Hooks: bound to rendering elements
	 * - Lifecycle Hooks: called at specific stages
	 * Specific implementations can hook by implementing
	 * specific interfaces and pushing themselves into
	 * the dedicated lists.
	 * This is far from a real wolrd engine implementation
	 * both for performance and for the simple fact
	 * that game implementation must be instantiated
	 * modifying the main program, but it's logically
	 * similar to a real engine.
	 */
#pragma region Frame Start
		//	LIFECYCLE: Broadcast frame-start event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnFrameStart();
#pragma endregion

#pragma region Prepare FPS Regulation
	/*
	 * To keep a steady frame rate, we need to know
	 * how long it takes for a frame to make all
	 * calculations and render and then wait for the
	 * target frame time.
	 * To do so, when the frame starts, i.e. when
	 * entering the main loop, we store the precise
	 * time.
	 * 
	 * When building for webgl, we let the browser decide
	 * the frame rate, which will typically match the
	 * monitor's refresh rate, so we're skipping all the
	 * framerate regulation stuff when targetting webgl.
	 */
#ifndef __EMSCRIPTEN__
	steady_clock::time_point frameStart = high_resolution_clock::now();
#endif
#pragma endregion

#pragma region Frame initialization
	// Get viewport size
	/*
	 * This part is highly inefficient. There's no need at
	 * all to recalculate viewport size and all the screen
	 * layout every frame as the game window doesn't cahgne
	 * every frame.
	 * In debug configuration, the window has been initialized
	 * as resizable so we can play with the window and see
	 * that the game contents do adapt.
	 * In a real world scenario, all those calculations should
	 * be done only when the viewport size actually changes.
	 */
	SDL_GetWindowSize(ctx.system.window, &ctx.game.lockpickingGameArea.w, &ctx.game.lockpickingGameArea.h);

	//	LIFECYCLE: Broadcast frame-initialization event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnFrameInitialization();
#pragma endregion

#pragma region Events/Input Loop
		//	LIFECYCLE: Broadcast pre-events-loop event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnPreEventsLoop();

	// Events loop
	SDL_Event currentEvent;
	while(SDL_PollEvent(&currentEvent))
	{
		switch(currentEvent.type)
		{
#ifndef __EMSCRIPTEN__
			/*
			 * Not handling quit event and escape key when targetting
			 * webgl, for a more platform-specific UX
			 */
			case SDL_QUIT:
				ctx.engine.closeRequested = true;
				break;
			case SDL_EventType::SDL_KEYDOWN:
				//	Handle static actions
				switch(currentEvent.key.keysym.sym)
				{
					//	On Escape, quit
					case SDLK_ESCAPE:
						ctx.engine.closeRequested = true;	//	Let's use the Escape button to quit the game
						break;
				}
				break;
#endif
			case SDL_MOUSEBUTTONDOWN:
				//	Only accept left mouse button (or touch emulation)
				if(currentEvent.button.button != 1)
					break;
				{
					SDL_Point mousePosition = {currentEvent.button.x, currentEvent.button.y};

					for(IInteractable *& interactable : ctx.engine.interactionQueue)
						interactable->BeginInteraction(mousePosition);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				//	Only accept left mouse button (or touch emulation)
				if(currentEvent.button.button != 1)
					break;
				for(IInteractable *& interactable : ctx.engine.interactionQueue)
					interactable->EndInteraction();
				break;
			case SDL_MOUSEMOTION:
				//	Calculate move boundaries
			{
				SDL_Point mousePosition = {currentEvent.motion.x, currentEvent.motion.y};
				SDL_Point mousePrevPosition = {mousePosition.x - currentEvent.motion.xrel, mousePosition.y - currentEvent.motion.yrel};

				// Feed move info to interactables
				for(IInteractable *& interactable : ctx.engine.interactionQueue)
					interactable->MoveInteraction(mousePrevPosition, mousePosition);
			}
			break;
		}
	}
#pragma endregion

#pragma region Render Loop
		//	LIFECYCLE: Broadcast pre-render event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnPreRender();

	// Clear
	SDL_SetRenderDrawColor(ctx.system.r, COL_CLEAR);
	SDL_RenderClear(ctx.system.r);

	//	LIFECYCLE: Broadcast post-render-clear event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnPostRenderClear();

	//	Render all subscribed renderers
	for(IRenderable const *& renderable : ctx.engine.renderQueue)
		renderable->Render(ctx.system.r);

	//	LIFECYCLE: Broadcast pre-render-present event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnPreRenderPresent();

	// Display render
	SDL_RenderPresent(ctx.system.r);

	//	LIFECYCLE: Broadcast post-render-present event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnPostRenderPresent();
#pragma endregion

#pragma region FPS Regulation
	/*
	 * We calculate the frame time, relative to the frame start time.
	 * If it's below the target frame time, we'll wait for the difference.
	 * If, instead, the frame take longer than the frame time we have two
	 * roads to walk:
	 * - variable frame time: we just don't wait and rush into the next frame
	 * - fixed frame time: we skip a frame to align to the next fixed frame time
	 *
	 * Important note: when working with high resolution clock (or numbers
	 * in general), data is usually stored in large data formats such as
	 * long int or even long long int, so it's good practice to avoid
	 * implicit conversions to make 100% clear what type of data we're
	 * carying around.
	 * 
	 * As stated above, FPS regulation is entrusted to the browser for
	 * webgl builds, so we'll skip the manual frame rate regulation here
	 * too.
	 */
#ifndef __EMSCRIPTEN__
	long long elapsedMillis = duration_cast<milliseconds>(high_resolution_clock::now() - frameStart).count();
#ifdef FRAME_SKIP
	elapsedMillis %= TARGET_FPS;
#endif
	long long waitMillis = (1000 / TARGET_FPS) - elapsedMillis;
	if(waitMillis > 0)
		SDL_Delay((int)waitMillis);
#endif
#pragma endregion

#pragma region Frame End
		//	LIFECYCLE: Broadcast frame-end event
	for(ILifecycle *& lifecycleReceiver : ctx.engine.lifecycleQueue)
		lifecycleReceiver->OnFrameEnd();
#pragma endregion

#pragma region WebGL Shutdown
	/*
	 * When targetting webgl this function (MainLoop) is
	 * called again and again by the browser. When the
	 * game requests a close, we need to shutdown the
	 * system.
	 */
#ifdef __EMSCRIPTEN__
	if(ctx.engine.closeRequested)
		SystemShutdown();
#endif
#pragma endregion
}

void SystemShutdown()
{
	/*
	 * Closing operations are very much important.
	 * Always remember to clean all created objects
	 * and to quit all the systems.
	 * This is especially useful when the same
	 * application opens and closes the resources
	 * multiple times in the same run.
	 *
	 * Specific for emscripten, we stop the
	 * main loop.
	 */
#ifdef __EMSCRIPTEN__
	emscripten_cancel_main_loop();
#endif
	TTF_Quit();
	SDL_DestroyRenderer(ctx.system.r);
	SDL_DestroyWindow(ctx.system.window);
	SDL_Quit();

}
