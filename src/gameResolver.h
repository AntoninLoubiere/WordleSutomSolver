#ifndef SRC_GAMERESOLVER_H_
#define SRC_GAMERESOLVER_H_

#include "game.h"
#include "word_list.h"
#include <vector>

class GameResolver {
  public:
    GameResolver(const WordList &wordList);

    void reset();
    void update(int word, int pattern);
    void update(Step step);
    Result bestChoice() const;
    int possibilitiesCount() const;
    double entropy() const;
    void cancelSteps(int number = 1);

  protected:
    void invalidatePossibilities();

    const WordList &m_wordList;
    std::vector<Step> m_steps;
    std::vector<int> m_possibilities;
};

class TerminalGameResolver : private GameResolver {
  public:
    TerminalGameResolver(const WordList &wordList);

    void play();
    Step inputWord(const std::string &prompt);
    int inputPattern() const;

  private:
};

#endif // !SRC_GAMERESOLVER_H_