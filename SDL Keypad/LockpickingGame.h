#pragma once

#pragma region Game Includes
#include "ILifecycle.h"
#include "IInteractable.h"
#include "IRenderable.h"

#include "GameState.h"
#include "CodeDisplay.h"
#include "Keypad.h"
#include "GameOverScreen.h"
#pragma endregion

#pragma region SDL Includes
//	SDL Core
#include <SDL.h>
#pragma endregion

/*
 * This class contains the main game logic implementation, assembling and
 * coordinating all game elements and creating the actual game flow.
 * Basically this class receives and broadcasts input and render events, and
 * hooks to a couple of lifecycle messages to prepare data and read/apply
 * actions results.
 * Distributing the responsibility, without scattering cross-references
 * around brought to not ideal choices such as game elements handling input
 * internally and producing a result outside.
 * In C++ events are not as straightforward as in other languages, and possible
 * solutions add too much complexity related to the complexity of this project.
 */
class LockpickingGame : public IRenderable, public IInteractable, public ILifecycle
{
	// Fields
public:
protected:
private:
	//	Game elements
	GameState gameState;
	CodeDisplay codeDisplay;
	Keypad keypad;
	GameOverScreen gameOverScreen;

	//	Viewport areas
	SDL_Rect gameStateArea;
	SDL_Rect codeDisplayArea;
	SDL_Rect keypadArea;
	SDL_Rect gameOverArea;

	//	Timing
	bool stageClearRoutine = false;
	Uint64 stageClearRoutineStart = 0;
	// Constructors
public:
	LockpickingGame();
protected:
private:
	// Methods
public:
	//	IInteractable implementation
	void BeginInteraction(const SDL_Point & point) override;
	void EndInteraction() override;
	void MoveInteraction(const SDL_Point & from, const SDL_Point & to) override;

	//	IRenderable implementation
	void Render(SDL_Renderer * r) const override;

	//	ILifecycle implementation
	void OnFrameInitialization() override;
	void OnPreRender() override;
protected:
	//	IInteractable implementation
	bool IsInteractionAllowed() const override;
private:
	void BeginStageClearRoutine();
	void EndStageClearRoutine();
};

