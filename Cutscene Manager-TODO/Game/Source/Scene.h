#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"
#include "Player.h"
#include "Item.h"
#include "GuiButton.h"

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Define multiple Gui Event methods
	bool OnGuiMouseClickEvent(GuiControl* control);

	void StartCutscene(int x, int y, bool bordered, bool tp, int speed, bool keepBorders);

	void EndCutscene();

	void FinishCutscene();

public:

	//L02: DONE 3: Declare a Player attribute 
	Player* player;

private:
	SDL_Texture* img;
	SDL_Texture* mouseTileTex = nullptr;
	SDL_Texture* originTex = nullptr;

	// L12: Debug pathfing
	iPoint origin;
	bool originSelected = false;

	//Cutscene
	bool CutsceneStarted = false, Bordered = false, TP = false, BorderAnimation = false, PosCalc = false, FadeIn = false, KeepBorders = true, FinishCutsceneAux = false;
	int fading = 0, X,Xdif,Xsum, Y,Ydif,Ysum, Speed = 100, BorderOffset = 0, XNeg, YNeg, XPos, YPos;


};

#endif // __SCENE_H__