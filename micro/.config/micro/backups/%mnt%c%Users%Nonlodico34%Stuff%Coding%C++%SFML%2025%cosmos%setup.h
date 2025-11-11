#ifndef SETUP_H
#define SETUP_H
#include "vars.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

using namespace sf;
using namespace std;

void setup() {
	camera = View(FloatRect(0.f, 0.f, windowWidth, windowHeight));

	Structure node;
	node.move(100, 100);
	structures.push_back(node);
	node.move(300, 300);
	structures.push_back(node);

	Road r;
	r.a = &structures[0];
	r.b = &structures[1];
	r.speed = 1;
	roads.push_back(r);

	Agent a;
	a.job = Job::BUILDING;
	a.speed = {3, 1};
	a.move(200, 200);
	population.push_back(a);
}

#endif // SETUP_H
