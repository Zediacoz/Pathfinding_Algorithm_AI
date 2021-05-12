//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <limits>

//Useful constants.
const int     MaxInt    = (std::numeric_limits<int>::max)();
const double  MaxDouble = (std::numeric_limits<double>::max)();
const double  MinDouble = (std::numeric_limits<double>::min)();
const float   MaxFloat  = (std::numeric_limits<float>::max)();
const float   MinFloat  = (std::numeric_limits<float>::min)();

//Angle manipulation.
const double   Pi							= 3.14159;
const double   TwoPi						= Pi * 2;
const double   HalfPi						= Pi / 2;
const double   QuarterPi					= Pi / 4;

inline double DegsToRads(double degrees) {return TwoPi * (degrees/360.0f);}
inline double RadsToDegs(double radians) {return radians * (180.0f/Pi);}

//Screen dimensions.
const int kScreenWidth						= 960;
const int kScreenHeight						= 640;

const int kTileDimensions					= 32;
const int kHalfTileDimensions				= kTileDimensions / 2;

const int kMapWidth							= kScreenWidth / kTileDimensions;
const int kMapHeight						= kScreenHeight / kTileDimensions;

//Spritesheet / sprite animations.
const int   kNumberOfSpritesPerMan			= 3;
const float kBulletFireDelay				= 0.1f;
const float kCannonFireDelay				= 0.25f;
const int	kNumberOfSpritesForCannon		= 3;
const int   kNumberOfSpritesPerExplosion	= 6;
const float kExplosionDelay					= 0.1f;

//Delays
const float kTimeBetweenBullets				= 0.5f;
const float kTimeBetweenRockets				= 1.0f;
const float kTimeBetweenMines				= 1.5f;
const float kTimeBetweenPickups				= 15.0f;

//Time
const float kSurvivalTimeUntilBonus			= 30.0f;

//Scores
const float kScore_BulletHit				= 2.0f;
const float kScore_RocketHit				= 1.0f;
const float kScore_MineHit					= 3.0f;
const float kScore_DestroyedTank			= 10.0f;
const float kScore_SurvivalBonus			= 5.0f;
const float kScore_PickUp					= 2.5f;
const float kMaxScore						= 20.0f;

//Weapon damage
const int		kBulletDamage				= 1;
const int		kRocketDamage				= 5;
const int		kMineDamage					= 10;

//Speeds
const float		kSpeedIncrement				= 1000.0f;
const float		kReboundSpeed				= -30.0f;
const float		kBulletSpeed				= 150.0f;
const float		kRocketSpeed				= 250.0f;

const float		kManTurnRate				= 2.0f;

//Tank details.
const float		kFieldOfView				= 0.85f;
const float		kFieldOfViewLength			= 100.0f;//250.0f;

const float		kAudioLargeRadius			= 60.0f;
const float		kAudioMediumRadius			= 47.0f;
const float		kAudioMediumRadius2			= 45.0f;
const float		kAudioSmallRadius			= 30.0f;

const double	kCollisionSmallRadius		= 15.0f;
const double	kCollisionMediumRadius		= 19.5f;
const double	kCollisionLargeRadius		= 22.0f;

const double	kCollisionBulletRadius		= 2.0f;
const double	kCollisionRocketRadius		= 6.0f;
const double	kCollisionMineRadius		= 10.0f;

//Pickup details.
const float		kSafeCrateDistance			= 25.0f;
const double	kCollisionCrateRadius		= 9.0f;

#endif //CONSTANTS_H