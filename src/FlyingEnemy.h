#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "Player.h"

struct SDL_Texture;

class FlyingEnemy : public Entity
{
public:

	FlyingEnemy();
	virtual ~FlyingEnemy();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	bool IsAlive() const { return alive; }

	void SetAlive(bool isAlive) { alive = isAlive; }

	bool IsPlayerClose(Player* player);

public:

private:
	void Patrol();

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	PhysBody* pbody;
	Pathfinding* pathfinding;

	bool alive = true;
	Vector2D position;
	bool isPlayerFound = false;

	float patrolSpeed = 1.2f;
	float patrolDistance = 2.5f;
	float initialX;
	float initialY;
	bool movingUp;
	bool alertPlayed = false;
};
