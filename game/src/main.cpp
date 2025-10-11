//Holy please work :pray:
#include <iostream>
#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"
#include <list>
#include <vector>
#include <string>
using namespace std;

//Window sizing
const int winWidth = 1500;
const int winHeight = 1000;

//Framerate stuff
const unsigned int FRAMERATE = 60;
float delTime;

//Default modifiable launch variables
Vector2 launchPos;
float launchAngle = 0;
float launchSpeed = 0;

Color bodyColor = VIOLET;
unsigned int objectCount = 0; //Global counter

//Each individual physic body
class PhysicsBody {
public:
	Vector2 position = { 0,0 }; //Body's position in world
	Vector2 velocity = { 0,0 }; //Body's travel speed and downward force
	Vector2 drag;
	float mass = 1;

	string name = "Object";

	virtual void draw()
	{
		DrawCircleV(position, 40, bodyColor);
		DrawText(name.c_str(), position.x, position.y, 40, BLACK);
	}
};

class PhysicsBodyBox : public PhysicsBody {
public:
	Vector2 size = { 0,0 };

	void draw() override
	{
		DrawRectangleV(position, size, bodyColor);
		DrawText(name.c_str(), position.x, position.y, 40, BLACK);
	}
};

class PhysicsBodyCircle : public PhysicsBody {
public:
	float radius = 30;
	Color color = VIOLET;

	void draw() override
	{
		DrawCircleV(position, radius, color);
		DrawText(name.c_str(), position.x, position.y, 40, BLACK);
	}
};

class PhysicsBodyHalfplane : public PhysicsBody {

};

bool CircleOverlap(PhysicsBodyCircle* circleA, PhysicsBodyCircle* circleB) 
{
	Vector2 ABDisplacement = circleB->position - circleA->position;
	float distance = Vector2Length(ABDisplacement);
	float radiiSum = circleA->radius + circleB->radius;

	return radiiSum > distance; //True if radii is greater, false otherwise
}

//Simulation control
class PhysicsSim {
public:
	Vector2 accelGrav = { 0, 9 };
	vector<PhysicsBody*> bodies;
	vector<PhysicsBodyBox> boxes;

	
	//Updates the physics bodies
	void update()
	{
		for (int i = 0; i < bodies.size(); i++)
		{
			bodies[i]->position = bodies[i]->position + bodies[i]->velocity * delTime;
			bodies[i]->velocity = bodies[i]->velocity + accelGrav * delTime;
		}

		collisionCheck();
	}

	void add(PhysicsBody* body)
	{
		objectCount++;
		body->name = to_string(objectCount);
		bodies.push_back(body);
	}

	void collisionCheck() 
	{
		for (int i = 0; i < bodies.size(); i++)
		{
			for (int j = 0; j < bodies.size(); j++)
			{
				if (i == j) continue;

				PhysicsBodyCircle* circlePointerA = (PhysicsBodyCircle*) bodies[i];
				PhysicsBodyCircle* circlePointerB = (PhysicsBodyCircle*) bodies[j];
			
				if (CircleOverlap(circlePointerA, circlePointerB)) 
				{
					circlePointerA->color = RED;
					circlePointerB->color = RED;
					circlePointerA->velocity = circlePointerA->velocity-circlePointerB->velocity;
					circlePointerB->velocity = circlePointerB->velocity-circlePointerA->velocity;
				}
				else
				{
					circlePointerA->color = bodyColor;
					circlePointerB->color = bodyColor;
				}
			}
		}
	}
};

PhysicsSim simulation; //Create simulation container

//Disintegrate bodies below or above screen height
void cleanup()
{
	for (int i = 0; i < simulation.bodies.size(); i++)
	{
		PhysicsBody* body = simulation.bodies[i];

		if (body->position.y > GetScreenHeight()
			|| body->position.y < -2
			|| body->position.x > GetScreenWidth()
			|| body->position.x < 0)
		{
			auto iterator = simulation.bodies.begin() + i;
			PhysicsBody* pointToPhysicsBody = *iterator;
			delete pointToPhysicsBody;

			simulation.bodies.erase(iterator);
			i--;
		}
	}
}

//Updates the simulation
void update()
{
	delTime = 1.0f / FRAMERATE;

	if (IsKeyDown(KEY_W)) launchPos.y -= 1;
	if (IsKeyDown(KEY_S)) launchPos.y += 1;
	if (IsKeyDown(KEY_A)) launchPos.x -= 1;
	if (IsKeyDown(KEY_D)) launchPos.x += 1;

	simulation.update();

	//Create a new physic body at the spawn point with initial velocity
	if (IsKeyPressed(KEY_SPACE)) 
	{
		PhysicsBodyCircle* newBody = new PhysicsBodyCircle;

		newBody->position = launchPos;
		newBody->velocity = { launchSpeed * (float)cos(launchAngle * DEG2RAD), -launchSpeed * (float)sin(launchAngle * DEG2RAD) };
		
		simulation.add(newBody);
	}
}

//Draw launch variable modifier bars and calculate the launch speed and angle as a line
void drawScene()
{
	BeginDrawing();

	GuiSliderBar(Rectangle{ 10, 15, 500, 40 }, "", TextFormat("Speed: %.0f", launchSpeed), &launchSpeed, -1000, 1000);
	GuiSliderBar(Rectangle{ 10, 70, 500, 40 }, "", TextFormat("Angle: %.0f Degrees", launchAngle), &launchAngle, -180, 180);
	GuiSliderBar(Rectangle{ 10, 130, 500, 40 }, "", TextFormat("Gravitas: %.0f Pixels/sec^2", simulation.accelGrav.y), &simulation.accelGrav.y, -1000, 1000);

	// Draw scene
	ClearBackground(SKYBLUE);
	DrawRectangle(0, GetScreenHeight() - 100, GetScreenWidth(), 100, DARKGREEN);

	//Calculate length of line
	Vector2 velocity = { launchSpeed * cos(launchAngle * DEG2RAD), -launchSpeed * sin(launchAngle * DEG2RAD) };

	//Draw initial circle
	DrawCircleV(launchPos, 30, bodyColor);
	//Draw launch line
	DrawLineEx(launchPos, launchPos + velocity, 3, RED);

	//Draw physics bodies
	for (int i = 0; i < simulation.bodies.size(); i++)
	{
		simulation.bodies[i]->draw();
		cleanup();
	}

	EndDrawing();
}

int main()
{
	InitWindow(winWidth, winHeight, "Test");
	SetTargetFPS(FRAMERATE);

	launchPos = { 100, (float)GetScreenHeight() - 100 };

	while (!WindowShouldClose()) 
	{
		update();
		drawScene();
	}

	CloseWindow();
	return 0;
}