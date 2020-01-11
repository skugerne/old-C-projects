CC = g++
CFLAGS = -O2 -Wall
EXE = col_test

OS = $(shell uname -s)
ifeq ("$(OS)","Darwin")
    # OSX install
    
    CFLAGS += -DOPEN_GL='<OpenGL/GL.h>'
    
    # if SDL and SDL_net are installed 'the unix way'
    LDFLAGS = -lSDL -lSDL_ttf -lSDLmain -framework Cocoa -framework OpenGL
    
    # if SDL and SDL_net are installed as frameworks
    # this fails to link, not sure if I should add -lSDLmain
    #LDFLAGS = -framework Cocoa -framework OpenGL -framework SDL

else
    # not-OSX install
    
    CFLAGS += -DOPEN_GL='<GL/gl.h>'
    LDFLAGS = -lSDL -lGL -lSDL_ttf
endif

#--------------------------------------------------------------------------

O_FILES = main.o other.o font.o object.o

#--------------------------------------------------------------------------

all: ttf_test
        
ttf_test: clear $(O_FILES) 
	$(CC) $(O_FILES) $(LDFLAGS) -o $(EXE)

#--------------------------------------------------------------------------
# source files

font.o: main.h font.h font.c
	@echo compiling font; $(CC) -c $(CFLAGS) font.c

main.o: main.h font.h object.h  main.c
	@echo compiling main; $(CC) -c $(CFLAGS) main.c

object.o: main.h object.h object.c
	@echo compiling object; $(CC) -c $(CFLAGS) object.c
        
other.o: main.h font.h object.h other.c
	@echo compiling other; $(CC) -c $(CFLAGS) other.c
    
#--------------------------------------------------------------------------
        
clear:
	clear
	rm -f core

clean:
	rm -f core *.o $(EXE)
