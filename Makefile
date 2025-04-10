CC = gcc
LDFLAGS = -ljansson -lX11

all:
	$(CC) -o contorler_app contorler_app.c $(LDFLAGS)

clean:
	rm -f contorler_app
