//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL.h>
#include "GameObject.h"

class BaseTank;

//--------------------------------------------------------------------------------------------------

class Projectile : public GameObject
{
	//--------------------------------------------------------------------------------------------------
public:
	Projectile(SDL_Renderer* renderer, ProjectileSetupDetails details, BaseTank* firer);
	~Projectile();

	virtual void		Update(float deltaTime);
	virtual void		Render();

	BaseTank*	GetFirer() { return mFiringTank; }
	bool		GetAlive() { return mAlive; }

	virtual void	Explode();
	bool			IsExploding() { return mExploding; }
	virtual bool	HasExploded();
	//--------------------------------------------------------------------------------------------------
protected:
	bool		mAlive;
	bool		mExploding;
	Vector2D	mDirection;
	double		mSpeed;
	BaseTank*	mFiringTank;
};

//--------------------------------------------------------------------------------------------------
#endif //Projectile_H