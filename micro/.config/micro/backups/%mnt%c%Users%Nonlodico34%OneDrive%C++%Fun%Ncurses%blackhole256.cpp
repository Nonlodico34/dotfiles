#include "pwetty256.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <ctime>

struct Particle
{
    Color color;
    float distance;
    int rotation;
    char c1;
    char c2;
};

struct Point
{
    float x, y;
};

Point pointAtDistanceAndAngle(float distance, float rotation)
{
    float rad = rotation * M_PI / 180.0f;
    Point result;
    result.x = distance * cos(rad);
    result.y = distance * sin(rad);
    return result;
}

int w, h;
int cx, cy;
int r;
float speed = 0.01;
vector<Particle> orbiting;
vector<Color> orbCols;
vector<Particle> borderParticles;
vector<Color> borCols;

void bg()
{
    auto [termWidth, termHeight] = terminalSize();
    w = termWidth / 2;
    h = termHeight - 2;
    //clearScreen(fromRGB(0x1c, 0x1c, 0x1c));
    clearScreen();
    srand(69);
    for (int x = 0; x < terminalSize().first; x++)
    {
        for (int y = 0; y < terminalSize().second; y++)
        {
            if (randomInt(0, 10) == 0)
                write(x, y, '.', COLOR_WHITE_256, COLOR_CLEAR_256);
        }
    }
    srand(time(0));
}

void spawnParticles(int o, int b)
{
    orbCols.push_back(COLOR_VIOLET_256);
    orbCols.push_back(COLOR_PINK_256);
    borCols.push_back(COLOR_WHITE_256);
    borCols.push_back(COLOR_YELLOW_256);
    
    for (int i = 0; i < o; ++i)
    {
        orbiting.push_back(Particle{
            orbCols[rand() % orbCols.size()],
            (float)pow(randomFloat(), 2),
            randomInt(0, 360),
            (char)randomInt(33, 127),
            (char)randomInt(33, 127),
        });
    }
    
    for (int i = 0; i < b; ++i)
    {
        borderParticles.push_back(Particle{
            borCols[rand() % borCols.size()],
            (1.f - (float)pow(randomFloat(), 12) / 1.5f),
            randomInt(0, 360),
            (char)randomInt(33, 127),
            (char)randomInt(33, 127),
        });
    }
}

void blackHole()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    float timeVal = ts.tv_sec + ts.tv_nsec / 1000000000.0f;
    
    float sen = (1 + sin(timeVal) / 15.f);
    
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            if (distanceInt(i, j, cx, cy) <= r * sen)
            {
                writePixel(i, j, COLOR_BLACK_256, COLOR_BLACK_256, '#', '#');
            }
        }
    }
    
    for (int i = 0; i < borderParticles.size(); i += max(1, 236 / w))
    {
        Particle *p = &borderParticles[i];
        p->rotation++;
        p->rotation %= 360;
        Point l = pointAtDistanceAndAngle(p->distance * sen, 360 - p->rotation);
        writePixel(cx + l.x * r, cy + l.y * r, p->color, COLOR_BLACK_256, p->c1, p->c2);
    }
}

void drawParticles(bool inFront)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    float timeVal = ts.tv_sec + ts.tv_nsec / 1000000000.0f;
    
    float mx = r * 2;
    float my = r * 0.5;
    
    for (int i = 0; i < orbiting.size(); i += max(1, 121 / h))
    {
        Particle *p = &orbiting[i];
        if (inFront != p->rotation > 180)
        {
            p->rotation += 3;
            p->rotation %= 360;
            Point l = pointAtDistanceAndAngle(p->distance + 0.5, p->rotation);
            float offset = l.x * mx * 0.2;
            offset *= sin(timeVal / 3.0f);
            writePixel(cx + l.x * mx, cy + l.y * my + offset, p->color, COLOR_CLEAR_256, p->c1, p->c2);
        }
    }
}

int main()
{
    srand(time(0));
    spawnParticles(1000, 1000);
    
    while (true)
    {
        showCursor(false);
        auto [termWidth, termHeight] = terminalSize();
        w = termWidth / 2;
        h = termHeight;
        cx = w / 2;
        cy = h / 2;
        r = w / 6;
        
        bg();
        drawParticles(false);
        blackHole();
        drawParticles(true);
        render();
        
        sleepMs(16 * 3);
    }
    
    showCursor(true);
    return 0;
}
