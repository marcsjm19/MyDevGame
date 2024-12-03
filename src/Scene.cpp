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
	player->SetParameters(configParameters.child("entities").child("player"));
	
	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	/*Item* item1 = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item1->position = Vector2D(768, 992);

    Item* item2 = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
    item2->position = Vector2D(960, 1376);

    Item* item3 = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
    item3->position = Vector2D(2944, 160);

	Item* item4 = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item4->position = Vector2D(2976, 160);

	Item* item5 = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item5->position = Vector2D(3008, 160);*/

    // Create a enemy using the entity manager 
    for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
        enemy->SetParameters(enemyNode);
        enemyList.push_back(enemy);
    }
	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load("Assets/Maps/", "Level1.tmx");

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
    int targetCameraX = 0;
    if (playerPos.getX() <= 480)
    {
        targetCameraX = -480 + (screenWidth / 2) + cameraOffsetX;
    }
	else if (playerPos.getX() >= 480 && playerPos.getX() <= 2407)
    {
		targetCameraX = -playerPos.getX() + (screenWidth / 2) + cameraOffsetX;
    } 
    else
    {
		targetCameraX = -2407 + (screenWidth / 2) + cameraOffsetX;
    }

    int targetCameraY = 0;
    if (playerPos.getY() <= 543)
    {
		targetCameraY = -543 + (screenHeight / 2) + cameraOffsetY;
    }
	else if (playerPos.getY() >= 543 && playerPos.getY() <= 1302)
	{
		targetCameraY = -playerPos.getY() + (screenHeight / 2) + cameraOffsetY;
	}
	else
    {
        targetCameraY = -1302 + (screenHeight / 2) + cameraOffsetY;
    }
        

    // Smooth camera movement
    float lerpFactor = 0.1f; // Adjust this factor for more or less smoothness
    Engine::GetInstance().render.get()->camera.x += (targetCameraX - Engine::GetInstance().render.get()->camera.x) * lerpFactor;
    Engine::GetInstance().render.get()->camera.y += (targetCameraY - Engine::GetInstance().render.get()->camera.y) * lerpFactor;

    // Different levels loaded with key inputs
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
    {
        Engine::GetInstance().map->CleanUp();
        Engine::GetInstance().map->Load("Assets/Maps/", "Level1.tmx");
        LOG("Loaded map1");
    }
    else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
    {
        Engine::GetInstance().map->CleanUp();
        Engine::GetInstance().map->Load("Assets/Maps/", "MapTemplate.tmx");
        LOG("Loaded map2");
    }
    else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_3) == KEY_DOWN)
    {
        Engine::GetInstance().map->CleanUp();
        Engine::GetInstance().map->Load("Assets/Maps/", "map3.tmx");
        LOG("Loaded map3");
    }

    // L10 TODO 6: Implement a method that repositions the player in the map with a mouse click

    //Get mouse position and obtain the map coordinate
    Vector2D mousePos = Engine::GetInstance().input.get()->GetMousePosition();
    Vector2D mouseTile = Engine::GetInstance().map.get()->WorldToMap(mousePos.getX() - Engine::GetInstance().render.get()->camera.x,
        mousePos.getY() - Engine::GetInstance().render.get()->camera.y);


    //Render a texture where the mouse is over to highlight the tile, use the texture 'mouseTileTex'
    Vector2D highlightTile = Engine::GetInstance().map.get()->MapToWorld(mouseTile.getX(), mouseTile.getY());
    SDL_Rect rect = { 0,0,32,32 };
    Engine::GetInstance().render.get()->DrawTexture(mouseTileTex,
        highlightTile.getX(),
        highlightTile.getY(),
        &rect);

    // saves the tile pos for debugging purposes
    if (mouseTile.getX() >= 0 && mouseTile.getY() >= 0 || once) {
        tilePosDebug = "[" + std::to_string((int)mouseTile.getX()) + "," + std::to_string((int)mouseTile.getY()) + "] ";
        once = true;
    }

    //If mouse button is pressed modify enemy position
    if (Engine::GetInstance().input.get()->GetMouseButtonDown(1) == KEY_DOWN) {
        enemyList[0]->SetPosition(Vector2D(highlightTile.getX(), highlightTile.getY()));
        enemyList[0]->ResetPath();
    }

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
		SaveState();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
		LoadState();
	}

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

// Return the player position
Vector2D Scene::GetPlayerPosition()
{
    return player->GetPosition();
}

// L15 TODO 1: Implement the Load function
void Scene::LoadState() {

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("saveload.xml");

    if (result == NULL)
    {
        LOG("Could not load file. Pugi error: %s", result.description());
        return;
    }

    pugi::xml_node sceneNode = loadFile.child("config").child("scene");

    //Read XML and restore information

    //Player position
    Vector2D playerPos = Vector2D(sceneNode.child("entities").child("player").attribute("x").as_int(),
        sceneNode.child("entities").child("player").attribute("y").as_int());
    player->SetPosition(playerPos);

    //enemies
    // ...

}

// L15 TODO 2: Implement the Save function
void Scene::SaveState() {

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    if (result == NULL)
    {
        LOG("Could not load file. Pugi error: %s", result.description());
        return;
    }

    pugi::xml_node sceneNode = loadFile.child("config").child("scene");

    //Save info to XML 

    //Player position
    sceneNode.child("entities").child("player").attribute("x").set_value(player->GetPosition().getX());
    sceneNode.child("entities").child("player").attribute("y").set_value(player->GetPosition().getY() - 8);

    //enemies
	sceneNode.remove_child("enemies");
	pugi::xml_node enemiesNode = sceneNode.append_child("enemies");


    //Saves the modifications to the XML 
    loadFile.save_file("saveload.xml");
}