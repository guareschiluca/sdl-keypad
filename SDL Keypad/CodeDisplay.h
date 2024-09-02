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
#include "IInteractable.h"
#pragma endregion

using namespace std;

/*
 * This class is responsible for displaying a code made of
 * a fixed amount of digits (decided at construction time).
 * Additionally, it handles input to implement a "clear"
 * button that can flush the input code.
 * This is the only place where the input code is stored. For
 * this reason, the "clear" button effects are applied internally
 * without the need to coordinate with a controller.
 * User input is not implemented here, must come from other
 * specific implementations.
 */
class CodeDisplay : public IRenderable, public IInteractable
{
	// Fields
public:
protected:
private:
	const int digitsCount;
	const int digitSpacing;
	string digits;
	vector<SDL_Color> digitsColors;
	const SDL_Color neutralColor;
	const SDL_Color deleteColor;
	// Constructors
public:
	CodeDisplay(const int & digitsCount, const int & digitSpacing, const SDL_Color & neutralColor, const SDL_Color & deleteColor);
protected:
private:
	// Methods
public:
	__inline bool IsFull() const { return digits.size() == digitsCount; }
	__inline const string & GetDigits() const { return digits; }
	void SetDigits(const string & newDigits, vector<SDL_Color> * colors = nullptr);
	__inline void Clear() { SetDigits(""); }

	//	IInteractable implementation
	void BeginInteraction(const SDL_Point & point) override;
	void EndInteraction() override { }
	void MoveInteraction(const SDL_Point & from, const SDL_Point & to) override { }

	//	IRenderable implementation
	void Render(SDL_Renderer * r) const override;
protected:
private:
	int GetDigitSize(const SDL_Rect & area) const;
	int GetCodeExtent(const SDL_Rect & area) const;
	void GetDigitArea(const SDL_Rect & area, const int digit, SDL_Rect & digitArea) const;
	void GetDeleteArea(const SDL_Rect & area, SDL_Rect & deleteArea) const;
};
