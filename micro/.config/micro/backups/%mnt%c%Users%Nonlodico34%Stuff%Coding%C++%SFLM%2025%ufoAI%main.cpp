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

sf::Vector2f normalize(const sf::Vector2f& v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y);
    if (length != 0)
        return v / length; // SFML supporta divisione tra vettore e scalare
    else
        return sf::Vector2f(0.f, 0.f); // evita divisione per zero
}

sf::Vector2f randomPosition(){
	return sf::Vector2f(rand()%windowWidth, rand()%windowHeight);
}

float DegToRad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

float RadToDeg(float radians) {
    return radians * (180.0f / M_PI);
}

float distance(sf::Vector2f a, sf::Vector2f b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrt(dx * dx + dy * dy);
}

float calculateAngle(sf::Vector2f a, sf::Vector2f b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    
    // Calculate the angle using arctan2 and convert to degrees
    return RadToDeg(atan2(dy, dx));
}

sf::Vector2f RotatePoint(sf::Vector2f point, sf::Vector2f center, float angleDeg)
{
    float angleRad = DegToRad(angleDeg); // Convert degrees to radians
    float s = sinf(angleRad);
    float c = cosf(angleRad);

    // Traslazione rispetto al centro
    point.x -= center.x;
    point.y -= center.y;

    // Rotazione
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;

    // Ritorna al sistema originale
    point.x = xnew + center.x;
    point.y = ynew + center.y;

    return point;
}

float AngleBetween(sf::Vector2f a, sf::Vector2f b)
{
    float angleRad = atan2f(b.y - a.y, b.x - a.x);
    return RadToDeg(angleRad); // Convert radians to degrees
}

#define LAYERS 4
#define POPULATION 1000
#define TARGET_CHANGE_TIME (60*5)
#define TRAINING_TIME (TARGET_CHANGE_TIME*10)

int architecture[LAYERS] = {4, 4, 4, 2};
struct NeuralNetwork{	
	float bias[4][4]; // layer, neuron
	float weight[4][4][4]; // layer, from, to
	float value[4][4]; // layer, neuron
};

void initNeuralNetwork(NeuralNetwork* nn){
	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){
			nn->bias[i][j] = 1;
			nn->value[i][j] = 0;
			for(int k=0; k<4; k++){
				nn->weight[i][j][k] = 1;
			}
		}
	}	
}

void runNeuralNetwork(NeuralNetwork* nn, float in1, float in2, float in3, float in4, float &out1, float &out2){
	// Layer 0: input
	nn->value[0][0] = in1;
	nn->value[0][1] = in2;
	nn->value[0][2] = in3;
	nn->value[0][3] = in4;

	for(int layer=1; layer<LAYERS; layer++){
		for(int neuron=0; neuron<architecture[layer]; neuron++){
			float sum = 0;
			for(int prev=0; prev<architecture[layer-1]; prev++){
				sum += nn->value[layer-1][prev] * nn->weight[layer-1][prev][neuron] + nn->bias[layer][neuron];
			}
			nn->value[layer][neuron] = atan(sum);
			//nn->value[layer][neuron] = sum;
		}
	}

	out1 = nn->value[LAYERS-1][0];
	out2 = nn->value[LAYERS-1][1];
}

void polarToCartesian(float angle, float magnitude, sf::Vector2f &pos){
	pos = RotatePoint(sf::Vector2f(0, magnitude), sf::Vector2f(0,0), angle);
}

void cartesianToPolar(sf::Vector2f pos, float &angle, float &magnitude){
	angle = AngleBetween(sf::Vector2f(0,0), pos);
	magnitude = distance(sf::Vector2f(0,0), pos);
}

void mutate(NeuralNetwork* nn){
	const int randFactor = 3; // 1 su N possibilità

	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){
			//if(rand()%randFactor == 0) nn->bias[i][j] = nn->bias[i][j]*3*(((float)(rand()%1000)-500.f)/500.f) - (((float)(rand()%1000)-500.f)/500.f);
			if(rand()%randFactor == 0) nn->bias[i][j] += (((float)(rand()%1000)-500.f)/500.f);
			for(int k=0; k<4; k++){
				//if(rand()%randFactor == 0) nn->weight[i][j][k] = nn->weight[i][j][k]*3*(((float)(rand()%1000)-500.f)/500.f) - (((float)(rand()%1000)-500.f)/500.f);
				if(rand()%randFactor == 0) nn->weight[i][j][k] += (((float)(rand()%1000)-500.f)/500.f);
			}
		}	
	}
}

void newPopulation(NeuralNetwork* nns[], float scores[]){
	int surviving = POPULATION/3;

	// Sort
	for(int i=0; i<POPULATION; i++){
		for(int j=0; j<POPULATION; j++){
			if(scores[i] > scores[j]){
				swap(scores[i], scores[j]);
				swap(nns[i], nns[j]);
			}
		}
	}

	// DEBUG print 10 spaced scores
	for(int i=0; i<POPULATION; i+=POPULATION/10){
		cout << scores[i] << " ";
	}
	cout << "\n";
	
	for(int i=surviving; i<POPULATION; i++){
		*nns[i] = *nns[i%surviving];
		mutate(nns[i]);
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
    window.setFramerateLimit(9999);

    // Target
    float targetRadius = 10.f;
    sf::CircleShape target(targetRadius);
    target.setFillColor(sf::Color::Red);
    target.setPosition(randomPosition());

	// Ufo
	sf::Texture ufoPng;
	ufoPng.loadFromFile("ufo.png");
	sf::Sprite ufo(ufoPng);
	ufo.setPosition(windowWidth / 2.f - ufo.getGlobalBounds().width/2, windowHeight / 2.f - ufo.getGlobalBounds().height/2);
	sf::Vector2f positions[POPULATION];
	float scores[POPULATION];
	sf::Vector2f speeds[POPULATION];
	NeuralNetwork* nns[POPULATION];

	for(int i=0; i<POPULATION; i++){
		positions[i] = sf::Vector2f(windowWidth/2, windowHeight/2);
		speeds[i] = sf::Vector2f(0, 0);
		nns[i] = new NeuralNetwork();
		initNeuralNetwork(nns[i]);
	}

    // Font e punteggio
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
        return EXIT_FAILURE;

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(windowWidth / 2.f - 50.f, 20.f);

    int score = 0;

    sf::Clock clock;
    sf::Clock time;

	//sf::Shader shader;
    //assert(shader.loadFromFile("fragment_shader.frag", sf::Shader::Type::Fragment));

    sf::RenderTexture renderTexture;
    renderTexture.create(windowWidth, windowHeight);
    sf::Sprite screenSprite(renderTexture.getTexture());
	//float t = 1;
	//sf::Vector2f center = {0,0};

	int frame = 0;
	int generation = 0;
    while (window.isOpen())
    {
		float deltaTime = clock.restart().asSeconds();

		frame++;
		if(frame%TARGET_CHANGE_TIME == 0){
			target.setPosition(randomPosition());
		}
		if(frame >= TRAINING_TIME){
			newPopulation(nns, scores);
			for(int i=0; i<POPULATION; i++){
				scores[i] = 0;
				positions[i] = sf::Vector2f(windowWidth/2, windowHeight/2);
				speeds[i] = sf::Vector2f(0, 0);
			}
			target.setPosition(randomPosition());
			frame = 0;
			generation++;
		}

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

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
			speeds[0] += sf::Vector2f(0, -1);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
			speeds[0] += sf::Vector2f(-1, 0);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
			speeds[0] += sf::Vector2f(0, 1);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
			speeds[0] += sf::Vector2f(1, 0);
		}

		for(int i=0; i<POPULATION; i++){
			sf::Vector2f acc = sf::Vector2f(0,0);
			runNeuralNetwork(nns[i], ufo.getPosition().x, ufo.getPosition().y, target.getPosition().x, target.getPosition().y, acc.x, acc.y);

			// DEBUG print NN outputs
			if(i==0 && frame==2) cout << "NN report: " << ufo.getPosition().x << ", " << ufo.getPosition().y << ", " << target.getPosition().x << ", " << target.getPosition().y << " -> " << acc.x << ", " << acc.y << endl;
			
			speeds[i] += acc;
			speeds[i] *= 0.9f;

			positions[i] += speeds[i];

			scores[i] -= distance(positions[i], target.getPosition());
		}

		ufo.setPosition(positions[0]);

		bool render = generation > 100;
		if(render){
			scoreText.setString(std::to_string(score) + " " + to_string(1.0/deltaTime));

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
			renderTexture.draw(target);
			renderTexture.draw(ufo);
			renderTexture.draw(scoreText);
			renderTexture.display();

			// Aggiorna lo sprite con la texture renderizzata
			screenSprite.setTexture(renderTexture.getTexture(), true);

			// Applica lo shader sullo sprite
			/*
			shader.setUniform("resolution", sf::Glsl::Vec2{window.getSize()});
			shader.setUniform("mouse", mouse_position);
			shader.setUniform("time", time.getElapsedTime().asSeconds());
			shader.setUniform("radius", 0.5f);
			shader.setUniform("center", sf::Vector2f(center.x, center.y));
			t += deltaTime * 2;
			shader.setUniform("t", (float)pow(t, 1/1.5));
			*/
			window.clear();
			window.draw(screenSprite); // , &shader
			window.display();
		}

		cout << 1.0f/deltaTime << "\r";
    }

    return 0;
}
