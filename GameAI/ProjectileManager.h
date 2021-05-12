//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef _PROJECTILEMANAGER_H
#define _PROJECTILEMANAGER_H

//--------------------------------------------------------------------------------------------------
// Bullet Manager is a singleton that keeps hold of all the bullets in the scene.
// It is a singleton so that bullets can be created from anywhere.

#include <SDL.h>
#include "Commons.h"
#include <vector>
using namespace::std;

class Projectile;
class BaseTank;
class GameObject;

//--------------------------------------------------------------------------------------------------

class ProjectileManager
{
	//---------------------------------------------------------------
public:
	~ProjectileManager();

	static ProjectileManager* Instance();
	void Init(SDL_Renderer* renderer);

	void CreateProjectile(SDL_Renderer* renderer, ProjectileSetupDetails details, BaseTank* firer);
	void UpdateProjectiles(float deltaTime);
	void RenderProjectiles();

	void CheckForCollisions(vector<BaseTank*> listOfObjects);
	void CheckForCollisionsOnMap(TileType** collisionMap);
	void CheckForCollisions(vector<GameObject*> listOfObjects);

	vector<GameObject*> GetVisibleMines(BaseTank* lookingTank);

	//---------------------------------------------------------------
private:
	ProjectileManager();
	void CheckForACollision(GameObject* gameObject);
	void SetUpIndestructibleMines();

	//---------------------------------------------------------------
private:
	static ProjectileManager* mInstance;
	SDL_Renderer*			  mRenderer;

	vector<Projectile*>		  mProjectiles;
	int						  mProjectileIndexToDelete;

	//These are to ensure there is a penalty of colliding with buuldings.
	vector<Projectile*>		  mIndestructibleMines;
};

//--------------------------------------------------------------------------------------------------

#endif //_PROJECTILEMANAGER_H