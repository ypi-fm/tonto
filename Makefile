OS := $(shell uname -s)

# Directories
PORTAUDIO_DIR  = portaudio
PORTMIDI_DIR   = portmidi

# Static Libraries
LIBPORTAUDIO   = $(PORTAUDIO_DIR)/lib/.libs/libportaudio.a
LIBPORTMIDI    = $(PORTMIDI_DIR)/libportmidi.a

# Common Compiler Settings
CC      = gcc
CFLAGS  = -I$(PORTAUDIO_DIR)/include -I$(PORTMIDI_DIR)/pm_common

# Base linker flags
LDFLAGS =
LDLIBS	= -lpthread -lm

# OS Specific linker flags
#TODO: Work in progress for other platforms and drivers
ifeq ($(OS),Darwin)
	LDLIBS  += \
		-framework AudioToolbox \
		-framework CoreAudio \
		-framework CoreFoundation \
		-framework CoreMIDI
endif

.PHONY: all clean

all: tontocore

# Build PortAudio
$(LIBPORTAUDIO):
	cd $(PORTAUDIO_DIR) && ./configure && make

# Build PortMidi
$(LIBPORTMIDI):
	cd $(PORTMIDI_DIR) && cmake . -DBUILD_SHARED_LIBS=OFF && make

# Build TontoCore
tontocore: main.c $(LIBPORTAUDIO) $(LIBPORTMIDI)
	$(CC) $(CFLAGS) main.c $(LIBPORTAUDIO) $(LIBPORTMIDI) $(LDLIBS) -o $@

clean:
	rm -f tontocore
	cd $(PORTAUDIO_DIR) && make clean
	cd $(PORTMIDI_DIR) && make clean
