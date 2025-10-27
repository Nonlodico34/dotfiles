#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <vector>
#include <string>

struct TermiosGuard {
    struct termios orig_term;
    TermiosGuard() {
        tcgetattr(STDIN_FILENO, &orig_term);
        struct termios raw = orig_term;
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        // Abilita mouse tracking SGR + motion reporting
        std::cout << "\033[?1000h\033[?1006h\033[?1003h";
        std::cout.flush();
    }

    ~TermiosGuard() {
        std::cout << "\033[?1000l\033[?1006l\033[?1003l";
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
    }
};

int readCharNonBlocking() {
    char c;
    int n = read(STDIN_FILENO, &c, 1);
    if (n == 1) return (unsigned char)c;
    return -1;
}

struct MouseEvent {
    int x = 0, y = 0;
    std::string button = "";
};

// Parse escape sequence per mouse
MouseEvent parseMouse(const std::string &buf) {
    MouseEvent me;
    if (buf.size() < 6 || buf[0] != '\033' || buf[1] != '[' || buf[2] != '<') return me;

    size_t semi1 = buf.find(';', 3);
    size_t semi2 = buf.find(';', semi1 + 1);
    if (semi1 == std::string::npos || semi2 == std::string::npos) return me;

    int b = std::stoi(buf.substr(3, semi1 - 3));
    me.x = std::stoi(buf.substr(semi1 + 1, semi2 - semi1 - 1));
    me.y = std::stoi(buf.substr(semi2 + 1, buf.size() - semi2 - 2));

    switch (b & 0b11) {
        case 0: me.button = "Left"; break;
        case 1: me.button = "Middle"; break;
        case 2: me.button = "Right"; break;
        case 3: me.button = "Release"; break;
    }
    if (buf.back() == 'm') me.button += " (released)";

    // Se non è release/click, è solo movimento
    return me;
}

void printStatus(int mx, int my, const std::string &button, const std::vector<int> &keys) {
    std::cout << "\033[H"; // Cursor top-left
    std::cout << "Mouse: " << mx << "," << my << " " << button << "           \n";
    std::cout << "Keys pressed: ";
    for (int k : keys) {
        if (k >= 32 && k <= 126) std::cout << (char)k << " ";
        else std::cout << "[" << k << "] ";
    }
    std::cout << "        " << std::flush;
}

int main() {
    TermiosGuard tg;

    int mx = 0, my = 0;
    std::string mouseButton = "";
    std::vector<int> keysPressed;
    std::string inputBuffer;

    std::cout << "\033[2J"; // Clear screen

    while (true) {
        // Legge tutto il buffer disponibile
        MouseEvent latestMotion;
        while (true) {
            int c = readCharNonBlocking();
            if (c == -1) break;

            if (c == 27) { // ESC
                inputBuffer = "\033";
                int c2 = readCharNonBlocking();
                if (c2 != -1) {
                    inputBuffer += (char)c2;
                    if (c2 == '[' || c2 == 'O') {
                        while (true) {
                            int c3 = readCharNonBlocking();
                            if (c3 == -1) break;
                            inputBuffer += (char)c3;
                            if ((c3 >= 64 && c3 <= 126)) break;
                        }
                    }
                }

                MouseEvent me = parseMouse(inputBuffer);
                    mx = me.x;
                    my = me.y;
                    mouseButton = me.button;
                    latestMotion = me;
                inputBuffer.clear();
            } else { // tasti
                keysPressed.push_back(c);
            }
        }

        printStatus(mx, my, mouseButton, keysPressed);
        usleep(20000); // 20ms refresh
    }

    return 0;
}
