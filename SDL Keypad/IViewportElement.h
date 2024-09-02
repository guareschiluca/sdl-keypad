#pragma once

#pragma region SDL Includes
//	Core module
#include <SDL.h>
#pragma endregion

/*
 * Hyper-generic interface for anything that needs
 * to be bound to the viewport or a part of it.
 * Most likely, multiple other interfaces will need
 * to implement this one, so remember to use virtual
 * inheritance to prevent multiple, non-shared instance
 * of this interface.
 */
class IViewportElement
{
	// Fields
public:
protected:
private:
	SDL_Rect const * viewportArea;
	// Constructors
public:
protected:
private:
	// Methods
public:
	void SetViewportArea(const SDL_Rect & area) { viewportArea = &area; }
	SDL_Rect const * GetViewportArea() const { return viewportArea; }
protected:
private:
};
