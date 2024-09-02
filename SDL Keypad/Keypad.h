#pragma once

#pragma region C++ Includes
#include <string>
#include <sstream>
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
 * This class is responsible for the rendering and the gameplay
 * logic of a wheel-based keyboard. It makes no assumption on what
 * its usage will be.
 * Interanlly, interactions are handled and the input is stored into
 * an internal buffer that can be picked and consumed from any,
 * utilizer, which will need to check and handle it.
 */
class Keypad : public IRenderable , public IInteractable
{
	// Fields
public:
protected:
private:
	const string charset;
	const int charsetLength;
	const SDL_Color mainColor;
	const SDL_Color accentColor;
	const float angleStep;
	float rotation;	//	Never ever set rotation directly, use instead the SetRotation() or the Rotate() methods to handle wrapping
	ostringstream buffer;
	bool dragging;
	// Constructors
public:
	Keypad(const string & charset, const SDL_Color & mainColor, const SDL_Color & accentColor);
protected:
private:
	// Methods
public:
	__inline const string & GetCharset() const { return charset; }
	__inline char PeekActiveCharacter() const { return charset[GetActiveCharacterIndex()]; }
	__inline bool HasPendingCharacter() const { return !buffer.str().empty(); }
	__inline string PeekBuffer() const { return buffer.str(); }
	string ReadBuffer();
	__inline void ClearBuffer() { buffer.str(""); }
	void GetPointOnWheel(const SDL_Rect & area, float angle, SDL_Point & point) const;
	void SetRotation(float angleRad);
	__inline void Rotate(float angleDeltaRad) { SetRotation(rotation + angleDeltaRad); }

	//	IInteractable implementation
	void BeginInteraction(const SDL_Point & point) override;
	void EndInteraction() override;
	void MoveInteraction(const SDL_Point & from, const SDL_Point & to) override;

	//	IRenderable implementation
	void Render(SDL_Renderer * r) const override;
protected:
private:
	int GetActiveCharacterIndex() const;
	__inline int GetShortestAxis(const SDL_Rect & area) const { return area.w < area.h ? area.w : area.h; }
	int GetDigitSize(const SDL_Rect & area) const;
	void GetWheelCenter(const SDL_Rect & area, SDL_Point & center) const;
	int GetWheelRadius(const SDL_Rect & area) const;
	void GetParts(const SDL_Rect & area, SDL_Rect * submitArea, SDL_Rect * activeCharacterArea) const;
};
