//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "Guardian.h"
#include "TankManager.h"
#include "C2DMatrix.h"

//--------------------------------------------------------------------------------------------------

Guardian::Guardian(SDL_Renderer* renderer, TankSetupDetails details) : BaseTank(renderer, details)
{
	mCurrentPosition = 0;
	mPathPositions.push_back(GetCentralPosition());
}

//--------------------------------------------------------------------------------------------------

Guardian::~Guardian()
{
}

//--------------------------------------------------------------------------------------------------

void Guardian::SetPath(vector<Vector2D> positions)
{
	mPathPositions.clear();
	mPathPositions = positions;
}

//--------------------------------------------------------------------------------------------------

void Guardian::ChangeState(BASE_TANK_STATE newState)
{
	BaseTank::ChangeState(newState);
}

//--------------------------------------------------------------------------------------------------

void Guardian::Update(float deltaTime, SDL_Event e)
{
	//This is a dumb tank. Do NOT copy this approach.

	//Did we see a tank?
	if(mTanksICanSee.size() == 0)
	{
		ChangeState(TANKSTATE_IDLE);

		//If there are no visible tanks, then keep moving.
		Vector2D vecToTarget = mPathPositions[mCurrentPosition]-GetCentralPosition();
		mHeading = Vec2DNormalize(vecToTarget);
		mCurrentSpeed = 75.0;

		if (vecToTarget.Length() <= 50)
		{
			mCurrentPosition++;
			if (mCurrentPosition >= (int)mPathPositions.size())
				mCurrentPosition = 0;
		}
	}
	else
	{
		//Rotate man to face enemy tank.
		Vector2D toTarget = mTanksICanSee[0]->GetCentralPosition()-GetCentralPosition();
		toTarget.Normalize();

		//Only GUARDIAN allowed to do this!
		Vector2D up(0.0, -1.0);
		Vector2D right(1.0, 0.0);
		double dot = up.Dot(toTarget);
		mManRotationAngle = RadsToDegs(acos(dot));
		double dotRight = right.Dot(toTarget);
		if (dotRight < 0)
			mManRotationAngle *= -1;

		mManFireDirection = toTarget;

		//Otherwise stop moving and fire at the visible tank.
		mVelocity = Vector2D();
		mCurrentSpeed = 0.0;
		ChangeState(TANKSTATE_MANFIRE);
	}

	BaseTank::Update(deltaTime, e);
}

//--------------------------------------------------------------------------------------------------

void Guardian::MoveInHeadingDirection(float deltaTime)
{
	//Get the force that propels in current heading.
	Vector2D force = (mHeading*mCurrentSpeed)-mVelocity;

	//Acceleration = Force/Mass
	Vector2D acceleration = force/GetMass();

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

//--------------------------------------------------------------------------------------------------