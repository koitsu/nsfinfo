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

Requirements
------------
* `MD5File()` and `SHA_File()` functions, part of the
  [Message Digest Support Library (libmd)](https://www.freebsd.org/cgi/man.cgi?query=SHA\_File&apropos=0&sektion=0&manpath=FreeBSD+11.1-stable&arch=default&format=html)
  in FreeBSD.  This is a requirement I'd like to remove in the future,
  mainly for usability on non-FreeBSD OSes.  These functions and their
  related code tend to be released under a whole slew of difference
  licenses, complicating inclusion of said code in this program.

Bugs/Notes
----------
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
artist       = "1989 Konami"
copyright    = "<?>"
speed_ntsc   = 4100
bankswitch   = 0b0c0a0a
speed_pal    = 0000
region       = 00
extra_sound  = 01
reserved     = 0000
md5          = d867645237ee14fd2c084f43ed88523b
sha1         = 5184128cc7bb67a3dab262525e07412a8f1e14d3

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
    "artist": "1989 Konami",
    "copyright": "<?>",
    "speed_ntsc": "4100",
    "bankswitch": "0b0c0a0a",
    "speed_pal": "0000",
    "region": "00",
    "extra_sound": "01",
    "reserved": "0000"
  },
  "metadata": {
    "md5": "d867645237ee14fd2c084f43ed88523b",
    "sha1": "5184128cc7bb67a3dab262525e07412a8f1e14d3"
  }
}
```

License
-------
nsfinfo is released under the 2-clause BSD license ("FreeBSD License") per [LICENSE](LICENSE).

Contributing
------------
Please contribute!
