#include <iostream>

int main() {
    // ╠ in UTF-8 è 0xE2 0x95 0xA0
    std::cout << "\xE2\x95\xA0" << std::endl;
    return 0;
}
