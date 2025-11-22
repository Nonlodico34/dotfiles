#include <iostream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "pwetty.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

#define TW (terminalWidth())
#define TH (terminalHeight())
#define CX (TW / 2)
#define CY (TH / 2)
#define FRAMES 6572
#define TARGET_FPS 30
#define FRAME_TIME (1000.0 / TARGET_FPS)

enum Style{
    PIXEL, BLACKWHITE, ASCII, STYLES
};

typedef vector<vector<Color>> frame;
vector<frame> video;
double lastFrameTime = 0;
double lastTime = 0;
Style style = PIXEL;

Color compareToBlackWhite(const Color& color) {
    double distToBlack = colorDistance(color, BLACK);
    double distToWhite = colorDistance(color, WHITE);
    // Restituisce il colore più vicino
    return (distToBlack < distToWhite) ? BLACK : WHITE;
}

void limitFPS() {
    double currentTime = getTime();
    double frameTime = (currentTime - lastFrameTime) * 1000.0; // Converti in millisecondi
    double sleepTime = FRAME_TIME - frameTime;
    
    if (sleepTime > 0) {
        sleepMs((int)sleepTime);
    }
    
    lastFrameTime = getTime();
}

string intToStringWithPadding(int number, int width = 4) {
    ostringstream oss;
    oss << setw(width) << setfill('0') << number;
    return oss.str();
}

frame loadImagePixels(string filename, int &width, int &height)
{
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, nullptr, 4);
    frame matrix(height, vector<Color>(width));

    if (!data)
    {
        cerr << "Error while loading the image\n";
        return {};
    }

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
        {
            int i = (y * width + x) * 4;
            matrix[y][x] = {data[i], data[i + 1], data[i + 2]}; // DISCARD ALPHA data[i + 3]
        }

    stbi_image_free(data);
    return matrix;
}

int main()
{
    int width, height;
    
    // Carica l'immagine
    showCursor(false);
    for(int i=0; i<FRAMES; i++){
        string filename = "frames/output_" + intToStringWithPadding(i+1) + ".jpg";
        frame image = loadImagePixels(filename, width, height);
        video.push_back(image);

        clearScreen();
        writeAligned(Alignment::Center, TH/2, "Loading: " + to_string((int)((float)i/(float)FRAMES*100.f)) + "%");
        render();
    }

    int frame = 0;
    lastTime = getTime();
    lastFrameTime = getTime();
    while (true)
    {
        clearScreen();
        frame++;
        frame%=FRAMES;

        updateInput();
        char c = getKey();
        if(keyPressed() && (c == KEY_SPACE || c == 's')){
            style = (Style)((int)style + 1);
            style = (Style)((int)style%(int)STYLES);
        }

        for (int y = 0; y < TH; y++)
        {
            for (int x = 0; x < TW; x++)
            {
                int image_x = ((float)x * (float)width)/(float)TW;
                int image_y = ((float)y * (float)height)/(float)TH;
                switch(style){
                    case PIXEL:
                        write(x, y, ' ', DEFAULT_FG, video[frame][image_y][image_x]);
                        break;
                    case BLACKWHITE:
                        write(x, y, ' ', DEFAULT_FG, compareToBlackWhite(video[frame][image_y][image_x]));
                        break;
                    case ASCII:
                        char c = compareToBlackWhite(video[frame][image_y][image_x])==BLACK?'.':'#';
                        write(x, y, c);
                        break;
                }
            }
        }

        render();
        limitFPS();
    }

    return 0;
}
