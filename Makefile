CFLAGS_DEBUG=-Wall -Wextra -std=c11 -pedantic -g
CFLAGS_BUILD=-Wall -Wextra -std=c11 -pedantic -s -O2

# CFLAGS=$(CFLAGS_DEBUG)
CFLAGS=$(CFLAGS_BUILD)

mines: main.o mines.o ansi.o settings.o system.o
	$(CC) $(CFLAGS) -o mines main.o mines.o ansi.o settings.o system.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

mines.o: mines.c mines.h
	$(CC) $(CFLAGS) -c mines.c

ansi.o: ansi.c ansi.h
	$(CC) $(CFLAGS) -c ansi.c

settings.o: settings.c settings.h
	$(CC) $(CFLAGS) -c settings.c

system.o: system.c system.h
	$(CC) $(CFLAGS) -c system.c

.PHONY: clean
clean:
	rm -f *.o ./mines *.exe
