CC = gcc

NAME = nougat
VERSION = 0.0.1

LIBS = -lgit2 -lconfig
INCLUDES =
CFLAGS = -Wall -O2 -Werror=format-security
CFLAGS += $(LIBS)

all: $(NAME)

HEADERS = \
	   src/utils.h \
	   src/index.h \
	   src/constants.h \
	   src/config.h

SOURCES = \
	   src/main.c \
	   src/utils.c \
	   src/index.c \
	   src/config.c

OBJECTS = ${SOURCES:.c=.o}

$(NAME): $(HEADERS) $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(NAME) $(OBJECTS)
