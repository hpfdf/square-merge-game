CXX=g++
LD=ld
RM=rm
CXXFLAGS="--std=c++0x -Wall -Werror -Wextra -O3 -c"

curses-ui: game.o curses-ui.o
	$LD game.o curses-ui.o -lcurses -osquare-merge-game

curses-ui.o: curses-ui.cpp game.h
	$CXX $CXXFLAGS curses-ui.cpp -omain.o

game.o: game.cpp game.h register.h
	$CXX $CXXFLAGS game.cpp -ogame.o

clean:
	$RM *.o
