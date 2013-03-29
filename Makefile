prefix=/usr/local
man7dir=/usr/local/man/man7

all: multiraw dcraw-m

multiraw:
	${CC} -std=c99 -O3 -Wall -Werror -pthread -o multiraw multiraw.c

dcraw-m:
	${CC} -std=c99 -O3 -I/usr/local/include/ImageMagick/ -I/usr/local/include -I/usr/include -L/usr/local/lib -L/usr/lib -lMagickWand -lm -ljasper -ljpeg -llcms -o dcraw-m dcraw-m.c

clean:
	rm multiraw dcraw-m

install:
	install -m 0755 -g wheel -o root multiraw ${prefix}/bin
	install -m 0755 -g wheel -o root dcraw-m ${prefix}/bin
	-install -m 0644 -g wheel -o root multiraw.7 ${man7dir}

deinstall:
	rm -f ${prefix}/bin/multiraw
	rm -f ${prefix}/bin/dcraw-m
	rm -f ${man7dir}/multiraw.7

