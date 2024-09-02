#pragma once

#pragma region SDL Includes
//	SDL Core
#include <SDL.h>
#pragma endregion

#pragma region Game Includes
#include "IViewportElement.h"
#pragma endregion

/*
 * Interface used by the main loop to dispatch render
 * messages.
 * Anything that will need to appear on-screen will
 * need to implement this interface and add itself
 * to the render queue, right before the main loop.
 */
class IRenderable : virtual public IViewportElement
{
public:
	virtual void Render(SDL_Renderer * r) const = 0;
};
