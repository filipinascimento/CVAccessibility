TARGET = CVAccessibility
LIBS = -lm
DEFINITIONS = 
CC = cc
CFLAGS = -g -Wall -std=c11 -m64 -Wall -O3 -fvisibility=hidden -funroll-loops -fstrict-aliasing

SRCDIR   = Source
OBJDIR   = Objects
BINDIR   = Build

ifeq ($(OS),Windows_NT)
    DEFINITIONS += -D WIN32
    CFLAGS += -fopenmp
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        DEFINITIONS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            DEFINITIONS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            DEFINITIONS += -D IA32
        endif
    endif
    BINDIR := $(BINDIR)_Windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        DEFINITIONS += -D LINUX
    	CFLAGS += -fopenmp
        BINDIR := $(BINDIR)_Linux
    endif
    ifeq ($(UNAME_S),Darwin)
        DEFINITIONS += -D OSX -DCV_USE_LIBDISPATCH=1
        BINDIR := $(BINDIR)_macOS
    endif
endif


.PHONY: default all clean

default: $(BINDIR)/$(TARGET)
all: default


SOURCES  := $(wildcard $(SRCDIR)/*.c)
#INCLUDES := $(wildcard $(SRCDIR)/*.h)
#OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
#OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
#HEADERS = $(wildcard *.h)

.PRECIOUS: $(TARGET) $(SOURCES)

$(BINDIR)/$(TARGET): $(SOURCES)| $(BINDIR)
	$(CC) $(SOURCES) -Wall $(LIBS) $(DEFINITIONS) -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	-rm -f $(BINDIR)/$(TARGET)
	-rm -fr $(BINDIR)

