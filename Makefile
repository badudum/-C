exec = minusC.out
sources = $(wildcard src/*.c)
sources += $(wildcard src/*/*.c)
objects = $(sources:.c=.o)
flags = -g -std=c99


$(exec) : $(objects)
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c include/%.h
	gcc -c $(flags) $< -o $@

install : 
	make
	cp ./minusC.out /usr/local/bin/minusC

clean:
	-rm $(objects)

	-rm -f *.o
	-rm -f *src/.o