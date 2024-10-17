#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"

Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	//L04: TODO 3b: Instantiate the player using the entity manager
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	
	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item->position = Vector2D(200, 672);
	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load("Assets/Maps/", "MapTemplate.tmx");

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
    // Get the player's position from the physics body
    Vector2D playerPos = player->position; // Already updated by the Player class

    // Get the screen size (width and height)
    int screenWidth, screenHeight;
    Engine::GetInstance().window->GetWindowSize(screenWidth, screenHeight);

    // Calculate the desired camera position with an offset
    // Offset ensures that the player isn't too close to the screen edges
    int cameraOffsetX = (screenWidth / 8) - (screenWidth / 4); // You can adjust this value to control horizontal offset
    int cameraOffsetY = screenHeight / 4; // You can adjust this value to control vertical offset

    // Set camera to follow player position with the offset
    int targetCameraX = -playerPos.getX() + (screenWidth / 2) + cameraOffsetX;
	int targetCameraY = -playerPos.getY() + (screenHeight) - cameraOffsetY;

    // Smooth camera movement
    float lerpFactor = 0.1f; // Adjust this factor for more or less smoothness
    Engine::GetInstance().render.get()->camera.x += (targetCameraX - Engine::GetInstance().render.get()->camera.x) * lerpFactor;
    Engine::GetInstance().render.get()->camera.y += (targetCameraY - Engine::GetInstance().render.get()->camera.y) * lerpFactor;

    return true;
}



// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	SDL_DestroyTexture(img);

	return true;
}
