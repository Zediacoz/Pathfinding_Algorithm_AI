//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "BaseTank.h"
#include "GameObject.h"
#include "Texture2D.h"
#include <SDL.h>
#include <iostream>
#include <fstream>
#include "Constants.h"
#include "ProjectileManager.h"
#include "TankManager.h"
#include <cassert>
#include "C2DMatrix.h"
#include "Commons.h"

#include "GameScreenLevel1.h"

using namespace::std;

//--------------------------------------------------------------------------------------------------

BaseTank::BaseTank(SDL_Renderer* renderer, TankSetupDetails details)
	: GameObject(renderer, GAMEOBJECT_TANK, details.StartPosition, details.TankImagePath)
{
	mManSpritesheet = new Texture2D(renderer);
	mManSpritesheet->LoadFromFile(details.ManImagePath);
	mManSingleSpriteWidth = mManSpritesheet->GetWidth();
	mManSingleSpriteHeight = mManSpritesheet->GetHeight() / kNumberOfSpritesPerMan;
	mManOffset.x = (mTexture->GetWidth()*0.5f) - (mManSingleSpriteWidth*0.5f);
	mManOffset.y = (mTexture->GetHeight()*0.5f) - (mManSingleSpriteHeight*0.5f);

	mManRotationAngle = 0.0f;
	mManFireDirection = Vector2D(0.0f, -1.0f);

	mExplosionSpritesheet = new Texture2D(renderer);
	mExplosionSpritesheet->LoadFromFile(kExplosionImagePath);
	mExplosionSingleSpriteWidth = mExplosionSpritesheet->GetWidth();
	mExplosionSingleSpriteHeight = mExplosionSpritesheet->GetHeight() / kNumberOfSpritesPerExplosion;
	mExploding = false;
	mExplosionTime = 0.0f;
	mExplosionOffset.x = -(mExplosionSingleSpriteWidth*0.5f);
	mExplosionOffset.y = -(mExplosionSingleSpriteHeight*0.5f);

	mBulletDelay = kTimeBetweenBullets;
	mRocketDelay = kTimeBetweenRockets;
	mMineDelay = kTimeBetweenMines;

	//Set starting state.
	ChangeState(TANKSTATE_IDLE);

	//Tank details.
	mTankType = (TANK_TYPE)details.TankType;
	mHealth = details.Health;
	mCurrentSpeed = 0.0f;
	mVelocity.Zero();
	mHeading = Vector2D(0.0f, -1.0f);
	mSide = Vector2D(1.0f, 0.0f);
	mMass = details.Mass;
	mMaxSpeed = details.MaxSpeed;
	mFuel = details.Fuel;

	//TODO: Read these details in from xml.
	mMaxForce = 10.0f;
	mMaxTurnRate = details.TurnRate / 500.0f;

	mRockets = details.NumOfRockets;
	mBullets = details.NumOfBullets;
	mMines = details.NumOfMines;
	mCannonAttachedLeft = details.LeftCannonAttached;
	mCannonAttachedRight = details.RightCannonAttached;

	if (mCannonAttachedLeft || mCannonAttachedRight)
	{
		mCannonSpritesheet = new Texture2D(renderer);
		mCannonSpritesheet->LoadFromFile(kCannonPath);
		mCannonSingleSpriteWidth = mCannonSpritesheet->GetWidth();
		mCannonSingleSpriteHeight = mCannonSpritesheet->GetHeight() / kNumberOfSpritesForCannon;
		mFiringRocket = false;
	}

	mScore = 0;
	mNumberOfBulletHits = 0;
	mNumberOfRocketHits = 0;
	mNumberOfMineHits = 0;
	mNumberOfDestroyedTanks = 0;
	mNumberOfSurvivalBonus = 0;
	mNumberOfPickUps = 0;

	mStudentName = details.StudentName;
	mAlive = true;

	//Set up audio / noise image sizes.
	mGunfireNoiseAffect = 0.0f;
	mNoiseDelay = 0.0f;
	switch (mTankType)
	{
		case TANK_SMALL:
			mNoiseRadius = kAudioSmallRadius;
			mHearingRadius = kAudioLargeRadius;
			SetCollisionRadius(kCollisionSmallRadius);
		break;

		case TANK_MEDIUM:
			mNoiseRadius = kAudioMediumRadius;
			mHearingRadius = kAudioMediumRadius2;
			SetCollisionRadius(kCollisionMediumRadius);
		break;

		case TANK_LARGE:
			mNoiseRadius = kAudioLargeRadius;
			mHearingRadius = kAudioSmallRadius;
			SetCollisionRadius(kCollisionLargeRadius);
		break;
	}

	//Set up the collision map.
	for (int i = 0; i < kMapWidth; i++)
	{
		for (int j = 0; j < kMapHeight; j++)
		{
			mCollisionMap[i][j] = details.CollisionMap[i][j];
		}
	}
}

//--------------------------------------------------------------------------------------------------

BaseTank::~BaseTank()
{
	mAlive = false;
	OutputScoreDetails();

	mRenderer = NULL;

	delete mManSpritesheet;
	mManSpritesheet = NULL;

	delete mExplosionSpritesheet;
	mExplosionSpritesheet = NULL;

	if (mCannonSpritesheet != NULL)
	{
		delete mCannonSpritesheet;
		mCannonSpritesheet = NULL;
	}

	mTanksICanSee.clear();
}

//--------------------------------------------------------------------------------------------------

void BaseTank::OutputScoreDetails()
{
	//Output to console.
	cout << "-------------------------------------------------------------" << endl;
	cout << "Tank name: " << mStudentName << "   SCORE: " << mScore << endl;
	cout << "Bullets hit: " << mNumberOfBulletHits << "\t\t= " << mNumberOfBulletHits * kScore_BulletHit << endl;
	cout << "Rockets hit: " << mNumberOfRocketHits << "\t\t= " << mNumberOfRocketHits * kScore_RocketHit << endl;
	cout << "Mines hit: " << mNumberOfMineHits << "\t\t= " << mNumberOfMineHits * kScore_MineHit << endl;
	cout << "Tanks destroyed: " << mNumberOfDestroyedTanks << "\t= " << mNumberOfDestroyedTanks * kScore_DestroyedTank << endl;
	cout << "Crates collected: " << mNumberOfPickUps << "\t= " << mNumberOfPickUps * kScore_PickUp << endl;
	cout << "Survival bonus': " << mNumberOfSurvivalBonus << "\t= " << mNumberOfSurvivalBonus * kScore_SurvivalBonus << endl;
	cout << "-------------------------------------------------------------" << endl;

	//Output to file - Append data if the file already exists.
	ofstream scorefile;
	scorefile.open(kScorePath, ios::out | ios::app);
	scorefile << "-------------------------------------------------------------" << endl;
	scorefile << "Tank name: " << mStudentName << "   SCORE: " << mScore << endl;
	scorefile << "Bullets hit: " << mNumberOfBulletHits << "\t\t= " << mNumberOfBulletHits * kScore_BulletHit << endl;
	scorefile << "Rockets hit: " << mNumberOfRocketHits << "\t\t= " << mNumberOfRocketHits * kScore_RocketHit << endl;
	scorefile << "Mines hit: " << mNumberOfMineHits << "\t\t= " << mNumberOfMineHits * kScore_MineHit << endl;
	scorefile << "Tanks destroyed: " << mNumberOfDestroyedTanks << "\t= " << mNumberOfDestroyedTanks * kScore_DestroyedTank << endl;
	scorefile << "Crates collected: " << mNumberOfPickUps << "\t= " << mNumberOfPickUps * kScore_PickUp << endl;
	scorefile << "Survival bonus': " << mNumberOfSurvivalBonus << "\t= " << mNumberOfSurvivalBonus * kScore_SurvivalBonus << endl;
	scorefile << "-------------------------------------------------------------" << endl;
	scorefile.close();
}

//--------------------------------------------------------------------------------------------------

void BaseTank::AddToScore(SCORE_TYPE scoreType)
{
	switch (scoreType)
	{
	case SCORE_BULLETHIT:
		mScore += kScore_BulletHit;
		mNumberOfBulletHits++;
		break;

	case SCORE_ROCKETHIT:
		mScore += kScore_RocketHit;
		mNumberOfRocketHits++;
		break;

	case SCORE_MINEHIT:
		mScore += kScore_MineHit;
		mNumberOfMineHits++;
		break;

	case SCORE_SURVIVAL:
		mScore += kScore_SurvivalBonus;
		mNumberOfSurvivalBonus++;
		break;

	case SCORE_DESTROYEDTANK:
		mScore += kScore_DestroyedTank;
		mNumberOfDestroyedTanks++;
		break;

	case SCORE_COLLECTEDPICKUP:
		mNumberOfPickUps++;
		mScore += kScore_PickUp;
		break;
	}

	//Cap score to 20 marks available.
	if (mScore > kMaxScore)
		mScore = kMaxScore;
}

//--------------------------------------------------------------------------------------------------

void BaseTank::Update(float deltaTime, SDL_Event e)
{
	if (!mAlive)
	{
		//Increment the time the current sprite has been displayed.
		mExplosionTime += deltaTime;

		//If the requisite time has passed, increment the frame number.
		if (mExplosionTime > kExplosionDelay)
		{
			mExplosionFrame++;
			mExplosionTime = 0.0f;
		}
	}
	else
	{
		//Reset noise from firing a weapon after a delay.
		mNoiseDelay -= deltaTime;
		if (mNoiseDelay <= 0.0f)
			mGunfireNoiseAffect = 0.0f;

		mBulletDelay -= deltaTime;
		mRocketDelay -= deltaTime;
		mMineDelay -= deltaTime;

		//Be sure you call this function from your child class.
		switch (mCurrentState)
		{
		case TANKSTATE_IDLE:
			break;

		case TANKSTATE_DEAD:
			break;

		case TANKSTATE_MANFIRE:
			//Increment the time the current sprite has been displayed.
			mManFireTime += deltaTime;

			//If the requisite time has passed, flip the flag.
			if (mManFireTime > kBulletFireDelay)
			{
				mManFireFrame = !mManFireFrame;
				mManFireTime = 0.0f;

				//Fire another bullet.
				FireABullet();

				//We need to ensure the tank can fire.
				if (mBullets <= 0.0f)
					ChangeState(TANKSTATE_IDLE);
			}
			break;

		case TANKSTATE_CANNONFIRE:
			//Increment the time the current sprite has been displayed.
			mCannonFireTime += deltaTime;

			//If the requisite time has passed, flip the flag.
			if (mCannonFireTime > kCannonFireDelay)
			{
				mCannonFireFrame = !mCannonFireFrame;

				//We need to ensure the tank can fire.
				if (mRockets <= 0 || mCannonFireTime >= (kCannonFireDelay * 2))
				{
					mFiringRocket = false;
					ChangeState(TANKSTATE_IDLE);
				}
			}
			break;

		case TANKSTATE_DROPMINE:
			ChangeState(TANKSTATE_IDLE);
			break;
		}

		mTanksICanSee = TankManager::Instance()->GetVisibleTanks(this);
		mTanksICanHear = TankManager::Instance()->GetAudibleTanks(this);

		//Deduct fuel each frame.
		if (mFuel > 0.0f)
		{
			mFuel -= deltaTime;
			MoveInHeadingDirection(deltaTime);
		}
		else
		{
			//Destroy tanks without fuel.
			TakeDamage(GAMEOBJECT_MINE);
		}
	}

	UpdateAdjustedBoundingBox();
}

//--------------------------------------------------------------------------------------------------

void BaseTank::Render()
{
	if (mAlive == true)
	{
		//Draw the noise radius.
#ifdef AUDIO_VISIBLE
		DrawDebugCircle(GetCentralPosition(), GetNoiseRadius(), 0, 255, 0);
#endif
		//Call parent render function.
		GameObject::Render();

		//Draw the left cannon.
		if (mCannonAttachedLeft)
		{
			SDL_Rect destRect = { (int)mPosition.x, (int)mPosition.y, mCannonSingleSpriteWidth, mCannonSingleSpriteHeight };
			mCannonSpritesheet->Render(GetCurrentCannonSprite(), destRect, mRotationAngle);
		}

		//Draw the right cannon.
		if (mCannonAttachedRight)
		{
			SDL_Rect destRect = { (int)mPosition.x, (int)mPosition.y, mCannonSingleSpriteWidth, mCannonSingleSpriteHeight };
			mCannonSpritesheet->Render(GetCurrentCannonSprite(), destRect, SDL_FLIP_HORIZONTAL, mRotationAngle);
		}

		//Draw the hearing radius.
#ifdef AUDIO_VISIBLE
		DrawDebugCircle(GetCentralPosition(), GetHearingRadius(), 255, 255, 0);
#endif

		//Draw the man image.
		SDL_Rect destRect = { (int)(mPosition.x + mManOffset.x), (int)(mPosition.y + mManOffset.y), mManSingleSpriteWidth, mManSingleSpriteHeight };
		mManSpritesheet->Render(GetCurrentManSprite(), destRect, mManRotationAngle);

		//Draw the field of view lines.
		DrawFoV();
	}
	else
	{
		//Draw the explosion image.
		SDL_Rect destRect = { (int)(GetCentralPosition().x + mExplosionOffset.x), (int)(GetCentralPosition().y + mExplosionOffset.y), mExplosionSingleSpriteWidth, mExplosionSingleSpriteHeight };
		mExplosionSpritesheet->Render(GetCurrentExplosionSprite(), destRect, 0);
	}
}

//--------------------------------------------------------------------------------------------------

void BaseTank::ChangeState(BASE_TANK_STATE newState)
{
	if (mCurrentState != TANKSTATE_DEAD && newState != mCurrentState)
	{
		mCurrentState = newState;

		switch (mCurrentState)
		{
		case TANKSTATE_IDLE:
			break;

		case TANKSTATE_DEAD:
			break;

		case TANKSTATE_MANFIRE:
			//Cannot go in to fire state if there are no bullets.
			if (mBullets <= 0)
				mCurrentState = TANKSTATE_IDLE;
			else
			{
				//Fire a bullet.
				FireABullet();

				mManFireFrame = true;
				mManFireTime = 0.0f;
			}

			break;

		case TANKSTATE_CANNONFIRE:
			if (!mFiringRocket)
			{
				//Cannot go in to fire state if there are no Rockets.
				if (mRockets <= 0)
					mCurrentState = TANKSTATE_IDLE;
				else
				{
					//Fire Rockets.
					FireRockets();

					mCannonFireFrame = true;
					mCannonFireTime = 0.0f;
				}
			}
			break;

		case TANKSTATE_DROPMINE:
			DropAMine();
			break;
		}
	}
}

//--------------------------------------------------------------------------------------------------

SDL_Rect BaseTank::GetCurrentManSprite()
{
	//NOTE: Spritesheets are currently made of a single column of images.
	SDL_Rect portionOfSpritesheet = { 0, 0, mManSingleSpriteWidth, mManSingleSpriteHeight };

	switch (mCurrentState)
	{
	case TANKSTATE_IDLE:
		portionOfSpritesheet.y = 0;
		break;

	case TANKSTATE_MANFIRE:
		if (mManFireFrame)
			portionOfSpritesheet.y = 1 * mManSingleSpriteHeight;
		else
			portionOfSpritesheet.y = 2 * mManSingleSpriteHeight;
		break;
	}

	//DEBUG: See which frame is being output.
	//cout << portionOfSpritesheet.y << endl;
	return portionOfSpritesheet;
}

//--------------------------------------------------------------------------------------------------

SDL_Rect BaseTank::GetCurrentExplosionSprite()
{
	//NOTE: Spritesheets are currently made of a single column of images.
	SDL_Rect portionOfSpritesheet = { 0, 0, mExplosionSingleSpriteWidth, mExplosionSingleSpriteHeight };

	portionOfSpritesheet.y = mExplosionFrame * mExplosionSingleSpriteHeight;

	//DEBUG: See which frame is being output.
	//cout << portionOfSpritesheet.y << endl;
	return portionOfSpritesheet;
}

//--------------------------------------------------------------------------------------------------

SDL_Rect BaseTank::GetCurrentCannonSprite()
{
	//NOTE: Spritesheets are currently made of a single column of images.
	SDL_Rect portionOfSpritesheet = { 0, 0, mCannonSingleSpriteWidth, mCannonSingleSpriteHeight };

	switch (mCurrentState)
	{
	case TANKSTATE_IDLE:
	case TANKSTATE_MANFIRE:
		portionOfSpritesheet.y = 0;
		break;

	case TANKSTATE_CANNONFIRE:
		mManFireFrame = 0;
		if (mCannonFireFrame)
			portionOfSpritesheet.y = 1 * mCannonSingleSpriteHeight;
		else
			portionOfSpritesheet.y = 2 * mCannonSingleSpriteHeight;
		break;
	}

	//DEBUG: See which frame is being output.
	//cout << portionOfSpritesheet.y << endl;
	return portionOfSpritesheet;
}

//--------------------------------------------------------------------------------------------------

void BaseTank::MoveInHeadingDirection(float deltaTime)
{
	//cout << "BASETANK: Must override MoveInHeadingDirection." << endl;
}

//--------------------------------------------------------------------------------------------------

bool BaseTank::RotateHeadingToFacePosition(Vector2D target, float deltaTime)
{
	Vector2D toTarget = Vec2DNormalize(GetCentralPosition() - target);

	//Determine the angle between the heading vector and the target.
	double angle = acos(mHeading.Dot(toTarget));

	//Ensure angle does not become NaN and cause the tank to disappear.
	if (angle != angle)
		angle = 0.0f;

	//Return true if the player is facing the target.
	if (angle < 0.00001)
		return true;

	RotateHeadingByRadian(angle, mHeading.Sign(toTarget), deltaTime);

	return true;
}

//--------------------------------------------------------------------------------------------------

void BaseTank::RotateHeadingByRadian(double radian, int sign, float deltaTime)
{
	//Incorporate delta time.
	radian *= deltaTime;

	//Clamp the amount to turn to the max turn rate.
	if (radian > mMaxTurnRate)
		radian = mMaxTurnRate;
	else if (radian < -mMaxTurnRate)
		radian = -mMaxTurnRate;

	//IncrementTankRotationAngle(RadsToDegs(radian));
	mRotationAngle += RadsToDegs(radian)*sign;

	//Usee a rotation matrix to rotate the player's heading
	C2DMatrix RotationMatrix;

	//Calculate the direction of rotation.
	RotationMatrix.Rotate(radian * sign);
	//Get the new heading.
	RotationMatrix.TransformVector2Ds(mHeading);

	//Side vector must always be perpendicular to the heading.
	mSide = mHeading.Perp();
}

//--------------------------------------------------------------------------------------------------

void BaseTank::SetHeading(Vector2D newHeading)
{
	//Check if the new heading is unit length.
	assert((newHeading.LengthSq() - 1.0) < 0.00001);

	//Set the new heading.
	mHeading = newHeading;

	//Side vector must always be perpendicular to the heading.
	mSide = mHeading.Perp();
}

//--------------------------------------------------------------------------------------------------

void BaseTank::IncrementTankRotationAngle(double deg)
{
	mRotationAngle += deg;
	if (mRotationAngle > 360.0f)
		mRotationAngle = 0.0f;
	else if (mRotationAngle < 0.0f)
		mRotationAngle = 360.0f;
}

//--------------------------------------------------------------------------------------------------

void BaseTank::IncrementManRotationAngle(double deg)
{
	mManRotationAngle += deg;
	if (mManRotationAngle > 360.0f)
		mManRotationAngle -= 360.0f;
	else if (mManRotationAngle < 0.0f)
		mManRotationAngle += 360.0f;
}

//--------------------------------------------------------------------------------------------------

void BaseTank::RotateManByRadian(double radian, int sign, float deltaTime)
{
	//Incorporate delta time.
	radian *= deltaTime;

	//Clamp the amount to turn to the max turn rate.
	if (radian > mMaxTurnRate)
		radian = mMaxTurnRate;
	else if (radian < -mMaxTurnRate)
		radian = -mMaxTurnRate;
	IncrementManRotationAngle(RadsToDegs(radian*sign));

	//Usee a rotation matrix to rotate the player's heading
	C2DMatrix RotationMatrix;

	//Calculate the direction of rotation.
	RotationMatrix.Rotate(radian * sign);

	//Get the new fire direction.
	RotationMatrix.TransformVector2Ds(mManFireDirection);
}

//--------------------------------------------------------------------------------------------------

void BaseTank::FireABullet()
{
	if (mBullets > 0)
	{
		if (mBulletDelay <= 0.0f)
		{
			mBulletDelay = kTimeBetweenBullets;

			//Calculate the fire position.
			Vector2D firePos = Vector2D(mPosition.x + mTexture->GetWidth()*0.5f, mPosition.y + mTexture->GetHeight()*0.5f);
			firePos += mManFireDirection * mManSingleSpriteHeight*0.3f;

			//Set the projectile setup details.
			ProjectileSetupDetails details;
			details.Direction = mManFireDirection;
			details.GameObjectType = GAMEOBJECT_BULLET;
			details.ImagePath = kBulletPath;
			details.RotationAngle = mManRotationAngle;
			details.StartPosition = firePos;

			ProjectileManager::Instance()->CreateProjectile(mRenderer, details, this);

			mNoiseDelay = kTimeBetweenBullets + 0.1f;
			mGunfireNoiseAffect = 5.0f;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void BaseTank::FireRockets()
{
	if (mRocketDelay <= 0.0f)
	{
		mRocketDelay = kTimeBetweenRockets;

		mFiringRocket = true;

		//Get the direction of fire from the current heading.
		Vector2D fireDirection = mHeading;

		//Set the projectile setup details.
		ProjectileSetupDetails details;
		details.Direction = fireDirection;
		details.GameObjectType = GAMEOBJECT_ROCKET;
		details.ImagePath = kRocketPath;
		details.RotationAngle = mRotationAngle;

		//Fire left rocket.
		if ((mRockets > 0) && (mCannonAttachedLeft))
		{
			details.StartPosition = GetCentralPosition();
			details.StartPosition += fireDirection.Perp()*-14.0f;
			ProjectileManager::Instance()->CreateProjectile(mRenderer, details, this);

			mNoiseDelay = kTimeBetweenRockets + 0.1f;
			mGunfireNoiseAffect = 5.0f;
		}

		//Fire right rocket.
		if ((mRockets > 0) && (mCannonAttachedRight))
		{
			details.StartPosition = GetCentralPosition();
			details.StartPosition += fireDirection.Perp()*14.0f;
			ProjectileManager::Instance()->CreateProjectile(mRenderer, details, this);

			mNoiseDelay = kTimeBetweenRockets + 0.1f;
			mGunfireNoiseAffect = 5.0f;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void BaseTank::DropAMine()
{
	if (mMineDelay <= 0.0f)
	{
		mMineDelay = kTimeBetweenMines;

		//Set the projectile setup details.
		ProjectileSetupDetails details;
		details.Direction = Vector2D();
		details.GameObjectType = GAMEOBJECT_MINE;
		details.ImagePath = kMinePath;
		details.RotationAngle = 0.0f;
		details.StartPosition = GetCentralPosition();

		//Drop the mine.
		ProjectileManager::Instance()->CreateProjectile(mRenderer, details, this);
	}
}

//--------------------------------------------------------------------------------------------------

Vector2D BaseTank::GetPointAtFrontOfTank()
{
	//Find the front central point of the tank.
	Vector2D frontPos = GetCentralPosition();
	frontPos.x += mHeading.x*mManSingleSpriteHeight*0.3f;
	frontPos.y -= mHeading.y*mManSingleSpriteHeight*0.3f;

	//DEBUG: Where is the front of the tank?
	//cout << "FRONT: x = " << frontPos.x << " y = " << frontPos.y << endl;

	return frontPos;
}

//--------------------------------------------------------------------------------------------------

Vector2D BaseTank::GetPointAtRearOfTank()
{
	//Find the rear central point of the tank.
	Vector2D rearPos = GetCentralPosition();
	rearPos.x -= mHeading.x*mManSingleSpriteHeight*0.3f;
	rearPos.y += mHeading.y*mManSingleSpriteHeight*0.3f;

	//DEBUG: Where is the rear of the tank?
	//cout << "REAR: x = " << rearPos.x << " y = " << rearPos.y << endl;
	return rearPos;
}

//--------------------------------------------------------------------------------------------------

void BaseTank::TakeDamage(GAMEOBJECT_TYPE projectileType)
{
	//Different projectiles do different damage.
	switch (projectileType)
	{
	case GAMEOBJECT_BULLET:
		mHealth -= kBulletDamage;
		break;

	case GAMEOBJECT_ROCKET:
		mHealth -= kRocketDamage;
		break;

	case GAMEOBJECT_MINE:
		mHealth -= kMineDamage;
		break;

	case GAMEOBJECT_OBSTACLE_BORDER:
		mHealth -= kMineDamage;
		break;
	}
}

//--------------------------------------------------------------------------------------------------

//Use this function to check if this tank can see you.
bool BaseTank::CanSee(BaseTank* tank)
{
	if (mTanksICanSee.size() == 0)
		return false;
	else
	{
		//Loop through visible tanks to determine if I can be seen.
		for (unsigned int i = 0; i < mTanksICanSee.size(); i++)
		{
			if (mTanksICanSee[i] == tank)
				return true;
		}
	}

	//If we reach here we are not visible to this tank.
	return false;
}

//--------------------------------------------------------------------------------------------------

float BaseTank::GetHearingRadius()
{
	return mHearingRadius - mGunfireNoiseAffect;
}

//--------------------------------------------------------------------------------------------------

float BaseTank::GetNoiseRadius()
{
	return mNoiseRadius + mGunfireNoiseAffect;
}

//--------------------------------------------------------------------------------------------------

void BaseTank::DrawFoV()
{
#ifdef DEBUG_FOV 
	Vector2D polarVec(0.0f, kFieldOfViewLength);

	//Get the dot product of heading by RIGHT vector
	Vector2D normalisedHeading = Vec2DNormalize(mHeading);

	//Create point rotated to the left of heading.
	Vector2D leftPoint;
	leftPoint.x = (normalisedHeading.x * cos(kFieldOfView)) - (normalisedHeading.y * sin(kFieldOfView));
	leftPoint.y = (normalisedHeading.x * sin(kFieldOfView)) + (normalisedHeading.y * cos(kFieldOfView));

	//Create point rotated to the right of heading.
	Vector2D rightPoint;
	rightPoint.x = (normalisedHeading.x * cos(-kFieldOfView)) - (normalisedHeading.y * sin(-kFieldOfView));
	rightPoint.y = (normalisedHeading.x * sin(-kFieldOfView)) + (normalisedHeading.y * cos(-kFieldOfView));

	//Move the left point out from the centre of the tank to the distance set by kFieldOfViewLength.
	Vector2D m_viewFrustumLeft;
	m_viewFrustumLeft.x = GetCentralPosition().x + (leftPoint.x*kFieldOfViewLength);
	m_viewFrustumLeft.y = GetCentralPosition().y + (leftPoint.y*kFieldOfViewLength);

	//Move the right point out from the centre of the tank to the distance set by kFieldOfViewLength.
	Vector2D m_viewFrustumRight;
	m_viewFrustumRight.x = GetCentralPosition().x + (rightPoint.x*kFieldOfViewLength);
	m_viewFrustumRight.y = GetCentralPosition().y + (rightPoint.y*kFieldOfViewLength);

	//Draw the field of view frustum
	DrawDebugLine(GetCentralPosition(), m_viewFrustumRight, 255, 0, 0);
	DrawDebugLine(GetCentralPosition(), m_viewFrustumLeft, 255, 0, 0);

	//Draw a line in the direction the tank is heading (HEADING).
	DrawDebugLine(GetCentralPosition(), GetCentralPosition() + mHeading * kFieldOfViewLength, 255, 0, 0);

	//Draw a line in the direction the tank is moving (VELOCITY).
	DrawDebugLine(GetCentralPosition(), GetCentralPosition() + mVelocity, 255, 255, 0);

	DrawDebugLine(mAdjustedBoundingBox[0], mAdjustedBoundingBox[1], 225, 225, 225);
	DrawDebugLine(mAdjustedBoundingBox[1], mAdjustedBoundingBox[2], 225, 225, 225);
	DrawDebugLine(mAdjustedBoundingBox[2], mAdjustedBoundingBox[3], 225, 225, 225);
	DrawDebugLine(mAdjustedBoundingBox[3], mAdjustedBoundingBox[0], 225, 225, 225);
#endif
}

//--------------------------------------------------------------------------------------------------

void BaseTank::UpdateAdjustedBoundingBox()
{
	//Return texture size - Override for more precise collision.
	//This assumes there is only one image on the texture.
	Vector2D center = GetCentralPosition();
	double    width = 0.0f;
	double    height = 0.0f;

	switch (mTankType)
	{
		case TANK_SMALL:
			width = (mTexture->GetWidth() / 4.5f);
			height = (mTexture->GetWidth() / 4.0f);
		break;

		case TANK_MEDIUM:
			width = (mTexture->GetWidth() / 4.0f);
			height = (mTexture->GetWidth() / 3.3f);
		break;

		case TANK_LARGE:
			width = (mTexture->GetWidth() / 3.4);
			height = (mTexture->GetWidth() / 2.5);
		break;
	}

	double minX = (float)center.x - width;
	double minY = (float)center.y - height;

	double maxX = (float)center.x + width;
	double maxY = (float)center.y + height;

	double botLeftXRotated = (minX - center.x) * cos(mRotationAngle * M_PI / 180) - (minY - center.y) * sin(mRotationAngle * M_PI / 180);
	double botLeftYRotated = (minX - center.x) * sin(mRotationAngle * M_PI / 180) + (minY - center.y) * cos(mRotationAngle * M_PI / 180);

	double botRightXRotated = (maxX - center.x) * cos(mRotationAngle * M_PI / 180) - (minY - center.y) * sin(mRotationAngle * M_PI / 180);
	double botRightYRotated = (maxX - center.x) * sin(mRotationAngle * M_PI / 180) + (minY - center.y) * cos(mRotationAngle * M_PI / 180);

	double topRightXRotated = (maxX - center.x) * cos(mRotationAngle * M_PI / 180) - (maxY - center.y) * sin(mRotationAngle * M_PI / 180);
	double topRightYRotated = (maxX - center.x) * sin(mRotationAngle * M_PI / 180) + (maxY - center.y) * cos(mRotationAngle * M_PI / 180);

	double topLeftXRotated = (minX - center.x) * cos(mRotationAngle * M_PI / 180) - (maxY - center.y) * sin(mRotationAngle * M_PI / 180);
	double topLeftYRotated = (minX - center.x) * sin(mRotationAngle * M_PI / 180) + (maxY - center.y) * cos(mRotationAngle * M_PI / 180);

	Vector2D rotatedRect[4];
	rotatedRect[0] = Vector2D(botLeftXRotated, botLeftYRotated) + center; //Bottom Left
	rotatedRect[1] = Vector2D(botRightXRotated, botRightYRotated) + center; //Bottom Right
	rotatedRect[2] = Vector2D(topRightXRotated, topRightYRotated) + center; //Top Right
	rotatedRect[3] = Vector2D(topLeftXRotated, topLeftYRotated) + center; //Top Left

	mAdjustedBoundingBox[0] = rotatedRect[0];
	mAdjustedBoundingBox[1] = rotatedRect[1];
	mAdjustedBoundingBox[2] = rotatedRect[2];
	mAdjustedBoundingBox[3] = rotatedRect[3];
}

//--------------------------------------------------------------------------------------------------
