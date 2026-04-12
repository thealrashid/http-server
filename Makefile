cc = GCC
CFLAGS = -Wall -Wextra -g
INCLUDES = -Iinclude

SRC = src/main.c \
	  src/server.c \
	  src/response.c \
	  src/parser.c \
	  src/file.c \
	  src/handler.c
OBJ = $(SRC:.c=.o)

TARGET = http_server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)