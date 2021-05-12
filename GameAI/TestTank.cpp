//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "TestTank.h"
#include "TankManager.h"
#include "Commons.h"
#include "C2DMatrix.h"
#include "PathFinder_DRS.h"

//--------------------------------------------------------------------------------------------------

TestTank::TestTank(SDL_Renderer* renderer, TankSetupDetails details)
	: BaseTank(renderer, details)
{
	mPathFinderDRS = new PathFinder_DRS(renderer, mCollisionMap);
}

//--------------------------------------------------------------------------------------------------

TestTank::~TestTank()
{
}

//--------------------------------------------------------------------------------------------------

void TestTank::ChangeState(BASE_TANK_STATE newState)
{
	BaseTank::ChangeState(newState);
}

//--------------------------------------------------------------------------------------------------

void TestTank::Update(float deltaTime, SDL_Event e)
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
					mPathFinderDRS->FindPath(GetCentralPosition(), mTargetPosition);
				break;
			}
		}
		break;
	}

	//Call parent update.
	BaseTank::Update(deltaTime, e);
}

//--------------------------------------------------------------------------------------------------

void TestTank::Render()
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

void TestTank::MoveInHeadingDirection(float deltaTime)
{
}

//--------------------------------------------------------------------------------------------------

void TestTank::RotateHeadingByRadian(double radian, int sign)
{
}

//--------------------------------------------------------------------------------------------------