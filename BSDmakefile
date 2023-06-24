#
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#

CFLAGS+=	-g3 -ggdb -Werror -Wall -Wextra -Wformat-security -Waggregate-return -Wbad-function-cast -Wcast-align -Wdeclaration-after-statement -Wdisabled-optimization -Wfloat-equal -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wold-style-definition -Wpacked -Wpointer-arith -Wredundant-decls -Wstrict-prototypes -Wunreachable-code -Wwrite-strings

all: nsfinfo

nsfinfo: nsfinfo.c
	${CC} ${CFLAGS} -o $@ $< -lmd

clean:
	rm -f nsfinfo *.core

