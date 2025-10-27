#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace std;

// Carica parole da file
vector<string> loadWords(const string &filename) {
    vector<string> words;
    ifstream file(filename);
    string word;
    while (getline(file, word)) {
        if (word.length() == 8) {
            transform(word.begin(), word.end(), word.begin(), ::toupper);
            words.push_back(word);
        }
    }
    return words;
}

// Conta quante lettere coincidono nella posizione giusta
int countCorrectPositions(const string &guess, const string &candidate) {
    int correct = 0;
    for (int i = 0; i < 8; i++) {
        if (guess[i] == candidate[i])
            correct++;
    }
    return correct;
}

// Calcola quante lettere diverse ci sono (più è alto, meglio è per il guess)
int uniqueLetterScore(const string &word) {
    set<char> letters(word.begin(), word.end());
    return (int)letters.size();
}

// Sceglie un “miglior tentativo” in base alla varietà di lettere
string chooseBestGuess(const vector<string> &words) {
    if (words.empty()) return "";

    string best = words[0];
    int bestScore = uniqueLetterScore(best);

    for (const auto &w : words) {
        int score = uniqueLetterScore(w);
        if (score > bestScore) {
            best = w;
            bestScore = score;
        }
    }
    return best;
}

int main() {
    string allowedLetters = "335AACEE";
    vector<string> words = loadWords("words.txt");

    if (words.empty()) {
        cout << "Errore: nessuna parola valida trovata in words.txt.\n";
        return 1;
    }

    cout << "Gioco Wordle (8 lettere)\n";
    cout << "Lettere disponibili: " << allowedLetters << endl;

    vector<string> possibleWords = words;

    while (true) {
        string guess;
        cout << "\nInserisci il tuo tentativo (8 lettere): ";
        cin >> guess;
        transform(guess.begin(), guess.end(), guess.begin(), ::toupper);

        if (guess.length() != 8) {
            cout << "La parola deve avere 8 lettere.\n";
            continue;
        }

        // Controlla che la parola usi solo le lettere permesse
        string tempAllowed = allowedLetters;
        bool valid = true;
        for (char c : guess) {
            size_t pos = tempAllowed.find(c);
            if (pos != string::npos)
                tempAllowed.erase(pos, 1);
            else {
                valid = false;
                break;
            }
        }
        if (!valid) {
            cout << "Errore: la parola contiene lettere non permesse.\n";
            continue;
        }

        int correctCount;
        cout << "Quante lettere sono nella posizione corretta? ";
        cin >> correctCount;

        vector<string> newList;
        for (const string &w : possibleWords) {
            if (countCorrectPositions(guess, w) == correctCount)
                newList.push_back(w);
        }

        possibleWords = newList;

        cout << "\nRestano " << possibleWords.size() << " combinazioni possibili.\n";

        if (possibleWords.empty()) {
            cout << "Nessuna parola compatibile trovata.\n";
            break;
        }

        if (correctCount == 8) {
            cout << "\nHai trovato la parola giusta! 🎉\n";
            break;
        }

        // Scegli un tentativo consigliato
        string nextGuess = chooseBestGuess(possibleWords);
        cout << "Prossimo tentativo consigliato: " << nextGuess << endl;
    }

    return 0;
}
