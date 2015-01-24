# ------------------------------------------------
# Generic Makefile
#
# Author: yanick.rochon@gmail.com
# Date  : 2010-11-05
#
# Changelog :
#   0.01 - first version
# ------------------------------------------------

# change these to set the proper directories where each files should be
SRCDIR   = ./src
OBJDIR   = ./obj
BINDIR   = ./bin
INCDIR   = ./include
LIBDIR   = ./lib

# project name (generate executable with this name)
CC       = gcc

# compiling flags here
CFLAGS   = -Wall -I.
LINKER   = $(CC) -o -g

# linking flags here
LFLAGS   = -Wall -L$(LIBDIR)
LIBS     = -lspeex

all: dir_create speex

speex: speex_obj
	$(CC) $(OBJDIR)/speex.o -o $(BINDIR)/speex $(LFLAGS) $(LIBS)
	
speex_obj: $(SRCDIR)/speex.c
	$(CC) -I$(INCDIR) $(CFLAGS) -c $(SRCDIR)/speex.c -o $(OBJDIR)/speex.o

dir_create:
ifeq "$(wildcard $(OBJDIR) )" ""
	mkdir -p $(OBJDIR)
endif
ifeq "$(wildcard $(BINDIR) )" ""
	mkdir -p $(BINDIR)
endif

clean:
	rm -rf $(BINDIR)/speex
	rm -rf $(OBJDIR)/*.*
