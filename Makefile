CC = gcc

NAME = nougat
VERSION = 0.0.1

LIBS = -lgit2
INCLUDES = -I/usr/local/include
CFLAGS = -Wall -O2 -Werror=format-security
CFLAGS += $(LIBS)

all: $(NAME)

HEADERS = \
	   src/utils.h \
	   src/index.h \
	   src/constants.h \

SOURCES = \
	   src/main.c \
	   src/utils.c \
	   src/index.c \

OBJECTS = ${SOURCES:.c=.o}

$(NAME): $(HEADERS) $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(NAME) $(OBJECTS)
