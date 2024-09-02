#pragma once

/*
 * Interface used by the main loop to dispatch lifecycle
 * messages.
 * Anything that will need to execute code at specific
 * stages of the main loop will need to implement this
 * interface and add itself to the lifecycle queue,
 * right before the main loop.
 */
class ILifecycle
{
public:
	//	Called at frame start, even before the FPS regulation initialization (heavy operations here may break the frame rate steadiness)
	virtual void OnFrameStart() { }
	//	Called at fram initialization, right after the viewport size was updated
	virtual void OnFrameInitialization() { }
	//	Called right before the events loop
	virtual void OnPreEventsLoop() { }
	//	Called right before clearing the render target
	virtual void OnPreRender() { }
	//	Called right after clearing the render target
	virtual void OnPostRenderClear() { }
	//	Called Right before presenting the render to screen, after all render operations concluded
	virtual void OnPreRenderPresent() { }
	//	Called right after the render has been displayed on screen
	virtual void OnPostRenderPresent() { }
	//	Called at frame end, even after the FPS regulation (heavy operations here may break the frame rate steadiness)
	virtual void OnFrameEnd() { }
};
