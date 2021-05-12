#include "c014944hTank.h"
#include "../../TankManager.h"
#include "../../Commons.h"
#include "../../C2DMatrix.h"
#include "c014944hSteeringBehaviours.h"
#include "../../PathFinder_DRS.h"

//--------------------------------------------------------------------------------------------------

c014944hTank::c014944hTank(SDL_Renderer* renderer, TankSetupDetails details)
	: BaseTank(renderer, details)
{
	mTargetPosition.Zero();
	mPathFinderDRS = new PathFinder_DRS(renderer, mCollisionMap);
	mSteering = new c014944hSteeringBehaviours(this);
}

//--------------------------------------------------------------------------------------------------

c014944hTank::~c014944hTank()
{
	delete mSteering;
	mSteering = NULL;
}

//--------------------------------------------------------------------------------------------------

void c014944hTank::ChangeState(BASE_TANK_STATE newState)
{
	BaseTank::ChangeState(newState);
}

//--------------------------------------------------------------------------------------------------

void c014944hTank::Update(float deltaTime, SDL_Event e)
{
	switch (e.type)
	{
		//Deal with mouse click input.
	case SDL_MOUSEBUTTONUP:
	{
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			mTargetPosition = Vector2D(e.button.x, e.button.y);
			//mSteering->SetSeek(true);
			//mSteering->SetArrive(true);
			//mSteering->SetFlee(true);
			mPathFinderDRS->FindPath(GetCentralPosition(), mTargetPosition);
			break;
		}
	}
	break;
	}



	//Call parent update.
	BaseTank::Update(deltaTime, e);
}

void c014944hTank::Render()
{
	if (!mPathFinderDRS->mPathData.empty())
	{
		for (unsigned int i = 0; i < mPathFinderDRS->mPathData.size() - 1; i++)
		{
			DrawDebugLine(mPathFinderDRS->mPathData.at(i), mPathFinderDRS->mPathData.at(i + 1), 255, 0, 0);
			DrawDebugCircle(mPathFinderDRS->mPathData.at(i), 2.5, 0, 255, 255);
		}
	}

	BaseTank::Render();
}

//--------------------------------------------------------------------------------------------------

void c014944hTank::MoveInHeadingDirection(float deltaTime)
{
	if (!(mTargetPosition.x == 0.0f && mTargetPosition.y == 0.0f))
	{
		//Get the force that propels in current heading.
		Vector2D force = mSteering->CombinedBehaviours(mTargetPosition);

		//Acceleration = Force/Mass
		Vector2D acceleration = force / GetMass();

		//Update velocity.
		mVelocity += acceleration * deltaTime;

		//Don't allow the tank does not go faster than max speed.
		mVelocity.Truncate(GetMaxSpeed()); //TODO: Add Penalty for going faster than MAX Speed.

										   //Finally, update the position.
		Vector2D newPosition = GetPosition();
		newPosition.x += mVelocity.x*deltaTime;
		newPosition.y += (mVelocity.y/**-1.0f*/)*deltaTime;	//Y flipped as adding to Y moves down screen.
		SetPosition(newPosition);
	}
}

//--------------------------------------------------------------------------------------------------

void c014944hTank::RotateHeadingByRadian(double radian, int sign)
{

}

//--------------------------------------------------------------------------------------------------