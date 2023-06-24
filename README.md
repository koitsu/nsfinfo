nsfinfo
=======
This is a userland program that attempts to read NSF files (music files
for the NES/Famicom) and output details in a somewhat "user-friendly"
format, as well as in JSON format.

This program was originally invented for a website I've been developing
off-and-on for several years (a database of all NSF files on the 'net);
think BootGod's NES Cart DB but for NSF files.

Handling binary file formats in C is dramatically faster (and easier!)
than in, say, Perl, PHP or similar higher-level languages.

Bugs/Notes
----------
* MD5 and SHA256 hash features are implemented using libmd on FreeBSD.
  On all other operating systems, md5.{c,h} and sha256.{c,h} are used.
* The `expand_escape()` function is an abomination, but adding true/full
  Unicode support (for "true" JSON output compliance) is painful.  This
  is compounded by complexities like some NSFs using Japanese character
  sets (both JIS and EUC have been seen), as well as other non-ASCII
  characters (ex. Latin-1 é, Swedish ö and å, Norwegian ø, etc.).  It's
  a problem that isn't easily/simply solved.

Usage
-----
```Shell
$ ./nsfinfo "Akumajou Densetsu (VRC6).nsf"
version      = 1
song_count   = 28
song_start   = 1
load_addr    = 8000
init_addr    = ffe0
play_addr    = ffd0
name         = "Akumajou Densetsu"
artist       = "<?>"
copyright    = "1989 Konami"
speed_ntsc   = 4100
bankswitch   = 0b0c0a0a
speed_pal    = 0000
region       = 00
extra_sound  = 01
reserved     = 0000
md5          = d867645237ee14fd2c084f43ed88523b
sha256       = 625077a2ac6eb251ac029c69098bff4d8f82b2ff1383ad955ab24459acd9605e

$ ./nsfinfo -J "Akumajou Densetsu (VRC6).nsf"
{
  "nsf": {
    "version": 1,
    "song_count": 28,
    "song_start": 1,
    "load_addr": "8000",
    "init_addr": "ffe0",
    "play_addr": "ffd0",
    "name": "Akumajou Densetsu",
    "artist": "<?>",
    "copyright": "1989 Konami",
    "speed_ntsc": "4100",
    "bankswitch": "0b0c0a0a",
    "speed_pal": "0000",
    "region": "00",
    "extra_sound": "01",
    "reserved": "0000"
  },
  "metadata": {
    "md5": "d867645237ee14fd2c084f43ed88523b",
    "sha256": "625077a2ac6eb251ac029c69098bff4d8f82b2ff1383ad955ab24459acd9605e"
  }
}
```

License
-------
nsfinfo is released under the 2-clause BSD license ("FreeBSD License") per [LICENSE](LICENSE).

md5.{c,h} and sha256.{c,h} do not specify a license, but were authored by Brad Conte
as part of his [crypto-algorithms](https://github.com/B-Con/crypto-algorithms) repository.

Contributing
------------
Please contribute!
