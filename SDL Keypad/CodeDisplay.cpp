#include "CodeDisplay.h"

#pragma region Game Includes
#include "Utilities.h"
#pragma endregion

#pragma region Constant Parameters
//	Text parameters
#define MISSING_CHAR '*'
#pragma endregion

CodeDisplay::CodeDisplay(
	const int & digitsCount, const int & digitSpacing,
	const SDL_Color & neutralColor, const SDL_Color & deleteColor
) :
	digitsCount(digitsCount),
	digitSpacing(digitSpacing),
	neutralColor(neutralColor),
	deleteColor(deleteColor)
{ }

void CodeDisplay::SetDigits(const string & newDigits, vector<SDL_Color> * colors)
{
	//	Update digits
	digits = newDigits.substr(0,digitsCount);

	//	Flush colors since a new code has been provided
	digitsColors.clear();

	//	If any color has been provided, copy it internally for future use during rendering
	if(colors)
		for(const SDL_Color & color : *colors)
			digitsColors.push_back(color);
}

void CodeDisplay::BeginInteraction(const SDL_Point & point)
{
	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent interaction
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Calcualte the delete area
	SDL_Rect deleteArea;
	GetDeleteArea(area, deleteArea);

	//	Clear if pressed the delete area
	if(SDL_PointInRect(&point, &deleteArea))
		Clear();
}

void CodeDisplay::Render(SDL_Renderer * r) const
{
	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent rednering
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Prepare data for rendering
	const int digitSize = GetDigitSize(area);
	const int inputLength = (int)digits.size();
	const int colorsSize = (int)digitsColors.size();
	const bool isFull = inputLength == digitsCount;

	//	Render code digits
	SDL_Rect targetArea;
	string digit;
	for(int d = 0; d < digitsCount; d++)
	{
		//	Calculate the target area for this digit
		GetDigitArea(area, d, targetArea);

		//	Prepare the current input digit or use the missing character if the digit in this place has not been input yet
		if(d < inputLength)
			digit = {digits[d]};
		else
			digit = {MISSING_CHAR};
		
		//	Render the digit in the current place
		RenderLabel(
			r,
			digit,
			targetArea.x + targetArea.w / 2,
			targetArea.y + targetArea.h / 2,
			isFull && d < colorsSize ? digitsColors[d] : neutralColor,
			digitSize
		);
	}

	//	Render the delete button (cross in a square shape)
	GetDeleteArea(area, targetArea);
	SDL_SetRenderDrawColor(r, deleteColor.r, deleteColor.g, deleteColor.b, deleteColor.a);
	SDL_RenderDrawRect(r, &targetArea);
	SDL_RenderDrawLine(r, targetArea.x, targetArea.y, targetArea.x + targetArea.w, targetArea.y + targetArea.h);
	SDL_RenderDrawLine(r, targetArea.x + targetArea.w, targetArea.y, targetArea.x, targetArea.y + targetArea.h);
}

int CodeDisplay::GetDigitSize(const SDL_Rect & area) const
{
	return area.h;
}

int CodeDisplay::GetCodeExtent(const SDL_Rect & area) const
{
	return
		(
			digitsCount * GetDigitSize(area) +
			(digitsCount - 1) * digitSpacing
		) / 2;
}

void CodeDisplay::GetDigitArea(const SDL_Rect & area, const int digit, SDL_Rect & digitArea) const
{
	SDL_Point center = {area.x + area.w / 2, area.y + area.h / 2};

	const int digitSize = GetDigitSize(area);
	const int extent = GetCodeExtent(area);

	digitArea.x = center.x - extent + digit * digitSize + (digit == 0 ? 0 : digit - 1) * digitSpacing;
	digitArea.y = center.y - digitSize / 2;
	digitArea.w = digitSize;
	digitArea.h = digitSize;
}

void CodeDisplay::GetDeleteArea(const SDL_Rect & area, SDL_Rect & deleteArea) const
{
	deleteArea.w = area.h;
	deleteArea.h = area.h;
	deleteArea.x = area.x + area.w - deleteArea.w;
	deleteArea.y = area.y;
}
