CC = gcc

DEBUG = -g

STD = -std=c99

ADD_CFLAGS = -Wall $(DEBUG) $(STD)

INCPATH = -I $$HOME/ffmpeg_build/include

INC_DIR = ../include

LFLAGS = -lstdc++ -lX11 -lGL

ATWB_LIB = $$HOME/AntTweakBar/lib

FFMPEG_LIB = $$HOME/ffmpeg_build/lib

LIBS = -L $(ATWB_LIB) -L $(FFMPEG_LIB)

OBJ_DIR = ../obj

SRC_FILES = $(wildcard *.c)

OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(wildcard *.c)))

PKGS = sdl2 libavformat libavcodec libswscale libswresample

PKG_CFLAGS = $$(pkg-config --cflags $(PKGS))

PKG_LDFLAGS = $$(pkg-config --libs $(PKGS))

CFLAGS  = $(ADD_CFLAGS)

LDFLAGS = $(LIBS)

vpath %.h ../include

#############################################################################

.PHONY: all
all: $(OBJ_DIR) ../jsplayer
	echo $(SRCS)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

../jsplayer: $(OBJ_FILES)
	$(CC) $(ADD_CFLAGS) $(LDFLAGS) -o $@ $^ libAntTweakBar.a $(LFLAGS) \
	$(PKG_CFLAGS) $(PKG_LDFLAGS)

$(OBJ_DIR)/jsplayer.o: jsplayer.c
	$(CC) $(ADD_CFLAGS) -c $(INCPATH) -I$(INC_DIR) $< -o $@

$(OBJ_DIR)/%.o: %.c %.h
	$(CC) $(ADD_CFLAGS) -c $(INCPATH) -I$(INC_DIR) $< -o $@

.PHONY: clean
clean:
	\rm $(OBJ_DIR)/*.o
	rmdir $(OBJ_DIR)

#############################################################################

