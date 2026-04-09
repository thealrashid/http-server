cc = GCC
CFLAGS = -Wall -Wextra -g

SRC = src/main.c src/server.c
OBJ = $(SRC:.c=.o)

TARGET = http_server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)