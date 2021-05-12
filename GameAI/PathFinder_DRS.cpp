//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "PathFinder_DRS.h"
#include "Commons.h"
#include <SDL.h>
#include <time.h>
#include <iomanip>

//--------------------------------------------------------------------------------------------------

PathFinder_DRS::PathFinder_DRS(SDL_Renderer* renderer, TileType map[kMapWidth][kMapHeight])
{
	srand((unsigned int)time(NULL));

	mRenderer			= renderer;
	
	//Set up the collision map.
	for (int x = 0; x < kMapWidth; x++)
	{
		for (int y = 0; y < kMapHeight; y++)
		{
			mMap[x][y] = map[x][y];
		}
	}
	
	ResetCostArray();

	mSearchLength		= kTileDimensions*1.1f;
	mRefineLength		= kTileDimensions*0.9f;
	mLengthOfSearchSq	= (float)(kTileDimensions*kTileDimensions) + (kTileDimensions*kTileDimensions);
	mRotationInDegrees  = 25.0f;
	mStepSize			= DegsToRads(mRotationInDegrees);	//step size is the radians to rotate for each explosion.
	mRadian180			= DegsToRads(180.0);
}

//--------------------------------------------------------------------------------------------------

PathFinder_DRS::~PathFinder_DRS()
{
}

//--------------------------------------------------------------------------------------------------

bool PathFinder_DRS::FindPath(const Vector2D startPosition, const Vector2D endPosition)
{
	//Ensure the data is clear before we begin.
	mPathData.clear();
	ResetCollisionMap();
	ResetCostArray();

	//Create a vector to store the search data throughout the DRS process.
	vector<Vector2D>* pathSearch = new vector<Vector2D>;

	//----------------------------------------------------------------------
	//Call the recursive SEARCH function.
	//----------------------------------------------------------------------
	bool success = Search(startPosition, endPosition, true, pathSearch);


	//If the path is not empty, we have found a path. Therefore we need to add the starting position to this path.
	if (!pathSearch->empty())
	{
		pathSearch->push_back(startPosition);

		//For debugging purposes, lets have a look at the costs.
		OutputCosts(startPosition, endPosition);


		//----------------------------------------------------------------------
		//Now ENHANCE the path.
		//----------------------------------------------------------------------
		pathSearch = EnhancePath(pathSearch);
		OutputCosts(startPosition, endPosition);
		
		//----------------------------------------------------------------------
		//Now REFINE the enhanced path.
		//----------------------------------------------------------------------
		mPathData = RefinePath(pathSearch);
		OutputCosts(startPosition, endPosition);
	}

	//Clean up.
	delete pathSearch;
	pathSearch = NULL;

	return success;
}

//--------------------------------------------------------------------------------------------------

//We should check if this cell is valid before expanding it.
bool PathFinder_DRS::IsCellValid(Vector2D currentPosition)
{
	int X = (int)(currentPosition.x / kTileDimensions);
	int Y = (int)(currentPosition.y / kTileDimensions);

	//Did we hit an obstacle? If so, get out of here [INVALID position]
	if (mMap[X][Y] == TILETYPE_BLOCKED)
		return false;

	//Did we already check here? If so, get out of here
	if (mMap[X][Y] == TILETYPE_EMPTY_CHECKED)
	{
		//Incur a cost for rechecking.
		mMapCosts[X][Y] += 0.5f;
		return false;
	}
	else //Mark this position as already being looked at.
		mMap[X][Y] = TILETYPE_EMPTY_CHECKED;

	//Increment the cost for this grid place.
	mMapCosts[X][Y]++;

	return true;
}

//--------------------------------------------------------------------------------------------------
bool PathFinder_DRS::Search(Vector2D currentPosition, Vector2D targetPosition, bool orderOfRotation, vector<Vector2D>* path)
{
	//Check if the cell at this position is valid, if not then return false.
	if (!IsCellValid(currentPosition))
		return false;
	
	//Centre current position on the grid cell.
	currentPosition.x = ((int)(currentPosition.x / kTileDimensions)*kTileDimensions) + kHalfTileDimensions;
	currentPosition.y = ((int)(currentPosition.y / kTileDimensions)*kTileDimensions) + kHalfTileDimensions;

	//Calculate a search vector from the current position to the target position.
	Vector2D searchVector = targetPosition - currentPosition;

	//Check if we have reached the target. Do this by checking if the length of the above vector is less that mLengthOfSearchSq
	//Also, check if the move is valid - Use the IsMoveAllowed() function.
	//if()
	if(searchVector.LengthSq() < mLengthOfSearchSq &&  IsMoveAllowed(currentPosition, targetPosition))
	{
		//Add the target position and the current position to path. Hint: Use push_back() vector function.
		path->push_back(currentPosition);
		path->push_back(targetPosition);
		//Find the maximum cost we will allow. Hint: There is a prewritten function for this.
		SetMaxCost();

		//Return true as we found the target.
		return true;
	}

	//If we reach here, we have not found target and not hit an obstacle so let's move mSearchLength ahead of current position in direction of endPosition
	//Normalize the search vector to create a unit vector.
	Vector2D unitVecToTarget = searchVector;// = ??
	unitVecToTarget.Normalize();
	//Create a vector to the target based upon the unit vector multiplied by the search length.
	Vector2D vecToTarget = unitVecToTarget * mSearchLength; // = ??

	//Use the mMovedPos variable to store the new position. Hint: The new position will be where we currently are plus the search vector.
	mMovedPos = currentPosition + vecToTarget;
	//Check that this move is within the bounds of the map, and that the move is allowed. Hint: Look for the supplied functions.
	//if()
	if(IsPositionInMapBounds(mMovedPos) && IsMoveAllowed(currentPosition, mMovedPos))
	{
		//We now know we are still on the grid - Can we move toward the target??
		//Call the Search() function again using the new position. Search will return true / false depending upon its success.
		if(Search(mMovedPos, targetPosition, orderOfRotation, path))
		{
			//Path found, filtering back up, but we dont want every position visited. 
			//Set current position as part of the path if the mMapCost at this position is <= mMaxCost.
			if(mMapCosts[(int)mMovedPos.x][(int)mMovedPos.y] <= mMaxCost)
			{		
				//Add currentPosition to path.
				path->push_back(currentPosition);
			}

			//Return true. This is how a recursive function unwinds.
			return true;
		}
	}

	//---------------------------------------------------------------
	//If we reach this stage, ie, we haven't returned true (indicating a path was found) or false (indicating the position was invalid) then
	//try rotating around the point to find a new position to try.
	
	//Set the start vectors to be the length of the search.
	Vector2D polarVecPos(0.0f, mSearchLength);

	//Calculate the rotation required to start the search from the direction to the target.
	float dotRight		 = (float)unitVecToTarget.Dot(Vector2D(1.0f,0.0f)); //Angle starts from right - how many radians from right vector is targte vector?
	float dotUp			 = (float)unitVecToTarget.Dot(Vector2D(0.0f,1.0f)); //dot product on up vector to enable positive / negative rotation.
	float startRadians	 = acos(dotRight);
	
	//And then rotate by the start radians - Is it positive or negative rotation?
	if(dotUp >= 0.0f)
		polarVecPos.x += startRadians;
	else
		polarVecPos.x -= startRadians;

	//Both positive and negative start off facing the same direction.
	Vector2D polarVecNeg = polarVecPos;

	bool rotationComplete	= false;
	bool posComplete		= false;
	bool negComplete		= false;

	while (!rotationComplete)
	{
		if(orderOfRotation)
		{
			//Increment the x component by required radians.
			polarVecPos.x += mStepSize;

			//---------------------------------------------------------------------
			//Check that we have completed the positive rotation, and that we havent gone beyond 180 degrees.
			if( !posComplete && polarVecPos.x < mRadian180+startRadians)
			{
				//---------------------------------------------------------------------
				//Calculate new positive position.
				mPolarAsCart = Vector2D(polarVecPos.y * cosf((float)polarVecPos.x), polarVecPos.y * sinf((float)polarVecPos.x));
				
				//Set mPositionToTry to equal the current position plus mPolAsCart vector.
				mPositionToTry = currentPosition + mPolarAsCart;

				//Check that this move is within the bounds of the map, and that the move is allowed.
				if(IsPositionInMapBounds(mPositionToTry)&&IsMoveAllowed(currentPosition, mPositionToTry))
				{
					//We now know we are still on the grid - Can we move toward the target??
					//Call the Search() function again using the new position. Search will return true / false depending upon its success.
					if(Search(mPositionToTry, targetPosition, orderOfRotation, path))
					{
						//Path found, filtering back up, but we dont want every position visited. 
						//Set current position as part of the path if the mMapCost at this position is <= mMaxCost.
						if (mMapCosts[(int)mPositionToTry.x][(int)mPositionToTry.y] <= mMaxCost)
						{
							//Add currentPosition to path.
							path->push_back(currentPosition);
						}

						//Return true. This is how a recursive function unwinds.
						return true;
					}
				}
			}
			else
			{
				//Set positive rotation as complete / true.
				posComplete = true;
			}
	
			//Flip to enable negative rotation next.
			orderOfRotation = !orderOfRotation;
		}

		if(!orderOfRotation)
		{
			//Decrement the x component by required radians.
			polarVecNeg.x -= mStepSize;

			//---------------------------------------------------------------------
			//Calculate new negative position.
			mPolarAsCart = Vector2D(polarVecNeg.y * cosf((float)polarVecNeg.x), polarVecNeg.y * sinf((float)polarVecNeg.x));

			//Set mPositionToTry to equal the current position plus mPolAsCart vector.
			mPositionToTry = currentPosition + mPolarAsCart;

			//Check that we have completed the negative rotation, and that we havent gone beyond 180 degrees.
			if( !negComplete && polarVecNeg.x > -mRadian180-startRadians)
			{
				//Check that this move is within the bounds of the map, and that the move is allowed.
				if (IsPositionInMapBounds(mPositionToTry) && IsMoveAllowed(currentPosition, mPositionToTry))
				{
					//We now know we are still on the grid - Can we move toward the target??
					//Call the Search() function again using the new position. Search will return true / false depending upon its success.
					if(Search(mPositionToTry, targetPosition, orderOfRotation, path))
					{
						//Path found, filtering back up, but we dont want every position visited. 
						//Set current position as part of the path if the mMapCost at this position is <= mMaxCost.
						if (mMapCosts[(int)currentPosition.x][(int)currentPosition.y] <= mMaxCost)
						{
							//Add currentPosition to path.
							path->push_back(currentPosition);
						}

						//Return true. This is how a recursive function unwinds.
						return true;
					}
				}
			}
			else
			{
				//Set negative rotation as complete / true.
				negComplete = true;
			}

			//Flip to enable positive rotation next.
			orderOfRotation = !orderOfRotation;

		}

		//Are both sets of rotation complete?
		rotationComplete = posComplete && negComplete;
	}
	
	//No good so return false.
	return false;
}

//--------------------------------------------------------------------------------------------------

vector<Vector2D>* PathFinder_DRS::EnhancePath(vector<Vector2D>* tempPathData)
{
	//Clear the collision map and costs, as we are going to research the environment and the values stored from the
	//previous search will still be held. These old values will mess up your new searches.
	//ResetCollisionMap();
	ResetCostArrayAndCollisionMap();

	//Create a vector to hold the enhanced path.
	vector<Vector2D>* pathDataEnhanced = new vector<Vector2D>;

	int index = 0;
	while (index < tempPathData->size() - 1) {
		Vector2D a = tempPathData->operator[](index);
		Vector2D b = tempPathData->operator[](index + 1);
		if(Search(a, b, true, tempPathData)){
			pathDataEnhanced->push_back(a);
			pathDataEnhanced->push_back(b);
		}
		index++;
	}


	/*if (tempPathData) {
		for (unsigned int i = 0; i < tempPathData->size() - 2; ++i) {
			if (Search(tempPathData->operator[](i), tempPathData->operator[](i+1), true, tempPathData)) {
				pathDataEnhanced->push_back(tempPathData->operator[](i));
			}

			//If not empty, Loop through path, calling Search() function for each pair of positions.
			//For example, path tempPathData holds - A,B,C,D,E
			//			   Call Search(A,B), then Search(B,C), etc.
			//
			//Do this in a loop, so that the result is tempPathData, but with the additional positions intersected where they need to be.
			//Hint: You may need another vector to hold the individual searches in.
		}
	}*/
	SetMaxCost();
	//Can't delete pathDataEnhanced.
	return pathDataEnhanced;
}

//--------------------------------------------------------------------------------------------------

vector<Vector2D> PathFinder_DRS::RefinePath(vector<Vector2D>* pathDataEnhanced)
{
	vector<Vector2D> pathDataRefined;
	
	//Get either end of pathDataEnhanced, and check if they can see each other without colliding with an obstacle.
	//If not, then move on to the neighbour of the left position and try again.
	//If so, remove all positions in the list between these two positions.
	//
	//For example, pathDataEnhanced contains A,B,C,D,E.
	//			   Check if E can see A, if not, repeat the process with E and B.
	//			   Check if E can see B, if not, repeat the process with E and C, etc
	//
	//			   Lets say E can see C, we then need to remove D from the list.
	//
    //	 		   pathDataEnhanced now contains A,B,C,E.
	//			   Now repeat the process with the neighbour of E (This is now C, as we removed D).
	//			   Continue until the left and right ends of pathDataEnhanced can see each other. (At worst it will be A and B).
	bool collided = false;
	int endIndex = pathDataEnhanced->size() - 1;
	int startIndex = 0;
	float fineTuneSearch = 0.5f;
	pathDataRefined.push_back(pathDataEnhanced->operator[](startIndex));
	while (pathDataEnhanced->size() > 1) {
		collided = false;

		Vector2D a = pathDataEnhanced->operator[](endIndex); // start
		Vector2D b = pathDataEnhanced->operator[](startIndex); // end
		Vector2D searchVector = b - a;

		if (a == b) {
			cout << "fail" << endl;
			break;
		}

		if (searchVector.Length() > 0) {
			Vector2D normalizedSearch = searchVector;
			normalizedSearch.Normalize();
			Vector2D v = normalizedSearch * mSearchLength;

			while (v.Length() < searchVector.Length()) {
				if (IsMoveAllowed(b, a+v))
					v += normalizedSearch * mSearchLength * fineTuneSearch;
				else {
					collided = true;
					break;
				}
			}
			if (collided) {
				endIndex = endIndex - 1;
				startIndex = 0;
			}
			else {
				pathDataRefined.push_back(a);
				//pathDataRefined.push_back(b);

				pathDataEnhanced->erase(pathDataEnhanced->begin(), pathDataEnhanced->begin() + endIndex);
				endIndex = pathDataEnhanced->size() - 1;


			}
		}

	}
	
	/*
	for (int i = pathDataEnhanced->size() -1; i >=0 ; i--) {
		Vector2D firstVector = pathDataEnhanced->operator[](0);
		int index = 0;
		bool crash = false;

		if (Search(firstVector, pathDataEnhanced->operator[](i), true, pathDataEnhanced)) {
			break;
		}
	
		while (!Search(firstVector, pathDataEnhanced->operator[](i), true, pathDataEnhanced)) {
			index++;
			if(index < pathDataEnhanced->size())
			firstVector = pathDataEnhanced->operator[](index);
			else {
				crash = true;
				break;
			}
		}
		if (!crash && pathDataEnhanced->operator[](index+1) != pathDataEnhanced->operator[](i)) {
			pathDataEnhanced->erase(pathDataEnhanced->begin() + index + 1);
		}
	}
	*/

	//Return the new refined path.
	
	return pathDataRefined;
}

//--------------------------------------------------------------------------------------------------

void PathFinder_DRS::SetMaxCost()
{
	//Find the maximum cost we will allow - It will be the average value  of scored cells.
	float totalCost = 0.0f;
	int numOfCosts = 0;
	for(int x = 0; x < kMapWidth; x++)
	{
		for(int y = 0; y < kMapHeight; y++)
		{
			if(mMapCosts[x][y] > 0)
			{
				numOfCosts++;
				totalCost += mMapCosts[x][y];
			}
		}
	}

	mMaxCost = (totalCost/numOfCosts);
}

//--------------------------------------------------------------------------------------------------

bool PathFinder_DRS::IsPositionInMapBounds(Vector2D position)
{
	return (position.x < kScreenWidth &&
			position.x > 0.0f && 
			position.y < kScreenHeight &&
			position.y > 0.0f );
}

//--------------------------------------------------------------------------------------------------

bool PathFinder_DRS::IsMoveAllowed(Vector2D fromPos, Vector2D toPos)
{
	//If the desired cell is blocked, just leave.
	int toX = (int)(toPos.x / kTileDimensions);
	int toY = (int)(toPos.y / kTileDimensions);
	if (mMap[toX][toY] == TILETYPE_BLOCKED)
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------

void PathFinder_DRS::OutputCosts(Vector2D startPosition, Vector2D endPosition)
{
	std::cout << "---MAX Cost = " << mMaxCost << endl;

	//Debug output to show the current costs of the board - These were cleared in the Enhance functions
	for(int y = 0; y < kScreenHeight/kTileDimensions; y++)
	{
		for(int x = 0; x < kScreenWidth/kTileDimensions; x++)
		{
				if( x == (int)(startPosition.x/kTileDimensions) &&
					y == (int)(startPosition.y/kTileDimensions) )
				{
					std::cout << " S," << std::setw(2);
				}
				else if( x == (int)(endPosition.x/kTileDimensions) &&
						y == (int)(endPosition.y/kTileDimensions) )
				{
					std::cout << " T," << std::setw(2);
				}
				else
				cout << std::setprecision(1) << mMapCosts[x][y] << "," << std::setw(3);
					//std::cout << std::setprecision(2) << mMapCosts[x][y] << "," << std::setw(2);
		}
		std::cout << endl;
	}
	std::cout << endl << endl;
}

//--------------------------------------------------------------------------------------------------

void PathFinder_DRS::ResetCostArray()
{
	//Set all values to zero for to ensure a fresh search.
	for(int x = 0; x < kMapWidth; x++)
		for(int y = 0; y < kMapHeight; y++)
			mMapCosts[x][y] = 0.0f;
}

//--------------------------------------------------------------------------------------------------

void PathFinder_DRS::ResetCollisionMap()
{
	for(int x = 0; x < kMapWidth; x++)
	{
		for(int y = 0; y < kMapHeight; y++)
		{
			if(mMap[x][y] != TILETYPE_BLOCKED)
				mMap[x][y] = TILETYPE_EMPTY;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void PathFinder_DRS::ResetCostArrayAndCollisionMap()
{
	//Set all values to zero for to ensure a fresh search.
	for(int x = 0; x < kMapWidth; x++)
	{
		for(int y = 0; y < kMapHeight; y++)
		{
			if(mMap[x][y] != TILETYPE_BLOCKED)
				mMap[x][y] = TILETYPE_EMPTY;

			mMapCosts[x][y] = 0;
		}
	}
}

//--------------------------------------------------------------------------------------------------
