CC = gcc
CFLAGS = -Wall -std=c99 
LDFLAGS = -lraylib -lm -ldl -lpthread -lGL -lrt -lX11

all: build run

build: main

main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LDFLAGS)

run: main
	./main

clean:
	rm -f main
