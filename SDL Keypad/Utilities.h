#pragma once

#pragma region C++ Includes
#include <string>
#pragma endregion

#pragma region SDL Includes
//	SDL Core
#include <SDL.h>
#pragma endregion

using namespace std;

#pragma region Constant Parameters
//	Font resource
#define FONT "digital-7.ttf"
#pragma endregion

/*
 * A set of common utility functions that are widely
 * used throughout the project.
 * This file contains only forward declarations.
 */

void RenderLabel(SDL_Renderer * r, string text, int posX, int posY, const SDL_Color & color, int size = 24);
int GetRandomNumber(const int minInclusive, const int maxExclusive);
__inline int GetRandomIndex(const int length) { return GetRandomNumber(0, length); }
string GetRandomCode(const string & charset, const int lenght);

