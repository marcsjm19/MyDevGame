#include "Enemy.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"
#include "EntityManager.h"
#include "Player.h"

Enemy::Enemy() : Entity(EntityType::ENEMY)
{

}

Enemy::~Enemy() {
	CleanUp();
	delete pathfinding;
}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {

	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;
	
	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	if (pbody == nullptr) {
		LOG("Error: pbody is nullptr in Start");
		return false;
	}

	//Assign collider type
	pbody->ctype = ColliderType::ENEMY;

	pbody->listener = this;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	if (!parameters.attribute("alive").as_bool()) {
		SetAlive(false);
	}

	// Initialize pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	initialX = position.getX();

	Engine::GetInstance().scene.get()->SaveState();

	return true;
}

bool Enemy::Update(float dt)
{
	// Pathfinding testing inputs
	/*if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_R) == KEY_DOWN) {
		Vector2D pos = GetPosition();
		Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(),pos.getY());
		pathfinding->ResetPath(tilePos);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
		pathfinding->PropagateBFS();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateBFS();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN) {
		pathfinding->PropagateDijkstra();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateDijkstra();
	}

	// L13: TODO 3:	Add the key inputs to propagate the A* algorithm with different heuristics (Manhattan, Euclidean, Squared)

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_DOWN) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_DOWN) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_DOWN) {
		pathfinding->PropagateAStar(SQUARED);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(SQUARED);
	}*/

	if (pbody != nullptr) {
		b2Vec2 pbodyPos = pbody->body->GetPosition();
		position.setX(METERS_TO_PIXELS(pbodyPos.x) - texW / 2);
		position.setY(METERS_TO_PIXELS(pbodyPos.y) - texH / 2);
	}

	Player* player = Engine::GetInstance().entityManager->GetPlayer();
	if (player != nullptr && IsPlayerClose(player)) {
		ResetPath();
		while (pathfinding->pathTiles.empty())
		{
			pathfinding->PropagateAStar(MANHATTAN);
		}
		
		Vector2D playerPos = player->GetPosition();
		Vector2D direction = playerPos - position;
		direction = direction.normalized();
		patrolSpeed = 1.0f;
		b2Vec2 velocity(direction.getX() * patrolSpeed, -GRAVITY_Y);
		pbody->body->SetLinearVelocity(velocity);
		if (!alertPlayed) {
			int enemyalertFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/enemyalert.wav");
			Engine::GetInstance().audio.get()->PlayFx(enemyalertFxId, 0);
			alertPlayed = true;
		}
	}
	else {
		Patrol();
		alertPlayed = false;
		ResetPath();
	}

	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	
	//position.setX(METERS_TO_PIXELS(pbodyPos.x) - texW / 2);
	//position.setY(METERS_TO_PIXELS(pbodyPos.y) - texH / 2);
    
	//position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	//position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	//position.getX();
	//position.getY();
	
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	// Draw pathfinding 
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
	{
		drawPath = !drawPath;

	}

	if (drawPath)
	{
		pathfinding->DrawPath();
	}

	return true;
}

bool Enemy::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void Enemy::SetPosition(Vector2D pos) {
	if (pbody == nullptr) {
		LOG("Error: pbody is nullptr in SetPosition");
		return;
	}
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Enemy::GetPosition() {
	if (pbody == nullptr) {
		// Handle the error, maybe log it or throw an exception
		return Vector2D(); // Return a default or error value
	}
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Enemy::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	// Get the world contact points (from Box2D contact data)
	b2WorldManifold worldManifold;
	physB->body->GetContactList()->contact->GetWorldManifold(&worldManifold);

	// Get the normal of the collision (this tells the direction of the impact)
	b2Vec2 normal = worldManifold.normal;

	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collided with player - DESTROY");
        if (normal.y >= 0.8 || (player->isShooting && (normal.x <= -0.8 || normal.x >= 0.8)))
		{
			//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
			int enemykilledFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/enemykilled.wav");
			Engine::GetInstance().audio.get()->PlayFx(enemykilledFxId);
		}
		break;
	}
}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision player");
		break;
	}
}

bool Enemy::IsPlayerClose(Player* player) {
	if (player == nullptr) {
		return false;
	}
	Vector2D playerPos = player->GetPosition();
	Vector2D enemyPos = GetPosition();
	float distance = enemyPos.distanceEuclidean(playerPos);
	return distance < METERS_TO_PIXELS(3.0f);
}

void Enemy::Patrol() {
	if (pbody == nullptr) {
		LOG("Error: pbody is nullptr in Patrol");
		return;
	}

	b2Vec2 velocity = pbody->body->GetLinearVelocity();

	if (movingRight) {
		velocity.x = patrolSpeed;
		if (position.getX() >= initialX + METERS_TO_PIXELS(patrolDistance)) {
			movingRight = false;
		}
	}
	else {
		velocity.x = -patrolSpeed;
		if (position.getX() <= initialX) {
			movingRight = true;
		}
	}

	pbody->body->SetLinearVelocity(velocity);
}