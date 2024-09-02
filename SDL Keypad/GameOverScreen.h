#pragma once

#pragma region Game Includes
#include "IRenderable.h"
#include "IInteractable.h"
#pragma endregion

/*
 * This simple class is responsible for rendering a game over
 * screen, both for win and lose scenarios, and to handle the
 * bare necessary input to skip over and start a new game.
 * New game message is not directly broadcasted to the game
 * flow or other game elements, but the communication is made
 * through a flag that the controller will need to check and
 * handle.
 */
class GameOverScreen : public IRenderable, public IInteractable
{
	// Fields
public:
protected:
private:
	bool success;
	const SDL_Color winBackColor;
	const SDL_Color loseBackColor;
	const SDL_Color foregroundColor;
	bool skipRequested;
	// Constructors
public:
	GameOverScreen(const SDL_Color & winBackColor, const SDL_Color & loseBackColor, const SDL_Color & foregroundColor);
protected:
private:
	// Methods
public:
	void SetSuccess(bool won) { success = won; }
	bool PeekSkipRequested() const { return skipRequested; }
	bool ConsumeSkipRequested();

	//	IInteractable implementation
	void BeginInteraction(const SDL_Point & point) override;
	void EndInteraction() override { }	//	Unused
	void MoveInteraction(const SDL_Point & from, const SDL_Point & to) override { }	//	Unused

	//	IRenderable implementation
	void Render(SDL_Renderer * r) const override;
protected:
private:
};
