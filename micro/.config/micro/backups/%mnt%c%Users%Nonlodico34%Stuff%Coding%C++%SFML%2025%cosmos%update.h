#ifndef UPDATE_H
#define UPDATE_H
#include "vars.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

using namespace sf;
using namespace std;

void update(RenderWindow &window){
	mouse_position = window.mapPixelToCoords(Mouse::getPosition(window), camera);

	if (Keyboard::isKeyPressed(Keyboard::Escape))
	    window.close();

	// Movimento camera
	float cameraSpeed = 200 * deltaTime;
	if (Keyboard::isKeyPressed(Keyboard::W)) camera.move(Vector2f( 0,-1 ) * cameraSpeed);
	if (Keyboard::isKeyPressed(Keyboard::A)) camera.move(Vector2f(-1, 0 ) * cameraSpeed);
	if (Keyboard::isKeyPressed(Keyboard::S)) camera.move(Vector2f( 0, 1 ) * cameraSpeed);
	if (Keyboard::isKeyPressed(Keyboard::D)) camera.move(Vector2f( 1, 0 ) * cameraSpeed);

	for(Agent &a : population){
		a.move(a.speed);
		a.speed += (a.target - a.getPosition()) * 0.01f;
		a.speed *= 0.9f;
		a.target = mouse_position;
	}
}

#endif // UPDATE_H
