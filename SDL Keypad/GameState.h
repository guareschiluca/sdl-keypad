#pragma once

#pragma region C++ Includes
#include <string>
#include <vector>
#pragma endregion

#pragma region SDL Includes
//	SDL Core
#include <SDL.h>
#pragma endregion

#pragma region Game Includes
#include "IRenderable.h"
#pragma endregion

using namespace std;

/*
 * This class has the responsibility to track and update
 * the game state, including win and lose conditions,
 * the game HUD (that's why it implements IRenderable)
 * and stores the main game information such as the code
 * to input and the game progression.
 */
class GameState : public IRenderable
{
	// Fields
public:
protected:
private:
	const string charset;
	const int charsetLength;
	const Uint8 codeLength;
	string code;
	const Uint8 stages;
	Uint8 stagesLeft;
	const Uint32 solveTime;
	Uint64 timerStart;
	const SDL_Color primaryColor;
	const SDL_Color accentColor;
	// Constructors
public:
	GameState(
		const string & charset, const Uint8 & codeLength,
		const Uint8 & stages, const Uint32 & stageTimeMilliseconds,
		const SDL_Color & primaryColor, const SDL_Color & accentColor
	);
protected:
private:
	// Methods
public:
	__inline const string & GetCharset() const { return charset; }
	void Restart();
	void GenerateNewCode();
	__inline Uint8 GetCodeLength() const { return codeLength; }
	__inline bool IsFullCode(const string & code) const { return code.size() == codeLength; }
	vector<Uint8> EvaluateCodeError(const string & codeInput) const;
	__inline bool CheckCode(const string & codeInput) const { return code == codeInput; }
	bool SubmitCode(const string & codeInput);
	float GetTimeLeft() const;
	__inline bool IsTimeUp() const { return GetTimeLeft() <= 0.0f; }
	__inline bool AreStagesCleared() const { return stagesLeft < 1; }
	__inline bool IsGameOver() const { return IsTimeUp() || AreStagesCleared(); }
	__inline bool IsGameOn() const { return !IsGameOver(); }

	//	IRenderable implementation
	void Render(SDL_Renderer * r) const override;
protected:
private:
	void GetStageArea(const SDL_Rect & area, const int sector, SDL_Rect & stageArea) const;
	void GetTimerArea(const SDL_Rect & area, SDL_Rect & timerArea) const;
	void RenderBar(SDL_Renderer * r, const SDL_Rect & area) const;
};
