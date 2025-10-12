#ifndef PWETTY_H
#define PWETTY_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <array>
#include <functional>
#include <unistd.h>
#include <termios.h>
#include <map>
#define M_PI 3.14159265358979323846

#include "uteels.h"

#define NCURSES_NO_SETBUF
#include <ncurses.h>

using namespace std;

using Color = short;

const Color COLOR_BLACK_256 = 0;
const Color COLOR_RED_256 = 1;
const Color COLOR_GREEN_256 = 2;
const Color COLOR_BROWN_256 = 3;
const Color COLOR_BLUE_256 = 4;
const Color COLOR_VIOLET_256 = 5;
const Color COLOR_CYAN_256 = 6;
const Color COLOR_LIGHT_GRAY_256 = 7;
const Color COLOR_GRAY_256 = 8;
const Color COLOR_LIGHT_RED_256 = 9;
const Color COLOR_LIME_256 = 10;
const Color COLOR_YELLOW_256 = 11;
const Color COLOR_LIGHT_BLUE_256 = 12;
const Color COLOR_PINK_256 = 13;
const Color COLOR_TURQUOISE_256 = 14;
const Color COLOR_WHITE_256 = 15;
const Color COLOR_CLEAR_256 = -1;

inline Color rgb(int r, int g, int b)
{
    r = max(0, min(5, r));
    g = max(0, min(5, g));
    b = max(0, min(5, b));
    return 16 + 36 * r + 6 * g + b;
}

inline Color gray(int level)
{
    level = max(0, min(23, level));
    return 232 + level;
}

enum Key
{
    PKEY_UP = 259,
    PKEY_DOWN = 258,
    PKEY_LEFT = 260,
    PKEY_RIGHT = 261,
    PKEY_ENTER = 10,
    PKEY_ESC = 27,
    PKEY_SPACE = 32,
    PKEY_BACKSPACE = 127,
    PKEY_TAB = 9,
    PKEY_DELETE = 330
};

#define KEY_UP PKEY_UP
#define KEY_DOWN PKEY_DOWN
#define KEY_LEFT PKEY_LEFT
#define KEY_RIGHT PKEY_RIGHT
#define KEY_ENTER PKEY_ENTER
#define KEY_ESC PKEY_ESC
#define KEY_SPACE PKEY_SPACE
#define KEY_BACKSPACE PKEY_BACKSPACE
#define KEY_TAB PKEY_TAB
#define KEY_DELETE PKEY_DELETE

enum class Alignment
{
    Left = 0,
    Center = 1,
    Right = 2
};

namespace detail
{
    class Console
    {
    private:
        static Console *instance;
        int width, height;
        vector<vector<char>> buffer, prevBuffer;
        vector<vector<Color>> fgBuffer, bgBuffer, prevFgBuffer, prevBgBuffer;
        bool pixelMode;
        bool cursorVisible;
        map<pair<Color, Color>, int> colorPairCache;
        int nextColorPair;

        Console()
        {
            initscr();
            start_color();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            nodelay(stdscr, TRUE);
            curs_set(0);
            
            nextColorPair = 1;
            
            pixelMode = false;
            cursorVisible = false;
            initBuffersToCurrentSize();
        }

        int getColorPair(Color fg, Color bg)
        {
            if (fg == COLOR_CLEAR_256) fg = COLOR_WHITE_256;
            if (bg == COLOR_CLEAR_256) bg = COLOR_BLACK_256;
            
            auto key = make_pair(fg, bg);
            auto it = colorPairCache.find(key);
            
            if (it != colorPairCache.end())
            {
                return it->second;
            }
            
            if (nextColorPair >= COLOR_PAIRS)
            {
                nextColorPair = 1;
                colorPairCache.clear();
            }
            
            int pairNum = nextColorPair++;
            init_pair(pairNum, fg, bg);
            colorPairCache[key] = pairNum;
            
            return pairNum;
        }

        void getCurrentSize(int &w, int &h) const
        {
            getmaxyx(stdscr, h, w);
        }

        void initBuffersToCurrentSize()
        {
            getCurrentSize(width, height);
            buffer.assign(height, vector<char>(width, ' '));
            fgBuffer.assign(height, vector<Color>(width, COLOR_WHITE_256));
            bgBuffer.assign(height, vector<Color>(width, COLOR_BLACK_256));
            prevBuffer.assign(height, vector<char>(width, '\0'));
            prevFgBuffer.assign(height, vector<Color>(width, -2));
            prevBgBuffer.assign(height, vector<Color>(width, -2));
        }

        void ensureSize()
        {
            int newW, newH;
            getCurrentSize(newW, newH);
            if (newW == width && newH == height)
                return;

            auto newBuffer = vector<vector<char>>(newH, vector<char>(newW, ' '));
            auto newFgBuffer = vector<vector<Color>>(newH, vector<Color>(newW, COLOR_WHITE_256));
            auto newBgBuffer = vector<vector<Color>>(newH, vector<Color>(newW, COLOR_BLACK_256));
            auto newPrevBuf = vector<vector<char>>(newH, vector<char>(newW, '\0'));
            auto newPrevFg = vector<vector<Color>>(newH, vector<Color>(newW, -2));
            auto newPrevBg = vector<vector<Color>>(newH, vector<Color>(newW, -2));

            int copyH = min(height, newH);
            int copyW = min(width, newW);

            for (int y = 0; y < copyH; ++y)
            {
                copy(buffer[y].begin(), buffer[y].begin() + copyW, newBuffer[y].begin());
                copy(fgBuffer[y].begin(), fgBuffer[y].begin() + copyW, newFgBuffer[y].begin());
                copy(bgBuffer[y].begin(), bgBuffer[y].begin() + copyW, newBgBuffer[y].begin());
            }

            width = newW;
            height = newH;
            buffer = move(newBuffer);
            fgBuffer = move(newFgBuffer);
            bgBuffer = move(newBgBuffer);
            prevBuffer = move(newPrevBuf);
            prevFgBuffer = move(newPrevFg);
            prevBgBuffer = move(newPrevBg);
        }

    public:
        ~Console()
        {
            endwin();
        }

        static Console &get()
        {
            if (!instance)
                instance = new Console();
            return *instance;
        }

        void clear(Color bg = COLOR_BLACK_256)
        {
            ensureSize();
            for (auto &row : buffer)
                fill(row.begin(), row.end(), ' ');
            for (auto &row : fgBuffer)
                fill(row.begin(), row.end(), COLOR_WHITE_256);
            for (auto &row : bgBuffer)
                fill(row.begin(), row.end(), bg);
            for (auto &row : prevBuffer)
                fill(row.begin(), row.end(), '\0');
            for (auto &row : prevFgBuffer)
                fill(row.begin(), row.end(), (Color)-2);
            for (auto &row : prevBgBuffer)
                fill(row.begin(), row.end(), (Color)-2);
        }

        void render()
        {
            ensureSize();
            
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    if (buffer[y][x] != prevBuffer[y][x] ||
                        fgBuffer[y][x] != prevFgBuffer[y][x] ||
                        bgBuffer[y][x] != prevBgBuffer[y][x])
                    {
                        int pair = getColorPair(fgBuffer[y][x], bgBuffer[y][x]);
                        attron(COLOR_PAIR(pair));
                        mvaddch(y, x, buffer[y][x]);
                        attroff(COLOR_PAIR(pair));
                    }
                }
            }

            refresh();

            prevBuffer = buffer;
            prevFgBuffer = fgBuffer;
            prevBgBuffer = bgBuffer;
        }

        void write(int x, int y, char c, Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256)
        {
            x *= pixelMode ? 2 : 1;
            if (x >= 0 && x + 1 < width && y >= 0 && y < height)
            {
                buffer[y][x] = c;
                fgBuffer[y][x] = (fg == COLOR_CLEAR_256) ? getFgColor(x, y) : fg;
                bgBuffer[y][x] = (bg == COLOR_CLEAR_256) ? getBgColor(x, y) : bg;
                
                if (pixelMode)
                {
                    buffer[y][x + 1] = c;
                    fgBuffer[y][x + 1] = (fg == COLOR_CLEAR_256) ? getFgColor(x, y) : fg;
                    bgBuffer[y][x + 1] = (bg == COLOR_CLEAR_256) ? getBgColor(x, y) : bg;
                }
            }
        }

        void write(int x, int y, string text, Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256)
        {
            if (pixelMode)
                x *= 2;
            for (size_t i = 0; i < text.size(); i += pixelMode ? 2 : 1)
            {
                write(x + i, y, text[i], fg, bg);
            }
        }

        void writeAligned(Alignment align, int y, string text, Color fg = COLOR_WHITE_256,
                          Color bg = COLOR_BLACK_256)
        {
            int x = 0;
            int auxW = pixelMode ? width / 2 : width;
            if (align == Alignment::Center)
                x = (auxW - text.size()) / 2;
            else if (align == Alignment::Right)
                x = auxW - text.size();
            write(x, y, text, fg, bg);
        }

        void showCursor(bool visible)
        {
            cursorVisible = visible;
            curs_set(visible ? 1 : 0);
        }

        int getCursorX() const
        {
            int y, x;
            getyx(stdscr, y, x);
            return pixelMode ? x / 2 : x;
        }

        int getCursorY() const
        {
            int y, x;
            getyx(stdscr, y, x);
            return y;
        }

        void setCursorX(int x)
        {
            int y = getCursorY();
            move(y, pixelMode ? x * 2 : x);
        }

        void setCursorY(int y)
        {
            int x = getCursorX();
            move(y, pixelMode ? x * 2 : x);
        }

        void setCursorPos(int x, int y)
        {
            move(y, pixelMode ? x * 2 : x);
        }

        int getWidth() const { return pixelMode ? width / 2 : width; }
        int getHeight() const { return height; }
        pair<int, int> getSize() const { return {pixelMode ? width / 2 : width, height}; }

        Color getFgColor(int x, int y) const
        {
            if (x >= 0 && x < width && y >= 0 && y < height)
                return fgBuffer[y][x];
            return COLOR_WHITE_256;
        }

        Color getBgColor(int x, int y) const
        {
            if (x >= 0 && x < width && y >= 0 && y < height)
                return bgBuffer[y][x];
            return COLOR_BLACK_256;
        }

        void setPixelMode(bool state)
        {
            pixelMode = state;
        }

        bool isInPixelMode() const
        {
            return pixelMode;
        }
    };

    Console *Console::instance = nullptr;
}

inline detail::Console &console() { return detail::Console::get(); }

inline void clearScreen(Color bg = COLOR_BLACK_256) { console().clear(bg); }
inline void render() { console().render(); }

inline void write(int x, int y, char c,
                  Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256)
{
    console().write(x, y, c, fg, bg);
}

inline void write(int x, int y, string text,
                  Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256)
{
    console().write(x, y, text, fg, bg);
}

inline void writeAligned(Alignment align, int y, string text,
                         Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256)
{
    console().writeAligned(align, y, text, fg, bg);
}

inline void showCursor(bool visible) { console().showCursor(visible); }

inline int terminalWidth() { return console().getWidth(); }
inline int terminalHeight() { return console().getHeight(); }
inline pair<int, int> terminalSize() { return console().getSize(); }

inline int getCursorX() { return console().getCursorX(); }
inline int getCursorY() { return console().getCursorY(); }
inline void setCursorX(int x) { console().setCursorX(x); }
inline void setCursorY(int y) { console().setCursorY(y); }
inline void setCursorPos(int x, int y) { console().setCursorPos(x, y); }

inline bool keyPressed() 
{ 
    int ch = getch();
    if (ch != ERR)
    {
        ungetch(ch);
        return true;
    }
    return false;
}

inline char getKey()
{
    int ch = getch();
    while (ch == ERR)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
        ch = getch();
    }
    return ch;
}

inline Color getFgColor(int x, int y) { return console().getFgColor(x, y); }
inline Color getBgColor(int x, int y) { return console().getBgColor(x, y); }

inline void setPixelMode(bool state) { console().setPixelMode(state); }
inline bool isInPixelMode() { return console().isInPixelMode(); }

inline void sleepMs(int ms)
{
    this_thread::sleep_for(chrono::milliseconds(ms));
}

inline void playBeep(int freq, int duration)
{
    thread([=]()
           { 
               string cmd = "beep -f " + to_string(freq) + " -l " + to_string(duration) + " 2>/dev/null &";
               system(cmd.c_str());
           })
        .detach();
}

inline string *advancedInput(string initialValue, function<void(string &, int &, Key)> onChange = nullptr)
{
    string buffer = initialValue;
    int cursorPos = initialValue.size();
    Key latestKey;

    if (onChange)
    {
        onChange(buffer, cursorPos, (Key)-1);
    }

    while (true)
    {
        if (!keyPressed())
        {
            sleepMs(16);
            continue;
        }

        int key = getch();
        latestKey = (Key)key;

        switch (key)
        {
        case PKEY_ENTER:
            return new string(buffer);
        case PKEY_BACKSPACE:
        case 8:
            if (cursorPos > 0)
            {
                buffer.erase(cursorPos - 1, 1);
                cursorPos--;
            }
            break;
        case PKEY_ESC:
            return nullptr;
        case PKEY_LEFT:
            if (cursorPos > 0)
                cursorPos--;
            break;
        case PKEY_RIGHT:
            if (cursorPos < buffer.size())
                cursorPos++;
            break;
        case PKEY_DELETE:
            if (cursorPos < buffer.size())
                buffer.erase(cursorPos, 1);
            break;
        default:
            if (key >= 32 && key <= 126)
            {
                buffer.insert(buffer.begin() + cursorPos, key);
                cursorPos++;
            }
            break;
        }

        if (onChange)
        {
            onChange(buffer, cursorPos, latestKey);
        }
    }
}

inline void writePixel(int x, int y, Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256, char c1 = '#', char c2 = '#')
{
    bool pixelMode = isInPixelMode();
    setPixelMode(false);
    write(x * 2, y, c1, fg, bg);
    write(x * 2 + 1, y, c2, fg, bg);
    setPixelMode(pixelMode);
}

void writeRectangle(int x1, int y1, int x2, int y2, char c = '#', Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256)
{
    if (x1 > x2)
        swap(x1, x2);
    if (y1 > y2)
        swap(y1, y2);

    for (int i = x1; i <= x2; ++i)
    {
        for (int j = y1; j <= y2; ++j)
        {
            write(i, j, c, fg, bg);
        }
    }
}

void writeBox(int x1, int y1, int x2, int y2, bool singleLine = false, Color fg = COLOR_WHITE_256,
              Color bg = COLOR_BLACK_256)
{
    if (x1 > x2)
        swap(x1, x2);
    if (y1 > y2)
        swap(y1, y2);

    for (int i = x1 + 1; i < x2; ++i)
    {
		write(i, y1, (char)(singleLine ? 196 : 205), fg, bg);
		write(i, y2, (char)(singleLine ? 196 : 205), fg, bg);
    }
    for (int i = y1 + 1; i < y2; ++i)
    {
        write(x1, i, (char)(singleLine ? 179 : 186), fg, bg);
        write(x2, i, (char)(singleLine ? 179 : 186), fg, bg);
    }

    write(x1, y1, (char)(singleLine ? 218 : 201), fg, bg); // top-left
    write(x1, y2, (char)(singleLine ? 192 : 200), fg, bg); // bottom-left
    write(x2, y1, (char)(singleLine ? 191 : 187), fg, bg); // top-right
    write(x2, y2, (char)(singleLine ? 217 : 188), fg, bg); // bottom-right
}

inline void writeLine(int x1, int y1, int x2, int y2, char c = '#', Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        write(x1, y1, c, fg, bg);
        if (x1 == x2 && y1 == y2)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

inline void writeCircleOutline(int x, int y, int r, Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256, char c = '#')
{
    for (int i = x - r - 1; i <= x + r + 1; i++)
    {
        for (int j = y - r - 1; j <= y + r + 1; j++)
        {
            if (distanceInt(i, j, x, y) == r)
            {
                writePixel(i, j, fg, bg, c, c);
            }
        }
    }
}

inline void writeCircleFilled(int x, int y, float r, Color fg = COLOR_WHITE_256, Color bg = COLOR_BLACK_256, char c = '#')
{
    for (int i = x - r - 1; i <= x + r + 1; i++)
    {
        for (int j = y - r - 1; j <= y + r + 1; j++)
        {
            if (distanceInt(i, j, x, y) <= r)
            {
                writePixel(i, j, fg, bg, c, c);
            }
        }
    }
}

inline Color fromRGB(unsigned char r, unsigned char g, unsigned char b)
{
    if (r == g && g == b)
    {
        if (r < 8)
            return COLOR_BLACK_256;
        if (r > 248)
            return COLOR_WHITE_256;
        return gray((r - 8) / 10);
    }
    
    int rIndex = (r * 5 + 127) / 255;
    int gIndex = (g * 5 + 127) / 255;
    int bIndex = (b * 5 + 127) / 255;
    
    return rgb(rIndex, gIndex, bIndex);
}

#endif
