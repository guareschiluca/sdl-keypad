#include "GameOverScreen.h"

#pragma region Engine Includes
#include "Utilities.h"
#pragma endregion

GameOverScreen::GameOverScreen(const SDL_Color & winBackColor, const SDL_Color & loseBackColor, const SDL_Color & foregroundColor) :
	success(false),
	winBackColor(winBackColor),
	loseBackColor(loseBackColor),
	foregroundColor(foregroundColor),
	skipRequested(false)
{ }

bool GameOverScreen::ConsumeSkipRequested()
{
	bool wasSkipRequested = skipRequested;
	skipRequested = false;
	return wasSkipRequested;
}

void GameOverScreen::BeginInteraction(const SDL_Point & point)
{
	skipRequested = true;
}

void GameOverScreen::Render(SDL_Renderer * r) const
{
	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent render
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Fill screen
	const SDL_Color & backColor = success ? winBackColor : loseBackColor;
	SDL_SetRenderDrawColor(r, backColor.r, backColor.g, backColor.b, backColor.a);
	SDL_RenderFillRect(r, &area);
	const string message = success ? "YOU SAVED THE WORLD!!" : "BOOOOOM!!!!!";
	RenderLabel(
		r,
		message,
		area.x + area.w / 2,
		area.y + area.h / 2,
		foregroundColor,
		area.h / 6
	);
}
