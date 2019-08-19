
CC=gcc
PWD=$(shell pwd)
INSTPATH=/usr/local/bin
CFLAGS+=-I${PWD}
LIBS+=-lsqlite3
LIBS+=-lm
LDFLAGS+=-L${PWD}
SRCFILES = $(wildcard *.c)
NAME=protool

all:    ${SRCFILES}
	    ${CC}  $^ ${CFLAGS} ${LDFLAGS}   ${LIBS} -o ${NAME}
		
		@echo " Compile over"

tag: 
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R . 
	@cscope -Rbq

install:
	@cp ${NAME} ${INSTPATH}

clean:
	@rm -f ${NAME}

distclean: clean
	@rm -rf tags cscope*


