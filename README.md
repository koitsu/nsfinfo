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

Contributing
------------
Please contribute!
