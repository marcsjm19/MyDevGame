#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void Respawn();

	bool godMode = false;
	float godSpeed = 0.2f;

public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	//Audio fx
	int pickCoinFxId;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 0.28f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	int jumpCount = 0;

	bool isDashing = false;
	bool canDash = true;
	float dashSpeed = 5.0f;
	float dashDuration = 0.2f;
	float dashTimer = 0.0f;

	bool isDead = false;       // Tracks whether the player is dead
	bool dieAnimationPlayed = false; // Tracks if the die animation has finished
	Uint32 deathTime = 0;

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	Animation walking;
	Animation jump;
	Animation die;
};