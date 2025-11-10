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
const int winWidth = 2000;
const int winHeight = 1200;

//Framerate stuff
const unsigned int FRAMERATE = 60;
float delTime;

//Default modifiable launch variables
Vector2 launchPos;
float launchAngle = 0;
float launchSpeed = 0;

Color bodyColor = VIOLET; //Global body color
unsigned int objectCount = 0; //Global counter

//PhysicBody typing
enum PhysicBodyType
{
	CIRCLE,
	BOX,
	HALFSPACE
};

//Each individual physic body
class PhysicsBody {
public:
	bool isStatic = false;
	Vector2 position = { 0,0 }; //Body's position in world
	Vector2 velocity = { 0,0 }; //Body's travel speed and downward force
	Vector2 drag;
	float mass = 1;

	PhysicBodyType type;
	string name = "Object";
	Color color = BLACK;

	virtual void draw()
	{
		DrawCircleV(position, 40, bodyColor);
		DrawText(name.c_str(), position.x, position.y, 40, BLACK);
	}
	
	virtual PhysicBodyType Shape() = 0;
};

class PhysicsBodyBox : public PhysicsBody {
public:
	Vector2 size = { 0,0 };
	PhysicBodyType type = BOX;

	void draw() override
	{
		DrawRectangleV(position, size, color);
		DrawText(name.c_str(), position.x, position.y, 40, BLACK);
	}

	PhysicBodyType Shape() override
	{
		return type;
	}
};

class PhysicsBodyCircle : public PhysicsBody {
public:
	float radius = 30;
	PhysicBodyType type = CIRCLE;

	void draw() override
	{
		DrawCircleV(position, radius, color);
		DrawText(name.c_str(), position.x, position.y, 40, BLACK);
	}

	PhysicBodyType Shape() override
	{
		return type;
	}
};

class PhysicsBodyHalfspace : public PhysicsBody {
	float radius = 10;
	float rotation = 0;
	Vector2 normal = { 0, -1 };
	PhysicBodyType type = HALFSPACE;

public:
	void setRotation(float rotationDegrees)
	{
		rotation = rotationDegrees;
		normal = Vector2Rotate({ 0, -1 }, rotation * DEG2RAD);
	}

	float getRotation()
	{
		return rotation;
	}

	Vector2 getNormal()
	{
		return normal;
	}

	void draw() override
	{
		DrawCircleV(position, radius, color);
		DrawLineEx(position, position + normal * 30, 1, color);
		Vector2 parellelToSurface = Vector2Rotate(normal, PI * 0.5f);
		DrawLineEx(position - parellelToSurface * 4000, position + parellelToSurface * 4000, 1, color);
	}

	PhysicBodyType Shape() override
	{
		return type;
	}
};

bool CircleOverlap(PhysicsBodyCircle* circleA, PhysicsBodyCircle* circleB) 
{
	Vector2 ABDisplacement = circleB->position - circleA->position;
	float distance = Vector2Length(ABDisplacement);
	float radiiSum = circleA->radius + circleB->radius;
	float overlap = radiiSum - distance;

	if (overlap > 0) //True if radii is greater, false otherwise
	{
		Vector2 normalAB;
		if (abs(distance) < 0.00001f)
		{
			normalAB = { 0, 1 };
		}
		else normalAB = ABDisplacement / distance;
		Vector2 mtv = normalAB * overlap;

		if (!circleA->isStatic)
		{
			circleA->position -= mtv * 0.5f;
		}
		else circleB->position += mtv * 0.5f;
		return true;
	}
	else return false;
}

bool CircleHalfspaceOverlap(PhysicsBodyCircle* circle, PhysicsBodyHalfspace* halfspace)
{
	//More to finish
	if (!circle->isStatic)
	{
		Vector2 circleDisplacement = circle->position - halfspace->position;
		float dot = Vector2DotProduct(circleDisplacement, halfspace->getNormal());
		Vector2 vectorProjection = halfspace->getNormal() * dot;

		DrawLineEx(circle->position, halfspace->position, 1, GRAY);

		Vector2 midpoint = circle->position - vectorProjection * 0.5f;
		DrawText(TextFormat("D: %6.0f", dot), midpoint.x + 10, midpoint.y + 10, 30, LIGHTGRAY);

		float overlap = circle->radius - dot;

		if (overlap > 0)
		{
			Vector2 mtv = halfspace->getNormal() * overlap;
			circle->position += mtv;
			return true;
		}
		else return false;
	}
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
			if (bodies[i]->isStatic) continue;
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
			bodies[i]->color = GREEN;
		}

		for (int i = 0; i < bodies.size(); i++)
		{
			for (int j = 0; j < bodies.size(); j++)
			{
				if (i == j) continue;

				PhysicsBody* objPointerA = bodies[i];
				PhysicsBody* objPointerB = bodies[j];

				PhysicBodyType shapeA = objPointerA->Shape();
				PhysicBodyType shapeB = objPointerB->Shape();

				bool overlapping = false;

				if (shapeA == CIRCLE && shapeB == CIRCLE)
				{
					overlapping = CircleOverlap((PhysicsBodyCircle*)objPointerA, (PhysicsBodyCircle*)objPointerB);
				}
				else if (shapeA == HALFSPACE && shapeB == CIRCLE)
				{
					overlapping = CircleHalfspaceOverlap((PhysicsBodyCircle*)objPointerB, (PhysicsBodyHalfspace*)objPointerA);
				}
				else if (shapeA == CIRCLE && shapeB == HALFSPACE)
				{
					overlapping = CircleHalfspaceOverlap((PhysicsBodyCircle*)objPointerA, (PhysicsBodyHalfspace*)objPointerB);
				}

				if (overlapping)
				{
					objPointerA->color = RED;
					objPointerB->color = RED;
				}
			}
		}
	}
};

PhysicsSim simulation; //Create simulation container
PhysicsBodyHalfspace halfspace;
PhysicsBodyHalfspace halfspace2;

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

	GuiSliderBar(Rectangle{ 10, 10, 500, 40 }, "", TextFormat("Speed: %.0f", launchSpeed), &launchSpeed, -1000, 1000);
	GuiSliderBar(Rectangle{ 10, 70, 500, 40 }, "", TextFormat("Angle: %.0f Degrees", launchAngle), &launchAngle, -180, 180);
	GuiSliderBar(Rectangle{ 10, 130, 500, 40 }, "", TextFormat("Gravitas: %.0f Pixels/sec^2", simulation.accelGrav.y), &simulation.accelGrav.y, -1000, 1000);

	GuiSliderBar(Rectangle{ 10, 190, 500, 40 }, "", TextFormat("Halfspace X: %.0f", halfspace.position.x), &halfspace.position.x, 0, winWidth);
	GuiSliderBar(Rectangle{ 10, 250, 500, 40 }, "", TextFormat("Halfspace Y: %.0f", halfspace.position.y), &halfspace.position.y, 0, winHeight);

	float halfspaceAngle = halfspace.getRotation();
	GuiSliderBar(Rectangle{ 10, 310, 500, 40 }, "", TextFormat("Halfspace rotation: %.0f", halfspaceAngle), &halfspaceAngle, -180, 180);
	halfspace.setRotation(halfspaceAngle);

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

PhysicsBodyCircle circle;

int main()
{
	InitWindow(winWidth, winHeight, "Test");
	SetTargetFPS(FRAMERATE);

	launchPos = { 100, (float)GetScreenHeight() - 100 };

	halfspace.isStatic = true;
	halfspace.position = { winWidth / 2, winHeight / 2 };
	simulation.add(&halfspace);

	halfspace2.isStatic = true;
	halfspace2.position = { winWidth / 2, winHeight / 2 };
	halfspace2.setRotation(45);
	simulation.add(&halfspace2);

	circle.isStatic = true;
	circle.position = { (winHeight / 2) + 500, (winHeight / 2) - 100 };
	simulation.add(&circle);

	while (!WindowShouldClose()) 
	{
		update();
		drawScene();
	}

	CloseWindow();
	return 0;
}