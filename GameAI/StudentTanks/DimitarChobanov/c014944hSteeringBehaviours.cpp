#include "c014944hSteeringBehaviours.h"
#include "../../BaseTank.h"


c014944hSteeringBehaviours::c014944hSteeringBehaviours(BaseTank* tank)
{
	myTank = tank;
	mSeekOn = false;
}

//-------------------------------------------------------------------------------------------

c014944hSteeringBehaviours::~c014944hSteeringBehaviours()
{

}

//-------------------------------------------------------------------------------------------

bool c014944hSteeringBehaviours::AccumulateForce(Vector2D &RunningTot,
	Vector2D ForceToAdd)
{

	//calculate how much steering force the vehicle has used so far
	double MagnitudeSoFar = RunningTot.Length();

	//calculate how much steering force remains to be used by this vehicle
	double MagnitudeRemaining = myTank->GetMaxForce() - MagnitudeSoFar;

	//return false if there is no more force left to use
	if (MagnitudeRemaining <= 0.0) return false;

	//calculate the magnitude of the force we want to add
	double MagnitudeToAdd = ForceToAdd.Length();

	//if the magnitude of the sum of ForceToAdd and the running total
	//does not exceed the maximum force available to this vehicle, just
	//add together. Otherwise add as much of the ForceToAdd vector is
	//possible without going over the max.
	if (MagnitudeToAdd < MagnitudeRemaining)
	{
		RunningTot += ForceToAdd;
	}

	else
	{
		//add it to the steering force
		RunningTot += (Vec2DNormalize(ForceToAdd) * MagnitudeRemaining);
	}

	return true;
}

//-------------------------------------------------------------------------------------------


Vector2D c014944hSteeringBehaviours::CombinedBehaviours(Vector2D target)
{
	if (mSeekOn)
		return Seek(target) ;
	if (mArriveOn)
		return Arrive(target, slow);
	if (mFleeOn)
		return Flee(target);

	return Vector2D();

}

//-------------------------------------------------------------------------------------------

Vector2D c014944hSteeringBehaviours::Seek(Vector2D target)
{
	Vector2D desired = Vec2DNormalize(target - myTank->GetCentralPosition()) * myTank->GetMaxSpeed();
	return desired - myTank->GetVelocity();
}

//-------------------------------------------------------------------------------------------
Vector2D c014944hSteeringBehaviours::Flee(Vector2D target)
{
	
	Vector2D desired = Vec2DNormalize(myTank->GetCentralPosition() - target) * myTank->GetMaxSpeed();
	return desired - myTank->GetVelocity();


}
//-------------------------------------------------------------------------------------------
Vector2D c014944hSteeringBehaviours::Arrive(Vector2D target, Deceleration deceleration)
{
	Vector2D ToTarget = target - myTank->GetCentralPosition();

	//calculate the distance to the target
	double dist = ToTarget.Length();

	if (dist > 0)
	{
		//because Deceleration is enumerated as an int, this value is required
		//to provide fine tweaking of the deceleration..
		const double DecelerationTweaker = 0.95;

		//calculate the speed required to reach the target given the desired
		//deceleration
		double speed = dist / ((double)deceleration * DecelerationTweaker);

		//make sure the velocity does not exceed the max
		speed = min(speed, myTank->GetMaxSpeed());

		//from here proceed just like Seek except we don't need to normalize 
		//the ToTarget vector because we have already gone to the trouble
		//of calculating its length: dist. 
		Vector2D DesiredVelocity = ToTarget * speed / dist;

		return (DesiredVelocity - myTank->GetVelocity());
	}

	return Vector2D(0, 0);
}
////-------------------------------------------------------------------------------------------
//Vector2D c014944hSteeringBehaviours::Pursuit(BaseTank* targetTank)
//{
//
//
//
//
//}
////-------------------------------------------------------------------------------------------
//Vector2D c014944hSteeringBehaviours::CollisionAvoidance(Vector2D target)
//{
//
//
//
//
//}
//-------------------------------------------------------------------------------------------
