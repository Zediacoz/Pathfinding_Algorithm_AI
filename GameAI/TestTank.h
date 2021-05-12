//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef TESTTANK_H
#define TESTTANK_H

#include "BaseTank.h"
#include <SDL.h>
#include "Commons.h"

class PathFinder_DRS;

//---------------------------------------------------------------

class TestTank : public BaseTank
{
	//---------------------------------------------------------------
public:
	TestTank(SDL_Renderer* renderer, TankSetupDetails details);
	~TestTank();

	void ChangeState(BASE_TANK_STATE newState);

	void Update(float deltaTime, SDL_Event e);
	void Render();

	void RotateHeadingByRadian(double radian, int sign);

	//---------------------------------------------------------------
protected:
	void	MoveInHeadingDirection(float deltaTime);

	//---------------------------------------------------------------
private:
	PathFinder_DRS* mPathFinderDRS;
	Vector2D		mTargetPosition;
};

//---------------------------------------------------------------

#endif //TESTTANK_H