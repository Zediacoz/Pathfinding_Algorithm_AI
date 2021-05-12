//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "Projectile.h"
#include "GameObject.h"
#include "BaseTank.h"
#include "Commons.h"
#include <iostream>

//--------------------------------------------------------------------------------------------------

Projectile::Projectile(SDL_Renderer* renderer, ProjectileSetupDetails details, BaseTank* firer)
	: GameObject(renderer, details.GameObjectType, details.StartPosition, details.ImagePath)
{
	//Centralise the shot using half the texture dimensions.
	mPosition.x -= mTexture->GetWidth()*0.5f;
	mPosition.y -= mTexture->GetHeight()*0.5f;

	mAlive = true;
	mExploding = false;
	mDirection = details.Direction;
	mRotationAngle = details.RotationAngle;
	mFiringTank = firer;

	switch (details.GameObjectType)
	{
	case GAMEOBJECT_BULLET:
		mCollisionRadius = kCollisionBulletRadius;
		mSpeed = kBulletSpeed;
		break;

	case GAMEOBJECT_ROCKET:
		mCollisionRadius = kCollisionRocketRadius;
		mSpeed = kRocketSpeed;
		break;

	case GAMEOBJECT_MINE:
		mCollisionRadius = kCollisionMineRadius;
		mSpeed = 0.0f;
		break;
	}
}

//--------------------------------------------------------------------------------------------------

Projectile::~Projectile()
{
	mFiringTank = NULL;
}

//--------------------------------------------------------------------------------------------------

void Projectile::Update(float deltaTime)
{
	mPosition += mDirection * mSpeed*deltaTime;
}

//--------------------------------------------------------------------------------------------------

void Projectile::Render()
{
	if (mAlive)
	{
		mTexture->Render(mPosition, mRotationAngle);

		//Draw the collision radius.
		DrawDebugCircle(GetCentralPosition(), mCollisionRadius, 0, 0, 255);
	}
}

//--------------------------------------------------------------------------------------------------

void Projectile::Explode()
{
	mAlive = false;
	mExploding = true;
}

//--------------------------------------------------------------------------------------------------

bool Projectile::HasExploded()
{
	return !mAlive;
}

//--------------------------------------------------------------------------------------------------