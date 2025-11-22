#ifndef PWETTY_H
#define PWETTY_H

#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS
    #include <windows.h>
    #include <conio.h>
#elif defined(__linux__)
    #define OS_LINUX
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
    #include <sys/ioctl.h>
    #include <time.h>
    #include <errno.h>
#else
    #error "Operating system not supported by pwetty"
#endif

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <array>
#include <functional>
#include <algorithm>
#include <csignal>

#define M_PI 3.14159265358979323846

#include "uteels.h"

using namespace std;

// ======================== KEY ENUMS ========================
enum Key {
    KEY_NONE = 0,
    KEY_UP = 1000, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_ENTER, KEY_SPACE, KEY_BACKSPACE, KEY_TAB,
    KEY_HOME, KEY_END, KEY_PAGEUP, KEY_PAGEDOWN,
    KEY_INSERT, KEY_DELETE, KEY_ESC = 27
};

// ======================== STRUCT COLOR RGB ========================
struct Color {
    int r, g, b;
    Color(int red = 255, int green = 255, int blue = 255) : r(red), g(green), b(blue) {}

    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
    bool operator!=(const Color& other) const {
        return !(*this == other);
    }
};

// Colori globali (TrueColor)
const Color BLACK      = {12, 12, 12};
const Color BLUE       = {0, 55, 218};
const Color GREEN      = {19, 161, 14};
const Color CYAN       = {58, 150, 221};
const Color RED        = {197, 15, 31};
const Color VIOLET     = {136, 23, 152};
const Color BROWN      = {193, 156, 0};
const Color LIGHT_GRAY = {204, 204, 204};
const Color GRAY       = {118, 118, 118};
const Color LIGHT_BLUE = {59, 120, 255};
const Color LIME       = {22, 198, 12};
const Color TURQUOISE  = {97, 214, 214};
const Color LIGHT_RED  = {231, 72, 86};
const Color PINK       = {180, 0, 158};
const Color YELLOW     = {249, 241, 165};
const Color WHITE      = {242, 242, 242};
const Color CLEAR      = {-1, -1, -1};
const Color DEFAULT_BG = {-2, -2, -2};
const Color DEFAULT_FG = {-3, -3, -3};

double colorDistance(const Color& c1, const Color& c2) {
    return sqrt(pow(c1.r - c2.r, 2) + pow(c1.g - c2.g, 2) + pow(c1.b - c2.b, 2));
}

enum class Alignment {
    Left = 0,
    Center = 1,
    Right = 2
};

// ======================== ANSI UTILS ========================
inline string rgbFg(const Color &c) {
    if (c == DEFAULT_FG)
        return "\033[39m";
    return "\033[38;2;" + to_string(c.r) + ";" + to_string(c.g) + ";" + to_string(c.b) + "m";
}

inline string rgbBg(const Color &c) {
    if (c == DEFAULT_BG)
        return "\033[49m";
    return "\033[48;2;" + to_string(c.r) + ";" + to_string(c.g) + ";" + to_string(c.b) + "m";
}

inline const string RESET_COLOR = "\033[0m";

#ifdef OS_LINUX
string charToUnicode(char c);
#endif

// ======================== CONSOLE SINGLETON ========================
namespace detail {
    class Console {
    public:
        // Dimensioni e buffer
        int width, height;
        vector<vector<char>> buffer, prevBuffer;
        vector<vector<Color>> fgBuffer, bgBuffer, prevFgBuffer, prevBgBuffer;
        bool pixelMode;
        bool rawModeEnabled;

        // Input
        queue<int> keyQueue;
        int mouseX, mouseY;
        bool mouseButtonDown[8];
        bool mouseButtonPressed[8];
        bool mouseButtonReleased[8];

        #ifdef OS_WINDOWS
            HANDLE hConsole;
            HANDLE hStdin;
            DWORD fdwSaveOldMode;
            bool prevMouseButtonState[8];
        #endif

        #ifdef OS_LINUX
            struct termios orig_termios;
            string incompleteSequence;
        #endif

        Console() : mouseX(0), mouseY(0), pixelMode(false), rawModeEnabled(false) {
            for(int i = 0; i < 8; i++) {
                mouseButtonDown[i] = false;
                mouseButtonPressed[i] = false;
                mouseButtonReleased[i] = false;
                #ifdef OS_WINDOWS
                    prevMouseButtonState[i] = false;
                #endif
            }
            
            #ifdef OS_WINDOWS
                hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                hStdin = GetStdHandle(STD_INPUT_HANDLE);
            #endif

            setupSignalHandler();
            enableRawMode();
            initBuffersToCurrentSize();
        }

        ~Console() {
            disableRawMode();
        }

        // ======================== SIGNAL HANDLERS ========================
        #ifdef OS_WINDOWS
            static BOOL WINAPI handle_ctrl_c(DWORD type) {
                if (type == CTRL_C_EVENT) {
                    Console::get().cleanup();
                }
                return FALSE;
            }

            void setupSignalHandler() {
                SetConsoleCtrlHandler(handle_ctrl_c, TRUE);
            }
        #else
            static void handle_sigint(int sig) {
                (void)sig;
                Console::get().cleanup();
                exit(0);
            }

            void setupSignalHandler() {
                signal(SIGINT, handle_sigint);
                signal(SIGTERM, handle_sigint);
            }
        #endif

        // ======================== RAW MODE ========================
        void enableRawMode() {
            if (rawModeEnabled) return;

            #ifdef OS_LINUX
                if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
                    return;
                }
                
                struct termios raw = orig_termios;
                raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
                raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
                raw.c_oflag &= ~(OPOST);
                raw.c_cflag |= CS8;
                raw.c_cc[VMIN] = 0;  // Cambiato a 0 per lettura non bloccante
                raw.c_cc[VTIME] = 1; // Timeout di 0.1 secondi
                
                if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
                    perror("tcsetattr");
                    return;
                }
                
                // Mouse reporting
                cout << "\033[?1000h\033[?1003h\033[?1006h" << flush;
                fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
            #endif

            #ifdef OS_WINDOWS
                GetConsoleMode(hStdin, &fdwSaveOldMode);
                DWORD fdwMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
                SetConsoleMode(hStdin, fdwMode);
            #endif

            rawModeEnabled = true;
        }

        void disableRawMode() {
            if (!rawModeEnabled) return;

            #ifdef OS_LINUX
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
                cout << "\033[?1000l\033[?1003l\033[?1006l" << flush;
            #endif

            #ifdef OS_WINDOWS
                SetConsoleMode(hStdin, fdwSaveOldMode);
            #endif

            rawModeEnabled = false;
        }

        // ======================== SIZE MANAGEMENT ========================
        void getCurrentSize(int &w, int &h) const {
            #ifdef OS_WINDOWS
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(hConsole, &csbi);
                w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            #else
                struct winsize ws;
                if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
                    w = ws.ws_col;
                    h = ws.ws_row;
                } else {
                    // se ioctl fallisce
                    exit(1);
                }
            #endif
        }

        void initBuffersToCurrentSize() {
            getCurrentSize(width, height);
            if (width <= 0) width = 80;
            if (height <= 0) height = 24;
            
            buffer.assign(height, vector<char>(width, ' '));
            fgBuffer.assign(height, vector<Color>(width, DEFAULT_FG));
            bgBuffer.assign(height, vector<Color>(width, DEFAULT_BG));
            prevBuffer.assign(height, vector<char>(width, '\0'));
            prevFgBuffer.assign(height, vector<Color>(width, DEFAULT_FG));
            prevBgBuffer.assign(height, vector<Color>(width, DEFAULT_BG));
        }

        void ensureSize() {
            int newW, newH;
            getCurrentSize(newW, newH);
            if (newW == width && newH == height)
                return;

            auto newBuffer = vector<vector<char>>(newH, vector<char>(newW, ' '));
            auto newFgBuffer = vector<vector<Color>>(newH, vector<Color>(newW, DEFAULT_FG));
            auto newBgBuffer = vector<vector<Color>>(newH, vector<Color>(newW, DEFAULT_BG));
            auto newPrevBuf = vector<vector<char>>(newH, vector<char>(newW, '\0'));
            auto newPrevFg = vector<vector<Color>>(newH, vector<Color>(newW, DEFAULT_FG));
            auto newPrevBg = vector<vector<Color>>(newH, vector<Color>(newW, DEFAULT_BG));

            int copyH = min(height, newH);
            int copyW = min(width, newW);

            for (int y = 0; y < copyH; ++y) {
                for (int x = 0; x < copyW; ++x) {
                    newBuffer[y][x] = buffer[y][x];
                    newFgBuffer[y][x] = fgBuffer[y][x];
                    newBgBuffer[y][x] = bgBuffer[y][x];
                }
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

        // ======================== INPUT PARSING ========================
        #ifdef OS_LINUX
            void parseKeySequence(const string &seq) {
                if(seq.empty()) return;
                
                if(seq[0] != '\033') {
                    char c = seq[0];
                    if(c >= 32 && c <= 126) keyQueue.push(c);
                    else if(c == '\n' || c == '\r') keyQueue.push(KEY_ENTER);
                    else if(c == '\t') keyQueue.push(KEY_TAB);
                    else if(c == 127) keyQueue.push(KEY_BACKSPACE);
                    else keyQueue.push(c);
                    return;
                }
                
                if(seq.size() < 3) {
                    keyQueue.push(KEY_ESC);
                    return;
                }
                
                if(seq[1] == '[') {
                    if(seq.size() == 3) {
                        switch(seq[2]) {
                            case 'A': keyQueue.push(KEY_UP); break;
                            case 'B': keyQueue.push(KEY_DOWN); break;
                            case 'C': keyQueue.push(KEY_RIGHT); break;
                            case 'D': keyQueue.push(KEY_LEFT); break;
                            case 'H': keyQueue.push(KEY_HOME); break;
                            case 'F': keyQueue.push(KEY_END); break;
                        }
                        return;
                    }
                    
                    int num = 0;
                    if(sscanf(seq.c_str() + 2, "%d~", &num) == 1) {
                        switch(num) {
                            case 1: keyQueue.push(KEY_HOME); break;
                            case 2: keyQueue.push(KEY_INSERT); break;
                            case 3: keyQueue.push(KEY_DELETE); break;
                            case 4: keyQueue.push(KEY_END); break;
                            case 5: keyQueue.push(KEY_PAGEUP); break;
                            case 6: keyQueue.push(KEY_PAGEDOWN); break;
                        }
                    }
                }
            }

            void parseMouseSequence(const string &seq) {
                if(seq.size() < 6) return;
                if(seq[0] != '\033' || seq[1] != '[' || seq[2] != '<') return;
                
                int b = 0, x = 0, y = 0;
                char type;
                if(sscanf(seq.c_str() + 3, "%d;%d;%d%c", &b, &x, &y, &type) == 4) {
                    mouseX = x - 1; // Convert to 0-based
                    mouseY = y - 1;
                    int button = b & 3;
                    bool isScroll = (b & 64) != 0;
                    
                    if(type == 'M') {
                        if(!isScroll) {
                            mouseButtonPressed[button] = !mouseButtonDown[button];
                            mouseButtonDown[button] = true;
                        }
                    } else if(type == 'm') {
                        if(!isScroll) {
                            mouseButtonReleased[button] = mouseButtonDown[button];
                            mouseButtonDown[button] = false;
                        } else {
                            // Mouse wheel release
                            int scrollBtn = (b & 1) ? 4 : 3;
                            mouseButtonReleased[scrollBtn] = true;
                        }
                    }
                }
            }

            bool isSequenceComplete(const string& seq) {
                if(seq.empty()) return true;
                if(seq[0] != '\033') return true;
                if(seq.size() < 2) return false;
                
                // Mouse sequences
                if(seq.size() >= 3 && seq[1] == '[' && seq[2] == '<') {
                    return seq.find('M') != string::npos || seq.find('m') != string::npos;
                }
                
                // CSI sequences
                if(seq[1] == '[') {
                    if(seq.size() >= 3) {
                        char last = seq.back();
                        // Sequences ending with letters or ~
                        return (last >= 'A' && last <= 'Z') || (last >= 'a' && last <= 'z') || last == '~';
                    }
                    return false;
                }
                
                return true;
            }
        #endif

    public:
        static Console& get() {
            static Console instance;
            return instance;
        }

        void cleanup() {
            resetTerminal();
            exit(0);
        }

        // ======================== INPUT UPDATE ========================
        void updateInput() {
            for(int i = 0; i < 8; i++) {
                mouseButtonPressed[i] = false;
                mouseButtonReleased[i] = false;
            }

            #ifdef OS_LINUX
                char buf[256];
                ssize_t n;
                
                while((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
                    for(ssize_t i = 0; i < n; i++) {
                        incompleteSequence += buf[i];
                        
                        if(isSequenceComplete(incompleteSequence)) {
                            if(incompleteSequence.size() > 3 && incompleteSequence[2] == '<') {
                                parseMouseSequence(incompleteSequence);
                            } else {
                                parseKeySequence(incompleteSequence);
                            }
                            incompleteSequence.clear();
                        }
                    }

                    // Prevent infinite loop if we keep getting data
                    if(n < (ssize_t)sizeof(buf)) break;
                }
            #endif

            #ifdef OS_WINDOWS
                DWORD numEvents = 0;
                GetNumberOfConsoleInputEvents(hStdin, &numEvents);
                if(numEvents == 0) return;

                INPUT_RECORD irInBuf[128];
                DWORD numRead = 0;
                if(!ReadConsoleInput(hStdin, irInBuf, min(numEvents, (DWORD)128), &numRead)) return;

                for(DWORD i = 0; i < numRead; i++) {
                    if(irInBuf[i].EventType == KEY_EVENT) {
                        KEY_EVENT_RECORD ker = irInBuf[i].Event.KeyEvent;
                        if(!ker.bKeyDown) continue;

                        if(ker.uChar.AsciiChar != 0) {
                            char c = ker.uChar.AsciiChar;
                            if(c == '\r') keyQueue.push(KEY_ENTER);
                            else if(c == '\t') keyQueue.push(KEY_TAB);
                            else if(c == '\b') keyQueue.push(KEY_BACKSPACE);
                            else if(c == ' ') keyQueue.push(KEY_SPACE);
                            else if(c >= 32 && c <= 126) keyQueue.push(c);
                        } else {
                            switch(ker.wVirtualKeyCode) {
                                case VK_UP: keyQueue.push(KEY_UP); break;
                                case VK_DOWN: keyQueue.push(KEY_DOWN); break;
                                case VK_LEFT: keyQueue.push(KEY_LEFT); break;
                                case VK_RIGHT: keyQueue.push(KEY_RIGHT); break;
                                case VK_HOME: keyQueue.push(KEY_HOME); break;
                                case VK_END: keyQueue.push(KEY_END); break;
                                case VK_PRIOR: keyQueue.push(KEY_PAGEUP); break;
                                case VK_NEXT: keyQueue.push(KEY_PAGEDOWN); break;
                                case VK_INSERT: keyQueue.push(KEY_INSERT); break;
                                case VK_DELETE: keyQueue.push(KEY_DELETE); break;
                                case VK_ESCAPE: keyQueue.push(KEY_ESC); break;
                            }
                        }
                    } else if(irInBuf[i].EventType == MOUSE_EVENT) {
                        MOUSE_EVENT_RECORD mer = irInBuf[i].Event.MouseEvent;
                        mouseX = mer.dwMousePosition.X;
                        mouseY = mer.dwMousePosition.Y;

                        bool currentButtonState[8] = {false};
                        currentButtonState[0] = (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
                        currentButtonState[1] = (mer.dwButtonState & RIGHTMOST_BUTTON_PRESSED) != 0;
                        currentButtonState[2] = (mer.dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED) != 0;

                        for(int b = 0; b < 3; b++) {
                            if(currentButtonState[b] && !prevMouseButtonState[b]) {
                                mouseButtonPressed[b] = true;
                                mouseButtonDown[b] = true;
                            } else if(!currentButtonState[b] && prevMouseButtonState[b]) {
                                mouseButtonReleased[b] = true;
                                mouseButtonDown[b] = false;
                            } else if(currentButtonState[b]) {
                                mouseButtonDown[b] = true;
                            }
                            prevMouseButtonState[b] = currentButtonState[b];
                        }

                        if(mer.dwEventFlags & MOUSE_WHEELED) {
                            int delta = (short)HIWORD(mer.dwButtonState);
                            int scrollBtn = (delta > 0) ? 3 : 4;
                            mouseButtonPressed[scrollBtn] = true;
                            mouseButtonReleased[scrollBtn] = true;
                        }
                    }
                }
            #endif
        }

        // ======================== INPUT ACCESSORS ========================
        bool isKeyPressed() const { return !keyQueue.empty(); }
        int popKey() {
            if(keyQueue.empty()) return KEY_NONE;
            int k = keyQueue.front();
            keyQueue.pop();
            return k;
        }

        int getMouseX() const { return (pixelMode?mouseX/2:mouseX); }
        int getMouseY() const { return mouseY; }
        bool isMouseButtonDown(int b) const { return b >= 0 && b < 8 ? mouseButtonDown[b] : false; }
        bool isMouseButtonPressed(int b) const { return b >= 0 && b < 8 ? mouseButtonPressed[b] : false; }
        bool isMouseButtonReleased(int b) const { return b >= 0 && b < 8 ? mouseButtonReleased[b] : false; }

        // ======================== RENDERING ========================
        void clear(Color bg = DEFAULT_BG) {
            ensureSize();
            for (auto &row : buffer)
                fill(row.begin(), row.end(), ' ');
            for (auto &row : fgBuffer)
                fill(row.begin(), row.end(), DEFAULT_FG);
            for (auto &row : bgBuffer)
                fill(row.begin(), row.end(), bg);
        }

        void resetTerminal() {
            #ifdef OS_LINUX
                printf("\033[0m");
                printf("\033[?25h");
                printf("\033[2J\033[H"); // Clear screen and move cursor to top
                fflush(stdout);
            #else
                system("cls");
            #endif
        }

        void render() {
            ensureSize();
            
            #ifdef OS_LINUX
                // Su Linux usa write diretto per evitare problemi di buffering
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        //if (buffer[y][x] != prevBuffer[y][x] ||
                        //    fgBuffer[y][x] != prevFgBuffer[y][x] ||
                        //    bgBuffer[y][x] != prevBgBuffer[y][x]) {
                        {  
                            string line = "\033["; 
                            line += to_string(y + 1) + ";";
                            line += to_string(x + 1) + "H";
                            line += rgbFg(fgBuffer[y][x]) + rgbBg(bgBuffer[y][x]);
                            line += charToUnicode(buffer[y][x]);

                            ::write(STDOUT_FILENO, line.c_str(), line.length());
                        }
                    }
                }
                
                ::write(STDOUT_FILENO, RESET_COLOR.c_str(), RESET_COLOR.length());
                fsync(STDOUT_FILENO);
            #else
                string output;
                
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (buffer[y][x] != prevBuffer[y][x] ||
                            fgBuffer[y][x] != prevFgBuffer[y][x] ||
                            bgBuffer[y][x] != prevBgBuffer[y][x]) {    
                            
                            output += "\033[" + to_string(y + 1) + ";" + to_string(x + 1) + "H";
                            output += rgbFg(fgBuffer[y][x]) + rgbBg(bgBuffer[y][x]);
                            output += buffer[y][x];
                        }
                    }
                }
                
                output += RESET_COLOR;
                cout << output << flush;
            #endif

            prevBuffer = buffer;
            prevFgBuffer = fgBuffer;
            prevBgBuffer = bgBuffer;
        }

        void write(int x, int y, char c, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
            if (pixelMode) x *= 2;
            if (x >= 0 && x < width && y >= 0 && y < height) {
                buffer[y][x] = c;
                fgBuffer[y][x] = (fg == CLEAR) ? getFgColor(x, y) : fg;
                bgBuffer[y][x] = (bg == CLEAR) ? getBgColor(x, y) : bg;
                if (pixelMode && x + 1 < width) {
                    buffer[y][x + 1] = c;
                    fgBuffer[y][x + 1] = (fg == CLEAR) ? getFgColor(x, y) : fg;
                    bgBuffer[y][x + 1] = (bg == CLEAR) ? getBgColor(x, y) : bg;
                }
            }
        }

        void write(int x, int y, string text, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
            if (pixelMode) x *= 2;
            for (size_t i = 0; i < text.size(); i++) {
                int currentX = x + (pixelMode ? i * 2 : i);
                if (currentX < width && y >= 0 && y < height) {
                    buffer[y][currentX] = text[i];
                    fgBuffer[y][currentX] = (fg == CLEAR) ? getFgColor(currentX, y) : fg;
                    bgBuffer[y][currentX] = (bg == CLEAR) ? getBgColor(currentX, y) : bg;
                    if (pixelMode){
                        currentX += 1;
                        buffer[y][currentX] = text[i];
                        fgBuffer[y][currentX] = (fg == CLEAR) ? getFgColor(currentX, y) : fg;
                        bgBuffer[y][currentX] = (bg == CLEAR) ? getBgColor(currentX, y) : bg;
                    }
                }
            }
        }

        void writeAligned(Alignment align, int y, string text, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
            int x = 0;
            int auxW = pixelMode ? width / 2 : width;
            if (align == Alignment::Center)
                x = (auxW - text.size()) / 2;
            else if (align == Alignment::Right)
                x = auxW - text.size();
            write(x, y, text, fg, bg);
        }

        void showCursor(bool visible) {
            if (visible)
                cout << "\033[?25h";
            else
                cout << "\033[?25l";
            cout.flush();
        }

        int getWidth() const { return pixelMode ? width / 2 : width; }
        int getHeight() const { return height; }
        pair<int, int> getSize() const { return {getWidth(), height}; }

        Color getFgColor(int x, int y) const {
            if (x >= 0 && x < width && y >= 0 && y < height)
                return fgBuffer[y][x];
            return DEFAULT_FG;
        }

        Color getBgColor(int x, int y) const {
            if (x >= 0 && x < width && y >= 0 && y < height)
                return bgBuffer[y][x];
            return DEFAULT_BG;
        }

        void setPixelMode(bool state) { pixelMode = state; }
        bool isInPixelMode() const { return pixelMode; }
    };
}

// ======================== GLOBAL FUNCTIONS ========================
inline detail::Console& console() { return detail::Console::get(); }

// Display functions
inline void clearScreen(Color bg = DEFAULT_BG) { console().clear(bg); }
inline void resetTerminal() { console().resetTerminal(); }
inline void render() { console().render(); }

inline void write(int x, int y, char c, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
    console().write(x, y, c, fg, bg);
}

inline void write(int x, int y, string text, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
    console().write(x, y, text, fg, bg);
}

inline void writeAligned(Alignment align, int y, string text, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
    console().writeAligned(align, y, text, fg, bg);
}

inline void showCursor(bool visible) { console().showCursor(visible); }

inline int terminalWidth() { return console().getWidth(); }
inline int terminalHeight() { return console().getHeight(); }
inline pair<int, int> terminalSize() { return console().getSize(); }

inline Color getFgColor(int x, int y) { return console().getFgColor(x, y); }
inline Color getBgColor(int x, int y) { return console().getBgColor(x, y); }

inline void setPixelMode(bool state) { console().setPixelMode(state); }
inline bool isInPixelMode() { return console().isInPixelMode(); }

// Input functions
inline void updateInput() { console().updateInput(); }
inline bool keyPressed() { return console().isKeyPressed(); }
inline int getKey() { return console().popKey(); }

inline int getMouseX() { return console().getMouseX(); }
inline int getMouseY() { return console().getMouseY(); }
inline bool isMouseButtonDown(int b = 0) { return console().isMouseButtonDown(b); }
inline bool isMouseButtonPressed(int b = 0) { return console().isMouseButtonPressed(b); }
inline bool isMouseButtonReleased(int b = 0) { return console().isMouseButtonReleased(b); }

inline bool isMouseInArea(int x1, int y1, int x2, int y2) {
    int mx = getMouseX(), my = getMouseY();
    return mx >= x1 && mx <= x2 && my >= y1 && my <= y2;
}

inline bool isMouseAtPosition(int x, int y) {
    return getMouseX() == x && getMouseY() == y;
}

// Utility functions
inline void sleepMs(int ms) {
    #ifdef OS_WINDOWS
        if (ms > 0) {
            Sleep((float)ms/2.f);
        }
    #else
        if (ms > 0) {
            usleep(ms * 1000); // usleep uses microseconds
        }
    #endif
}

static inline double getTime() {
    #ifdef OS_WINDOWS
        static LARGE_INTEGER freq, start;
        static int init = 0;
        LARGE_INTEGER now;
        if (!init) { QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&start); init = 1; }
        QueryPerformanceCounter(&now);
        return (now.QuadPart - start.QuadPart) / (double)freq.QuadPart;
    #else
        static struct timespec start;
        static int init = 0;
        struct timespec now;
        if (!init) { clock_gettime(CLOCK_MONOTONIC, &start); init = 1; }
        clock_gettime(CLOCK_MONOTONIC, &now);
        return (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
    #endif
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
        updateInput();
        if (!keyPressed())
        {
            sleepMs(16); // 60 fps
            continue;
        }

        int key = getKey();
        latestKey = (Key)key;

        switch (key)
        {
        case KEY_ENTER:
            return new string(buffer);
        case KEY_BACKSPACE:
            if (cursorPos > 0)
            {
                buffer.erase(cursorPos - 1, 1);
                cursorPos--;
            }
            break;
        case KEY_ESC:
            return nullptr;
        case KEY_LEFT:
            if (cursorPos > 0)
                cursorPos--;
            break;
        case KEY_RIGHT:
            if (cursorPos < buffer.size())
                cursorPos++;
            break;
        case KEY_DELETE:
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

// ==== Drawing functions ====

// @deprecated (use pixelMode instead)
inline void writePixel(int x, int y, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG, char c1 = (char)219, char c2 = (char)219) {
    bool pixelMode = isInPixelMode();
    setPixelMode(false);
    write(x * 2, y, c1, fg, bg);
    write(x * 2 + 1, y, c2, fg, bg);
    setPixelMode(pixelMode);
}

inline void writeRectangle(int x1, int y1, int x2, int y2, char c = (char)219, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);

    for (int i = x1; i <= x2; ++i) {
        for (int j = y1; j <= y2; ++j) {
            write(i, j, c, fg, bg);
        }
    }
}

inline void writeBox(int x1, int y1, int x2, int y2, bool singleLine = false, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);

    for (int i = x1 + 1; i < x2; ++i) {
        write(i, y1, (char)(singleLine ? 196 : 205), fg, bg);
        write(i, y2, (char)(singleLine ? 196 : 205), fg, bg);
    }
    for (int i = y1 + 1; i < y2; ++i) {
        write(x1, i, (char)(singleLine ? 179 : 186), fg, bg);
        write(x2, i, (char)(singleLine ? 179 : 186), fg, bg);
    }

    write(x1, y1, (char)(singleLine ? 218 : 201), fg, bg); // top-left
    write(x1, y2, (char)(singleLine ? 192 : 200), fg, bg); // bottom-left
    write(x2, y1, (char)(singleLine ? 191 : 187), fg, bg); // top-right
    write(x2, y2, (char)(singleLine ? 217 : 188), fg, bg); // bottom-right
}

inline void writeLine(int x1, int y1, int x2, int y2, char c = (char)219, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        write(x1, y1, c, fg, bg);
        if (x1 == x2 && y1 == y2)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

inline void writeCircleOutline(int x, int y, int r, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG, char c = (char)219) {
    for (int i = x - r - 1; i <= x + r + 1; i++) {
        for (int j = y - r - 1; j <= y + r + 1; j++) {
            if (distanceInt(i, j, x, y) == r) {
                writePixel(i, j, fg, bg, c, c);
            }
        }
    }
}

inline void writeCircleFilled(int x, int y, float r, Color fg = DEFAULT_FG, Color bg = DEFAULT_BG, char c = (char)219) {
    for (int i = x - r - 1; i <= x + r + 1; i++) {
        for (int j = y - r - 1; j <= y + r + 1; j++) {
            if (distanceInt(i, j, x, y) <= r) {
                writePixel(i, j, fg, bg, c, c);
            }
        }
    }
}

inline Color randColor() {
    return Color(randomInt(0, 255), randomInt(0, 255), randomInt(0, 255));
}

#ifdef OS_LINUX
// https://symbl.cc/en/2563/ NON rimuovere MAI il commento.
string charToUnicode(char c){
    string str;
    /*
    -128 -> 'Ç' (0x80)
    -127 -> 'ü' (0x81)
    -126 -> 'é' (0x82)
    -125 -> 'â' (0x83)
    -124 -> 'ä' (0x84)
    -123 -> 'à' (0x85)
    -122 -> 'å' (0x86)
    -121 -> 'ç' (0x87)
    -120 -> 'ê' (0x88)
    -119 -> 'ë' (0x89)
    -118 -> 'è' (0x8a)
    -117 -> 'ï' (0x8b)
    -116 -> 'î' (0x8c)
    -115 -> 'ì' (0x8d)
    -114 -> 'Ä' (0x8e)
    -113 -> 'Å' (0x8f)
    -112 -> 'É' (0x90)
    -111 -> 'æ' (0x91)
    -110 -> 'Æ' (0x92)
    -109 -> 'ô' (0x93)
    -108 -> 'ö' (0x94)
    -107 -> 'ò' (0x95)
    -106 -> 'û' (0x96)
    -105 -> 'ù' (0x97)
    -104 -> 'ÿ' (0x98)
    -103 -> 'Ö' (0x99)
    -102 -> 'Ü' (0x9a)
    -101 -> 'ø' (0x9b)
    -100 -> '£' (0x9c)
    -99 -> 'Ø' (0x9d)
    -98 -> '×' (0x9e)
    -97 -> 'ƒ' (0x9f)
    -96 -> 'á' (0xa0)
    -95 -> 'í' (0xa1)
    -94 -> 'ó' (0xa2)
    -93 -> 'ú' (0xa3)
    -92 -> 'ñ' (0xa4)
    -91 -> 'Ñ' (0xa5)
    -90 -> 'ª' (0xa6)
    -89 -> 'º' (0xa7)
    -88 -> '¿' (0xa8)
    -87 -> '®' (0xa9)
    -86 -> '¬' (0xaa)
    -85 -> '½' (0xab)
    -84 -> '¼' (0xac)
    -83 -> '¡' (0xad)
    -82 -> '«' (0xae)
    -81 -> '»' (0xaf)
    -80 -> '░' (0xb0)
    -79 -> '▒' (0xb1)
    -78 -> '▓' (0xb2)
    -77 -> '│' (0xb3)
    -76 -> '┤' (0xb4)
    -75 -> 'Á' (0xb5)
    -74 -> 'Â' (0xb6)
    -73 -> 'À' (0xb7)
    -72 -> '©' (0xb8)
    -71 -> '╣' (0xb9)
    -70 -> '║' (0xba)
    -69 -> '╗' (0xbb)
    -68 -> '╝' (0xbc)
    -67 -> '¢' (0xbd)
    -66 -> '¥' (0xbe)
    -65 -> '┐' (0xbf)
    -64 -> '└' (0xc0)
    -63 -> '┴' (0xc1)
    -62 -> '┬' (0xc2)
    -61 -> '├' (0xc3)
    -60 -> '─' (0xc4)
    -59 -> '┼' (0xc5)
    -58 -> 'ã' (0xc6)
    -57 -> 'Ã' (0xc7)
    -56 -> '╚' (0xc8)
    -55 -> '╔' (0xc9)
    -54 -> '╩' (0xca)
    -53 -> '╦' (0xcb)
    -52 -> '╠' (0xcc)
    -51 -> '═' (0xcd)
    -50 -> '╬' (0xce)
    -49 -> '¤' (0xcf)
    -48 -> 'ð' (0xd0)
    -47 -> 'Ð' (0xd1)
    -46 -> 'Ê' (0xd2)
    -45 -> 'Ë' (0xd3)
    -44 -> 'È' (0xd4)
    -43 -> 'ı' (0xd5)
    -42 -> 'Í' (0xd6)
    -41 -> 'Î' (0xd7)
    -40 -> 'Ï' (0xd8)
    -39 -> '┘' (0xd9)
    -38 -> '┌' (0xda)
    -37 -> '█' (0xdb)
    -36 -> '▄' (0xdc)
    -35 -> '¦' (0xdd)
    -34 -> 'Ì' (0xde)
    -33 -> '▀' (0xdf)
    -32 -> 'Ó' (0xe0)
    -31 -> 'ß' (0xe1)
    -30 -> 'Ô' (0xe2)
    -29 -> 'Ò' (0xe3)
    -28 -> 'õ' (0xe4)
    -27 -> 'Õ' (0xe5)
    -26 -> 'µ' (0xe6)
    -25 -> 'þ' (0xe7)
    -24 -> 'Þ' (0xe8)
    -23 -> 'Ú' (0xe9)
    -22 -> 'Û' (0xea)
    -21 -> 'Ù' (0xeb)
    -20 -> 'ý' (0xec)
    -19 -> 'Ý' (0xed)
    -18 -> '¯' (0xee)
    -17 -> '´' (0xef)
    -16 -> '­' (0xf0)
    -15 -> '±' (0xf1)
    -14 -> '‗' (0xf2)
    -13 -> '¾' (0xf3)
    -12 -> '¶' (0xf4)
    -11 -> '§' (0xf5)
    -10 -> '÷' (0xf6)
    -9 -> '¸' (0xf7)
    -8 -> '°' (0xf8)
    -7 -> '¨' (0xf9)
    -6 -> '·' (0xfa)
    -5 -> '¹' (0xfb)
    -4 -> '³' (0xfc)
    -3 -> '²' (0xfd)
    -2 -> '■' (0xfe)
    -1 -> ' ' (0xff)
    0 -> '' (0x0)
    1 -> '' (0x1)
    2 -> '' (0x2)
    3 -> '' (0x3)
    4 -> '' (0x4)
    5 -> '' (0x5)
    6 -> '' (0x6)
    7 -> '' (0x7)
    8 -> ' (0x8)
    9 -> '       ' (0x9)
    10 -> '
    ' (0xa)
    11 -> '
    ' (0xb)
    12 -> '
    ' (0xc)
    ' (0xd) '
    14 -> '' (0xe)
    15 -> '' (0xf)
    16 -> '' (0x10)
    17 -> '' (0x11)
    18 -> '' (0x12)
    19 -> '' (0x13)
    20 -> '' (0x14)
    21 -> '' (0x15)
    22 -> '' (0x16)
    23 -> '' (0x17)
    24 -> '' (0x18)
    25 -> '' (0x19)
    26 -> '␦' (0x1a)
    27 -> 'x1b)
    28 -> '' (0x1c)
    29 -> '' (0x1d)
    30 -> '' (0x1e)
    31 -> '' (0x1f)
    32 -> ' ' (0x20)
    33 -> '!' (0x21)
    34 -> '"' (0x22)
    35 -> '#' (0x23)
    36 -> '$' (0x24)
    37 -> '%' (0x25)
    38 -> '&' (0x26)
    39 -> ''' (0x27)
    40 -> '(' (0x28)
    41 -> ')' (0x29)
    42 -> '*' (0x2a)
    43 -> '+' (0x2b)
    44 -> ',' (0x2c)
    45 -> '-' (0x2d)
    46 -> '.' (0x2e)
    47 -> '/' (0x2f)
    48 -> '0' (0x30)
    49 -> '1' (0x31)
    50 -> '2' (0x32)
    51 -> '3' (0x33)
    52 -> '4' (0x34)
    53 -> '5' (0x35)
    54 -> '6' (0x36)
    55 -> '7' (0x37)
    56 -> '8' (0x38)
    57 -> '9' (0x39)
    58 -> ':' (0x3a)
    59 -> ';' (0x3b)
    60 -> '<' (0x3c)
    61 -> '=' (0x3d)
    62 -> '>' (0x3e)
    63 -> '?' (0x3f)
    64 -> '@' (0x40)
    65 -> 'A' (0x41)
    66 -> 'B' (0x42)
    67 -> 'C' (0x43)
    68 -> 'D' (0x44)
    69 -> 'E' (0x45)
    70 -> 'F' (0x46)
    71 -> 'G' (0x47)
    72 -> 'H' (0x48)
    73 -> 'I' (0x49)
    74 -> 'J' (0x4a)
    75 -> 'K' (0x4b)
    76 -> 'L' (0x4c)
    77 -> 'M' (0x4d)
    78 -> 'N' (0x4e)
    79 -> 'O' (0x4f)
    80 -> 'P' (0x50)
    81 -> 'Q' (0x51)
    82 -> 'R' (0x52)
    83 -> 'S' (0x53)
    84 -> 'T' (0x54)
    85 -> 'U' (0x55)
    86 -> 'V' (0x56)
    87 -> 'W' (0x57)
    88 -> 'X' (0x58)
    89 -> 'Y' (0x59)
    90 -> 'Z' (0x5a)
    91 -> '[' (0x5b)
    92 -> '\' (0x5c)
    93 -> ']' (0x5d)
    94 -> '^' (0x5e)
    95 -> '_' (0x5f)
    96 -> '`' (0x60)
    97 -> 'a' (0x61)
    98 -> 'b' (0x62)
    99 -> 'c' (0x63)
    100 -> 'd' (0x64)
    101 -> 'e' (0x65)
    102 -> 'f' (0x66)
    103 -> 'g' (0x67)
    104 -> 'h' (0x68)
    105 -> 'i' (0x69)
    106 -> 'j' (0x6a)
    107 -> 'k' (0x6b)
    108 -> 'l' (0x6c)
    109 -> 'm' (0x6d)
    110 -> 'n' (0x6e)
    111 -> 'o' (0x6f)
    112 -> 'p' (0x70)
    113 -> 'q' (0x71)
    114 -> 'r' (0x72)
    115 -> 's' (0x73)
    116 -> 't' (0x74)
    117 -> 'u' (0x75)
    118 -> 'v' (0x76)
    119 -> 'w' (0x77)
    120 -> 'x' (0x78)
    121 -> 'y' (0x79)
    122 -> 'z' (0x7a)
    123 -> '{' (0x7b)
    124 -> '|' (0x7c)
    125 -> '}' (0x7d)
    126 -> '~' (0x7e)
    127 -> '' (0x7f)
    */
    switch(c) {
        case -128: str = "\u00C7"; break; // Ç
        case -127: str = "\u00FC"; break; // ü
        case -126: str = "\u00E9"; break; // é
        case -125: str = "\u00E2"; break; // â
        case -124: str = "\u00E4"; break; // ä
        case -123: str = "\u00E0"; break; // à
        case -122: str = "\u00E5"; break; // å
        case -121: str = "\u00E7"; break; // ç
        case -120: str = "\u00EA"; break; // ê
        case -119: str = "\u00EB"; break; // ë
        case -118: str = "\u00E8"; break; // è
        case -117: str = "\u00EF"; break; // ï
        case -116: str = "\u00EE"; break; // î
        case -115: str = "\u00EC"; break; // ì
        case -114: str = "\u00C4"; break; // Ä
        case -113: str = "\u00C5"; break; // Å
        case -112: str = "\u00C9"; break; // É
        case -111: str = "\u00E6"; break; // æ
        case -110: str = "\u00C6"; break; // Æ
        case -109: str = "\u00F4"; break; // ô
        case -108: str = "\u00F6"; break; // ö
        case -107: str = "\u00F2"; break; // ò
        case -106: str = "\u00FB"; break; // û
        case -105: str = "\u00F9"; break; // ù
        case -104: str = "\u00FF"; break; // ÿ
        case -103: str = "\u00D6"; break; // Ö
        case -102: str = "\u00DC"; break; // Ü
        case -101: str = "\u00F8"; break; // ø
        case -100: str = "\u00A3"; break; // £
        case -99: str = "\u00D8"; break; // Ø
        case -98: str = "\u00D7"; break; // ×
        case -97: str = "\u0192"; break; // ƒ
        case -96: str = "\u00E1"; break; // á
        case -95: str = "\u00ED"; break; // í
        case -94: str = "\u00F3"; break; // ó
        case -93: str = "\u00FA"; break; // ú
        case -92: str = "\u00F1"; break; // ñ
        case -91: str = "\u00D1"; break; // Ñ
        case -90: str = "\u00AA"; break; // ª
        case -89: str = "\u00BA"; break; // º
        case -88: str = "\u00BF"; break; // ¿
        case -87: str = "\u00AE"; break; // ®
        case -86: str = "\u00AC"; break; // ¬
        case -85: str = "\u00BD"; break; // ½
        case -84: str = "\u00BC"; break; // ¼
        case -83: str = "\u00A1"; break; // ¡
        case -82: str = "\u00AB"; break; // «
        case -81: str = "\u00BB"; break; // »
        case -80: str = "\u2591"; break; // ░
        case -79: str = "\u2592"; break; // ▒
        case -78: str = "\u2593"; break; // ▓
        case -77: str = "\u2502"; break; // │
        case -76: str = "\u2524"; break; // ┤
        case -75: str = "\u00C1"; break; // Á
        case -74: str = "\u00C2"; break; // Â
        case -73: str = "\u00C0"; break; // À
        case -72: str = "\u00A9"; break; // ©
        case -71: str = "\u2563"; break; // ╣
        case -70: str = "\u2551"; break; // ║
        case -69: str = "\u2557"; break; // ╗
        case -68: str = "\u255D"; break; // ╝
        case -67: str = "\u00A4"; break; // ¤
        case -66: str = "\u00A5"; break; // ¥
        case -65: str = "\u2510"; break; // ┐
        case -64: str = "\u2514"; break; // └
        case -63: str = "\u2534"; break; // ┴
        case -62: str = "\u252C"; break; // ┬
        case -61: str = "\u251C"; break; // ├
        case -60: str = "\u2500"; break; // ─
        case -59: str = "\u253C"; break; // ┼
        case -58: str = "\u00E3"; break; // ã
        case -57: str = "\u00C3"; break; // Ã
        case -56: str = "\u255A"; break; // ╚
        case -55: str = "\u2554"; break; // ╔
        case -54: str = "\u2569"; break; // ╩
        case -53: str = "\u2566"; break; // ╦
        case -52: str = "\u2560"; break; // ╠
        case -51: str = "\u2550"; break; // ═
        case -50: str = "\u256C"; break; // ╬
        case -49: str = "\u00A4"; break; // ¤
        case -48: str = "\u00F0"; break; // ð
        case -47: str = "\u00D0"; break; // Ð
        case -46: str = "\u00CA"; break; // Ê
        case -45: str = "\u00CB"; break; // Ë
        case -44: str = "\u00C8"; break; // È
        case -43: str = "\u0131"; break; // ı
        case -42: str = "\u00CD"; break; // Í
        case -41: str = "\u00CE"; break; // Î
        case -40: str = "\u00CF"; break; // Ï
        case -39: str = "\u2518"; break; // ┘
        case -38: str = "\u250C"; break; // ┌
        case -37: str = "\u2588"; break; // █
        case -36: str = "\u2584"; break; // ▄
        case -35: str = "\u00A6"; break; // ¦
        case -34: str = "\u00CC"; break; // Ì
        case -33: str = "\u2580"; break; // ▀
        case -32: str = "\u00D3"; break; // Ó
        case -31: str = "\u00DF"; break; // ß
        case -30: str = "\u00D4"; break; // Ô
        case -29: str = "\u00D2"; break; // Ò
        case -28: str = "\u00F5"; break; // õ
        case -27: str = "\u00D5"; break; // Õ
        case -26: str = "\u00B5"; break; // µ
        case -25: str = "\u00FE"; break; // þ
        case -24: str = "\u00DE"; break; // Þ
        case -23: str = "\u00DA"; break; // Ú
        case -22: str = "\u00DB"; break; // Û
        case -21: str = "\u00D9"; break; // Ù
        case -20: str = "\u00FD"; break; // ý
        case -19: str = "\u00DD"; break; // Ý
        case -18: str = "\u00AF"; break; // ¯
        case -17: str = "\u00B4"; break; // ´
        case -16: str = "\u00AD"; break; // ­
        case -15: str = "\u00B1"; break; // ±
        case -14: str = "\u2017"; break; // ‗
        case -13: str = "\u00BE"; break; // ¾
        case -12: str = "\u00B6"; break; // ¶
        case -11: str = "\u00A7"; break; // §
        case -10: str = "\u00F7"; break; // ÷
        case -9: str = "\u00B8"; break; // ¸
        case -8: str = "\u00B0"; break; // °
        case -7: str = "\u00A8"; break; // ¨
        case -6: str = "\u00B7"; break; // ·
        case -5: str = "\u00B9"; break; // ¹
        case -4: str = "\u00B3"; break; // ³
        case -3: str = "\u00B2"; break; // ²
        case -2: str = "\u25A0"; break; // ■
        case -1: str = "\u00A0"; break; // ' ' (NB: non-breaking space)
        default:
            str = c;
            break;
    }
    return str;
}
#endif

#endif // PWETTY_H
