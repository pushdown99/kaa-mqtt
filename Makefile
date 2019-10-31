OUT=main
SRC=main.c 
OBJ=$(SRC:.c=.o)

CC=gcc
CFLAGS=-I. -I./lib -I/usr/local/include -ggdb
LDFLAGS=libpaho-mqtt3a.so 



all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -rf $(OUT) $(OBJ)
