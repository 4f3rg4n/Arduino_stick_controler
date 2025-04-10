CC = gcc
LDFLAGS = -ljansson -lX11

all:
	$(CC) -o controler_app controler_app.c $(LDFLAGS)

clean:
	rm -f controler_app
