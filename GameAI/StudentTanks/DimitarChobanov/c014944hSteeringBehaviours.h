#ifndef C014944HSTEERINGBEHAVIOURS_H
#define C014944HSTEERINGBEHAVIOURS_H

#include "../../BaseTank.h"
#include <SDL.h>
#include "../../Commons.h"

//---------------------------------------------------------------

class c014944hSteeringBehaviours
{
	//---------------------------------------------------------------
public:
	c014944hSteeringBehaviours(BaseTank* tank);
	~c014944hSteeringBehaviours();

	bool AccumulateForce(Vector2D & RunningTot, Vector2D ForceToAdd);

	Vector2D Calculate();

	Vector2D CalculateForce(Vector2D m_vSteeringForce);

	Vector2D CalculateForce();

	Vector2D CombinedBehaviours(Vector2D target);
	Vector2D Seek(Vector2D target);
	Vector2D Flee(Vector2D target);
	Vector2D Arrive(Vector2D target, Deceleration deceleration);
	Vector2D Pursuit(BaseTank* targetTank);
	Vector2D CollisionAvoidance(Vector2D target);


	void SetSeek(bool trueFalse)						{ mSeekOn = trueFalse; }
	void SetFlee(bool trueFalse)                        { mFleeOn = trueFalse; }
	void SetArrive(bool trueFalse)                      { mArriveOn = trueFalse; }
	void SetPursuit(bool trueFalse)                     { mPursuitOn = trueFalse; }

	//---------------------------------------------------------------
private:
	BaseTank * myTank;
	bool mSeekOn;
	bool mFleeOn;
	bool mArriveOn;
	bool mPursuitOn;

};

//---------------------------------------------------------------

#endif //C014944HSTEERINGBEHAVIOURS_H