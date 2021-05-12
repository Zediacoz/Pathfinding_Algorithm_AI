//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "TankManager.h"
#include "GameObject.h"
#include "BaseTank.h"
#include "ControlledTank.h"
#include <SDL.h>
#include "TinyXML\tinyxml.h"
#include "Commons.h"
#include "ObstacleManager.h"
#include "Guardian.h"
#include "TestTank.h"
#include <cassert>
#include "StudentTanks/DimitarChobanov/c014944hTank.h"


//Initialise the instance to null.
TankManager* TankManager::mInstance = NULL;

//--------------------------------------------------------------------------------------------------

TankManager::TankManager()
{
	
}

//--------------------------------------------------------------------------------------------------

TankManager::~TankManager()
{
	for(unsigned int i = 0; i < mTanks.size(); i++)
		delete mTanks[i];
	mTanks.clear();
}

//--------------------------------------------------------------------------------------------------

TankManager* TankManager::Instance()
{
	if(!mInstance)
	{
		mInstance = new TankManager;
	}

	return mInstance;
}

//--------------------------------------------------------------------------------------------------

void TankManager::Init(SDL_Renderer* renderer, TileType** collisionMap)
{
	mCollisionMap = collisionMap;
	mTankIndexToDelete = -1;
	LoadTanks(renderer);
	mAccumulatedTimeUntilBonusPoints = 0.0f;
}

//--------------------------------------------------------------------------------------------------

void TankManager::UpdateTanks(float deltaTime, SDL_Event e)
{
	//In the event of the player pressing the S key, output score for all surviving tanks.
	switch (e.type)
	{
	case SDL_KEYUP:
		switch (e.key.keysym.sym)
		{
			//Tank speed keys.
		case SDLK_s:
			for (unsigned int i = 0; i < mTanks.size(); i++)
				mTanks[i]->OutputScoreDetails();
			break;
		}
		break;
	}

	//Accumulate time and at set point give all living tanks a bonus.
	mAccumulatedTimeUntilBonusPoints += deltaTime;

	for (unsigned int i = 0; i < mTanks.size(); i++)
	{
		//Has the required survival time passed for a bonus?
		if (mAccumulatedTimeUntilBonusPoints > kSurvivalTimeUntilBonus)
			mTanks[i]->AddToScore(SCORE_SURVIVAL);

		mTanks[i]->Update(deltaTime, e);

		//Check if tank has gone beyond the limits of the map - If so BOOM time!
		if (mTanks[i]->GetCentralPosition().x < kTileDimensions)
			mTanks[i]->TakeDamage(GAMEOBJECT_OBSTACLE_BORDER);
		else if (mTanks[i]->GetCentralPosition().x > kScreenWidth - kTileDimensions)
			mTanks[i]->TakeDamage(GAMEOBJECT_OBSTACLE_BORDER);

		if (mTanks[i]->GetCentralPosition().y < kTileDimensions)
			mTanks[i]->TakeDamage(GAMEOBJECT_OBSTACLE_BORDER);
		else if (mTanks[i]->GetCentralPosition().y > kScreenHeight - kTileDimensions)
			mTanks[i]->TakeDamage(GAMEOBJECT_OBSTACLE_BORDER);


		//If the health is below zero, delete this tank.
		if (mTanks[i]->GetHealth() <= 0)
		{
			if (!mTanks[i]->IsExploding())
				mTanks[i]->Explode();
			else if (mTanks[i]->HasExploded())
			{
				if (mTankIndexToDelete == -1)
					mTankIndexToDelete = i;
			}
		}
	}

	//If the bonus time has passed then reset it.
	if (mAccumulatedTimeUntilBonusPoints > kSurvivalTimeUntilBonus)
		mAccumulatedTimeUntilBonusPoints = 0.0f;

	//Remove one Tank a frame.
	if (mTankIndexToDelete != -1)
	{
		BaseTank* tankToDelete = mTanks.at(mTankIndexToDelete);
		mTanks.erase(mTanks.begin() + mTankIndexToDelete);
		//delete tankToDelete;
		tankToDelete->OutputScoreDetails();
		mTankIndexToDelete = -1;
	}
}

//--------------------------------------------------------------------------------------------------

void TankManager::RenderTanks()
{
	for(unsigned int i = 0; i < mTanks.size(); i++)
		mTanks[i]->Render();
}

//--------------------------------------------------------------------------------------------------

void TankManager::LoadTanks(SDL_Renderer* renderer)
{
	string imagePath;

	//Get the whole xml document.
	TiXmlDocument doc;
	if(!doc.LoadFile(kTankPath))
	{
		cerr << doc.ErrorDesc() << endl;
	}

	//Now get the root element.
	TiXmlElement* root = doc.FirstChildElement();
	if(!root)
	{
		cerr << "Failed to load file: No root element." << endl;
		doc.Clear();
	}
	else
	{
		TankSetupDetails details;
				
		//Jump to the first 'tank' element - within 'data'
		for(TiXmlElement* tankElement = root->FirstChildElement("tank"); tankElement != NULL; tankElement = tankElement->NextSiblingElement())
		{
			details.StudentName			= tankElement->Attribute("studentName");
			details.TankType			= atoi(tankElement->Attribute("tankType"));
			details.TankImagePath		= tankElement->Attribute("tankPath");
			details.ManImagePath		= tankElement->Attribute("manPath");
			details.TurnRate			= (float)atof(tankElement->Attribute("turnRate"));
			details.StartPosition		= Vector2D((float)atof(tankElement->Attribute("x")), (float)atof(tankElement->Attribute("y")));
			details.Health				= atoi(tankElement->Attribute("health"));
			details.NumOfBullets		= atoi(tankElement->Attribute("bullets"));
			details.NumOfRockets		= atoi(tankElement->Attribute("rockets"));
			details.NumOfMines			= atoi(tankElement->Attribute("mines"));
			details.Fuel				= (float)atof(tankElement->Attribute("fuel"));
			details.Mass				= (float)atof(tankElement->Attribute("mass"));
			details.MaxSpeed			= (float)atof(tankElement->Attribute("maxspeed"));
			details.LeftCannonAttached	= (bool)atoi(tankElement->Attribute("leftCannon"));
			details.RightCannonAttached = (bool)atoi(tankElement->Attribute("rightCannon"));
			details.CollisionMap		= mCollisionMap;

			//Add the new tank with the read in details.
			mTanks.push_back(GetTankObject(renderer, details));
		}
	}
}

//--------------------------------------------------------------------------------------------------

BaseTank* TankManager::GetTankObject(SDL_Renderer* renderer, TankSetupDetails details)
{
	//Create a new tank of required type, but then cast it to BaseTank for the vector list.
	BaseTank* newBaseTank = NULL;

	if(details.StudentName == "ControlledTank")
	{
		ControlledTank* newControlledTank = new ControlledTank(renderer, details);
		newBaseTank = (BaseTank*)newControlledTank;
	}
	if(details.StudentName == "Guardian1")
	{
		Guardian* newTank = new Guardian(renderer, details);
		vector<Vector2D> path;
		path.push_back(Vector2D(60.0, 200.0));
		path.push_back(Vector2D(800.0, 200.0));
		newTank->SetPath(path);
		newBaseTank = (BaseTank*)newTank;
	}
	if (details.StudentName == "Guardian2")
	{
		Guardian* newTank = new Guardian(renderer, details);
		vector<Vector2D> path;
		path.push_back(Vector2D(760.0, 550.0));
		path.push_back(Vector2D(800.0, 450.0));
		path.push_back(Vector2D(880.0, 450.0));
		path.push_back(Vector2D(880.0, 340.0));
		path.push_back(Vector2D(630.0, 340.0));
		path.push_back(Vector2D(630.0, 550.0));
		newTank->SetPath(path);
		newBaseTank = (BaseTank*)newTank;
	}
	if (details.StudentName == "DimitarChobanov")
	{
		c014944hTank* newTank = new c014944hTank(renderer, details);
		newBaseTank = (BaseTank*)newTank;
	}

	//Assert if no tank was setup.
	assert(newBaseTank != NULL);

	//Return our new tank.
	return newBaseTank;
}

//--------------------------------------------------------------------------------------------------
/*
void TankManager::CheckForCollisions()
{
	vector<GameObject*> listOfObjects = ObstacleManager::Instance()->GetObstacles();
	Vector2D tl, tr, bl, br;

	for(unsigned int i = 0; i < listOfObjects.size(); i++)
	{
		Rect2D rect = listOfObjects[i]->GetAdjustedBoundingBox();
		for(unsigned int j = 0; j < mTanks.size(); j++)
		{
			if(PointInBox(mTanks[j]->GetPointAtFrontOfTank(), rect) ||
			   PointInBox(mTanks[j]->GetPointAtRearOfTank(), rect))
			{
				mTanks[j]->Rebound(listOfObjects[i]->GetCentralPosition());
			}
		}
	}
}
*/
//--------------------------------------------------------------------------------------------------

vector<BaseTank*>	TankManager::GetVisibleTanks(BaseTank* lookingTank)
{
	vector<BaseTank*> mVisibleTanks;

	for (unsigned int i = 0; i < mTanks.size(); i++)
	{
		//Don't test self.
		if (mTanks[i] != lookingTank)
		{
			/*if(mTanks.size() == 2)
			{
			//If only two tanks remain then return the toher tank (not self) even though we might not be able to see it.
			mVisibleTanks.push_back(mTanks[i]);
			cout << "Can see you!!" << mTanks[i]->GetTankName() << endl;
			}
			else*/
			{
				//More than 2 tanks remaining, so only return those within the FoV.
				Vector2D heading = lookingTank->GetHeading();
				heading.Normalize();
				Vector2D vecToTarget = mTanks[i]->GetCentralPosition() - lookingTank->GetCentralPosition();
				double vecToTargetLength = vecToTarget.Length();

				//If tank is too far away then it can't be seen.
				if (vecToTargetLength < kFieldOfViewLength)
				{
					vecToTarget.Normalize();
					//cout << "Heading x = " << heading.x << " y = " << heading.y << endl;
					double dotProduct = heading.Dot(vecToTarget);
					//cout << "dot = " << dotProduct << endl;
					if (dotProduct > kFieldOfView)
					{
						Vector2D point1 = lookingTank->GetCentralPosition() + (vecToTarget*(vecToTargetLength*0.33f));
						Vector2D point2 = lookingTank->GetCentralPosition() + (vecToTarget*(vecToTargetLength*0.5f));
						Vector2D point3 = lookingTank->GetCentralPosition() + (vecToTarget*(vecToTargetLength*0.66f));

						//Tank is within fov, but is there a building in the way?
						for (unsigned int j = 0; j < ObstacleManager::Instance()->GetObstacles().size(); j++)
						{
							GameObject* currentObstacle = ObstacleManager::Instance()->GetObstacles().at(j);

							//Check if we have collided with this obstacle.
							vector<Vector2D> rect = currentObstacle->GetAdjustedBoundingBox();

							if ((!TriangleCollision(rect[1], rect[2], rect[3], point1)) || (!TriangleCollision(rect[0], rect[1], rect[3], point1)) &&
								(!TriangleCollision(rect[1], rect[2], rect[3], point2)) || (!TriangleCollision(rect[0], rect[1], rect[3], point2)) &&
								(!TriangleCollision(rect[1], rect[2], rect[3], point3)) || (!TriangleCollision(rect[0], rect[1], rect[3], point3)))
							{
								mVisibleTanks.push_back(mTanks[i]);
								//cout << "Can see you!!" << endl;

								//Get completely out of the obstacle check.
								break;
							}
						}
					}
				}
			}
		}
	}

	return mVisibleTanks;
}

//--------------------------------------------------------------------------------------------------

vector<BaseTank*>	TankManager::GetAudibleTanks(BaseTank* hearingTank)
{
	vector<BaseTank*> mAudibleTanks;

	for(unsigned int i = 0; i < mTanks.size(); i++)
	{
		//Don't test self.
		if(mTanks[i] != hearingTank)
		{
			Vector2D vecToTarget = hearingTank->GetCentralPosition()-mTanks[i]->GetCentralPosition();
			double vecToTargetLength = vecToTarget.Length();

			//If tank is too far away then it can't be seen.
			if(vecToTargetLength < mTanks[i]->GetNoiseRadius()+hearingTank->GetHearingRadius())
			{
				mAudibleTanks.push_back(mTanks[i]);
				cout << "Can hear you!!  " << mTanks[i]->GetTankName() << endl;
			}
		}
	}

	return mAudibleTanks;
}

//--------------------------------------------------------------------------------------------------