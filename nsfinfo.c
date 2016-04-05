/*
Copyright (C) 2016 Jeremy Chadwick. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <md5.h>
#include <sha.h>

struct nsf {
	char		header[5];		/* offset 0x00 */
	uint8_t		version;		/* offset 0x05 */
	uint8_t		song_count;		/* offset 0x06 */
	uint8_t		song_start;		/* offset 0x07 */
	uint16_t	load_addr;		/* offset 0x08 */
	uint16_t	init_addr;		/* offset 0x0a */
	uint16_t	play_addr;		/* offset 0x0c */
	char		name[32];		/* offset 0x0e */
	char		artist[32];		/* offset 0x2e */
	char		copyright[32];		/* offset 0x4e */
	uint16_t	speed_ntsc;		/* offset 0x6e */
	uint8_t		bankswitch70;		/* offset 0x70 */
	uint8_t		bankswitch71;		/* offset 0x71 */
	uint8_t		bankswitch72;		/* offset 0x72 */
	uint8_t		bankswitch73;		/* offset 0x73 */
	uint8_t		bankswitch74;		/* offset 0x74 */
	uint8_t		bankswitch75;		/* offset 0x75 */
	uint8_t		bankswitch76;		/* offset 0x76 */
	uint8_t		bankswitch77;		/* offset 0x77 */
	uint16_t	speed_pal;		/* offset 0x78 */
	uint8_t		region;			/* offset 0x7a */
	uint8_t		extra_sound;		/* offset 0x7b */
	uint32_t	reserved;		/* offset 0x7c */
						/* offset 0x80 */
};

#define NSF_HEADER_SIZE sizeof(struct nsf)

static void
USAGE(void)
{
	printf("Usage: nsfinfo [options] filename\n");
	printf("\n");
	printf("  -J            JSON-formatted output\n");
	printf("  -h            print this message\n");
	printf("\n");
	printf("https://github.com/koitsu/nsfinfo\n");
	printf("Report bugs at https://github.com/koitsu/nsfinfo/issues\n");
	exit(0);
}

int
main(int argc, char *argv[])
{
	int ch;
	int json_output  = 0;
	int exitcode     = 0;
	int fd           = -1;
	char *filename   = NULL;
	char *buf        = NULL;
	char *md5        = NULL;
	char *sha1       = NULL;
	struct nsf *data = NULL;

	while ((ch = getopt(argc, argv, "Jh?")) != -1) {
		switch(ch) {
			case 'J':
				json_output = 1;
				break;
			case 'h':
			case '?':
			default:
				USAGE();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1) {
		USAGE();
	}

	filename = argv[0];

	buf = calloc(NSF_HEADER_SIZE, 1);
	if (buf == NULL) {
		printf("calloc() failed (buf)\n");
		exitcode = 1;
		goto finish;
	}

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("open() failed\n");
		exitcode = 1;
		goto finish;
	}

	if (read(fd, buf, NSF_HEADER_SIZE) != NSF_HEADER_SIZE) {
		printf("read() was short: file too small\n");
		exitcode = 1;
		goto finish;
	}

	close(fd);

	data = calloc(NSF_HEADER_SIZE, 1);
	if (data == NULL) {
		printf("calloc() failed (data)\n");
		exitcode = 1;
		goto finish;
	}

	if (buf[0x00] != 'N' || buf[0x01] != 'E' || buf[0x02] != 'S' ||
	    buf[0x03] != 'M' || buf[0x04] != 0x1a) {
		printf("file is not in NSF format\n");
		exitcode = 1;
		goto finish;
	}

	memcpy(data->header, buf+0x00, sizeof(data->header));

	data->version      = buf[0x05];
	data->song_count   = buf[0x06];
	data->song_start   = buf[0x07];
	data->load_addr    = buf[0x08] + (buf[0x09] << 8);
	data->init_addr    = buf[0x0a] + (buf[0x0b] << 8);
	data->play_addr    = buf[0x0c] + (buf[0x0d] << 8);

	memcpy(data->name,      buf+0x0e, sizeof(data->name));
	memcpy(data->copyright, buf+0x2e, sizeof(data->copyright));
	memcpy(data->artist,    buf+0x4e, sizeof(data->artist));

	data->speed_ntsc   = buf[0x6e] + (buf[0x6f] << 8);
	data->bankswitch70 = buf[0x70];
	data->bankswitch71 = buf[0x71];
	data->bankswitch72 = buf[0x72];
	data->bankswitch73 = buf[0x73];
	data->bankswitch74 = buf[0x74];
	data->bankswitch75 = buf[0x75];
	data->bankswitch76 = buf[0x76];
	data->bankswitch77 = buf[0x77];
	data->speed_pal    = buf[0x78] + (buf[0x79] << 8);
	data->region       = buf[0x7a];
	data->extra_sound  = buf[0x7b];
	data->reserved     = buf[0x7c] + (buf[0x7d] << 8) + (buf[0x7e] << 16) + (buf[0x7f] << 24);

	md5 = MD5File(filename, NULL);
	sha1 = SHA_File(filename, NULL);

	if (json_output) {
		printf("{\n");
		printf("  \"nsf\": {\n");
		printf("    \"version\": %u,\n",              data->version);
		printf("    \"song_count\": %u,\n",           data->song_count);
		printf("    \"song_start\": %u,\n",           data->song_start);
		printf("    \"load_addr\": \"0x%04x\",\n",    data->load_addr);
		printf("    \"init_addr\": \"0x%04x\",\n",    data->init_addr);
		printf("    \"play_addr\": \"0x%04x\",\n",    data->play_addr);
		printf("    \"name\": \"%s\",\n",             data->name);
		printf("    \"artist\": \"%s\",\n",           data->artist);
		printf("    \"copyright\": \"%s\",\n",        data->copyright);
		printf("    \"speed_ntsc\": \"0x%04x\",\n",   data->speed_ntsc);
		printf("    \"bankswitch70\": \"0x%02x\",\n", data->bankswitch70);
		printf("    \"bankswitch71\": \"0x%02x\",\n", data->bankswitch71);
		printf("    \"bankswitch72\": \"0x%02x\",\n", data->bankswitch72);
		printf("    \"bankswitch73\": \"0x%02x\",\n", data->bankswitch73);
		printf("    \"bankswitch74\": \"0x%02x\",\n", data->bankswitch74);
		printf("    \"bankswitch75\": \"0x%02x\",\n", data->bankswitch75);
		printf("    \"bankswitch76\": \"0x%02x\",\n", data->bankswitch76);
		printf("    \"bankswitch77\": \"0x%02x\",\n", data->bankswitch77);
		printf("    \"speed_pal\": \"0x%04x\",\n",    data->speed_pal);
		printf("    \"region\": \"0x%02x\",\n",       data->region);
		printf("    \"extra_sound\": \"0x%02x\",\n",  data->extra_sound);
		printf("    \"reserved\": \"0x%04x\"\n",      data->reserved);
		printf("  },\n");
		printf("  \"metadata\": {\n");
		printf("    \"md5\": \"%s\",\n", md5);
		printf("    \"sha1\": \"%s\"\n", sha1);
		printf("  }\n");
	}
	else {
		printf("version      = %u\n",     data->version);
		printf("song_count   = %u\n",     data->song_count);
		printf("song_start   = %u\n",     data->song_start);
		printf("load_addr    = $%04x\n",  data->load_addr);
		printf("init_addr    = $%04x\n",  data->init_addr);
		printf("play_addr    = $%04x\n",  data->play_addr);
		printf("name         = \"%s\"\n", data->name);
		printf("artist       = \"%s\"\n", data->artist);
		printf("copyright    = \"%s\"\n", data->copyright);
		printf("speed_ntsc   = $%04x\n",  data->speed_ntsc);
		printf("bankswitch70 = $%02x\n",  data->bankswitch70);
		printf("bankswitch71 = $%02x\n",  data->bankswitch71);
		printf("bankswitch72 = $%02x\n",  data->bankswitch72);
		printf("bankswitch73 = $%02x\n",  data->bankswitch73);
		printf("bankswitch74 = $%02x\n",  data->bankswitch74);
		printf("bankswitch75 = $%02x\n",  data->bankswitch75);
		printf("bankswitch76 = $%02x\n",  data->bankswitch76);
		printf("bankswitch77 = $%02x\n",  data->bankswitch77);
		printf("speed_pal    = $%04x\n",  data->speed_pal);
		printf("region       = $%02x\n",  data->region);
		printf("extra_sound  = $%02x\n",  data->extra_sound);
		printf("reserved     = $%04x\n",  data->reserved);
		printf("md5          = %s\n",     md5);
		printf("sha1         = %s\n",     sha1);
	}

finish:
	if (fd != -1) {
		close(fd);
	}
	free(sha1);
	free(md5);
	free(data);
	free(buf);
	exit(exitcode);
}

