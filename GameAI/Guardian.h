//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef GUARDIAN_H
#define GUARDIAN_H

#include "BaseTank.h"
#include <SDL.h>
#include "Commons.h"

//---------------------------------------------------------------

class Guardian : protected BaseTank
{
	//---------------------------------------------------------------
public:
	Guardian(SDL_Renderer* renderer, TankSetupDetails details);
	~Guardian();

	void ChangeState(BASE_TANK_STATE newState);

	void Update(float deltaTime, SDL_Event e);

	void SetPath(vector<Vector2D> positions);

	//---------------------------------------------------------------
protected:
	void	MoveInHeadingDirection(float deltaTime);

	//---------------------------------------------------------------
private:
	vector<Vector2D>	mPathPositions;
	int mCurrentPosition;
};

//---------------------------------------------------------------

#endif //GUARDIAN_H