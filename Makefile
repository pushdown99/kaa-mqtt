OUT=main
SRC=main.c 
OBJ=$(SRC:.c=.o)

CC=gcc
CFLAGS=-I. -I./lib -I/usr/local/include -ggdb
LDFLAGS=-L./lib -lpopup -lpaho-mqtt3c -lpaho-mqtt3a 



all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -rf $(OUT) $(OBJ)
