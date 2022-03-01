#ifndef SRC_WORD_LIST_H_
#define SRC_WORD_LIST_H_

#include <fstream>
#include <list>
#include <ostream>
#include <string>
#include <vector>

struct Word {
    std::string word;
    float frq;
    double score;
};

std::istream &operator>>(std::istream &is, Word &word);

struct Result {
    int word;
    double score;
};

struct Step {
    int word;
    unsigned int pattern;
};

class WordList {
  public:
    WordList(unsigned int wordLength, const std::string &mask = "", bool loadFromCache = true,
             bool saveToCache = true);
    ~WordList();

    Word getWord(int index) const;
    int getWordIndex(const std::string &word) const;
    bool doWordExist(const std::string &word) const;
    unsigned int numberOfWords() const;
    unsigned int wordLength() const;
    std::vector<Word> words() const;
    int getWordPattern(const std::string &word1, const std::string &word2) const;
    int getWordPattern(int word1, int word2) const;
    double totalScore(const std::vector<int> &possibleWords) const;
    double entropy(int word) const;
    double entropy(int word, const std::vector<int> &possibleWords) const;
    double entropy(const std::vector<int> &words) const;
    double score(int word) const;
    double score(int word, const std::vector<int> &possibleWords) const;
    Result topWord() const;
    Result topWord(std::vector<int> possibleWords) const;
    std::list<Result> topWords(unsigned int number = 10) const;
    std::list<Result> topWords(const std::vector<int> &possibleWords,
                               unsigned int number = 10) const;
    bool isWordCompatible(int word, const Step &step) const;
    bool isWordCompatible(int word, const std::vector<Step> &steps) const;
    std::vector<int> compatibleWords(const std::vector<int> &possibilities, const Step &step) const;
    std::vector<int> initialCompatibleWords() const;
    std::string patternToString(const Step &step) const;
    std::string patternToString(int word, int pattern) const;

    void load(unsigned int wordLength, const std::string &mask = "", bool loadFromCache = true,
              bool saveToCache = true);

  private:
    void cleanMask(const std::string &mask);
    void loadWords();
    bool loadPatterns();
    void generatePatterns(bool saveToCache = true);

    bool isWordValid(const Word &word) const;
    unsigned int m_wordsLength;
    std::string m_mask;
    std::vector<Word> m_words;
    std::vector<int> m_wordsValids;
    unsigned int m_numberWords;
    // matrix of all pattern :
    // [word1 index + word 2 index * total words] = wordPattern(word1, word2);
    unsigned int *m_patternCache;
};

bool compareWords(const Word &a, const Word &b);
std::string wordsPath(int wordLength);
std::string wordsMatrixPath(int wordLength);
std::string cleanMask(const unsigned int wordLength, const std::string &mask);

#endif // !SRC_WORD_LIST_H_