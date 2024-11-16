CFLAGS= -Wall
CFLAGS+=-I./include
CSTFLAGS =-DDEBUG

LIBS= -lncurses
LIBS+= -lmenu
LIBS+= -lcurl

SRC = $(wildcard ./src/*.c)

all: bin/main bin/mainDEBUG bin/detection bin/detectionDEBUG

bin/detection: src/detection.cpp
	@g++ $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4`

bin/detectionDEBUG: src/detection.cpp
	@g++ $(CSTFLAGS) $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4`

execDetection:
	./bin/detection "http://serveur:serveur@192.168.1.12/axis-cgi/mjpg/video.cgi?resolution=1280x720&fps=25&compression=25"

bin/main: $(SRC) 
	@gcc $(CFLAGS) $^ -o $@ $(LIBS)

bin/mainDEBUG: $(SRC)
	@touch ./debug.log
	@gcc $(CSTFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)

bin/positions: src/positions.c
	@gcc $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	@rm -f ./bin/*





