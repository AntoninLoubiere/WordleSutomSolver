#include "game.h"
#include "utils.h"
#include "word_list.h"
#include <algorithm>
#include <cctype>
#include <iostream>

using namespace std;

Game::Game(const WordList &wordList, int maxSteps)
    : m_wordList(wordList), m_maxSteps(maxSteps), m_steps() {
    reset();
};

void Game::reset(int w) {
    m_steps.clear();
    // get a random word
    if (w >= 0) {
        m_word = w;
    } else {
        Word word;
        int i = 0;
        while (i++ < 5000) {
            m_word = randomInt(0, m_wordList.numberOfWords());
            word = m_wordList.getWord(m_word);
            if (word.frq > 10)
                break;
        }
    }
#if DEBUG
    cout << "Current word: " << m_word << "\n";
#endif
}

int Game::gameStatus() const {
    if (m_steps.size() > 0 && m_steps.back().word == m_word)
        return 2;
    else if (m_maxSteps >= 0 && (int)m_steps.size() >= m_maxSteps)
        return 1;
    return 0;
}

int Game::word() const { return m_word; }
unsigned int Game::numberSteps() const { return m_steps.size(); }

Step Game::update(int word) {
    Step step;
    step.word = word;
    step.pattern = getWordPattern(word);
    m_steps.push_back(step);
    return step;
}

int Game::getWordPattern(int word) { return m_wordList.getWordPattern(m_word, word); }

TerminalGame::TerminalGame(const WordList &wordList, int maxSteps) : Game(wordList, maxSteps) {}

int TerminalGame::inputWord(const string &prompt) const {
    string word;
    while (true) {
        cout << prompt;
        cin >> word;
        for_each(word.begin(), word.end(), [](char &c) { c = toupper(c); });

        const unsigned int size = word.size();
        int index;
        if (size == 0)
            cout << "Pour quittez entrez \"q\".\n";
        else if (word == "Q")
            return -1;
        else if (size != m_wordList.wordLength())
            cout << "Vous devez rentrez un mot de " << m_wordList.wordLength()
                 << " caractères (et non " << size << ")\n";
        else if ((index = m_wordList.getWordIndex(word)) >= 0)
            return index;
        else
            cout << "Le mot " << word << " n'existe pas dans ce dictionnaire.\n";
    }
}

void TerminalGame::play(bool allowRestart) {
    int status;
    while (!(status = gameStatus())) {
        cout << "\nEssai " << numberSteps() + 1;
        if (m_maxSteps > 0)
            cout << "/" << m_maxSteps;
        cout << "\n";
        const int word = inputWord("Proposer un mot : ");
        if (word < 0) {
            cout << "Quitter.\n";
            break;
        }
        const int result = update(word).pattern;
        if (result == pow(3, m_wordList.wordLength()) - 1) {
            continue;
        }
        cout << m_wordList.getWord(word).word << "\n";
        cout << m_wordList.patternToString(word, result) << "\n";
    }
    if (status == 2) {
        const int steps = numberSteps();
        cout << "Vous avez gagné en " << steps << (steps > 1 ? " coups" : " coup")
             << ", le mot était bien " << word() << ".\n";
    } else {
        cout << "Vous avez perdu, le mot était " << word() << "\n";
    }
}
