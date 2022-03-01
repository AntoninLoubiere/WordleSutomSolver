#ifndef SRC_GAME_H_
#define SRC_GAME_H_

#include "word_list.h"
#include <string>
#include <vector>

class Game {
  public:
    Game(const WordList &wordList, int maxSteps = -1);

    void reset(int word = -1);
    // return the pattern that gives the word
    Step update(int word);
    // 0 - in progress / aborted, 1 - loss, 2 - win
    int gameStatus() const;
    int getWordPattern(int word);
    int word() const;
    unsigned int numberSteps() const;

  protected:
    const WordList &m_wordList;
    const int m_maxSteps;

  private:
    std::vector<Step> m_steps;
    int m_word;
};

class TerminalGame : private Game {
  public:
    TerminalGame(const WordList &wordList, int maxSteps = -1);

    void play(bool allowRestart = true);

  private:
    int inputWord(const std::string &prompt) const;
};

#endif // !SRC_GAME_H_