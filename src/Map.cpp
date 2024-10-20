
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"

#include <math.h>

Map::Map() : Module(), mapLoaded(false)
{
    name = "map";
}

// Destructor
Map::~Map()
{}

// Called before render is available
bool Map::Awake()
{
    name = "map";
    LOG("Loading Map Parser");

    return true;
}

bool Map::Start() {

    return true;
}

bool Map::Update(float dt)
{
    bool ret = true;

    if (mapLoaded) {

        // L07 TODO 5: Prepare the loop to draw all tiles in a layer + DrawTexture()
        // iterate all tiles in a layer
        for (const auto& mapLayer : mapData.layers) {
            //Check if the property Draw exist get the value, if it's true draw the lawyer
            if (mapLayer->properties.GetProperty("Draw") != NULL && mapLayer->properties.GetProperty("Draw")->value == true) {
                for (int i = 0; i < mapData.width; i++) {
                    for (int j = 0; j < mapData.height; j++) {

                        // L07 TODO 9: Complete the draw function

                        //Get the gid from tile
                        int gid = mapLayer->Get(i, j);
                        //Check if the gid is different from 0 - some tiles are empty
                        if (gid != 0) {
                            //L09: TODO 3: Obtain the tile set using GetTilesetFromTileId
                            TileSet* tileSet = GetTilesetFromTileId(gid);
                            if (tileSet != nullptr) {
                                //Get the Rect from the tileSetTexture;
                                SDL_Rect tileRect = tileSet->GetRect(gid);
                                //Get the screen coordinates from the tile coordinates
                                Vector2D mapCoord = MapToWorld(i, j);
                                //Draw the texture
                                Engine::GetInstance().render->DrawTexture(tileSet->texture, mapCoord.getX(), mapCoord.getY(), &tileRect);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// L09: TODO 2: Implement function to the Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int gid) const
{
	TileSet* set = nullptr;

    for (const auto& tileset : mapData.tilesets) {
    	if (gid >= tileset->firstGid && gid < (tileset->firstGid + tileset->tileCount)) {
			set = tileset;
			break;
		}
    }

    return set;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    // L06: TODO 2: Make sure you clean up any memory allocated from tilesets/map
    for (const auto& tileset : mapData.tilesets) {
        delete tileset;
    }
    mapData.tilesets.clear();

    // L07 TODO 2: clean up all layer data
    for (const auto& layer : mapData.layers)
    {
        delete layer;
    }
    mapData.layers.clear();

    return true;
}

// Load new map
bool Map::Load(std::string path, std::string fileName)
{
    bool ret = false;

    // Assigns the name of the map file and the path
    mapFileName = fileName;
    mapPath = path;
    std::string mapPathName = mapPath + mapFileName;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapPathName.c_str());

    if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", mapPathName.c_str(), result.description());
		ret = false;
    }
    else {

        // L06: TODO 3: Implement LoadMap to load the map properties
        // retrieve the paremeters of the <map> node and store the into the mapData struct
        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();

        // L06: TODO 4: Implement the LoadTileSet function to load the tileset properties
       
        //Iterate the Tileset
        for(pugi::xml_node tilesetNode = mapFileXML.child("map").child("tileset"); tilesetNode!=NULL; tilesetNode = tilesetNode.next_sibling("tileset"))
		{
            //Load Tileset attributes
			TileSet* tileSet = new TileSet();
            tileSet->firstGid = tilesetNode.attribute("firstgid").as_int();
            tileSet->name = tilesetNode.attribute("name").as_string();
            tileSet->tileWidth = tilesetNode.attribute("tilewidth").as_int();
            tileSet->tileHeight = tilesetNode.attribute("tileheight").as_int();
            tileSet->spacing = tilesetNode.attribute("spacing").as_int();
            tileSet->margin = tilesetNode.attribute("margin").as_int();
            tileSet->tileCount = tilesetNode.attribute("tilecount").as_int();
            tileSet->columns = tilesetNode.attribute("columns").as_int();

			//Load the tileset image
			std::string imgName = tilesetNode.child("image").attribute("source").as_string();
            tileSet->texture = Engine::GetInstance().textures->Load((mapPath+imgName).c_str());

            // Check if the texture was successfully loaded
            if (tileSet->texture == nullptr) {
                LOG("Failed to load tileset image: %s", imgName.c_str());
                return false;  // Return failure if the tileset image is missing
            }

			mapData.tilesets.push_back(tileSet);
		}

        // L07: TODO 3: Iterate all layers in the TMX and load each of them
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode != NULL; layerNode = layerNode.next_sibling("layer")) {

            // L07: TODO 4: Implement the load of a single layer 
            //Load the attributes and saved in a new MapLayer
            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();

            //L09: TODO 6 Call Load Layer Properties
            LoadProperties(layerNode, mapLayer->properties);

            //Iterate over all the tiles and assign the values in the data array
            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode != NULL; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_int());
            }

            // Add colliders if this is the "Colliders" layer
            if (mapLayer->name == "Colliders") {
                AddCollidersFromLayer(mapLayer);  // Call the function to add colliders
            }

            //add the layer to the map
            mapData.layers.push_back(mapLayer);
        }

        // L08 TODO 3: Create colliders
        // L08 TODO 7: Assign collider type
        // Later you can create a function here to load and create the colliders from the map
        //PhysBody* c1 = Engine::GetInstance().physics.get()->CreateRectangle(224 + 128, 544 + 32, 256, 64, STATIC);
        //c1->ctype = ColliderType::PLATFORM;

        //PhysBody* c2 = Engine::GetInstance().physics.get()->CreateRectangle(352 + 64, 384 + 32, 128, 64, STATIC);
        //c2->ctype = ColliderType::PLATFORM;

        //PhysBody* c3 = Engine::GetInstance().physics.get()->CreateRectangle(256, 704 + 32, 576, 64, STATIC);
        //c3->ctype = ColliderType::PLATFORM;

        //PhysBody* c4 = Engine::GetInstance().physics.get()->CreateRectangle(512 + 640, 704 + 32, 1024, 64, STATIC);
        //c4->ctype = ColliderType::PLATFORM;

        ret = true;

        // L06: TODO 5: LOG all the data loaded iterate all tilesetsand LOG everything
        if (ret == true)
        {
            LOG("Successfully parsed map XML file :%s", fileName.c_str());
            LOG("width : %d height : %d", mapData.width, mapData.height);
            LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);

            LOG("Tilesets----");

            //iterate the tilesets
            for (const auto& tileset : mapData.tilesets) {
                LOG("name : %s firstgid : %d", tileset->name.c_str(), tileset->firstGid);
                LOG("tile width : %d tile height : %d", tileset->tileWidth, tileset->tileHeight);
                LOG("spacing : %d margin : %d", tileset->spacing, tileset->margin);
            }
            			
            LOG("Layers----");

            for (const auto& layer : mapData.layers) {
                LOG("id : %d name : %s", layer->id, layer->name.c_str());
				LOG("Layer width : %d Layer height : %d", layer->width, layer->height);
            }   
        }
        else {
            LOG("Error while parsing map file: %s", mapPathName.c_str());
        }

        if (mapFileXML) mapFileXML.reset();

    }

    mapLoaded = ret;
    return ret;
}

// Add the AddCollidersFromLayer function here
void Map::AddCollidersFromLayer(MapLayer* layer)
{
    for (int i = 0; i < layer->width; i++)
    {
        for (int j = 0; j < layer->height; j++)
        {
            // Get the tile ID (gid) for this position
            int gid = layer->Get(i, j);

            // Check if the gid is 57, which indicates a collider
            if (gid == 57)  // Assuming 57 is the collider GID
            {
                // Convert tile coordinates to world coordinates
                Vector2D position = MapToWorld(i, j);

                // Adjust the position to the center of the tile
                position.setX(position.getX() + mapData.tileWidth / 2);
                position.setY(position.getY() + mapData.tileHeight / 2);

                // Create a rectangular collider at this position
                PhysBody* collider = Engine::GetInstance().physics->CreateRectangle(position.getX(), position.getY(), mapData.tileWidth, mapData.tileHeight, STATIC);

                // Set collider type
                collider->ctype = ColliderType::PLATFORM;

                LOG("Collider created at tile (%d, %d) with gid 57", i, j);
            }
        }
    }
}

//ColliderType Map::DetermineColliderType(int gid)
//{
//    TileSet* tileSet = GetTilesetFromTileId(gid);
//
//    if (tileSet != nullptr)
//    {
//        Properties::Property* colliderTypeProperty = tileSet->GetProperty(gid, "collider_type");
//        if (colliderTypeProperty != nullptr)
//        {
//            if (colliderTypeProperty->stringValue == "wall")
//                return WALL;
//            else if (colliderTypeProperty->stringValue == "platform")
//                return PLATFORM;
//            else if (colliderTypeProperty->stringValue == "hazard")
//                return HAZARD;
//            else if (colliderTypeProperty->stringValue == "interact")
//                return INTERACT;
//        }
//    }
//
//    // Default to wall if no property is found
//    return WALL;
//}

// L07: TODO 8: Create a method that translates x,y coordinates from map positions to world positions
Vector2D Map::MapToWorld(int x, int y) const
{
    Vector2D ret;

    ret.setX(x * mapData.tileWidth);
    ret.setY(y * mapData.tileHeight);

    return ret;
}

// L09: TODO 6: Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = false;

    for (pugi::xml_node propertyNode = node.child("properties").child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property();
        p->name = propertyNode.attribute("name").as_string();

        // Get the type of the property, defaulting to "string" if not specified
        const char* type = propertyNode.attribute("type").as_string("string");

        // Handle property types
        if (strcmp(type, "bool") == 0) {
            // Boolean property
            p->value = propertyNode.attribute("value").as_bool();
        }
        else if (strcmp(type, "int") == 0) {
            // Integer property
            p->intValue = propertyNode.attribute("value").as_int();
        }
        else if (strcmp(type, "string") == 0) {
            // String property
            p->stringValue = propertyNode.attribute("value").as_string();
        }
        else {
            LOG("Unknown property type: %s", type);  // Log unknown types for debugging
        }

        // Add the property to the list of properties
        properties.propertyList.push_back(p);
    }

    return ret;
}

// L09: TODO 7: Implement a method to get the value of a custom property
Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
			return property;
		}
    }

    return nullptr;
}




