#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

int main() {
    // I simboli iniziali (con ripetizioni)
    std::vector<char> symbols = {'3', '3', '5', 'A', 'A', 'C', 'E', 'E'};

    // Ordiniamo per usare next_permutation correttamente
    std::sort(symbols.begin(), symbols.end());

    std::ofstream outfile("words.txt");
    if (!outfile) {
        std::cerr << "Errore: impossibile creare words.txt\n";
        return 1;
    }

    // Conta quante permutazioni vengono generate
    size_t count = 0;

    do {
        for (char c : symbols)
            outfile << c;
        outfile << '\n';
        ++count;
    } while (std::next_permutation(symbols.begin(), symbols.end()));

    outfile.close();

    std::cout << "Generate " << count << " permutazioni distinte e salvate in words.txt\n";
    return 0;
}
