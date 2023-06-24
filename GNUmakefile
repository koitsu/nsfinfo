#
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#

CFLAGS+=	-g3 -ggdb -Werror -Wall -Wextra -Wformat-security -Waggregate-return -Wbad-function-cast -Wcast-align -Wdeclaration-after-statement -Wdisabled-optimization -Wfloat-equal -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wold-style-definition -Wpacked -Wpointer-arith -Wredundant-decls -Wstrict-prototypes -Wunreachable-code -Wwrite-strings

all: nsfinfo

md5.o: md5.c md5.h
	${CC} -c ${CFLAGS} -o $@ $<

sha256.o: sha256.c sha256.h
	${CC} -c ${CFLAGS} -o $@ $<

nsfinfo: nsfinfo.c md5.o sha256.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f nsfinfo *.o *.core

