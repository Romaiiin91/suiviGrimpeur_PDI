CFLAGS= -Wall
CFLAGS+=-I./include
CSTFLAGS =-DDEBUG 

ifeq ($(PC), 1)
	CSTFLAGS +=-DPC
endif


LIBS= -lcurl
LIBS+= -ljansson
LIBS+= -lpthread
LIBS+= -lrt

execPC: 
	xhost +SI:localuser:www-data
	make ExecDEBUG PC=1 -j4
	sudo -u www-data ./bin/mainDEBUG

execPi: ExecDEBUG
	sudo -u www-data env LD_LIBRARY_PATH=/usr/local/lib /home/pi/suiviGrimpeur/bin/mainDEBUG


all: bin/detection bin/detectionDEBUG ./bin/detectionVIDEO bin/main bin/mainDEBUG action actionDEBUG video videoDEBUG bin/ecritureMemoire bin/ecritureMemoireDEBUG bin/enregistrementVideo bin/enregistrementVideoDEBUG

###########################
#		DETECTION		  #
###########################																

bin/detection: src/detection.cpp src/ptz.c
	g++ $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` $(LIBS)

bin/detectionDEBUG: src/detection.cpp src/ptz.c
	g++ $(CSTFLAGS) $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` $(LIBS)

bin/detectionVIDEO: src/detection.cpp src/ptz.c
	g++ $(CSTFLAGS) -DVIDEO $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` $(LIBS)

detectionTest : ./bin/detectionVIDEO
	./bin/detectionVIDEO "server/videos/20250109_2148_2_2_voie1_resize.mp4"

DetectionDirect: ./bin/detectionDEBUG
	./bin/detectionDEBUG "http://serveur:serveur@192.168.1.1.4/axis-cgi/mjpg/video.cgi?resolution=1280x720&fps=25&compression=25"


###########################
#		   MAIN			  #
###########################	


bin/main: src/main.c src/positions.c src/ptz.c 
	gcc $(CFLAGS) $^ -o $@ $(LIBS)

bin/mainDEBUG: src/main.c  src/ptz.c src/positions.c
	touch ./log/debug.log
	sudo chown :suiviGrimpeur ./log/debug.log
	sudo chmod g+w ./log/debug.log
	touch ./log/debugCgi.log
	sudo chown :suiviGrimpeur ./log/debugCgi.log
	sudo chmod g+w ./log/debugCgi.log
	gcc $(CSTFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)

ExecDEBUG: bin/mainDEBUG  bin/ecritureMemoireDEBUG serverDEBUG bin/detectionDEBUG bin/enregistrementVideoDEBUG


###########################
#		 SERVEUR		  #
###########################	


action: server/cgi-bin/action.c
	gcc $(CFLAGS) -I./server/cgi-bin $^ -o server/cgi-bin/action.cgi -lpthread -lrt

actionDEBUG: server/cgi-bin/action.c
	gcc $(CSTFLAGS) $(CFLAGS) -I./server/cgi-bin $^ -o server/cgi-bin/action.cgi -lpthread -lrt

video: server/cgi-bin/video.cpp
	g++ $(CFLAGS) -I./server/cgi-bin $^ -o server/cgi-bin/video.cgi `pkg-config --cflags --libs opencv4` $(LIBS) -ljpeg

videoDEBUG: server/cgi-bin/video.cpp
	g++ $(CSTFLAGS) $(CFLAGS) -I./server/cgi-bin $^ -o server/cgi-bin/video.cgi `pkg-config --cflags --libs opencv4` $(LIBS) -ljpeg

serverDEBUG: actionDEBUG videoDEBUG

###########################
#		 Mémoire		  #
###########################	

bin/ecritureMemoire: src/ecritureMemoire.cpp
	g++ $(CFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` -lrt -lpthread

bin/ecritureMemoireDEBUG: src/ecritureMemoire.cpp
	g++ $(CFLAGS) $(CSTFLAGS) $^ -o $@ `pkg-config --cflags --libs opencv4` -lrt -lpthread 

###########################
#	   ENREGISTREMENT	  #
###########################	


bin/enregistrementVideo: src/enregistrementVideo.c
	gcc $(CFLAGS) $^ -o $@ $(LIBS)

bin/enregistrementVideoDEBUG: src/enregistrementVideo.c
	gcc $(CFLAGS) $(CSTFLAGS) $^ -o $@ $(LIBS)


###########################
#		 CLEAN			  #
###########################	

clean: resetLog cleanServeur
	@rm -f ./bin/*

cleanServeur:
	@rm -f ./server/cgi-bin/*.cgi

resetLog:
	@echo 2> ./log/debug.log
	@echo 2> ./log/debugCgi.log






