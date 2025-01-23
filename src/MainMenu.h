#include "Engine.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "GuiControlButton.h"
#include "Log.h"
#include "Textures.h"
#include "Module.h"

struct SDL_Texture;

class MainMenu : public Module {
public:
    MainMenu();

    virtual ~MainMenu();

    bool Awake();

    bool Start();

	bool PreUpdate();

    bool Update(float dt);

	bool PostUpdate();

    bool CleanUp();

    bool OnGuiMouseClickEvent(GuiControl* control);

    bool IsPlayButtonClicked() const;
    bool IsExitButtonClicked() const;

private:
    GuiControlButton* playButton;
    GuiControlButton* exitButton;
	SDL_Texture* backgroundTexture;
    bool playButtonClicked = false;
    bool exitButtonClicked = false;
};
