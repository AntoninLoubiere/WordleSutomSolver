#include "gameResolver.h"
#include "iostream"
#include "utils.h"
#include "word_list.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <vector>

using namespace std;

GameResolver::GameResolver(const WordList &wordList)
    : m_wordList(wordList), m_steps(), m_possibilities(wordList.numberOfWords()) {
    reset();
}

void GameResolver::reset() {
    m_steps.clear();
    invalidatePossibilities();
}

void GameResolver::invalidatePossibilities() {
    m_possibilities.clear();
    // const int size = m_wordList.numberOfWords();
    vector<int> compatibleWords = m_wordList.initialCompatibleWords();
    for (int i : compatibleWords) {
        if (m_wordList.isWordCompatible(i, m_steps))
            m_possibilities.push_back(i);
    }
}

void GameResolver::update(int word, int pattern) {
    Step step;
    step.word = word;
    step.pattern = pattern;
    update(step);
}
void GameResolver::update(Step step) {
    m_steps.push_back(step);
    m_possibilities = m_wordList.compatibleWords(m_possibilities, step);
}

void GameResolver::cancelSteps(int number) {
    while (number && m_steps.size()) {
        m_steps.pop_back();
        number--;
    }

    invalidatePossibilities();
}

Result GameResolver::bestChoice() const {
    const int size = m_possibilities.size();
    if (size == 1) {
        Result result;
        result.word = m_possibilities.front();
        result.score = 0;
        return result;
    } else if (size <= 0) {
        throw runtime_error("Not enough possibilities to choose.");
    }
    return m_wordList.topWord(m_possibilities);
}
int GameResolver::possibilitiesCount() const { return m_possibilities.size(); }
double GameResolver::entropy() const { return m_wordList.entropy(m_possibilities); }

TerminalGameResolver::TerminalGameResolver(const WordList &wordList) : GameResolver(wordList) {}

void TerminalGameResolver::play() {
    reset();
    int possibilities;
    while ((possibilities = m_possibilities.size()) > 1) {
        Result bestChoice = this->bestChoice();
        cout << "\n" << possibilities << " possibilités (" << entropy() << " bits).\n";
        cout << "Meilleure option: " << m_wordList.getWord(bestChoice.word).word << " ("
             << bestChoice.score + m_steps.size() << " coups).\n";
        Step step = inputWord("Entrez le mot choisi : ");
        if (step.word < 0 || step.pattern < 0) {
            cout << "Quitter\n";
            return;
        }

        update(step);
    }

    if (possibilities == 1) {
        cout << "\nUnique mot restant: " << m_wordList.getWord(m_possibilities[0]).word << "\n";
    } else if (possibilities <= 0) {
        cout << "\nAucun mot restant !\n";
    }
}

Step TerminalGameResolver::inputWord(const std::string &prompt) {
    string word;
    while (true) {
        cout << prompt;
        cin >> word;
        for_each(word.begin(), word.end(), [](char &c) { c = toupper(c); });

        const unsigned int size = word.size();
        int index;
        if (size == 0)
            cout << "Pour quittez entrez \"q\".\nPour annuler la dernière étape, tapez "
                    "\"c\".\nPour voir les possibilités tapez \"p\".\nPour voir les suggestions "
                    "tapez \"s\".\n";
        else if (word == "Q") {
            Step step;
            step.word = -1;
            return step;
        } else if (word == "C") {
            cancelSteps();
            cout << "Dernière étape annulée.\n";
        } else if (word == "P") {
            const int numberToShow = 10;
            const int possibilitiesSize = m_possibilities.size();
            const double totalScore = m_wordList.totalScore(m_possibilities);
            const int currentNumberSteps = m_steps.size();

            for (int i = 0; i < numberToShow && i < possibilitiesSize; i++) {
                const Word word = m_wordList.getWord(m_possibilities[i]);
                cout << word.word << " ("
                     << m_wordList.score(m_possibilities[i], m_possibilities) + currentNumberSteps
                     << " coups - " << word.score / totalScore * 100 << "%)\n";
            }
            int remaining = possibilitiesSize - numberToShow;
            if (remaining > 0) {
                cout << "et " << remaining << " de plus.";
            }
            cout << "\n";
        } else if (word == "S") {
            cout << "Suggestions :\n";
            const int currentNumberSteps = m_steps.size();
            list<Result> choices = m_wordList.topWords(m_possibilities);
            for (Result &result : choices) {
                cout << m_wordList.getWord(result.word).word << " ("
                     << result.score + currentNumberSteps << " coups).\n";
            }
        } else if (size != m_wordList.wordLength())
            cout << "Vous devez rentrez un mot de " << m_wordList.wordLength()
                 << " caractères (et non " << size << ")\n";
        else if ((index = m_wordList.getWordIndex(word)) >= 0) {
            Step step;
            step.word = index;
            step.pattern = inputPattern();
            return step;
        } else
            cout << "Le mot " << word << " n'existe pas dans ce dictionnaire.\n";
    }
}

int TerminalGameResolver::inputPattern() const {
    string input;
    while (true) {
        cout << "Entrez le motif reçu (./a/A): ";
        cin >> input;

        const unsigned int size = input.size();
        if (size == 0)
            cout << "Pour quittez entrez \"q\".\n    .     -> Lettre invalide.\na,b,c,... -> "
                    "Lettre mal placée.\nA,B,C... -> Lettre bien placé.\n";
        else if (input == "Q" || input == "q") {
            return -1;
        } else if (size != m_wordList.wordLength())
            cout << "Vous devez rentrez un motif de " << m_wordList.wordLength()
                 << " caractères (et non " << size << ")\n";
        else {
            int pattern = 0;
            for (unsigned int i = 0; i < size; i++) {
                char c = input[i];
                if ('a' <= c && c <= 'z') {
                    pattern += ::pow(3, i);
                } else if ('A' <= c && c <= 'Z') {
                    pattern += 2 * ::pow(3, i);
                } else if (c != '.') {
                    cout << "    .     -> Lettre invalide.\na,b,c,... -> "
                            "Lettre mal placée.\nA,B,C... -> Lettre bien placé.\n";
                    pattern = -1;
                    break;
                }
            }
            if (pattern < 0)
                continue;
            d_assert(0 <= pattern && pattern < pow(3, size));
            return pattern;
        }
    }
}