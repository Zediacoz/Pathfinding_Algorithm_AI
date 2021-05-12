//------------------------------------------------------------------------
//  Author: Paul Roberts (2019)
//------------------------------------------------------------------------
#include "ObstacleManager.h"
#include "GameObject.h"
#include "ProjectileManager.h"
#include <SDL.h>
#include "TinyXML\tinyxml.h"

//Initialise the instance to null.
ObstacleManager* ObstacleManager::mInstance = NULL;

//--------------------------------------------------------------------------------------------------

ObstacleManager::ObstacleManager()
{
}

//--------------------------------------------------------------------------------------------------

ObstacleManager::~ObstacleManager()
{
	for(unsigned int i = 0; i < mObstacles.size(); i++)
		delete mObstacles[i];
	mObstacles.clear();
}

//--------------------------------------------------------------------------------------------------

void ObstacleManager::Init(SDL_Renderer* renderer)
{
	LoadObstacles(renderer);
}

//--------------------------------------------------------------------------------------------------

ObstacleManager* ObstacleManager::Instance()
{
	if(!mInstance)
	{
		mInstance = new ObstacleManager;
	}

	return mInstance;
}

//--------------------------------------------------------------------------------------------------

void ObstacleManager::UpdateObstacles(float deltaTime, SDL_Event e)
{
}

//--------------------------------------------------------------------------------------------------

void ObstacleManager::RenderObstacles()
{
	for(unsigned int i = 0; i < mObstacles.size(); i++)
		mObstacles[i]->Render();
}

//--------------------------------------------------------------------------------------------------

void ObstacleManager::LoadObstacles(SDL_Renderer* renderer)
{
	string imagePath;

	//Get the whole xml document.
	TiXmlDocument doc;
	if(!doc.LoadFile(kTilemapPath))
	{
		cerr << doc.ErrorDesc() << endl;
	}

	//Now get the root element.
	TiXmlElement* root = doc.FirstChildElement();
	if(!root)
	{
		cerr << "Failed to load file: No root element." << endl;
		doc.Clear();
	}
	else
	{
		float x = 0;
		float y = 0;
		string imagePath;
		string id;

		//Jump to the first 'objectgroup' element.
		for(TiXmlElement* groupElement = root->FirstChildElement("objectgroup"); groupElement != NULL; groupElement = groupElement->NextSiblingElement())
		{
			string name = groupElement->Attribute("name");
			if(name == "ObjectLayer")
			{
				//Jump to the first 'object' element - within 'objectgroup'
				for(TiXmlElement* objectElement = groupElement->FirstChildElement("object"); objectElement != NULL; objectElement = objectElement->NextSiblingElement())
				{
					string name = objectElement->Attribute("name");
					if(name == "Obstacle")
					{
						x = (float)atof(objectElement->Attribute("x"));
						y = (float)atof(objectElement->Attribute("y"));

						//Jump to the first 'properties' element - within 'object'
						for(TiXmlElement* propertiesElement = objectElement->FirstChildElement("properties"); propertiesElement != NULL; propertiesElement = propertiesElement->NextSiblingElement())
						{
							//Loop through the 'property' elements - within 'properties'
							for(TiXmlElement* propertyElement = propertiesElement->FirstChildElement("property"); propertyElement != NULL; propertyElement = propertyElement->NextSiblingElement())
							{	
								string name = propertyElement->Attribute("name");
								if(name == "ID")
								{
									id = propertyElement->Attribute("value");
									int num = atoi(id.c_str());
									if(num == 8)
										imagePath = "Images/InaccessibleTile.bmp";
									else
										imagePath = "Images/Obstacle" + id + ".png";
								}
							}

							//Set the correct obstacle type. Obstacles have indestructable mines attached, borders do not!
							GAMEOBJECT_TYPE type = GAMEOBJECT_OBSTACLE; 
							if( (id.compare("6") == 0) || (id.compare("7") == 0) )
								type = GAMEOBJECT_OBSTACLE_BORDER;

							//Add the new obstacle with the read in details.
							mObstacles.push_back(new GameObject(renderer, type, Vector2D(x,y), imagePath));
						}		
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------