CFLAGS= -Wall
CFLAGS+=-I./include
CSTFLAGS =-DDEBUG

LIBS= -lcurl
LIBS+= -ljansson
LIBS+= -lpthread

SRC = $(wildcard ./src/*.c)

all: bin/detection bin/detectionDEBUG ./bin/detectionVIDEO bin/main bin/mainDEBUG action actionDEBUG video videoDEBUG bin/ecritureMemoire bin/ecritureMemoireDEBUG bin/enregistrementVideo bin/enregistrementVideoDEBUG

###########################
#		DETECTION		  #
###########################																

bin/detection: src/detection.cpp src/ptz.c
	@g++ $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` $(LIBS)

bin/detectionDEBUG: src/detection.cpp src/ptz.c
	@g++ $(CSTFLAGS) $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` $(LIBS)

bin/detectionVIDEO: src/detection.cpp src/ptz.c
	@g++ $(CSTFLAGS) -DVIDEO $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` $(LIBS)

detectionTest : ./bin/detectionVIDEO
	./bin/detectionVIDEO "serveur/videos/20250109_2148_2_2_voie1_resize.mp4"

DetectionDirect: ./bin/detectionDEBUG
	./bin/detectionDEBUG "http://serveur:serveur@192.168.0.4/axis-cgi/mjpg/video.cgi?resolution=1280x720&fps=25&compression=25"


###########################
#		   MAIN			  #
###########################	


bin/main: src/main.c src/positions.c src/ptz.c 
	@gcc $(CFLAGS) $^ -o $@ $(LIBS)

bin/mainDEBUG: src/main.c src/positions.c src/ptz.c 
	@touch ./debug.log
	@gcc $(CSTFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)

execDEBUG: bin/mainDEBUG serveurDEBUG bin/detectionDEBUG bin/ecritureMemoireDEBUG bin/enregistrementVideoDEBUG
	xhost +SI:localuser:www-data
	sudo -u www-data ./bin/mainDEBUG


###########################
#		 SERVEUR		  #
###########################	


action: serveur/action.c
	@gcc $(CFLAGS) -I./serveur $^ -o serveur/action.cgi -lpthread

actionDEBUG: serveur/action.c
	@gcc $(CSTFLAGS) $(CFLAGS) -I./serveur $^ -o serveur/action.cgi -lpthread

video: serveur/video.cpp
	@g++ $(CFLAGS) -I./serveur $^ -o serveur/video.cgi `pkg-config --cflags --libs opencv4` -lpthread -ljpeg

videoDEBUG: serveur/video.cpp
	@g++ $(CSTFLAGS) $(CFLAGS) -I./serveur $^ -o serveur/video.cgi `pkg-config --cflags --libs opencv4` -lpthread -ljpeg

serveurDEBUG : actionDEBUG videoDEBUG

###########################
#		 Mémoire		  #
###########################	

bin/ecritureMemoire: src/ecritureMemoire.cpp
	g++ $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` 

bin/ecritureMemoireDEBUG: src/ecritureMemoire.cpp
	g++ $(CFLAGS) $(CSTFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` 

###########################
#	   ENREGISTREMENT	  #
###########################	


bin/enregistrementVideo: src/enregistrementVideo.c
	@gcc $(CFLAGS) $^ -o $@ $(LIBS)

bin/enregistrementVideoDEBUG: src/enregistrementVideo.c
	@gcc $(CFLAGS) $(CSTFLAGS) $^ -o $@ $(LIBS)


###########################
#		 CLEAN			  #
###########################	

clean: resetLog
	@rm -f ./bin/*

cleanServeur:
	@rm -f ./serveur/*.cgi

resetLog:
	@echo 2> ./debug.log
	@echo 2> ./debugCgi.log






