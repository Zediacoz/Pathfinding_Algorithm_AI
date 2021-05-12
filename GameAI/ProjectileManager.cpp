//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "ProjectileManager.h"
#include <SDL.h>
#include <iostream>
#include "Projectile.h"
#include "BaseTank.h"
#include <algorithm>
#include "Commons.h"
#include "ObstacleManager.h"

//Initialise the instance to null.
ProjectileManager* ProjectileManager::mInstance = NULL;

//--------------------------------------------------------------------------------------------------

ProjectileManager* ProjectileManager::Instance()
{
	if(!mInstance)
	{
		mInstance = new ProjectileManager;
	}

	return mInstance;
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::Init(SDL_Renderer* renderer)
{
	mInstance->mRenderer = renderer;
	mInstance->mProjectileIndexToDelete = -1;
	mInstance->SetUpIndestructibleMines();
}

//--------------------------------------------------------------------------------------------------

ProjectileManager::ProjectileManager()
{
}

//--------------------------------------------------------------------------------------------------

ProjectileManager::~ProjectileManager()
{
	mInstance = NULL;

	for(unsigned int i = 0; i < mInstance->mProjectiles.size(); i++)
		delete mInstance->mProjectiles[i];
	mInstance->mProjectiles.clear();
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::UpdateProjectiles(float deltaTime)
{
	//Update the indestructbile mines.
	for(unsigned int i = 0; i < mInstance->mIndestructibleMines.size(); i++)
		mInstance->mIndestructibleMines[i]->Update(deltaTime);

	//Update the projectiles.
	for(unsigned int i = 0; i < mInstance->mProjectiles.size(); i++)
		mInstance->mProjectiles[i]->Update(deltaTime);

	//Check if any projectile have left the screen.
	Rect2D screenBox(0.0f, 0.0f, kScreenWidth, kScreenHeight);
	for(unsigned int i = 0; i < mInstance->mProjectiles.size(); i++)
	{
		if(!PointInBox(mInstance->mProjectiles[i]->GetCentralPosition(), screenBox))
		{
			//Prepare this projectile for deletion.
			mInstance->mProjectileIndexToDelete = i;
			break;
		}
	}

	//Remove one projectile a frame.
	if(mInstance->mProjectileIndexToDelete != -1)
	{
		Projectile* projectileToDelete = mInstance->mProjectiles.at(mInstance->mProjectileIndexToDelete);
		mInstance->mProjectiles.erase(mInstance->mProjectiles.begin()+mInstance->mProjectileIndexToDelete);
		delete projectileToDelete;
		mProjectileIndexToDelete = -1;
	}
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::RenderProjectiles()
{
	for(unsigned int i = 0; i < mIndestructibleMines.size(); i++)
		mInstance->mIndestructibleMines[i]->Render();

	for(unsigned int i = 0; i < mProjectiles.size(); i++)
		mInstance->mProjectiles[i]->Render();
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::CheckForCollisions(vector<BaseTank*> listOfObjects)
{
	for(unsigned int i = 0; i < listOfObjects.size(); i++)
	{
		GameObject* gameObject = (GameObject*)listOfObjects[i];
		CheckForACollision(gameObject);
	}
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::CheckForCollisionsOnMap(TileType** collisionMap)
{
	for (unsigned int i = 0; i < mInstance->mProjectiles.size(); i++)
	{
		if (collisionMap[(int)(mInstance->mProjectiles[i]->GetCentralPosition().x / kTileDimensions)][(int)(mInstance->mProjectiles[i]->GetCentralPosition().y / kTileDimensions)] == TILETYPE_BLOCKED)
		{
			//Prepare this projectile for deletion.
			if (!mProjectiles[i]->IsExploding())
				mProjectiles[i]->Explode();
			//mInstance->mProjectiles[i]->RegisterHit();
		}
	}
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::CheckForCollisions(vector<GameObject*> listOfObjects)
{
	for(unsigned int i = 0; i < listOfObjects.size(); i++)
	{
		CheckForACollision(listOfObjects[i]);

		//Prepare this bullet for deletion.
		mInstance->mProjectileIndexToDelete = i;

		break;
	}
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::CheckForACollision(GameObject* gameObject)
{
	if (gameObject != NULL)
	{
		vector<Vector2D> rect = gameObject->GetAdjustedBoundingBox();
		for (unsigned int i = 0; i < mProjectiles.size(); i++)
		{
			if (mProjectiles[i]->GetAlive())
			{
				if (TriangleCollision(rect[1], rect[2], rect[3], mInstance->mProjectiles[i]->GetPosition()) || TriangleCollision(rect[0], rect[1], rect[3], mInstance->mProjectiles[i]->GetPosition()))
				{
					if (gameObject->GetGameObjectType() == GAMEOBJECT_TANK)
					{
						if ((GameObject*)mInstance->mProjectiles[i]->GetFirer() == gameObject)
						{
							//Do nothing if this is a bullet from the firing tank.
							return;
						}
						else
						{
							//Damage Tank.
							BaseTank* tank = (BaseTank*)gameObject;
							tank->TakeDamage(mInstance->mProjectiles[i]->GetGameObjectType());

							//If the hit tank has no health then it is destroyed and a bonus score is awared to the firing tank.
							if (tank->GetHealth() <= 0)
								mInstance->mProjectiles[i]->GetFirer()->AddToScore(SCORE_DESTROYEDTANK);

							//Add score to firing tank.
							switch (mInstance->mProjectiles[i]->GetGameObjectType())
							{
							case GAMEOBJECT_BULLET:
								mInstance->mProjectiles[i]->GetFirer()->AddToScore(SCORE_BULLETHIT);
								break;

							case GAMEOBJECT_ROCKET:
								mInstance->mProjectiles[i]->GetFirer()->AddToScore(SCORE_ROCKETHIT);
								break;

							case GAMEOBJECT_MINE:
								mInstance->mProjectiles[i]->GetFirer()->AddToScore(SCORE_MINEHIT);
								break;
							}
						}

						//Prepare this projectile for deletion.
						if (!mProjectiles[i]->IsExploding())
							mProjectiles[i]->Explode();
					}
					else
					{
						//Prepare this projectile for deletion.
						if (!mProjectiles[i]->IsExploding())
							mProjectiles[i]->Explode();
					}
				}
			}
			else
			{
				if (mProjectiles[i]->HasExploded())
				{
					if (mProjectileIndexToDelete == -1)
					{
						mProjectileIndexToDelete = i;
						break;
					}
				}
			}
		}

		//Check against the indestructible mines.
		for (unsigned int i = 0; i < mInstance->mIndestructibleMines.size(); i++)
		{
			if (Circle(mInstance->mIndestructibleMines[i]->GetCentralPosition(), mInstance->mIndestructibleMines[i]->GetCollisionRadius(), gameObject->GetCentralPosition(), gameObject->GetCollisionRadius()))
			{
				if (gameObject->GetGameObjectType() == GAMEOBJECT_TANK)
				{
					//Damage Tank.
					BaseTank* tank = (BaseTank*)gameObject;
					tank->TakeDamage(mInstance->mIndestructibleMines[i]->GetGameObjectType());
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::CreateProjectile(SDL_Renderer* renderer, ProjectileSetupDetails details, BaseTank* firer)
{
	switch(details.GameObjectType)
	{
		case GAMEOBJECT_BULLET:
			if(firer->GetBullets() > 0)
			{
				mInstance->mProjectiles.push_back(new Projectile(renderer, details, firer));
				firer->DeductABullet();
			}
		break;

		case GAMEOBJECT_ROCKET:
			if(firer->GetRockets() > 0)
			{
				mInstance->mProjectiles.push_back(new Projectile(renderer, details, firer));
				firer->DeductARocket();
			}
		break;

		case GAMEOBJECT_MINE:
			if(firer->GetMines() > 0)
			{
				mInstance->mProjectiles.push_back(new Projectile(renderer, details, firer));
				firer->DeductAMine();
			}
		break;
	}
}

//--------------------------------------------------------------------------------------------------

void ProjectileManager::SetUpIndestructibleMines()
{
	//Set the projectile setup details.
	ProjectileSetupDetails details;
	details.Direction = Vector2D();
	details.GameObjectType = GAMEOBJECT_MINE;
	details.ImagePath = kMinePath;
	details.RotationAngle = 0.0f;

	//Indestructable mines on buildings.
	for (unsigned int i = 0; i < ObstacleManager::Instance()->GetObstacles().size(); i++)
	{
		GameObject* currentObstacle = ObstacleManager::Instance()->GetObstacles().at(i);

		//Mine at the centre of an obstacle
		if (currentObstacle->GetGameObjectType() != GAMEOBJECT_OBSTACLE_BORDER)
		{
			details.StartPosition = currentObstacle->GetCentralPosition();
			mInstance->mIndestructibleMines.push_back(new Projectile(mRenderer, details, NULL));
		}
	}
}

//--------------------------------------------------------------------------------------------------

vector<GameObject*> ProjectileManager::GetVisibleMines(BaseTank* lookingTank)
{
	vector<GameObject*> mines;

	for (unsigned int i = 0; i < mInstance->mProjectiles.size(); i++)
	{
		GameObject* currentProjectile = mInstance->mProjectiles.at(i);
		if (currentProjectile->GetGameObjectType() == GAMEOBJECT_MINE)
		{
			Vector2D heading = lookingTank->GetHeading();
			heading.Normalize();
			Vector2D vecToTarget = lookingTank->GetCentralPosition() - currentProjectile->GetCentralPosition();
			double vecToTargetLength = vecToTarget.Length();

			//If Mine is too far away then it can't be seen.
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

					//Mine is within fov, but is there a building in the way?
					for (unsigned int j = 0; j < ObstacleManager::Instance()->GetObstacles().size(); j++)
					{
						GameObject* currentObstacle = ObstacleManager::Instance()->GetObstacles().at(j);
						vector<Vector2D> rect = currentObstacle->GetAdjustedBoundingBox();

						//Check if we have collided with this obstacle.
						if ((!TriangleCollision(rect[1], rect[2], rect[3], point1)) || (!TriangleCollision(rect[0], rect[1], rect[3], point1)) &&
							(!TriangleCollision(rect[1], rect[2], rect[3], point2)) || (!TriangleCollision(rect[0], rect[1], rect[3], point2)) &&
							(!TriangleCollision(rect[1], rect[2], rect[3], point3)) || (!TriangleCollision(rect[0], rect[1], rect[3], point3)))
						{
							//Add mine projectile to the list that will be returned.
							mines.push_back(currentProjectile);

							//cout << "Can see mine!!" << endl;

							//Get out of the obstacle check.
							break;
						}
					}
				}
			}
		}
	}

	//Return mines.
	return mines;
}

//--------------------------------------------------------------------------------------------------