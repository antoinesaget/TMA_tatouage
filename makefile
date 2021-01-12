HEADER	= pixmap_io.h
OBJS = decodeur.o codeur.o pixmap_io.o
OUT	= decodeur codeur
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: decodeur codeur

decodeur: decodeur.o pixmap_io.o
	$(CC) -g decodeur.o pixmap_io.o -o decodeur $(LFLAGS)

codeur: codeur.o pixmap_io.o
	$(CC) -g codeur.o pixmap_io.o -o codeur $(LFLAGS)

decodeur.o: decodeur.c
	$(CC) $(FLAGS) decodeur.c 

pixmap_io.o: pixmap_io.c
	$(CC) $(FLAGS) pixmap_io.c 

clean:
	rm -f $(OBJS) $(OUT)