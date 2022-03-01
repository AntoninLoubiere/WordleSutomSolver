#include "src/game.h"
#include "src/gameResolver.h"
#include "src/utils.h"
#include "src/word_list.h"
#include <chrono>
#include <iostream>

using namespace std;
int autoGame(const WordList &wordList, int word = -1);

double doAllGames(const WordList &wordList) {
    int sum = 0;
    auto pos = wordList.initialCompatibleWords();
    for (int &w : pos) {
        sum += autoGame(wordList, w);
    }
    const double av = (double)sum / pos.size();
    cout << "Score moyen: " << av << endl;
    return av;
}

int autoGame(const WordList &wordList, int word) {
    Game game(wordList, 20);
    GameResolver gameResolver(wordList);
    // double entropy[20] = {0.};
    // entropy[game.numberSteps()] = gameResolver.entropy();
    while (game.gameStatus() <= 0) {
        gameResolver.update(game.update(gameResolver.bestChoice().word));
        // entropy[game.numberSteps()] = gameResolver.entropy();
    }
    // for (int i = 0; i < 20; i++) {
    //     const double e = entropy[i];
    //     if (e <= 0)
    //         break;
    //     cout << e << "\t" << game.numberSteps() - i << "\n";
    // }
    // cout << game.numberSteps() << "\n";
    cout << game.numberSteps() << " - " << wordList.getWord(game.word()).word << "\n";
    return game.numberSteps();
}

void autoGamePrint(const WordList &wordList) {
    Game game(wordList, 20);
    GameResolver gameResolver(wordList);

    while (game.gameStatus() <= 0) {
        cout << "\n"
             << gameResolver.possibilitiesCount() << " possibilités (" << gameResolver.entropy()
             << " bits).\nMot choisi:\n";
        Result result = gameResolver.bestChoice();
        cout << wordList.getWord(result.word).word << " (" << result.score << " bits)\n";
        const Step step = game.update(result.word);
        cout << wordList.patternToString(step) << "\n";
        gameResolver.update(step);
    }

    if (game.gameStatus() == 2) {
        cout << "Gagné en " << game.numberSteps() << " étapes.\n";
    } else {
        cout << "Perdu, le mot était " << game.word() << "\n";
    }
}

int main(int argc, const char **argv) {
    int nbLetters;
    string mask;
    cout << "Nombre de lettres : ";
    cin >> nbLetters;
    cout << "Masque : ";
    cin >> mask;
    WordList wordList(nbLetters, mask);

    // double av = 0;

    // auto clock = chrono::steady_clock();
    // for (int i = 0; i < 20; i++) {
    //     const auto start = clock.now();

    //     const EntropyResult bestResult = wordList.maxEntropy();
    //     cout << "Best result: " << wordList.getWord(bestResult.word).word << " ("
    //          << bestResult.entropy << " bits)\n";

    //     const auto end = clock.now();
    //     chrono::nanoseconds dt = end - start;
    //     const double time = double(dt.count()) / 1'000'000;
    //     cout << "Time spent: " << time << "ms" << endl;
    //     av += time;
    // }
    // cout << "Average: " << av / 20 << endl;
    TerminalGameResolver gameResolver(wordList);
    while (true) {
        gameResolver.play();
    }
    // doAllGames(wordList);
    // int total = 0;
    // const int IT = 100;
    // for (int i = 0; i < IT; i++) {
    //     const int score = autoGame(wordList);
    //     total += score;
    //     cout << "Moyenne: " << (double)total / (i + 1) << "\n";
    // }
}
