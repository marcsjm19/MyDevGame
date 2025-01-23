#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"

struct SDL_Texture;

class Item : public Entity
{
public:

	Item();
	virtual ~Item();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:

	bool isPicked = false;
	SDL_Texture* itemTexture;

private:

	
	const char* texturePath;
	int texW, texH;

	//L08 TODO 4: Add a physics to an item
	PhysBody* pbody;
};
