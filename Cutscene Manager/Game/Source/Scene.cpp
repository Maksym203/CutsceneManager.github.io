#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Map.h"
#include "PathFinding.h"
#include "GuiManager.h"

#include "Defs.h"
#include "Log.h"

Scene::Scene() : Module()
{
	name.Create("scene");
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	bool ret = true;

	// iterate all objects in the scene
	// Check https://pugixml.org/docs/quickstart.html#access
	for (pugi::xml_node itemNode = config.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
	{
		Item* item = (Item*)app->entityManager->CreateEntity(EntityType::ITEM);
		item->parameters = itemNode;
	}

	//L02: DONE 3: Instantiate the player using the entity manager
	if (config.child("player")) {
		player = (Player*)app->entityManager->CreateEntity(EntityType::PLAYER);
		player->parameters = config.child("player");
	}
	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//img = app->tex->Load("Assets/Textures/test.png");
	//app->audio->PlayMusic("Assets/Audio/Music/music_spy.ogg");
	
	// L03: DONE: Load map
	bool retLoad = app->map->Load();

	// L12 Create walkability map
	if (retLoad) {
		int w, h;
		uchar* data = NULL;

		bool retWalkMap = app->map->CreateWalkabilityMap(w, h, &data);
		if(retWalkMap) app->pathfinding->SetMap(w, h, data);

		RELEASE_ARRAY(data);

	}

	//Sets the camera to be centered in isometric map
	if (app->map->mapData.type == MapTypes::MAPTYPE_ISOMETRIC) {
		uint width, height;
		app->win->GetWindowSize(width, height);
		app->render->camera.x = width / 2;

		// Texture to highligh mouse position 
		mouseTileTex = app->tex->Load("Assets/Maps/path.png");

		// Texture to show path origin 
		originTex = app->tex->Load("Assets/Maps/x.png");
	}

	if (app->map->mapData.type == MapTypes::MAPTYPE_ORTHOGONAL) {

		// Texture to highligh mouse position 
		mouseTileTex = app->tex->Load("Assets/Maps/path_square.png");

		// Texture to show path origin 
		originTex = app->tex->Load("Assets/Maps/x_square.png");
	}

	// L15: TODO 2: Declare a GUI Button and create it using the GuiManager

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
	// L03: DONE 3: Request App to Load / Save when pressing the keys F5 (save) / F6 (load)
	if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		app->SaveGameRequest();

	if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		app->LoadGameRequest();

	// L14: TODO 4: Make the camera movement independent of framerate
	float speed = 0.2 * dt;
	if (app->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		app->render->camera.y += ceil(speed);;

	if (app->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		app->render->camera.y -= ceil(speed);;

	if (app->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		app->render->camera.x += ceil(speed);;

	if (app->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		app->render->camera.x -= ceil(speed);;

	// Draw map
	app->map->Draw();

	//L15: Draw GUI
	app->guiManager->Draw();

	if (app->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN) {
		StartCutscene(-300,100,true,false,50);
	}

	if (CutsceneStarted == true) {
		//Border animation
		if (Bordered == true) {
			app->render->DrawRectangle({ -app->render->camera.x, -app->render->camera.y - 100 + BorderOffset,1280,100 }, 0, 0, 0);
			app->render->DrawRectangle({ -app->render->camera.x, -app->render->camera.y + 720 - BorderOffset,1280,100 }, 0, 0, 0);
			if (BorderOffset < 100) {
				BorderOffset += 2;
				if (BorderOffset >= 100) {
					BorderOffset = 100;
					BorderAnimation = true;
				}
			}
		}
		//If border animation is done or if theres no border, execute this
		if (BorderAnimation == true) {
			//Only done once, to move camera and make sure camera doesnt go away
			if (PosCalc == false) {
				Xdif = app->render->camera.x - X;
				Ydif = app->render->camera.y - Y;
				Xsum = Xdif / Speed;
				Ysum = Ydif / Speed;
				if (app->render->camera.x >= X) {
					XNeg = 0;
				}
				if (app->render->camera.x < X) {
					XNeg = 1;
				}
				if (app->render->camera.y >= Y) {
					YNeg = 0;
				}
				if (app->render->camera.y > Y) {
					YNeg = 1;
				}
				PosCalc = true;
			}
			if (app->render->camera.x >= X) {
				XPos = 0;
			}
			if (app->render->camera.x < X) {
				XPos = 1;
			}
			if (app->render->camera.y >= Y) {
				YPos = 0;
			}
			if (app->render->camera.y > Y) {
				YPos = 1;
			}
			//Move camera X
			if (app->render->camera.x != X) {
				app->render->camera.x -= Xsum;
				if (XNeg != XPos) {
					app->render->camera.x = X;
				}
			}
			//Move camera Y
			if (app->render->camera.y != Y) {
				app->render->camera.y -= Ysum;
				if (YNeg != YPos) {
					app->render->camera.y = Y;
				}
			}
		}
	}

	return true;
}

void Scene::StartCutscene(int x, int y, bool bordered, bool tp, int speed) {
	CutsceneStarted = true;
	Bordered = bordered;
	TP = tp;
	Speed = speed;
	X = x;
	Y = y;
	if (Bordered == false) {
		BorderAnimation = true;
	}
}

void Scene::EndCutscene() {
	CutsceneStarted = false;
	Bordered = false;
	X = 0;
	Y = 0;
	Speed = 100;
	TP = false;
	BorderOffset = 0;
	BorderAnimation = false;
	PosCalc = false;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

bool Scene::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: TODO 5: Implement the OnGuiMouseClickEvent method

	return true;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}
