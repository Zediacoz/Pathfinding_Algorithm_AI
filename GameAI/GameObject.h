//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Texture2D.h"
#include "Commons.h"
#include <SDL.h>
#include <vector>

//---------------------------------------------------------------

class GameObject
{
	//---------------------------------------------------------------
public:
	GameObject(SDL_Renderer* renderer, GAMEOBJECT_TYPE typeOfGameObject, Vector2D startPosition, string imagePath);
	~GameObject();

	virtual void				Update(float deltaTime, SDL_Event e);
	virtual void				Render();

	virtual void				UpdateAdjustedBoundingBox();
	virtual vector<Vector2D>	GetAdjustedBoundingBox()									{ return mAdjustedBoundingBox; }
	Vector2D					GetPosition()												{ return mPosition; }
	Vector2D					GetCentralPosition();

	void						SetCollisionRadius(double newCollisionRadius)				{ mCollisionRadius = newCollisionRadius; mCollisionRadiusSq = newCollisionRadius*newCollisionRadius; }
	double						GetCollisionRadius()										{ return mCollisionRadius; }
	double						GetCollisionRadiusSq()										{ return mCollisionRadiusSq; }

	GAMEOBJECT_TYPE				GetGameObjectType()											{ return mGameObjectType; }

	//---------------------------------------------------------------
protected:
	void						DrawDebugCircle(Vector2D centrePoint, double radius, int red, int green, int blue);
	void						DrawDebugLine(Vector2D startPoint, Vector2D endPoint, int red, int green, int blue);
	void						DrawCollisionBox();

	//---------------------------------------------------------------
protected:
	SDL_Renderer*	 mRenderer;
	Texture2D*		 mTexture;
	Vector2D		 mPosition;
	double			 mRotationAngle;

	vector<Vector2D> mAdjustedBoundingBox;

	double			 mCollisionRadius;
	double			 mCollisionRadiusSq;

	//---------------------------------------------------------------
private:
	GAMEOBJECT_TYPE mGameObjectType;
};

//---------------------------------------------------------------

#endif //GAMEOBJECT_H