#include "LockpickingGame.h"

#pragma region Constant Parameters
//	Color palette
#define COL_TEXT 0, 230, 255, 255
#define SDL_COL_TEXT SDL_Color{COL_TEXT}
#define COL_CODE_NEUTRAL 200, 200, 200, 255
#define SDL_COL_CODE_NEUTRAL SDL_Color{COL_CODE_NEUTRAL}
#define COL_CODE_TOTALLY_WRONG 235, 64, 52, 255
#define SDL_COL_CODE_WRONG SDL_Color{COL_CODE_TOTALLY_WRONG}
#define COL_CODE_WRONG 235, 171, 52, 255
#define SDL_COL_CODE_CLOSE_ENOUGH SDL_Color{COL_CODE_WRONG}
#define COL_CODE_CORRECT 82, 235, 52, 255
#define SDL_COL_CODE_CORRECT SDL_Color{COL_CODE_CORRECT}
#define COL_DEL 235, 64, 52, 255
#define SDL_COL_DEL SDL_Color{COL_DEL}
#define COL_GAME_OVER_BG_WIN 148, 213, 247, 255
#define SDL_COL_GAME_OVER_BG_WIN SDL_Color{COL_GAME_OVER_BG_WIN}
#define COL_GAME_OVER_BG_LOSE 247, 148, 148, 255
#define SDL_COL_GAME_OVER_BG_LOSE SDL_Color{COL_GAME_OVER_BG_LOSE}
#define COL_GAME_OVER_FG 94, 94, 94, 255
#define SDL_COL_GAME_OVER_FG SDL_Color{COL_GAME_OVER_FG}

//	Gameplay
#define CHARSET "0123456789"
#define CODE_DIGITS 4
#define STAGES_COUNT 3
#define TICKS_PER_SECOND 1000
#define SECONDS_PER_STAGE 60
#define STAGE_CLEAR_ROUTINE_DURATION 1050

//	Layout
#define CODE_DIGITS_SIZE 64
#define DIGITS_SPACING 15
#pragma endregion

LockpickingGame::LockpickingGame() :
	/*
	 * Building core game elements
	 */
	gameState(
		CHARSET,
		CODE_DIGITS,
		STAGES_COUNT,
		SECONDS_PER_STAGE * TICKS_PER_SECOND,
		SDL_COL_CODE_WRONG,
		SDL_COL_CODE_CLOSE_ENOUGH
	),
	codeDisplay(
		gameState.GetCodeLength(),
		DIGITS_SPACING,
		SDL_COL_TEXT,
		SDL_COL_DEL
	),
	keypad(
		gameState.GetCharset(),
		SDL_COL_TEXT,
		SDL_COL_CODE_CORRECT
	),
	gameOverScreen(
		SDL_COL_GAME_OVER_BG_WIN,
		SDL_COL_GAME_OVER_BG_LOSE,
		SDL_COL_GAME_OVER_FG
	),
	/*
	 * Initialzing areas with default values for completeness but
	 * this must be updated from outside before they get used!
	 */
	gameStateArea{0, 0, 100, 100},
	codeDisplayArea{0, 0, 100, 100},
	keypadArea{0, 0, 100, 100},
	gameOverArea{0, 0, 100, 100}
{
	//	Assign viewport areas to all relevant game elements (stored as pointers so they automatically update when modified anywhere else)
	gameState.SetViewportArea(gameStateArea);
	codeDisplay.SetViewportArea(codeDisplayArea);
	keypad.SetViewportArea(keypadArea);
	gameOverScreen.SetViewportArea(gameOverArea);
}

void LockpickingGame::BeginInteraction(const SDL_Point & point)
{
	//	Prevent interaction when not allowed
	if(!IsInteractionAllowed())
		return;

	if(gameState.IsGameOn())
	{	//	Feed interactions to game elements
		codeDisplay.BeginInteraction(point);
		keypad.BeginInteraction(point);
	}
	else
	{	//	Feed interaction to game over screen
		gameOverScreen.BeginInteraction(point);
	}
}

void LockpickingGame::EndInteraction()
{
	/*
	 * Not blocking EndInteraction when iteraction is not
	 * allowed to prevent inconsistent states.
	 */

	if(gameState.IsGameOn())
	{	//	Feed interactions to game elements
		codeDisplay.EndInteraction();
		keypad.EndInteraction();
	}
	else
	{	//	Feed interaction to game over screen
		gameOverScreen.EndInteraction();
	}
}

void LockpickingGame::MoveInteraction(const SDL_Point & from, const SDL_Point & to)
{
	//	Prevent interaction when not allowed
	if(!IsInteractionAllowed())
		return;

	if(gameState.IsGameOn())
	{	//	Feed interactions to game elements
		codeDisplay.MoveInteraction(from, to);
		keypad.MoveInteraction(from, to);
	}
	else
	{	//	Feed interaction to game over screen
		gameOverScreen.MoveInteraction(from, to);
	}
}

void LockpickingGame::Render(SDL_Renderer * r) const
{
	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent render
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Determine what to render, based on the game state
	if(
		stageClearRoutine ||	//	If a stage was just cleared, wait for the end of the routine to display game over screen
		gameState.IsGameOn()
	)
	{	//	Standard gameplay, just feed render in the correct order
		keypad.Render(r);
		codeDisplay.Render(r);
		gameState.Render(r);
	}
	else
	{	//	Feed render to game over screen
		gameOverScreen.Render(r);
	}
}

void LockpickingGame::OnFrameInitialization()
{
	//	Handle stage clear routine
	if(
		stageClearRoutine &&
		SDL_GetTicks64() > stageClearRoutineStart + STAGE_CLEAR_ROUTINE_DURATION
	)
	{
		stageClearRoutine = false;
		EndStageClearRoutine();
	}

	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent interaction
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Refresh viewport areas so input and render can make their calculations
	codeDisplayArea = {area.x + 25, area.y + 25, area.w - 50, CODE_DIGITS_SIZE};
	keypadArea = {area.x, codeDisplayArea.y + codeDisplayArea.h, area.w, area.h - codeDisplayArea.y - codeDisplayArea.h};
	gameStateArea = {area.x, area.y, area.w, area.h};	//	HUD goes full size
	gameOverArea = {area.x, area.y, area.w, area.h};	//	Game over screen goes full size
}

void LockpickingGame::OnPreRender()
{
	if(gameState.IsGameOn())
	{	//	Check pending actions during gameplay and apply effects
		//	Update the code if a character was fed
		if(keypad.HasPendingCharacter())
		{
			//	Build the new code input
			ostringstream newCodeInputStream;
			if(!codeDisplay.IsFull())
				newCodeInputStream << codeDisplay.GetDigits();	//	If full, start over
			newCodeInputStream << keypad.ReadBuffer();
			string newCodeInput = newCodeInputStream.str().substr(0, gameState.GetCodeLength());	//	Make sure to not overflow

			//	Feed the code input to the display
			if(!gameState.IsFullCode(newCodeInput))
				//	Set only digits, render with default color
				codeDisplay.SetDigits(newCodeInput);
			else
			{
				//	Evaluate digit-by-digit error
				vector<Uint8> newCodeErrors = gameState.EvaluateCodeError(newCodeInput);

				//	Prepare colors for all digits to display error hints
				vector<SDL_Color> newCodeColors;
				for(const Uint8 & digitError : newCodeErrors)
					if(digitError < 1)	//	No error, correct digit
						newCodeColors.push_back(SDL_COL_CODE_CORRECT);
					else if(digitError < 2)	//	Nearby digit, wrong but close
						newCodeColors.push_back(SDL_COL_CODE_CLOSE_ENOUGH);
					else	//	The digit wasn't even close
						newCodeColors.push_back(SDL_COL_CODE_WRONG);

				//	Set both digits and colors
				codeDisplay.SetDigits(newCodeInput, &newCodeColors);
			}

			//	Finally, submit the code to the game state
			const bool stageCompleted = gameState.SubmitCode(codeDisplay.GetDigits());
			if(stageCompleted)
				BeginStageClearRoutine();
		}
	}
	else
	{	//	Check pending actions during game over and apply effects
		//	Handle restart requested
		if(gameOverScreen.ConsumeSkipRequested())
		{
			gameOverScreen.SetSuccess(false);
			codeDisplay.Clear();
			keypad.ClearBuffer();
			gameState.Restart();
		}
	}
}

bool LockpickingGame::IsInteractionAllowed() const
{
	return IInteractable::IsInteractionAllowed() && !stageClearRoutine /* ...more locking conditions here... */;
}

/*
 * The stage completion routine is a ugly way to add a timed
 * sequence that aims at leaving the success condition visible
 * before passing to next stage or to game over screen.
 * It's ugly because it's hardcoded: a better version could
 * rely on a sort of "break" system that could request a hold
 * for a parameterized time and accept a callback for when the
 * timer comes to an end.
 * Simply not worth the hassle in such a small project.
 */

void LockpickingGame::BeginStageClearRoutine()
{
	stageClearRoutineStart = SDL_GetTicks64();
	stageClearRoutine = true;
	gameOverScreen.SetSuccess(gameState.AreStagesCleared());
}

void LockpickingGame::EndStageClearRoutine()
{
	codeDisplay.Clear();
}
