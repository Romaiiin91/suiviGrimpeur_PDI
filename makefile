# Flags

CFLAGS = -Wall -I./include
LIBS = -lcurl -ljansson -lpthread -lrt
OPENCV_LIBS = `pkg-config --cflags --libs opencv4`


# Variables
ifeq ($(PC), 1)
	CFLAGS += -DPC -DX264
endif

ifeq ($(X264), 1)
	CFLAGS += -DX264
endif

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG
	CFLAGS += -g
endif



# Directories
BIN_DIR = bin
LOG_DIR = log
SERVER_DIR = server/cgi-bin
VIDEO_DIR = data/videos



# Compiler
CC = gcc
CXX = g++

# Build rules
all: $(BIN_DIR)/main $(BIN_DIR)/ecritureMemoire $(BIN_DIR)/enregistrementVideo $(BIN_DIR)/detection $(SERVER_DIR)/action.cgi $(SERVER_DIR)/video.cgi
 

#################
#  Executables  #
#################

# Server
$(SERVER_DIR)/action.cgi: src/action.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(SERVER_DIR)/video.cgi: src/video.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) $(OPENCV_LIBS) -ljpeg

# Main
$(BIN_DIR)/main: src/main.c src/positions.c src/ptz.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Write Images in memory
$(BIN_DIR)/ecritureMemoire: src/ecritureMemoire.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS) $(OPENCV_LIBS)

# Recording video
$(BIN_DIR)/enregistrementVideo: src/enregistrementVideo.c
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS)

# Detection of the climber
$(BIN_DIR)/detection: src/detection.cpp src/ptz.c
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS) $(OPENCV_LIBS)


# Directories setup
directoriesSetup:
	@if [ "$$(id -u)" -ne 0 ]; then echo "This script must be run as root. Use sudo." >&2; exit 1; fi
	mkdir -p $(BIN_DIR) $(LOG_DIR) $(SERVER_DIR) $(VIDEO_DIR)
	chown :suiviGrimpeur $(LOG_DIR) data $(BIN_DIR) $(SERVER_DIR) $(VIDEO_DIR)
	chmod g+w $(LOG_DIR) $(BIN_DIR) $(SERVER_DIR)
	
	touch $(LOG_DIR)/debug.log $(LOG_DIR)/debugCgi.log
	chown :suiviGrimpeur $(LOG_DIR)/debug.log $(LOG_DIR)/debugCgi.log
	chmod g+w $(LOG_DIR)/debug.log $(LOG_DIR)/debugCgi.log

	chmod o+w data/positionsEnregistrees.json


# Build
build: directoriesSetup $(BIN_DIR)/main $(BIN_DIR)/ecritureMemoire $(BIN_DIR)/enregistrementVideo $(BIN_DIR)/detection $(SERVER_DIR)/action.cgi $(SERVER_DIR)/video.cgi

# Cleaning rules
clean: resetLog
	@rm -f $(BIN_DIR)/*  $(SERVER_DIR)/*

resetLog:
	@echo "" > $(LOG_DIR)/debug.log
	@echo "" > $(LOG_DIR)/debugCgi.log

uninstall: clean
	@rm -rf $(BIN_DIR) $(LOG_DIR) $(SERVER_DIR) $(VIDEO_DIR)
	@echo "Directories and files removed"
