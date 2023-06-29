/*
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>
#include <stdint.h>

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
	uint64_t	bankswitch;		/* offset 0x70 */
	uint16_t	speed_pal;		/* offset 0x78 */
	uint8_t		region;			/* offset 0x7a */
	uint8_t		extra_sound;		/* offset 0x7b */
	uint32_t	reserved;		/* offset 0x7c */
						/* offset 0x80 */
};

#define NSF_HEADER_SIZE sizeof(struct nsf)

/* Helper functions for calculating the MD5 and SHA256 hashes
 * of a file.
 *
 * On FreeBSD we simply reference the libmd equivalent functions,
 * while on Linux we have to write our own which leverage the
 * functions from md5.c and sha256.c.  These are similar in concept
 * to the FreeBSD libmd functions.
 *
 * Note: we blindly assume any non-FreeBSD system is Linux, which
 * is a bad assumption but for now it keeps things simple.
 * */

#ifdef __FreeBSD__
#include <sys/types.h>
#include <md5.h>
#include <sha256.h>
#define my_MD5_File(a) MD5File(a, NULL)
#define my_SHA256_File(a) SHA256_File(a, NULL)
#else
#include "md5.h"
#include "sha256.h"
static char *
my_MD5_File(const char *filename)
{
	FILE *fd;
	size_t filelen;
	unsigned char *filebuf = NULL;
	MD5_CTX c;
	BYTE hash[MD5_BLOCK_SIZE];
	char *hashstr = NULL;
	char *hashstrp;

	fd = fopen(filename, "rb");
	if (fd == NULL) {
		fprintf(stderr, "my_MD5_File(): fopen() failed\n");
		return NULL;
	}

	/* TODO handle fseek/ftell errors */
	fseek(fd, 0, SEEK_END);
	filelen = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	filebuf = calloc(filelen, 1);
	if (filebuf == NULL) {
		fprintf(stderr, "my_MD5_File(): calloc() failed\n");
		fclose(fd);
		return NULL;
	}
	if (fread(filebuf, 1, filelen, fd) != filelen) {
		fprintf(stderr, "my_MD5_File(): fread() was short\n");
		free(filebuf);
		fclose(fd);
		return NULL;
	}
	fclose(fd);

	md5_init(&c);
	md5_update(&c, filebuf, filelen);
	md5_final(&c, hash);
	free(filebuf);

	/*
	 * MD5_BLOCK_SIZE * 2 + 1, since two ASCII characters represent
	 * a byte, and we need one extra byte for the trailing NULL.
	 */
	/* TODO handle calloc errors */
	hashstr = calloc((MD5_BLOCK_SIZE * 2) + 1, 1);
	hashstrp = &hashstr[0];

	for (unsigned int i = 0; i < MD5_BLOCK_SIZE; i++) {
		hashstrp += sprintf(hashstrp, "%02x", hash[i]);
	}

	return hashstr;
}

static char *
my_SHA256_File(const char *filename)
{
	FILE *fd;
	size_t filelen;
	unsigned char *filebuf = NULL;
	SHA256_CTX c;
	BYTE hash[SHA256_BLOCK_SIZE];
	char *hashstr = NULL;
	char *hashstrp;

	fd = fopen(filename, "rb");
	if (fd == NULL) {
		fprintf(stderr, "my_SHA256_File(): fopen() failed\n");
		return NULL;
	}

	/* TODO handle fseek/ftell errors */
	fseek(fd, 0, SEEK_END);
	filelen = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	filebuf = calloc(filelen, 1);
	if (filebuf == NULL) {
		fprintf(stderr, "my_SHA256_File(): calloc() failed\n");
		fclose(fd);
		return NULL;
	}
	if (fread(filebuf, 1, filelen, fd) != filelen) {
		fprintf(stderr, "my_SHA256_File(): fread() was short\n");
		free(filebuf);
		fclose(fd);
		return NULL;
	}
	fclose(fd);

	sha256_init(&c);
	sha256_update(&c, filebuf, filelen);
	sha256_final(&c, hash);
	free(filebuf);

	/*
	 * SHA256_BLOCK_SIZE * 2 + 1, since two ASCII characters represent
	 * a byte, and we need one extra byte for the trailing NULL.
	 */
	/* TODO handle calloc errors */
	hashstr = calloc((SHA256_BLOCK_SIZE * 2) + 1, 1);
	hashstrp = &hashstr[0];

	for (unsigned int i = 0; i < SHA256_BLOCK_SIZE; i++) {
		hashstrp += sprintf(hashstrp, "%02x", hash[i]);
	}

	return hashstr;
}
#endif

/*
 * The world's most ridiculous escaping routine, doing an
 * atrocious job of two things:
 *
 * 1. Turning any non-ASCII byte into a UTF-16 equivalent
 * 2. Escaping any double-quotes (i.e. " becomes \")
 */
static void
expand_escapes(char *dest, const char *src)
{
	char c;
	char rawhex[3];

	while((c = *(src++))) {
		if (c < 0x20 || c > 0x7e) {
			snprintf(rawhex, sizeof(rawhex), "%02x", c);
			*(dest++) = '\\';
			*(dest++) = 'u';
			*(dest++) = '0';
			*(dest++) = '0';
			*(dest++) = rawhex[0];
			*(dest++) = rawhex[1];
			continue;
		}
		else if (c == '"') {
			*(dest++) = '\\';
		}
		*(dest++) = c;
	}
	*dest = '\0';
}

static void
USAGE(void)
{
	fprintf(stderr,
		"Usage: nsfinfo [options] filename\n"
		"\n"
		"Options:\n"
		"  -J            JSON-formatted output\n"
		"  -h            print this message\n"
		"\n"
		"https://github.com/koitsu/nsfinfo\n"
		"Report bugs at https://github.com/koitsu/nsfinfo/issues\n"
	);
	exit(EX_USAGE);
}

int
main(int argc, char *argv[])
{
	int ch;
	int json_output   = 0;
	int exitcode      = EX_OK;
	FILE *fd;
	char *filename    = NULL;
	char *buf         = NULL;
	char *name_e      = NULL;
	char *artist_e    = NULL;
	char *copyright_e = NULL;
	char *md5         = NULL;
	char *sha256      = NULL;
	struct nsf *data  = NULL;

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
		fprintf(stderr, "calloc() failed (buf)\n");
		exitcode = EX_OSERR;
		goto finish_depth1;
	}

	data = calloc(NSF_HEADER_SIZE, 1);
	if (data == NULL) {
		fprintf(stderr, "calloc() failed (data)\n");
		exitcode = EX_OSERR;
		goto finish_depth2;
	}

	fd = fopen(filename, "rb");
	if (fd == NULL) {
		fprintf(stderr, "fopen() failed\n");
		exitcode = EX_NOINPUT;
		goto finish_depth3;
	}

	if (fread(buf, 1, NSF_HEADER_SIZE, fd) != NSF_HEADER_SIZE) {
		fprintf(stderr, "fread() was short: file too small\n");
		exitcode = EX_DATAERR;
		goto finish_depth4;
	}

	/*
	 * NSF file validation
	 */
	if (buf[0x00] != 'N' || buf[0x01] != 'E' || buf[0x02] != 'S' ||
	    buf[0x03] != 'M' || buf[0x04] != 0x1a) {
		fprintf(stderr, "file is not in NSF format\n");
		exitcode = EX_DATAERR;
		goto finish_depth4;
	}
	if (buf[0x2d] != 0x00) {
		fprintf(stderr, "NSF name field has no trailing null\n");
		exitcode = EX_DATAERR;
		goto finish_depth4;
	}
	if (buf[0x4d] != 0x00) {
		fprintf(stderr, "NSF artist field has no trailing null\n");
		exitcode = EX_DATAERR;
		goto finish_depth4;
	}
	if (buf[0x6d] != 0x00) {
		fprintf(stderr, "NSF copyright field has no trailing null\n");
		exitcode = EX_DATAERR;
		goto finish_depth4;
	}

	memcpy(data->header, buf+0x00, sizeof(data->header));

	data->version      = buf[0x05];
	data->song_count   = buf[0x06];
	data->song_start   = buf[0x07];
	data->load_addr    = buf[0x08] | ((uint16_t) buf[0x09] << 8);
	data->init_addr    = buf[0x0a] | ((uint16_t) buf[0x0b] << 8);
	data->play_addr    = buf[0x0c] | ((uint16_t) buf[0x0d] << 8);

	memcpy(data->name,      buf+0x0e, sizeof(data->name));
	memcpy(data->artist,    buf+0x2e, sizeof(data->artist));
	memcpy(data->copyright, buf+0x4e, sizeof(data->copyright));

	data->speed_ntsc   = buf[0x6e] | ((uint16_t) buf[0x6f] << 8);
	data->bankswitch   = ((uint64_t) buf[0x70] << 56) |
	                     ((uint64_t) buf[0x71] << 48) |
	                     ((uint64_t) buf[0x72] << 40) |
	                     ((uint64_t) buf[0x73] << 32) |
	                     ((uint64_t) buf[0x74] << 24) |
	                     ((uint64_t) buf[0x75] << 16) |
	                     ((uint64_t) buf[0x76] << 8)  |
	                     buf[0x77];
	data->speed_pal    = buf[0x78] | ((uint16_t) buf[0x79] << 8);
	data->region       = buf[0x7a];
	data->extra_sound  = buf[0x7b];
	data->reserved     = ((uint32_t) buf[0x7c] << 24) |
	                     ((uint32_t) buf[0x7d] << 16) |
	                     ((uint32_t) buf[0x7e] << 8)  |
	                     buf[0x7f];

	md5 = my_MD5_File(filename);
	if (md5 == NULL) {
		fprintf(stderr, "my_MD5_File() failed\n");
		exitcode = EX_OSERR;
		goto finish_depth4;
	}

	sha256 = my_SHA256_File(filename);
	if (sha256 == NULL) {
		fprintf(stderr, "my_SHA256_File() failed\n");
		exitcode = EX_OSERR;
		goto finish_depth5;
	}

	/*
	 * The * 6 is to ensure we have space in the case that every single
	 * byte has to be a UTF-16 equivalent.  The value 6 comes from the
	 * length of literal string "\u00xx".
	 */
	name_e = calloc(sizeof(data->name) * 6, 1);
	if (name_e == NULL) {
		fprintf(stderr, "calloc() failed (name_e)\n");
		exitcode = EX_OSERR;
		goto finish_depth6;
	}

	artist_e = calloc(sizeof(data->artist) * 6, 1);
	if (artist_e == NULL) {
		fprintf(stderr, "calloc() failed (artist_e)\n");
		exitcode = EX_OSERR;
		goto finish_depth7;
	}

	copyright_e = calloc(sizeof(data->copyright) * 6, 1);
	if (copyright_e == NULL) {
		fprintf(stderr, "calloc() failed (copyright_e)\n");
		exitcode = EX_OSERR;
		goto finish_depth8;
	}

	expand_escapes(name_e, data->name);
	expand_escapes(artist_e, data->artist);
	expand_escapes(copyright_e, data->copyright);

	if (json_output) {
		printf("{\n");
		printf("  \"nsf\": {\n");
		printf("    \"version\": %u,\n",           data->version);
		printf("    \"song_count\": %u,\n",        data->song_count);
		printf("    \"song_start\": %u,\n",        data->song_start);
		printf("    \"load_addr\": \"%04x\",\n",   data->load_addr);
		printf("    \"init_addr\": \"%04x\",\n",   data->init_addr);
		printf("    \"play_addr\": \"%04x\",\n",   data->play_addr);
		printf("    \"name\": \"%s\",\n",          name_e);
		printf("    \"artist\": \"%s\",\n",        artist_e);
		printf("    \"copyright\": \"%s\",\n",     copyright_e);
		printf("    \"speed_ntsc\": \"%04x\",\n",  data->speed_ntsc);
		printf("    \"bankswitch\": \"%08lx\",\n", data->bankswitch);
		printf("    \"speed_pal\": \"%04x\",\n",   data->speed_pal);
		printf("    \"region\": \"%02x\",\n",      data->region);
		printf("    \"extra_sound\": \"%02x\",\n", data->extra_sound);
		printf("    \"reserved\": \"%04x\"\n",     data->reserved);
		printf("  },\n");
		printf("  \"metadata\": {\n");
		printf("    \"md5\": \"%s\",\n", md5);
		printf("    \"sha256\": \"%s\"\n", sha256);
		printf("  }\n");
		printf("}\n");
	}
	else {
		printf("version      = %u\n",     data->version);
		printf("song_count   = %u\n",     data->song_count);
		printf("song_start   = %u\n",     data->song_start);
		printf("load_addr    = %04x\n",   data->load_addr);
		printf("init_addr    = %04x\n",   data->init_addr);
		printf("play_addr    = %04x\n",   data->play_addr);
		printf("name         = \"%s\"\n", data->name);
		printf("artist       = \"%s\"\n", data->artist);
		printf("copyright    = \"%s\"\n", data->copyright);
		printf("speed_ntsc   = %04x\n",   data->speed_ntsc);
		printf("bankswitch   = %08lx\n",  data->bankswitch);
		printf("speed_pal    = %04x\n",   data->speed_pal);
		printf("region       = %02x\n",   data->region);
		printf("extra_sound  = %02x\n",   data->extra_sound);
		printf("reserved     = %04x\n",   data->reserved);
		printf("md5          = %s\n",     md5);
		printf("sha256       = %s\n",     sha256);
	}

	free(copyright_e);
finish_depth8:
	free(artist_e);
finish_depth7:
	free(name_e);
finish_depth6:
	free(sha256);
finish_depth5:
	free(md5);
finish_depth4:
	fclose(fd);
finish_depth3:
	free(data);
finish_depth2:
	free(buf);
finish_depth1:
	exit(exitcode);
}

