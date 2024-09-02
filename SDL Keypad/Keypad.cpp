#include "Keypad.h"

#pragma region C++ Includes
#include <vector>
#pragma endregion

#pragma region Game Includes
#include "Utilities.h"
#pragma endregion

#pragma region Constant Parameters

//	Layout parameters
#define RADIUS_RATIO 0.35f
#define CHAR_SIZE_RATIO 0.125f
#define ACTIVE_CHAR_SIZE_RATIO 1.2f
#define SUBMIT_CHAR_SIZE_RATIO 1.2f

//	Trigonometric utilities
/*
 * Defining our own PI constants, instead of M_PI
 * from cmath, to keep calculations as floats and
 * not as doubles. In games it's common practice
 * to trade precision with performance as the
 * precision loss isn't as critical as performance.
 * PI is defined with the same value as M_PI but
 * as a float, so less precise (most likely, many
 * of the decimal digits written here cannot be
 * represented in a float).
 * PI2 represents (2 * PI), so a full circumference.
 * HPI represents (PI / 2) so a rect angle (90°).
 */
#define PI 3.14159265358979323846264338327950288f
#define PI2 (2 * PI)
#define HPI (PI / 2)
#pragma endregion

Keypad::Keypad(const string & charset, const SDL_Color & mainColor, const SDL_Color & accentColor) :
	charset(charset),
	charsetLength((int)charset.size()),
	mainColor(mainColor),
	accentColor(accentColor),
	angleStep(PI2 / charsetLength),
	rotation(0.0f),
	dragging(false)
{ }

string Keypad::ReadBuffer()
{
	string bufferContent = buffer.str();
	ClearBuffer();
	return bufferContent;
}

void Keypad::GetPointOnWheel(const SDL_Rect & area, float angle, SDL_Point & point) const
{
	//	Find the circumference center
	GetWheelCenter(area, point);

	//	Get the wheel radius
	int radius = GetWheelRadius(area);

	//	Shift angle to have the first character on top instead of on side
	angle -= HPI;

	//	Move the point to the circumference, at the given angle
	point.x += (int)(radius * cos(angle));
	point.y += (int)(radius * sin(angle));
}

void Keypad::SetRotation(float angleRad)
{
	/*
	 * Set the rotation making sure it stays in a high-precision
	 * interval by wrapping it between 0 and 2 * PI
	 */
	rotation = fmod(angleRad + PI2, PI2);
}

void Keypad::BeginInteraction(const SDL_Point & point)
{
	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent interaction
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Retrieve interactive parts
	SDL_Rect submitArea;
	GetParts(area, &submitArea, nullptr);

	if(SDL_PointInRect(&point, &submitArea))
	{	//	Handle submit clicked
		buffer << PeekActiveCharacter();
 	}
	else if(SDL_PointInRect(&point, &area))
	{	//	Handle drag start
		dragging = true;
	}
}

void Keypad::EndInteraction()
{
	dragging = false;
}

void Keypad::MoveInteraction(const SDL_Point & from, const SDL_Point & to)
{
	//	Handle move interaction only when dragging
	if(!dragging)
		return;

	//	Check viewport aera is valid
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent interaction
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	// Calculate angle shift
	/*
	 * From the position before and after the drag we
	 * can build vectors relative to the center of the keypad,
	 * calculate their angle via the atan(y / x) (aka atan2)
	 * and the difference between these angles will be the
	 * rotation imparted by the player during this frame.
	 */
	const SDL_Point fromBiased =
	{
		from.x - (area.x + area.w / 2),
		from.y - (area.y + area.h / 2)
	};
	const SDL_Point toBiased =
	{
		to.x - (area.x + area.w / 2),
		to.y - (area.y + area.h / 2)
	};
	float currentAngle = (float)atan2(toBiased.y, toBiased.x);
	float prevAngle = (float)atan2(fromBiased.y, fromBiased.x);

	//	Impart the rotation
	Rotate(currentAngle - prevAngle);
}

void Keypad::Render(SDL_Renderer * r) const
{
	SDL_Rect const * areaPtr = GetViewportArea();

	//	If no viewport area is set, prevent rednering
	if(!areaPtr)
		return;

	const SDL_Rect & area = *areaPtr;

	//	Get layout parameters
	SDL_Point wheelCenter;
	GetWheelCenter(area, wheelCenter);
	int radius = GetWheelRadius(area);

	SDL_Rect submitArea;
	SDL_Rect activeCharacterArea;
	GetParts(area, &submitArea, &activeCharacterArea);

	int characterSize = GetDigitSize(area);

	//	Render all characters in the charset in a circle
	SDL_Point pointOnWheel;
	for(int c = 0; c < charsetLength; c++)
	{
		float angle = rotation + angleStep * c;
		GetPointOnWheel(area, angle, pointOnWheel);
		RenderLabel(
			r,
			string{charset[c]},
			pointOnWheel.x,
			pointOnWheel.y,
			mainColor,
			characterSize
		);
	}

	// Render active character at the center of the wheel
	char activeChar = PeekActiveCharacter();
	RenderLabel(
		r,
		string{activeChar},
		wheelCenter.x,
		wheelCenter.y,
		mainColor,
		submitArea.h
	);

	//	Render UX frame for submit button
	vector<SDL_Point> submitFramePoints;
	submitFramePoints.push_back({submitArea.x, submitArea.y + submitArea.h / 4});
	submitFramePoints.push_back({submitArea.x, submitArea.y + submitArea.h});
	submitFramePoints.push_back({submitArea.x + submitArea.w, submitArea.y + submitArea.h});
	submitFramePoints.push_back({submitArea.x + submitArea.w, submitArea.y + submitArea.h / 4});
	submitFramePoints.push_back({submitArea.x + submitArea.w / 2, submitArea.y - submitArea.h / 4});
	submitFramePoints.push_back({submitArea.x, submitArea.y + submitArea.h / 4});
	SDL_SetRenderDrawColor(r, accentColor.r, accentColor.g, accentColor.b, accentColor.a);
	SDL_RenderDrawLines(r, submitFramePoints.data(), (int)submitFramePoints.size());

	// Render active character frame on the wheel
	SDL_SetRenderDrawColor(r, mainColor.r, mainColor.g, mainColor.g, mainColor.a);
	SDL_RenderDrawRect(r, &activeCharacterArea);

}

int Keypad::GetActiveCharacterIndex() const
{
	int curChar = (int)((PI2 - rotation + angleStep / 2) / angleStep);
	if(
		curChar < 0 ||
		curChar >= charsetLength
		)
		curChar = 0;
	return curChar;
}

/*
 * I'm not inlining this simple function to keep the CHAR_SIZE_RATIO define private
 */
int Keypad::GetDigitSize(const SDL_Rect & area) const
{
	return (int)(GetShortestAxis(area) * CHAR_SIZE_RATIO);
}

void Keypad::GetWheelCenter(const SDL_Rect & area, SDL_Point & center) const
{
	center.x = area.x + area.w / 2;
	center.y = area.y + area.h / 2;
}

/*
 * I'm not inlining this simple function to keep the RADIUS_RATIO define private
 */
int Keypad::GetWheelRadius(const SDL_Rect & area) const
{
	return (int)(GetShortestAxis(area) * RADIUS_RATIO);
}

void Keypad::GetParts(const SDL_Rect & area, SDL_Rect * submitArea, SDL_Rect * activeCharacterArea) const
{
	//	Calculate submit area, if requested
	if(submitArea)
	{
		SDL_Point wheelCenter;
		GetWheelCenter(area, wheelCenter);
		const int submitCharacterAreaSize = (int)(GetDigitSize(area) * SUBMIT_CHAR_SIZE_RATIO);
		*submitArea =
		{
			wheelCenter.x - submitCharacterAreaSize / 2,
			wheelCenter.y - submitCharacterAreaSize / 2,
			submitCharacterAreaSize,
			submitCharacterAreaSize
		};
	}

	//	Calculate active character area, if requested
	if(activeCharacterArea)
	{
		SDL_Point pointOnWheel;
		GetPointOnWheel(area, 0.0f, pointOnWheel);
		const int activeCharacterAreaSize = (int)(GetDigitSize(area) * ACTIVE_CHAR_SIZE_RATIO);
		*activeCharacterArea =
		{
			pointOnWheel.x - activeCharacterAreaSize / 2,
			pointOnWheel.y - activeCharacterAreaSize / 2,
			activeCharacterAreaSize,
			activeCharacterAreaSize
		};
	}
}
