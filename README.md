
# Bomm

Bomm is a program for breaking Enigma ciphertext. It was written as part of the Bachelor thesis Breaking Enigma ciphertext in 2023 by Fränz Friederes.

## Getting started

Make sure the following dependencies are present on your system:

```bash
brew install criterion jansson
```

Build and test the program using `make`:

```bash
make clean && make test && make build
```

The input to a program is provided through a query JSON file. You can execute it like so:

```bash
./build/bomm ./data/queries/authentic-1945-01-10.json
```

## Sources

### Frequency of English text

- `en-trigram.txt` - Trigram frequency generated from around 4.5 billion characters of English text, sourced from Wortschatz, provided by [Practical Cryptography](http://practicalcryptography.com/cryptanalysis/letter-frequencies-various-languages/english-letter-frequencies/):

### Frequency of raw 1941 Enigma message decrypts

The following frequency tables have been published by [Frode Weierud's CryptoCellar](https://cryptocellar.org/bgac/keyofE.html).

- `enigma1941-bigram.txt`
- `enigma1941-trigram.txt`

## License

The source code in this repository is published under the MIT license. See [LICENSE.txt](LICENSE.txt).
