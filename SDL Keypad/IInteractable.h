#pragma once

#pragma region SDL Includes
//	Core module
#include <SDL.h>
#pragma endregion

#pragma region Game Includes
#include "IViewportElement.h"
#pragma endregion

/*
 * Interface used by the main loop to dispatch input
 * messages.
 * Anything that will need to respond to mouse/touch
 * will need to implement this interface and add itself
 * to the interaciton queue, right before the main loop.
 */
class IInteractable : virtual public IViewportElement
{
public:
	virtual void BeginInteraction(const SDL_Point & point) = 0;
	virtual void EndInteraction() = 0;
	virtual void MoveInteraction(const SDL_Point & from, const SDL_Point & to) = 0;
protected:
	virtual bool IsInteractionAllowed() const { return true; }
};
