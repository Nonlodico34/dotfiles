#ifndef VARS_H
#define VARS_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>

using namespace sf;
using namespace std;

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;
float deltaTime = 0;

Vector2f mouse_position;
View camera;
Text text;

enum Job {
    IDLE,
    BUILDING,
    TRANSPORT,
    PRODUCTION,
    JOBS_COUNT
};

enum ResourceType {
    METAL,
    PEARL,
    RESOURCE_TYPES_COUNT
};

enum StructureType {
    NODE,

    FACTORY,
    MINE,
    FARM,
    MEAT_GRINDER,
    LAB,

    HOUSE,
    JUNKYARD,

    STRUCTURES_COUNT
};

struct Agent : public Transformable {
    Job job = Job::IDLE;
    Vector2f speed = Vector2f(0, 0);
    Vector2f target = Vector2f(windowWidth/2, windowHeight/2);
};

struct Resource : public Transformable {
    ResourceType type = ResourceType::METAL;
};

struct Structure : public Transformable {
    StructureType type = StructureType::NODE;
};

struct Road {
    Structure* a;
    Structure* b;
    float speed;
};

vector<Agent> population;
vector<Resource> resources;
vector<Structure> structures;
vector<Road> roads;

/*
Altrimenti:

unordered_map<Structure*, vector<pair<Structure*, float>>> graph;
graph[&structures[0]].push_back({ &structures[1], 3.5f });
graph[&structures[1]].push_back({ &structures[0], 3.5f }); // non orientato
*/

#endif // VARS_H
