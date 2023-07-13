
# bomm

bomm, initiated as part of the Bachelor thesis *Breaking Enigma ciphertext in 2023* by Fränz Friederes, is a C-based command-line program designed to break Enigma ciphertext in a ciphertext-only scenario.

The thesis script will be made available, soon.

## Getting started

First, download the [latest stable release](https://github.com/cryptii/cryptii/releases/latest) or build the program from source (see instructions below).

Command line usage:

```
Usage: bomm data/queries/kr-blitz.json
Options:
  -h, --help        display this help message
  -n, --num-hold    number of hold elements to collect
  -t, --num-threads number of concurrent threads to use
  -q, --quiet       quiet mode
  -v, --verbose     verbose mode
```

To evaluate a ciphertext messages with bomm, a query needs to be composed and passed as the only argument. It contains the ciphertext itself, the key space to be searched (referencing known or custom wheels and wirings), and a set of passes that describe the strategies (e.g. hill climbing) to be applied. A schema for such query files can be found at `data/schemas/query.json`. Example queries are stored in `data/queries`.

Exemplary, the following command and query can be used to run an attack against the KR Blitz message, targeting the practical key space of Enigma I with UKW-B using the E-Stecker technique.

```bash
bomm data/queries/kr-blitz.json
```

This will result in the following view that is kept up-to-date while processing the query, displaying the top scoring keys accompanied by a preview of the putative plaintext:

```
┌──────┬───────────────────────────────────────────────────────────────────────┐
│ Bomm │ Progress      0.228 % │ Elapsed      00:03:44 │ Remaining    26:06:54 │
├──────┴───────────────────────────────────────────────────────────┬───────────┤
│ tgenpjnrjdqubenahrbefohsamtjnenexsiesgalieldninsnptcqssenidfqxan   -8.702661 │
│ UKW-B,III,IV,V aaaa aber bf ch dt in kv or ps qz wy                          │
├──────────────────────────────────────────────────────────────────┬───────────┤
│ rlverarmxplinsxjerpisjdsixfeheiarvifuetentxschxefojaxungthxeutha   -8.737875 │
│ UKW-B,II,III,V aaaa abjt bz cy dp fv hi jm lt ou qr                          │
├──────────────────────────────────────────────────────────────────┬───────────┤
│ rderdsjxsseliseilunidpbenidxzhlqhtbeqnafkorprexrgvniniweantbenix   -8.739881 │
│ UKW-B,V,II,IV aaaa aadj ad bj cq es fo iy kt lr mz nx pw uv                  │
├──────────────────────────────────────────────────────────────────┬───────────┤
│ eciemxkztpyxejhmlerkxdiesierregsieigfaratewfgarkyhydjzbnhoilonss   -8.745330 │
│ UKW-B,II,III,V aaaa aapy ad bt ek fq gu ir jy lw ns oz pv                    │
├──────────────────────────────────────────────────────────────────┬───────────┤
│ rlgsigezeodnfstaurmitgensexonfcsienenssgewapxximnrkxsonsaangreit   -8.756366 │
│ UKW-B,V,III,IV aaaa aaxo am do ip lz nw qt rx vy                             │
├──────────────────────────────────────────────────────────────────┬───────────┤
│ tgenpjnrjdqubenahrbefobsartjnenexsiesialieldninsnptqqssenuqfqxan   -8.762827 │
│ UKW-B,III,IV,V aaaa aber bf ch dt in or ps qz vy                             │
╞══════════════════════════════════════════════════════════════════╤═══════════╡
│ hitlutsunqrtliabftqrnuwlqvnitrsctnqipklmaheefdcabcuworubswyrbggf   -10.09713 │
│ Unchanged ciphertext (97 letters)                                            │
└──────────────────────────────────────────────────────────────────────────────┘
```

By default, half the number of detected CPU cores is used as the number of parallel threads spawned. This may be overridden by the `-t` flag.

When the query completes or the program is terminated by the SIGINT (pressing `Ctrl+C`) or SIGTERM signal, the full hold is printed out before exiting.

## Wheels

In a query key space, wheels can be referenced by their name. The following options are available:

- Entry wheels (all models)<br>
  `ETW-ABC`, `ETW-QWE`
- Enigma I, M1, M2, M3, and Enigma M4<br>
  `I`, `II`, `III`, `IV`, `V`, `VI`, `VII`, `VIII`, `beta`, `gamma`, `UKW-A`, `UKW-B`, `UKW-C`, `UKW-B-thin` `UKW-C-thin`
- Enigma B 207<br>
  `B207-I`, `B207-II`, `B207-III`, `B207-UKW`
- Enigma S “Sondermaschine”<br>
  `S-I`, `S-II`, `S-III`, `S-UKW`
- “Spanish Enigma”<br>
  `ES-I`, `ES-II`, `ES-III`, `ES-IV`, `ES-V`, `ES-UKW`
- Enigma “Zagreb Delta”<br>
  `ZD-I`, `ZD-II`, `ZD-III`, `ZD-IV`, `ZD-V`, `ZD-UKW`

If a wheel is missing in the built-in library, it can be specified in the `wheels` array in the query object:

```json
{
  "name": "B207-I",
  "wiring": "iqynwgavkozscpfbumlextjdhr",
  "turnovers": "q"
}
```

## Measures

In the query, built-in measures (or scoring functions) are referenced by string tokens like `ic` or `sinkov_trigram`. The following options are available:

- [Sinkov statistic](https://en.wikipedia.org/wiki/Sinkov_statistic) or log-weight statistic for n-grams; Requires a frequency map<br>
  Available options: `sinkov_monogram`, `sinkov_bigram`, `sinkov_trigram`, `sinkov_quadgram`, `sinkov_pentagram`, `sinkov_hexagram`
- [Index of coincidence](https://en.wikipedia.org/wiki/Index_of_coincidence) measure<br>
  Available options: `ic`, `ic_bigram`, `ic_trigram`, `ic_quadgram`, `ic_pentagram`, `ic_hexagram`
- [Shannon entropy](https://en.wikipedia.org/wiki/Entropy_(information_theory)) measure<br>
  Available options: `entropy`, `entropy_bigram`, `entropy_trigram`, `entropy_quadgram`, `entropy_pentagram`, `entropy_hexagram`

## Build from source

First, make sure the libraries [jansson](https://github.com/akheron/jansson) and [criterion](https://github.com/Snaipe/Criterion) are available to the compiler.

Build and test the program using make:

```bash
make clean && make test
# Creates build/bomm
```

A non-Latin alphabet may be used with an especially compiled version of the program:

```bash
make clean && make test ALPHABET=0123456789
# Creates build/bomm-0123456789
```

Note, that as of now, only path-safe ASCII alphabets are allowed here.

## Data sources

- The frequency tables for English text `en-bigram.txt` and `en-trigram.txt` were generated by [Practical Cryptography](http://practicalcryptography.com/cryptanalysis/letter-frequencies-various-languages/english-letter-frequencies/) from around 4.5 billion characters of English text, sourced from [Wortschatz](https://wortschatz-leipzig.de/en).
- The frequency tables for raw 1941 Enigma message decrypts `enigma1941-bigram.txt` and `enigma1941-trigram.txt` have been published on [Frode Weierud's CryptoCellar](https://cryptocellar.org/bgac/keyofE.html).

## License

The source code in this repository is published under the MIT license. See [LICENSE.txt](LICENSE.txt).
