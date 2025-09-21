
#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"

const int winWidth = 1500;
const int winHeight = 1000;

const unsigned int frameRate = 60;
float dt = 1.0f / frameRate;

Vector2 launchPos;
float launchAngle = 0;
float launchSpeed = 0;

Vector2 position = { 500, 500 };
Vector2 velocity = { 0, 0 };
Vector2 accelGrav = { 0, 9 };

void update()
{
	dt = 1.0f / frameRate;

	if (IsKeyDown(KEY_W)) launchPos.y -= 1;
	if (IsKeyDown(KEY_S)) launchPos.y += 1;
	if (IsKeyDown(KEY_A)) launchPos.x -= 1;
	if (IsKeyDown(KEY_D)) launchPos.x += 1;
}

void drawScene() 
{
	BeginDrawing();

	GuiSliderBar(Rectangle{ 10, 15, 500, 40 }, "", TextFormat("Speed: %.0f", launchSpeed), &launchSpeed, -1000, 1000);
	GuiSliderBar(Rectangle{ 10, 70, 500, 40 }, "", TextFormat("Angle: %.0f Degrees", launchAngle), &launchAngle, -180, 180);

	// Draw scene
	ClearBackground(SKYBLUE);
	DrawRectangle(0, GetScreenHeight() - 100, GetScreenWidth(), 100, DARKGREEN);
	DrawCircleV(launchPos, 50, RED); // Draw bird

	Vector2 velocity = { launchSpeed * cos(launchAngle * DEG2RAD), -launchSpeed * sin(launchAngle * DEG2RAD) };

	DrawLineEx(launchPos, launchPos + velocity, 3, RED);

	EndDrawing();
}

int main()
{
	InitWindow(winWidth, winHeight, "Test");
	launchPos = { 100, (float)GetScreenHeight() - 100 };

	while (!WindowShouldClose()) {
		update();
		drawScene();
	}

	CloseWindow();
	return 0;
}