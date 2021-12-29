CC=gcc
COMMON= -Wall -Wextra -Wfloat-equal -pedantic -std=c99 -Werror
SDLCFLAGS=`sdl2-config --cflags`
SDLLIBS=`sdl2-config --libs`
DEBUG= -g3
SANITIZE= $(COMMON) -fsanitize=undefined -fsanitize=address $(DEBUG)
VALGRIND= $(COMMON) $(DEBUG)
PRODUCTION= $(COMMON) -O3
LDLIBS = -lm

interp: interp.c neillsdl2.c neillsdl2.h stack.h Linked/specific.h Linked/linked.c General/general.h General/general.c
	$(CC) interp.c neillsdl2.c Linked/linked.c General/general.c -o interp -I./Realloc $(COMMON) $(SDLCFLAGS) $(SDLLIBS) $(LDLIBS)

sortingViz: sortingViz.c neillsdl2.c neillsdl2.h stack.h Linked/specific.h Linked/linked.c General/general.h General/general.c
	$(CC) sortingViz.c neillsdl2.c Linked/linked.c General/general.c -o sortingViz -I./Realloc $(COMMON) $(SDLCFLAGS) $(SDLLIBS) $(LDLIBS)

clean:
	rm -fr demo_neillsimplescreen demo_neillncurses demo_neillsdl2































