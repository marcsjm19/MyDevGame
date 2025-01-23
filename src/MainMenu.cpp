#include "Engine.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "GuiControlButton.h"
#include "Log.h"
#include "Textures.h"
#include "MainMenu.h"
#include "Scene.h"

MainMenu::MainMenu() : Module() {
    name = "mainmenu";
}

MainMenu::~MainMenu() {}

bool MainMenu::Awake() {
    LOG("Loading Main Menu");
    return true;
}

bool MainMenu::Start() {
    // Crear botones de "Play" y "Exit"
    //playButton = new GuiControlButton(1, { 100, 100, 200, 50 }, "Play");
    //exitButton = new GuiControlButton(2, { 100, 200, 200, 50 }, "Exit");
    //
    //playButton->SetObserver(this);
    //exitButton->SetObserver(this);
    //
    //backgroundTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/main_menu.png");
    //
    //SDL_Rect playBtPos = { 100, 100, 200, 50 };
    //playButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "Play", playBtPos, this);
    //SDL_Rect exitBtPos = { 100, 200, 200, 50 };
    //playButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 2, "Exit", exitBtPos, this);

    return true;
}

bool MainMenu::PreUpdate() {
	return true;
}

bool MainMenu::Update(float dt) {
    //if (backgroundTexture != nullptr) {
    //    //Engine::GetInstance().render->DrawTexture(background, 0, 0, NULL);
    //    Engine::GetInstance().textures.get()->Load("Assets/Textures/main_menu.png");
    //}
    //if (playButton != nullptr && playButton->state == GuiControlState::PRESSED) {
    //    // Transición al juego
    //
	//	Engine::GetInstance().currentState = GameState::SCENE;
    //}
    //else if (exitButton != nullptr && exitButton->state == GuiControlState::PRESSED) {
    //    // Salir del juego
	//	return false;
    //}
    return true;
}

bool MainMenu::PostUpdate() {
	return true;
}

bool MainMenu::CleanUp() {
    //if (playButton != nullptr) {
    //    delete playButton;
    //    playButton = nullptr;
    //}
    //if (exitButton != nullptr) {
    //    delete exitButton;
    //    exitButton = nullptr;
    //}
    //
    //if (backgroundTexture != nullptr) {
    //    SDL_DestroyTexture(backgroundTexture);
    //    backgroundTexture = nullptr;
    //}
    return true;
}

bool MainMenu::OnGuiMouseClickEvent(GuiControl* control) {
    //if (control->id == 1) {
    //    playButtonClicked = true;
    //    return true;
    //}
    //else if (control->id == 2) {
    //    // Salir del juego
    //    exitButtonClicked = true;
    //}
    return false;
}

bool MainMenu::IsPlayButtonClicked() const {
	return playButtonClicked;
}

bool MainMenu::IsExitButtonClicked() const {
	return exitButtonClicked;
}