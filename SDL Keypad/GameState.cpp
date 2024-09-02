#include "GameState.h"

#pragma region C++ Includes
#include <sstream>
#include <cmath>
#pragma endregion

#pragma region Game Includes
#include "Utilities.h"
#pragma endregion

#pragma region Constant Parameters
//	Layout parameters
#define BARS_THCKNESS 10
#pragma endregion


GameState::GameState(
	const string & charset, const Uint8 & codeLength,
	const Uint8 & stages, const Uint32 & stageTimeMilliseconds,
		const SDL_Color & primaryColor, const SDL_Color & accentColor
) :
	charset(charset),
	charsetLength((int)charset.size()),
	codeLength(codeLength),
	stages(stages),
	solveTime(stageTimeMilliseconds * stages),
	primaryColor(primaryColor),
	accentColor(accentColor)
{
	//	Run state clear operations at cosntruction time
	Restart();
}

void GameState::Restart()
{
	//	Reset game progress
	timerStart = SDL_GetTicks64();
	stagesLeft = stages;

	//	Generate a new code
	GenerateNewCode();
}

void GameState::GenerateNewCode()
{
	ostringstream codeBuffer;

	for(int c = 0; c < codeLength; c++)
		codeBuffer << charset[GetRandomIndex(charsetLength)];

	code = codeBuffer.str();
}

vector<Uint8> GameState::EvaluateCodeError(const string & codeInput) const
{
	const int inputSize = (int)codeInput.size();
	vector<Uint8> digitErrors;

	/*
	 * Set the error for each digit to the distance of the input digit
	 * from the requested digit. If the digit has not been provided,
	 * set the error to an arbitrary high distance (relevant ditances
	 * are 0 for match, 1 for almost match and > 1 for wrong).
	 */
	for(int c = 0; c < codeLength; c++)
	{
		if(c < inputSize)
			digitErrors.push_back((Uint8)abs((int)codeInput[c] - (int)code[c]));
		else
			digitErrors.push_back((Uint8)9);
	}

	return digitErrors;
}

bool GameState::SubmitCode(const string & codeInput)
{
	const bool match = CheckCode(codeInput);

	//	If the code just provided matches the requested one (and there are any more stages left), advance one stage and prepare for next
	if(
		match &&
		stagesLeft > 0
	)
	{
		stagesLeft--;
		GenerateNewCode();
	}

	return match;
}

float GameState::GetTimeLeft() const
{
	//	Calculate the remaining time
	Uint32 timeLeft = (Uint32)(SDL_GetTicks64() - timerStart);

	//	Return a normalized representation of the remaining time
	return 1.0f - SDL_clamp((float)timeLeft / solveTime, 0.0f, 1.0f);
}

void GameState::Render(SDL_Renderer * r) const
{
	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent rednering
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Prepare a rect for rendering the different areas of the HUD
	SDL_Rect targetArea;

	//	Render stages
	for(int s = 0; s < stagesLeft; s++)
	{
		GetStageArea(area, s, targetArea);
		RenderBar(r, targetArea);
	}

	//	Render timer
	GetTimerArea(area, targetArea);
	RenderBar(r, targetArea);
}

void GameState::GetStageArea(const SDL_Rect & area, const int sector, SDL_Rect & stageArea) const
{
	const int stagePortion = area.w / stages;

	stageArea.w = stagePortion - BARS_THCKNESS;
	stageArea.h = BARS_THCKNESS;
	stageArea.x = area.x + sector * stagePortion;
	stageArea.y = area.y;
}

void GameState::GetTimerArea(const SDL_Rect & area, SDL_Rect & timerArea) const
{
	timerArea.w = (int)(GetTimeLeft() * area.w);
	timerArea.h = BARS_THCKNESS;
	timerArea.x = area.x; 
	timerArea.y = area.y + area.h - timerArea.h;
}

void GameState::RenderBar(SDL_Renderer * r, const SDL_Rect & area) const
{
	//	Render fill
	SDL_SetRenderDrawColor(r, accentColor.r, accentColor.g, accentColor.b, accentColor.a);
	SDL_RenderFillRect(r, &area);

	//	Render border
	SDL_SetRenderDrawColor(r, primaryColor.r, primaryColor.g, primaryColor.b, primaryColor.a);
	SDL_RenderDrawRect(r, &area);
}
