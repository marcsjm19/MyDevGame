#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() 
{

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(30, 0);
	return true;
}

bool Player::Start() {

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;

	walking.LoadAnimations(parameters.child("animations").child("walking"));

	jump.LoadAnimations(parameters.child("animations").child("jump"));

	die.LoadAnimations(parameters.child("animations").child("die"));

	dash.LoadAnimations(parameters.child("animations").child("dash"));

	shoot.LoadAnimations(parameters.child("animations").child("shoot"));

	// L08 TODO 5: Add physics to the player - initialize physics body
	//Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 3, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//pbody->body->GetFixtureList()->SetFriction(0);
	return true;
}

bool Player::Update(float dt)
{
	// Toggle God Mode when the G key is pressed
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		godMode = !godMode;
		if (godMode) {
			// Set physics body to kinematic so it ignores collisions
			pbody->body->SetType(b2_kinematicBody);
			LOG("God Mode Enabled");
			
			// Stop the player's movement by setting its velocity to zero
			pbody->body->SetLinearVelocity(b2Vec2(0, 0));
			LOG("God Mode Enabled");
		}
		else {
			// Return to dynamic body when disabling godMode
			pbody->body->SetType(b2_dynamicBody);
			LOG("God Mode Disabled");
		}
	}

	// If God Mode is enabled, allow free movement without physics or animations
	if (godMode) {
		b2Vec2 godVelocity(0, 0);

		// Move left
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			godVelocity.x = -godSpeed * 16;
		}

		// Move right
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			godVelocity.x = godSpeed * 16;
		}

		// Move up
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			godVelocity.y = -godSpeed * 16;
		}

		// Move down
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			godVelocity.y = godSpeed * 16;
		}

		// Set the position directly
		position.setX(position.getX() + godVelocity.x);
		position.setY(position.getY() + godVelocity.y);

		// Render the player idle
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());

		// No further updates are needed in God Mode
		return true;
	}

	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, -GRAVITY_Y);

	// Move left
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -0.2 * 16;
		currentAnimation = &walking;
	}

	// Move right
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = 0.2 * 16;
		currentAnimation = &walking;
	}

	// Jump
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && jumpCount < 2 && !godMode) {
		pbody->body->SetLinearVelocity(b2Vec2(0, -jumpForce));
		jumpCount++;
		isJumping = true;
	}

	// If the player is jumping, we don't want to apply gravity, we use the current velocity produced by the jump
	if (isJumping)
	{
		velocity = pbody->body->GetLinearVelocity();
		// Move left while jumping
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x = -0.2 * 16;
		}
		// Move right while jumping
		else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x = 0.2 * 16;
		}
		else
		{
			velocity.x = 0;
		}
		currentAnimation = &jump;
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN && canDash && !godMode)
	{
		if (velocity.x >= 0)
		{
			velocity.x = dashSpeed * 16;
		}
		else
		{
			velocity.x = -dashSpeed * 16;
		}
		canDash = false;
		isDashing = true;
		dash.Reset();
		dashTimer = SDL_GetTicks();
		currentAnimation = &dash;
	}
	if (isDashing)
	{
		currentAnimation = &dash;
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();
		if (currentAnimation->HasFinished())
		{
			if (SDL_GetTicks() - dashTimer >= 1000) {
				dash.Reset();
				isDashing = false;
				canDash = true;
				return true;
			}
		}
	}

	if (velocity.x == 0 && currentAnimation != &die && !isDead)
	{
		currentAnimation = &idle;
	}

	// If the player is dead, play the die animation
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_R) == KEY_REPEAT ||
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_REPEAT)
	{
		isDead = true;
		currentAnimation = &die;
		die.Reset();
		deathTime = SDL_GetTicks();
	}

	if (isDead)
	{
		currentAnimation = &die;
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();

		// If the die animation is finished, wait for 2 seconds to respawn
		// Wait 2 seconds before respawn
		if (currentAnimation->HasFinished())
		{
			// Wait 2 seconds before respawn
			if (SDL_GetTicks() - deathTime >= 2000) {
				Respawn();  // Respawn the player after 2 seconds
				return true;  // Stop further updates until respawn
			}
		}
		return true;  // Don't update the rest if the player is dead
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F) == KEY_DOWN && canShoot && !godMode)
	{
		canShoot = false;
		isShooting = true;
		currentAnimation = &shoot;
		shoot.Reset();
		shootTimer = SDL_GetTicks();
	}

	if (isShooting)
	{
		currentAnimation = &shoot;
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();
		if (currentAnimation->HasFinished())
		{
			if (SDL_GetTicks() - shootTimer >= 500) {
				isShooting = false;
				canShoot = true;
				shoot.Reset();
				return true;
			}
		}
	}

	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (velocity.x >= 0)
	{
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	}
	else
	{
		Engine::GetInstance().render.get()->DrawTextureFlip(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	}

	currentAnimation->Update();
	return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {

	// Get the world contact points (from Box2D contact data)
	b2WorldManifold worldManifold;
	physA->body->GetContactList()->contact->GetWorldManifold(&worldManifold);

	// Get the normal of the collision (this tells the direction of the impact)
	b2Vec2 normal = worldManifold.normal;

	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		// Reset jump only if the collision normal is pointing upwards (player landing from above)
		if (normal.y <= -0.96) {  // Adjust this threshold if needed
			isJumping = false;
			jumpCount = 0;
			jump.Reset();
		}
		break;
	case ColliderType::WALL:
		LOG("Collision WALL");
		break;
	case ColliderType::DIE:
		LOG("Collision DIE");
		isDead = true;
		break;
	/*case ColliderType::SPIKES:
		LOG("Collision SPIKES");
		isDead = true;
		break;*/
	//case ColliderType::ITEM:
	//	LOG("Collision ITEM");
	//	Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
	//	Engine::GetInstance().physics.get()->DeletePhysBody(physB); // Deletes the body of the item from the physics world
	//	break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::WALL:
		LOG("End Collision WALL");
	/*case ColliderType::SPIKES:
		LOG("End Collision SPIKES");
		break;*/
	/*case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;*/
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::Respawn()
{
	// Reset the player's position to the initial spawn point
	Player::position.setX(3);
	Player::position.setY(6);

	// Reset physics body position
	pbody->body->SetTransform(b2Vec2(position.getX(), position.getY()), 0);

	// Reset other variables like jump count, health, etc.
	currentAnimation = &idle;
	jumpCount = 0;
	isJumping = false;
	dieAnimationPlayed = false;
	isDead = false;  // Player is no longer dead
}

void Player::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Player::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}