prefix=/usr/local

all: multiraw dcraw-m

multiraw:
	${CC} -std=c99 -O3 -Wall -Werror -pthread -o multiraw multiraw.c

dcraw-m:
	${CC} -std=c99 -O3 -I/usr/local/include/ImageMagick/ -I/usr/local/include -I/usr/include -L/usr/local/lib -L/usr/lib -lMagickWand -lm -ljasper -ljpeg -llcms -o dcraw-m dcraw-m.c

clean:
	rm multiraw dcraw-m

install:
	install -m 0755 multiraw ${prefix}/bin
	install -m 0755 dcraw-m ${prefix}/bin

deinstall:
	rm -f ${prefix}/bin/multiraw
	rm -f ${prefix}/bin/dcraw-m

