#include <iostream>
#include <cmath>
#include <cassert>
#include <vector>

#include "vars.h"
#include "setup.h"
#include "update.h"
#include "draw.h"

int main()
{
    // Calcolo posizione centrale sullo schermo
    VideoMode desktop = VideoMode::getDesktopMode();
    int posX = (desktop.width  - windowWidth)  / 2;
    int posY = (desktop.height - windowHeight) / 2;

    RenderWindow window(VideoMode(windowWidth, windowHeight), "Cosmos", Style::Titlebar | Style::Close);
    window.setPosition(Vector2i(posX, posY));
    window.setFramerateLimit(60);

    // Font e punteggio
    Font font;
    if (!font.loadFromFile("arial.ttf"))
        return EXIT_FAILURE;

    text.setFont(font);
    text.setCharacterSize(30);
    text.setFillColor(Color::White);
    text.setPosition(windowWidth / 2.f - 50.f, 20.f);

    Clock clock;
    Clock time;

	Shader shader;
    assert(shader.loadFromFile("fragment_shader.frag", Shader::Type::Fragment));

    RenderTexture renderTexture;
    renderTexture.create(windowWidth, windowHeight);
    Sprite screenSprite(renderTexture.getTexture());

    setup();

    while (window.isOpen())
    {
        deltaTime = clock.restart().asSeconds();
    
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }
    
        update(window);
    
        draw(renderTexture);
    
        screenSprite.setTexture(renderTexture.getTexture(), true);
    
        shader.setUniform("resolution", Glsl::Vec2{window.getSize()});
        shader.setUniform("mouse", mouse_position);
        shader.setUniform("time", time.getElapsedTime().asSeconds());
        window.clear();
        window.draw(screenSprite, &shader);
        window.display();
    }

    return 0;
}
