CC      := cc
CFLAGS  := -O2 -Wall -Wextra -Ithird_party/raylib/include
LDFLAGS := third_party/raylib/lib/libraylib.a \
           -lm -ldl -lpthread \
           -lGL -lX11 -lXrandr -lXi -lXcursor -lXinerama

all: game

game: main.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f game *.o
