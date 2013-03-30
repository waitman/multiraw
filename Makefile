
all: multiraw

multiraw:
	${CC} ${CFLAGS} ${LDFLAGS} -o multiraw multiraw.c

clean:
	rm multiraw 

install:
	install -m 0755 -g wheel -o root multiraw ${prefix}/bin

deinstall:
	rm -f ${prefix}/bin/multiraw

