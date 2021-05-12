//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef C014944HTANK_H
#define C014944HTANK_H

#include "../../BaseTank.h"
#include <SDL.h>
#include "../../Commons.h"

class c014944hSteeringBehaviours;
class PathFinder_DRS;

//---------------------------------------------------------------

class c014944hTank : public BaseTank
{
	//---------------------------------------------------------------
public:
	c014944hTank(SDL_Renderer* renderer, TankSetupDetails details);
	~c014944hTank();

	void ChangeState(BASE_TANK_STATE newState);

	void Update(float deltaTime, SDL_Event e);

	void Render();

	void RotateHeadingByRadian(double radian, int sign);

	//---------------------------------------------------------------
protected:
	void	MoveInHeadingDirection(float deltaTime);

private:
	Vector2D mTargetPosition;
	c014944hSteeringBehaviours* mSteering;
	PathFinder_DRS* mPathFinderDRS;
};

//---------------------------------------------------------------

#endif //C014944HTANK_H