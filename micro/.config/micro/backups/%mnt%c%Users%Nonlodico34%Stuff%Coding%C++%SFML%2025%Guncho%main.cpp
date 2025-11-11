#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <cmath>

using namespace std;

// Dimensione finestra
const unsigned int windowWidth  = 800;
const unsigned int windowHeight = 600;

#define MAP_W 10
#define MAP_H 10

struct Cell{
	int x, y;
};

Cell level[MAP_W][MAP_H];

void setup(){
	for(int x=0; x<MAP_W; x++){
		for(int y=0; y<MAP_H; y++){
			level[x][y].x = x;
			level[x][y].y = y;
		}
	}
}

void draw(sf::RenderTexture &renderTexture){
	for(int x=0; x<MAP_W; x++){
		for(int y=0; y<MAP_H; y++){
			sf::Polygon exagon;
		}
	}
}

int main()
{
	srand(time(NULL));

    sf::Vector2f mouse_position;

    // Calcolo posizione centrale sullo schermo
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    int posX = (desktop.width  - windowWidth)  / 2;
    int posY = (desktop.height - windowHeight) / 2;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Pong", sf::Style::Titlebar | sf::Style::Close);
    window.setPosition(sf::Vector2i(posX, posY));
    window.setFramerateLimit(60);

    // Font e punteggio
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
        return EXIT_FAILURE;

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(30);
    text.setFillColor(sf::Color::White);
    text.setPosition(20.f, 20.f);

    sf::Clock clock;

	//sf::Shader shader;
    //assert(shader.loadFromFile("fragment_shader.frag", sf::Shader::Type::Fragment));

    sf::RenderTexture renderTexture;
    renderTexture.create(windowWidth, windowHeight);
    sf::Sprite screenSprite(renderTexture.getTexture());

	int frame = 0;
	setup();
    while (window.isOpen())
    {
		float dt = clock.restart().asSeconds();
		frame++;

		sf::Event event;
		while (window.pollEvent(event))
		{
		    if (event.type == sf::Event::Closed){
		        window.close();
		    }
			else if (event.type == sf::Event::MouseMoved)
			{
				mouse_position = window.mapPixelToCoords({ event.mouseMove.x, event.mouseMove.y });
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		    window.close();

		text.setString(to_string((int)(1.0/dt)) + " " + to_string(frame));

		const int FACTOR = 25;
		sf::RectangleShape rectangle(sf::Vector2f(windowWidth/FACTOR, windowHeight/FACTOR));

		// Rendering su renderTexture
		renderTexture.clear(sf::Color(0x18,0x18,0x18));
		for(int x=0; x<FACTOR; x++){
			for(int y=0; y<FACTOR; y++){
				rectangle.setFillColor((x+y)%2==0? sf::Color(0x18,0x18,0x18) : sf::Color(0x27,0x27,0x27));
				rectangle.setPosition(x*windowWidth/FACTOR, y*windowHeight/FACTOR);
				renderTexture.draw(rectangle);
			}
		}
		draw(renderTexture);
		renderTexture.draw(text);
		renderTexture.display();

		// Aggiorna lo sprite con la texture renderizzata
		screenSprite.setTexture(renderTexture.getTexture(), true);

		// Applica lo shader sullo sprite
		/*
		shader.setUniform("resolution", sf::Glsl::Vec2{window.getSize()});
		shader.setUniform("mouse", mouse_position);
		shader.setUniform("time", time.getElapsedTime().asSeconds());
		*/
		window.clear();
		window.draw(screenSprite); // , &shader
		window.display();

		cout << 1.0f/dt << "\r";
    }

    return 0;
}
