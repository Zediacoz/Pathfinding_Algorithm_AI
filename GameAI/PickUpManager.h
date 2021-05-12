//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef _PICKUPMANAGER_H
#define _PICKUPMANAGER_H

// PickUp Manager is a singleton that keeps hold of all the pick ups in the scene.
// It collects its information from the XML file and can give useful information on request.

#include <SDL.h>
#include <vector>
#include "Commons.h"
using namespace::std;

class GameObject;
class BaseTank;

//--------------------------------------------------------------------------------------------------

class PickUpManager
{
	//---------------------------------------------------------------
public:
	~PickUpManager();

	static PickUpManager*		Instance();
	void						Init(SDL_Renderer* renderer);
	void						RenderPickUps();
	void						UpdatePickUps(float deltaTime);

	vector<GameObject*>			GetAllPickUps()								{return mPickups;}

	void						CheckForCollisions(vector<BaseTank*> listOfTanks);

	//---------------------------------------------------------------
private:
	PickUpManager();

	void LoadPickUps();
	void CheckForACollision(BaseTank* tank);
	void GenerateAPickUp();
	bool CollisionWithAnObstacle(Vector2D positionToCheck);

	//---------------------------------------------------------------
private:
	static PickUpManager* mInstance;
	SDL_Renderer*		  mRenderer;

	vector<GameObject*>   mPickups;
	vector<int>			  mPickUpIndicesToDelete;

	float				  mTimeUntilNextPickupGeneration;
};

//--------------------------------------------------------------------------------------------------

#endif //_PICKUPMANAGER_H