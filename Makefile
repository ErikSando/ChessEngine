NAME=ErikEngine

all:
	gcc -Ofast -o bin/$(NAME) src/*.c -I include

debug:
	gcc -o bin/$(NAME)Debug src/*.c -I include -D DEBUG