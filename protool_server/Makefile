PWD=$(shell pwd)
INSTPATH=/usr/local/bin
CFLAGS+=-I$(PWD)
LIBS+=-lm
LDFLAGS+=-L$(PWD)
CROSS_COMPILE:=/opt/xtools/arm920t/bin/arm-linux-

SRCFILES = $(wildcard *.c)
NAME=protoold

export CC=${CROSS_COMPILE}gcc

all:	${SRCFILES}
	${CC}  $^ ${CFLAGS} ${LDFLAGS}   ${LIBS} -o ${NAME}
	@echo " Compile over"
	sz ${NAME}


tag: 
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R .  
	@cscope -Rbq

install:
	@cp $(NAME) ${INSTPATH}

clean:
	@rm -f ${NAME}

distclean: clean 
	@rm -f  tags cscope*

