CFLAGS= -Wall
CFLAGS+=-I./include
CSTFLAGS =-DDEBUG

LIBS= -lcurl
LIBS+= -ljansson
LIBS+= -lpthread

SRC = $(wildcard ./src/*.c)

all: bin/main bin/mainDEBUG bin/detection bin/detectionDEBUG serveurDEBUG

bin/detection: src/detection.cpp src/ptz.c
	@g++ $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` -lcurl

bin/detectionDEBUG: src/detection.cpp src/ptz.c
	@g++ $(CSTFLAGS) $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` -lcurl

execDetection: ./bin/detection
	./bin/detection "videos/videoEntrainementDev.mp4"

DetectionDirect: ./bin/detection
	./bin/detection "http://serveur:serveur@192.168.1.48/axis-cgi/mjpg/video.cgi?resolution=1280x720&fps=25&compression=25"

exec: bin/mainDEBUG serveurDEBUG bin/detectionDEBUG
	sudo -u www-data ./bin/mainDEBUG

bin/main: $(SRC) 
	@gcc $(CFLAGS) $^ -o $@ $(LIBS)

bin/mainDEBUG: $(SRC)
	@touch ./debug.log
	@gcc $(CSTFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)

bin/positions: src/positions.c
	@gcc $(CFLAGS) $^ -o $@ $(LIBS)

serveur: serveur/action.c
	@gcc -Wall -I./serveur $^ -o serveur/action.cgi -lpthread

serveurDEBUG: serveur/action.c
	@gcc -Wall $(CSTFLAGS) -I./serveur $^ -o serveur/action.cgi -lpthread

clean:
	@rm -f ./bin/*

cleanServeur:
	@rm -f ./serveur/*.cgi

resetLog:
	@echo 2> ./debug.log
	@echo 2> ./debugCgi.log






