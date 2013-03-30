
all: multiraw

multiraw:
	${CC} ${CFLAGS} ${LDFLAGS} -o multiraw multiraw.c

clean:
	rm multiraw 

install:
	install -m 0755 -g wheel -o root multiraw ${prefix}/bin
	-install -m 0644 -g wheel -o root multiraw.7 ${man7dir}

deinstall:
	rm -f ${prefix}/bin/multiraw
	rm -f ${man7dir}/multiraw.7

