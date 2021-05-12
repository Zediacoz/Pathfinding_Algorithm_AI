//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "GameScreenLevel1.h"
#include <iostream>
#include "LevelMap.h"
#include "Texture2D.h"
#include "ObstacleManager.h"
#include "TankManager.h"
#include "ProjectileManager.h"
#include "PickUpManager.h"
#include "GameObject.h"

using namespace::std;

//--------------------------------------------------------------------------------------------------

GameScreenLevel1::GameScreenLevel1(SDL_Renderer* renderer) : GameScreen(renderer)
{
	srand(NULL);
		
	//Set the level map.
	mLevelMap = new LevelMap(renderer);

	//Set up the obstacles.
	ObstacleManager::Instance()->Init(renderer);

	//Set up PickUps
	PickUpManager::Instance()->Init(renderer);

	//Set up projectiles that start on screen.
	ProjectileManager::Instance()->Init(renderer);

	//Set up the collision map.
	mCollisionMap = new TileType*[kMapWidth];
	for (int i = 0; i < kMapWidth; i++)
		mCollisionMap[i] = new TileType[kMapHeight];
	SetUpCollisionMap();
	OutputCollisionMap();

	//Set up the tanks.
	TankManager::Instance()->Init(renderer, mCollisionMap);
}

//--------------------------------------------------------------------------------------------------

GameScreenLevel1::~GameScreenLevel1()
{
	//Level map.
	delete mLevelMap;
	mLevelMap = NULL;

	//Clear up the collision map memory.
	for (int i = 0; i < kMapWidth; i++)
	{
		delete mCollisionMap[i];
		mCollisionMap[i] = NULL;
	}
	delete mCollisionMap;
	mCollisionMap = NULL;
}

//--------------------------------------------------------------------------------------------------

void GameScreenLevel1::Render()
{
	//Draw the background.
	mLevelMap->Render();

	//Draw the bullets.
	ProjectileManager::Instance()->RenderProjectiles();
	
	//Draw the obstacles over the level map.
	ObstacleManager::Instance()->RenderObstacles();

	//Draw the pickUps.
	PickUpManager::Instance()->RenderPickUps();

	//Draw the tanks after everything else.
	TankManager::Instance()->RenderTanks();
}

//--------------------------------------------------------------------------------------------------

void GameScreenLevel1::Update(float deltaTime, SDL_Event e)
{
	//Update the obstacles.
//	ObstacleManager::Instance()->UpdateObstacles(deltaTime, e);

	//Update the tanks.
	TankManager::Instance()->UpdateTanks(deltaTime, e);

	//Update the bullets.
	ProjectileManager::Instance()->UpdateProjectiles(deltaTime);

	//Do collision checks.
	ProjectileManager::Instance()->CheckForCollisionsOnMap(mCollisionMap);
	ProjectileManager::Instance()->CheckForCollisions(TankManager::Instance()->GetTanks());

	//Update the pickups.
	PickUpManager::Instance()->UpdatePickUps(deltaTime);
	PickUpManager::Instance()->CheckForCollisions(TankManager::Instance()->GetTanks());

}

//--------------------------------------------------------------------------------------------------

void GameScreenLevel1::SetUpCollisionMap()
{
	//All tiles start out as empty.
	for (int x = 0; x < kMapWidth; x++)
	{
		for (int y = 0; y < kMapHeight; y++)
		{
			mCollisionMap[x][y] = TILETYPE_EMPTY;
		}
	}

	//Outer edges are always blocked.
	for (int x = 0; x < kMapWidth; x++)
	{
		mCollisionMap[x][0] = TILETYPE_BLOCKED;
		mCollisionMap[x][kMapHeight-1] = TILETYPE_BLOCKED;
	}
	for (int y = 0; y < kMapHeight; y++)
	{
		mCollisionMap[0][y] = TILETYPE_BLOCKED;
		mCollisionMap[kMapWidth-1][y] = TILETYPE_BLOCKED;
	}

	//Get obstacles from ObstacleManager and block out where the buildings are located.
	vector<GameObject*> obstacles = ObstacleManager::Instance()->GetObstacles();
	for (unsigned int i = 0; i < obstacles.size(); i++)
	{
		if (obstacles.at(i)->GetGameObjectType() != GAMEOBJECT_OBSTACLE_BORDER)
		{
			vector<Vector2D> rect = obstacles.at(i)->GetAdjustedBoundingBox();
			int width  = (int)(rect[0]-rect[1]).Length();
			int height = (int)(rect[0]-rect[3]).Length();

			for (int y = (int)(obstacles.at(i)->GetPosition().y / kTileDimensions); y < (int)((obstacles.at(i)->GetPosition().y + height + kHalfTileDimensions) / kTileDimensions); y++)
			{
				for (int x = (int)(obstacles.at(i)->GetPosition().x / kTileDimensions); x < (int)((obstacles.at(i)->GetPosition().x + width + kHalfTileDimensions) / kTileDimensions); x++)
				{
					mCollisionMap[x][y] = TILETYPE_BLOCKED;
				}
			}
		}
	}

}

//--------------------------------------------------------------------------------------------------

void GameScreenLevel1::OutputCollisionMap()
{
	cout << endl << endl;
	//All tiles start out as empty.
	for (int y = 0; y < kMapHeight; y++)
	{
		for (int x = 0; x < kMapWidth; x++)
		{
			if (mCollisionMap[x][y] == TILETYPE_EMPTY)
				cout << "-";
			else if (mCollisionMap[x][y] == TILETYPE_BLOCKED)
				cout << "X";
		}

		cout << endl;
	}
	cout << endl << endl;
}

//--------------------------------------------------------------------------------------------------