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
		StartCutscene(-300,-500,true,true,100, true);
	}
	if (app->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN) {
		StartCutscene(-500, 0, true, false, 100, true);
	}
	if (app->input->GetKey(SDL_SCANCODE_H) == KEY_DOWN) {
		StartCutscene(-100, -600, true, false, 100, true);
	}
	if (app->input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
		FinishCutscene();
	}

	//To keep borders in between animations
	if (KeepBorders == true) {
		app->render->DrawRectangle({ -app->render->camera.x, -app->render->camera.y - 100 + BorderOffset,1280,100 }, 0, 0, 0);
		app->render->DrawRectangle({ -app->render->camera.x, -app->render->camera.y + 720 - BorderOffset,1280,100 }, 0, 0, 0);
	}

	//Border animation to finish cutscene
	if (FinishCutsceneAux == true) {
		if (BorderOffset > 0) {
			BorderOffset -= 2;
			if (BorderOffset <= 0) {
				BorderOffset = 0;
				EndCutscene();
			}
		}
	}

	if (CutsceneStarted == true) {
		//Border animation
		if (Bordered == true && TP == false) {
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
		if (TP == true) {
			app->render->DrawRectangle({ -app->render->camera.x, -app->render->camera.y,1280,1280 }, 0, 0, 0, fading);
			if (fading < 255 && FadeIn == false) {
				fading += 5;
				if (fading >= 255 ) {
					fading = 255;
					app->render->camera.x = X;
					app->render->camera.y = Y;
					FadeIn = true;
				}
			}
			if (fading > 0 && FadeIn == true) {
				fading -= 5;
				if (fading <= 0) {
					fading = 0;
					TP = false;
				}
			}
		}
		//If border animation is done or if theres no border, execute this
		if (BorderAnimation == true && TP == false) {
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

void Scene::StartCutscene(int x, int y, bool bordered, bool tp, int speed, bool keepBorders) {
	if (CutsceneStarted == true) {
		EndCutscene();
	}
	CutsceneStarted = true;
	Bordered = bordered;
	TP = tp;
	Speed = speed;
	X = x;
	Y = y;
	XNeg = -1;
	XPos = -1;
	YNeg = -1;
	YPos = -1;
	if (Bordered == false) {
		BorderAnimation = true;
	}
	KeepBorders = keepBorders;
	if (KeepBorders == true) {
		if (BorderOffset != 0) {
			BorderAnimation = true;
			Bordered = false;
		}
	}
}

void Scene::EndCutscene() {
	CutsceneStarted = false;
	Bordered = false;
	X = 0;
	Y = 0;
	Speed = 100;
	TP = false;
	if (KeepBorders == false) {
		BorderOffset = 0;
	}
	BorderAnimation = false;
	PosCalc = false;
	fading = 0;
	FadeIn = false;
	FinishCutsceneAux = false;
}

void Scene::FinishCutscene() {
	FinishCutsceneAux = true;
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
