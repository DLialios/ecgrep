[![demo](https://asciinema.org/a/486661.svg)](https://asciinema.org/a/486661?autoplay=1)

Traverses the current working directory. 

For each regular file (no fifos, symlinks, etc.) under 100MB which does not contain a null byte:

1. load data into memory
2. perform regex search with pattern
3. print name of file, line number of match, and contents of line
4. free resources

A single file may contain many matches.

Workload is placed on as many threads as the platform has available.

Has two switches, c (case-sensitive) and v (verbose output).

`ecgrep -c 'Hello [wW]orld!\n'`
