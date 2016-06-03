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
  [Message Digest library (libmd)](https://www.freebsd.org/cgi/man.cgi?query=SHA\_File&apropos=0&sektion=0&manpath=FreeBSD+9.3-stable&arch=default&format=html)
  in FreeBSD 9.x and earlier.  This is a requirement I'd like to remove
  in the future (for usability on newer FreeBSD, as well as Linux).

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
