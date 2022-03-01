### Wordle / Sutom / Le Mot

A Wordle / Sutom / Le Mot solver with french words data (and words of 5).
The UI is in French.

### About

This program find the best words (technically, not *the* best but very good words) in order to
find the word of a Wordle game.

The program use entropy in order to find the word that maximise entropy, it find the word
which maximise the average score (the probability of the word + number of guess from the entropy
remaining (using the formula `0.9 ln(entropy remaining) + 1.5`, find by regression with french words
of 5 letters)).
It also take in account the frequency of a word in order to favour more likely words (using
the formula: `weight = tanh(frequency * 3 - 2) + 1` with the frequency in occurrences per
million (of the lemma for the french dictionary)).

### Installation and use

Run `cmake .` then `make` to build it. Run `./WordleSutom` to run the program
Update the main function to choose what to run.

### Credit

The data used is the dictionary [lexique.org](http://www.lexique.org/) version 3.8 (and from
[3b1b](https://github.com/3b1b/videos/tree/master/_2022/wordle) for the english dictionary).


Highly inspired by projects of
- [3Blue1Brown](https://github.com/3b1b/videos/tree/master/_2022/wordle)
- [Science Étonnante](https://github.com/scienceetonnante/WordleSutom/)

Wordle (en): https://www.nytimes.com/games/wordle/index.html

Sutom (fr): https://sutom.nocle.fr/

Le Mot (fr): https://www.solitaire-play.com/lemot/ 