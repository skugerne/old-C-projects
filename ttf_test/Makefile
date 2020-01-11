CC = g++
CFLAGS = -O2 -Wall
EXE = ttf_test

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

O_FILES = main.o other.o input.o font.o

#--------------------------------------------------------------------------

all: ttf_test
        
ttf_test: clear $(O_FILES) 
	$(CC) $(O_FILES) $(LDFLAGS) -o $(EXE)

#--------------------------------------------------------------------------
# source files

main.o: main.h font.h main.c
	@echo compiling main; $(CC) -c $(CFLAGS) main.c
        
other.o: main.h font.h other.c
	@echo compiling other; $(CC) -c $(CFLAGS) other.c

input.o: main.h font.h input.c
	@echo compiling input; $(CC) -c $(CFLAGS) input.c

font.o: main.h font.h font.c
	@echo compiling font; $(CC) -c $(CFLAGS) font.c
    
#--------------------------------------------------------------------------
        
clear:
	clear
	rm -f core

clean:
	rm -f core *.o $(S_EXE) $(EXE)
