#include "Utilities.h"

#pragma region C++ Includes
#include <sstream>
#include <string>
#include <cmath>
#include <random>
#pragma endregion

#pragma region SDL Includes
//	SDL Core
#include <SDL.h>

//	SDL Modules
#include <SDL_ttf.h>
#pragma endregion

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

/*
 * This is a very much inefficient way to render
 * a label. It was intentionally done this way
 * for simplicity and to make the process clearer.
 */
void RenderLabel(SDL_Renderer * r, string text, int posX, int posY, const SDL_Color & color, int size)
{
	// Load font
	ostringstream fontFullPath;
	fontFullPath << SDL_GetBasePath() << PATH_SEPARATOR;
	fontFullPath << "res" << PATH_SEPARATOR;
	fontFullPath << "fonts" << PATH_SEPARATOR;
	fontFullPath << FONT;
	/*
#ifdef __EMSCRIPTEN__
	fontFullPath << "/res/fonts/" << FONT;
#else
	fontFullPath << SDL_GetBasePath() << "res\\fonts\\" << FONT;
#endif
	*/

	TTF_Font * font = TTF_OpenFont(fontFullPath.str().c_str(), size);
	if(!font)
	{
		/*
		 * When fonts don't get loaded this game is impossible to try so
		 * let's at least draw a small rect so we know that the font
		 * wasn't loaded but still we can test the game.
		 */
		SDL_Rect missingFontArea{posX - size / 2, posY - size / 2, size, size};
		SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
		SDL_RenderDrawRect(r, &missingFontArea);

		return;
	}

	// Render text
	SDL_Surface * surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(r, surf);

	// Calculate target portion
	SDL_Rect target;
	target.w = surf->w;
	target.h = surf->h;
	target.x = posX - target.w / 2;
	target.y = posY - target.h / 2;

	// Copy to render target
	SDL_RenderCopy(r, texture, nullptr, &target);

	// Cleanup
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
}

/*
 * This funciton returns a random number between minInclusive
 * and maxExclusive - 1.
 * The random device and the random engine could have been
 * made static to be shared between all calls, instead of
 * creating and disposing them each time.
 * This is less performant but gives a better random, in
 * this context we're far from CPU-bound and we have a
 * practival advantage from a better random, since it's
 * the core of the game.
 */
int GetRandomNumber(const int minInclusive, const int maxExclusive)
{
	random_device rd;
	default_random_engine engine(rd());
	uniform_int_distribution<int> dist(minInclusive, maxExclusive - 1);

	return dist(engine);
}

/*
 * This funciton uses a charset and the GetRandomIndex function
 * to build a random code of a given length.
 */
string GetRandomCode(const string & charset, const int length)
{
	ostringstream codeStream;

	for(int c = 0; c < length; c++)
		codeStream << charset[GetRandomIndex((int)charset.size())];

	return codeStream.str();
}
