import unicodedata

VOCABULARY_FILE = 'Lexique.tsv'
OUTPUT_FILES = 'words-{0}.txt'

def strip_accents(s):
   return ''.join(c for c in unicodedata.normalize('NFD', s)
                  if unicodedata.category(c) != 'Mn')

def split_words(start_length, end_length):
    words = [dict() for _ in range(start_length, end_length + 1)]

    with open(VOCABULARY_FILE, 'r') as fir:
        fir.readline()
        while line := fir.readline():
            line = line.split('\t')
            if line[0].isalpha() and start_length <= (length := len(line[0])) <= end_length:
                i = length - start_length
                word = strip_accents(line[0]).upper()
                words[i][word] = max((float(line[6]) + words[i].get(word, float(line[6]))) / 2, 0.005)

    for i, words_list in enumerate(words):
        with open(OUTPUT_FILES.format(start_length + i), 'w') as fiw:
            for word in words_list:
                fiw.write(f'{word}\t{words_list[word]:.3f}\n')

if __name__ == '__main__':
    split_words(4, 12)