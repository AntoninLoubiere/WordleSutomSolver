#include "word_list.h"
#include "utils.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

constexpr int patterCacheSize(int numberWords) { return numberWords * numberWords; }
constexpr double entropyToScore(double entropy) {
    if (entropy <= 1)
        return 1;
    return 0.9 * log(entropy) + 1.5;
}

#define WORDS_UNIFORM_SCORE false

#if !WORDS_UNIFORM_SCORE
const double WORD_SCORE_MUL = 3;
const double WORD_SCORE_OFFSET = -2;
#endif
string wordsPath(int wordLength) {
    d_assert(4 <= wordLength && wordLength <= 12);
    string filename = "data/words-" + to_string(wordLength) + ".txt";
    return filename;
}

string wordsMatrixPath(int wordLength) {
    d_assert(4 <= wordLength && wordLength <= 12);
    string filename = "data/words-" + to_string(wordLength) + "-patterns.cache.bin";
    return filename;
}

bool compareWords(const Word &a, const Word &b) { return a.word.compare(b.word) < 0; }

WordList::WordList(unsigned int wordLength, const std::string &mask, bool loadFromCache,
                   bool saveToCache)
    : m_words(), m_wordsValids(), m_patternCache(nullptr) {
    load(wordLength, mask, loadFromCache, saveToCache);
}

void WordList::load(unsigned int wordLength, const std::string &mask, bool loadFromCache,
                    bool saveToCache) {
    cout << "Loading word list.\n";
    auto clock = chrono::steady_clock();
    const auto start = clock.now();

    m_wordsLength = wordLength;
    cleanMask(mask);
    loadWords();
    if (!loadFromCache || !loadPatterns()) {
        generatePatterns(saveToCache);
    }
    // We remember compatible words.
    m_wordsValids.clear();
    for (unsigned int i = 0; i < m_numberWords; i++) {
        if (isWordValid(m_words[i]))
            m_wordsValids.push_back(i);
    }

    const auto end = clock.now();
    const chrono::nanoseconds dt = end - start;

    cout << "Loading finished in " << double(dt.count()) / 1'000'000 << "ms.\n";
}

void WordList::cleanMask(const string &mask) {
    if (mask.size() <= 0 || (mask.size() == 1 && mask[0] == '.')) {
        m_mask = "";
        return;
    }

    if (mask.size() != m_wordsLength) {
        cerr << "Mask should have the same length as the word.";
        m_mask = "";
        return;
    }

    bool hasLetters = false;
    for (unsigned int i = 0; i < m_wordsLength; i++) {
        char c = mask[i];
        if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
            hasLetters = true;
            break;
        }
    }
    if (hasLetters) {
        m_mask = mask;
    } else {
        m_mask = "";
    }
}

void WordList::loadWords() {
    cout << "Loading words.\n";
    const string filePath = wordsPath(m_wordsLength);
    ifstream file(filePath);
    istream_iterator<Word> it(file);
    copy_if(it, istream_iterator<Word>(), back_inserter(m_words),
            [this](const Word &word) { return word.word.size() == m_wordsLength; });
    sort(m_words.begin(), m_words.end(), &compareWords);
    m_numberWords = m_words.size();
}

bool WordList::loadPatterns() {
    cout << "Loading patterns.\n";
    const string matrixPath = wordsMatrixPath(m_wordsLength);
    ifstream matrixFile(matrixPath);
    if (!matrixFile) {
        cerr << "Cannot open the file \"" << matrixPath << "\" containing the matrix.\n";
        return false;
    }

    bool errors = false;
    const int matrixSize = patterCacheSize(m_numberWords);

    if (m_patternCache != nullptr)
        delete[] m_patternCache;
    m_patternCache = new unsigned int[matrixSize];

    char bytes;
    matrixFile >> bytes;

    if (bytes > 4 || !matrixFile) {
        errors = true;
    }

    for (int i = 0; i < matrixSize && !errors; i++) {
        matrixFile.read((char *)&m_patternCache[i], bytes);
        // check that we can continue reading.
        errors = !matrixFile && matrixFile.gcount() < matrixSize * bytes + 1;
    }

    if (errors) {
        delete[] m_patternCache;
        m_patternCache = nullptr;

        cerr << "An error occurred while reading the file \"" << matrixPath
             << "\" containing the matrix.\n";
        return false;
    }

    cout << "Patterns loaded from file.\n";
    return true;
}

void WordList::generatePatterns(bool saveToCache) {
    if (m_numberWords <= 0)
        return;

    // If we generate patterns we don't need to save the words that doesn't respect the mask.
    vector<Word> words;
    copy_if(m_words.begin(), m_words.end(), back_inserter(words),
            [this](const Word &word) { return isWordValid(word); });
    m_words = words;
    m_numberWords = m_words.size();

    cout << "Generating pattern matrix (" << m_numberWords << " words).\n";
    if (m_patternCache != nullptr)
        delete[] m_patternCache;
    m_patternCache = new unsigned int[patterCacheSize(m_numberWords)];

    for (unsigned int i = 0; i < m_numberWords; i++) {
        const string word1 = getWord(i).word;
        for (unsigned int j = 0; j < m_numberWords; j++) {
            m_patternCache[i + j * m_numberWords] =
                i == j ? ::pow(3, m_wordsLength) - 1 : getWordPattern(word1, getWord(j).word);
        }
    }

    // save matrix
    if (!m_mask.size() && saveToCache && m_numberWords) {
        cout << "Saving matrix.\n";
        ofstream cacheFile(wordsMatrixPath(m_wordsLength), ios::out | ios::binary);
        if (!cacheFile) {
            cerr << "Cannot open the cache file !\n";
            return;
        }

        const int sizeRequired = ::pow(3, m_wordsLength);
        char bytes = 0;
        while (::pow(2, bytes * 8) < sizeRequired) {
            bytes++;
        }
        cacheFile << bytes;
        const int size = patterCacheSize(m_numberWords);

        for (int i = 0; i < size; i++) {
            cacheFile.write((char *)&m_patternCache[i], bytes);
        }
        cout << "Matrix saved.\n";
    }
}

bool WordList::isWordValid(const Word &word) const {
    if (m_mask.size() != m_wordsLength)
        return true; // the mask is empty

    bool usedLetters[m_wordsLength];
    for (unsigned int i = 0; i < m_wordsLength; i++) {
        const char c = m_mask[i];
        if ('A' <= c && c <= 'Z') {
            if (c != word.word[i])
                return false;
            usedLetters[i] = true;
        } else {
            usedLetters[i] = false;
        }
    }
    for (unsigned int i = 0; i < m_wordsLength; i++) {
        char c = m_mask[i];
        if ('a' <= c && c <= 'z') {
            c = toupper(c);
            bool found = false;
            for (unsigned int j = 0; j < m_wordsLength; j++) {
                if (!usedLetters[j] && c == word.word[j]) {
                    found = true;
                    usedLetters[j] = true;
                    break;
                }
            }
            if (!found)
                return false;
        }
    }
    return true;
}

WordList::~WordList() {
    if (m_patternCache != nullptr)
        delete[] m_patternCache;
}

Word WordList::getWord(int index) const {
    d_assert(0 <= index && index < m_numberWords);
    return m_words[index];
}

int WordList::getWordIndex(const std::string &word) const {
    Word word_s;
    word_s.word = word;
    const auto it = lower_bound(m_words.begin(), m_words.end(), word_s, &compareWords);
    if (it->word == word) {
        d_assert(getWord(it - m_words.begin()).word == word);
        return it - m_words.begin();
    }
    return -1;
}

bool WordList::doWordExist(const std::string &word) const {
    Word word_s;
    word_s.word = word;
    return binary_search(m_words.begin(), m_words.end(), word_s, &compareWords);
}

int WordList::getWordPattern(const std::string &word1, const std::string &word2) const {
    d_assert(word1.size() == word2.size() and word1.size() == m_wordsLength);

    int pattern = 0;
    bool wordLetterUsed[m_wordsLength];
    bool inputtedWordLetterUsed[m_wordsLength];

    for (size_t i = 0; i < m_wordsLength; i++) {
        if (word2[i] == word1[i]) {
            inputtedWordLetterUsed[i] = true;
            wordLetterUsed[i] = true;
            pattern += 2 * ::pow(3, i);
        } else {
            inputtedWordLetterUsed[i] = false;
            wordLetterUsed[i] = false;
        }
    }

    for (size_t i = 0; i < m_wordsLength; i++) {
        char c = word2[i];
        if (inputtedWordLetterUsed[i])
            continue;

        for (size_t j = 0; j < m_wordsLength; j++) {
            if (c == word1[j] && !wordLetterUsed[j]) {
                // will not be used later so useless
                // inputtedWordLetterUsed[i] = true;
                wordLetterUsed[j] = true;
                pattern += ::pow(3, i);
                break;
            }
        }
    }

    d_assert(0 <= pattern && pattern < ::pow(3, m_wordsLength));
    return pattern;
}

int WordList::getWordPattern(int word1, int word2) const {
    d_assert(0 <= word1 && word1 < m_numberWords);
    d_assert(0 <= word2 && word2 < m_numberWords);
    d_assert_l(m_patternCache[word1 + word2 * m_numberWords] ==
                   getWordPattern(getWord(word1).word, getWord(word2).word),
               20);

    return m_patternCache[word1 + word2 * m_numberWords];
}

unsigned int WordList::numberOfWords() const {
    d_assert(m_numberWords == (int)m_words.size());
    return m_numberWords;
}

double WordList::totalScore(const std::vector<int> &possibleWords) const {
    double totalScore = 0;
    const int size = possibleWords.size();
    for (int i = 0; i < size; i++) {
        totalScore += m_words[possibleWords[i]].score;
    }
    return totalScore;
}

double WordList::entropy(int word) const { return entropy(word, initialCompatibleWords()); }

double WordList::entropy(int word, const vector<int> &possibleWords) const {
    const int numberPattern = ::pow(3, m_wordsLength);

    double scores[numberPattern];
    fill(scores, scores + numberPattern, 0);
    double totalScore = 0;

    double score;
    for (int w : possibleWords) {
        score = m_words[word].score;
        scores[m_patternCache[w + word * m_numberWords]] += score;
        totalScore += score;
    }

    double entropy = 0.f;
    for (int i = 0; i < numberPattern; i++) {
        const double score = scores[i];
        const double p = score / totalScore;
        if (p > 0) {
            // entropy = sum( p * sub_entropy )
            // sub_entropy = log2(1/p)
            // sub_entropy = -log(p) / log(2)
            // entropy = sum(-p * log(p) / log(2))
            // entropy = sim(-p * log(p)) / log2(p)
            entropy -= p * log(p);
        }
    }
    return entropy / log(2.);
}

double WordList::entropy(const vector<int> &possibleWords) const {
    const double totalScore = this->totalScore(possibleWords);
    const int size = possibleWords.size();
    double entropy = 0;
    for (int i = 0; i < size; i++) {
        const double p = m_words[possibleWords[i]].score / totalScore;
        if (p > 0)
            entropy -= p * log(p);
    }
    return entropy / log(2);
}

double WordList::score(int word) const { return score(word, initialCompatibleWords()); }

double WordList::score(int word, const vector<int> &possibleWords) const {
    const double entropyScore =
        entropyToScore(entropy(possibleWords) - entropy(word, possibleWords)) + 1;
    if (entropyScore < 0)
        cout << getWord(word).word << " - " << entropy(possibleWords) << " - "
             << entropy(word, possibleWords) << " - " << entropyScore << "\n";
    if (binary_search(possibleWords.begin(), possibleWords.end(), word)) {
        const double totalWordScore = totalScore(possibleWords);
        const double p = getWord(word).score / totalWordScore;
        if (p < 0)
            cout << "p " << getWord(word).word << " - " << p << " - " << getWord(word).score
                 << " - " << totalWordScore << "\n";
        return p + (1 - p) * entropyScore;
    } else {
        return entropyScore;
    }
}

Result WordList::topWord() const { return topWord(initialCompatibleWords()); }

Result WordList::topWord(std::vector<int> possibleWords) const {
    Result bestResult;
    bestResult.score = 10000;
    double score;
    const int size = m_wordsValids.size();
    int w;
    for (int i = 0; i < size; i++) {
        w = m_wordsValids[i];
        if (bestResult.score > (score = this->score(w, possibleWords))) {
            bestResult.word = w;
            bestResult.score = score;
        }
    }
    return bestResult;
}

std::list<Result> WordList::topWords(unsigned int number) const {
    return topWords(initialCompatibleWords(), number);
}

std::list<Result> WordList::topWords(const std::vector<int> &possibleWords,
                                     unsigned int number) const {
    list<Result> topEntropy;

    double score = 0;
    const int size = m_wordsValids.size();
    int w;
    for (int i = 0; i < size; i++) {
        w = m_wordsValids[i];
        if (topEntropy.back().score > (score = this->score(w, possibleWords)) ||
            topEntropy.size() < number) {

            Result result;
            result.word = w;
            result.score = score;

            const auto it = lower_bound(topEntropy.begin(), topEntropy.end(), result,
                                        [](const Result &currentResult, const Result &newResult) {
                                            return currentResult.score < newResult.score;
                                        });

            topEntropy.insert(it, result);
            if (topEntropy.size() > number)
                topEntropy.pop_back();
        }
    }

    return topEntropy;
}

bool WordList::isWordCompatible(int word, const Step &step) const {
    return m_patternCache[word + step.word * m_numberWords] == step.pattern;
}

bool WordList::isWordCompatible(int word, const vector<Step> &steps) const {
    return all_of(steps.begin(), steps.end(),
                  [this, &word](const Step &step) { return isWordCompatible(word, step); });
}

std::vector<int> WordList::compatibleWords(const std::vector<int> &possibilities,
                                           const Step &step) const {
    std::vector<int> new_possibilities;
    copy_if(possibilities.begin(), possibilities.end(), back_inserter(new_possibilities),
            [this, &step](int word) {
                return m_patternCache[word + step.word * m_numberWords] == step.pattern;
            });
    return new_possibilities;
}

std::vector<int> WordList::initialCompatibleWords() const { return m_wordsValids; }

string WordList::patternToString(const Step &step) const {
    return patternToString(step.word, step.pattern);
}
string WordList::patternToString(int word, int pattern) const {
    string result = "";
    const string w = getWord(word).word;
    const int length = m_wordsLength;
    for (int i = 0; i < length; i++) {
        const int r = pattern % 3;
        result.push_back(r == 2 ? w[i] : r == 1 ? tolower(w[i]) : '.');
        pattern /= 3;
    }
    return result;
}

unsigned int WordList::wordLength() const { return m_wordsLength; }
vector<Word> WordList::words() const { return m_words; }

std::istream &operator>>(std::istream &is, Word &word) {
    is >> word.word >> word.frq;
#if WORDS_UNIFORM_SCORE
    word.score = 1;
#else
    word.score = tanh(word.frq * WORD_SCORE_MUL + WORD_SCORE_OFFSET) + 1;
#endif
    // cout << word.word << "\t" << word.frq << "\t" << word.score << "\n";
    return is;
};