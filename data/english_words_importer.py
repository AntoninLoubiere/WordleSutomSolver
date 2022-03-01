import json

with open('words-english.json') as fir:
    data = json.load(fir)
with open('words-5-english.txt', 'w') as fiw:
    w: str
    for w in data:
        fiw.write(f'{w.upper()}\t{data[w]*1_000_000:.5f}\n')