CC = g++
CFLAGS = -O2 -Wall
EXE = roids

OS = $(shell uname -s)
ifeq ("$(OS)","Darwin")
    # OSX install
    
    CFLAGS += -DOPEN_GL='<OpenGL/GL.h>'
    
    # if SDL and SDL_net are installed 'the unix way'
    CFLAGS += -DSDL_NET='<SDL/SDL_net.h>'
    LDFLAGS = -lSDL -lSDLmain -lSDL_net -framework Cocoa -framework OpenGL
    
    # if SDL and SDL_net are installed as frameworks
    # this fails to link, not sure if I should add -lSDLmain
    #CFLAGS += -DSDL_NET='<SDL_net/SDL_net.h>'
    #LDFLAGS = -framework Cocoa -framework OpenGL -framework SDL \
    #    -framework SDL_net

else ifneq (,$(wildcard /etc/redhat-release))
    # centos or redhat 8
    CFLAGS += -DOPEN_GL='<GL/gl.h>' -DSDL_NET='<SDL/SDL_net.h>' -DLUA51 -I/usr/include/SDL
    LDFLAGS = -lSDL -lSDL_net -lGL -lpthread -llua-5.1
else
    CFLAGS += -DOPEN_GL='<GL/gl.h>' -DSDL_NET='<SDL/SDL_net.h>' -DLUA53
    LDFLAGS = -lSDL -lSDL_net -lGL -lpthread -llua5.3
endif

#--------------------------------------------------------------------------

O_FILES = main.o update.o draw.o input.o menu.o textureFont.o create.o \
        collision.o net.o particles.o objecttype.o shiptype.o roidtype.o \
        shottype.o startype.o starfieldtype.o playertype.o alientype.o \
        alienfightertype.o alienrambotype.o alienwandertype.o alienluatype.o

#--------------------------------------------------------------------------

all: roids
        
roids: clear $(O_FILES) 
	$(CC) $(O_FILES) $(LDFLAGS) -o $(EXE)

#--------------------------------------------------------------------------
# non-class files

main.o: main.hpp main.cpp
	@echo compiling main; $(CC) -c $(CFLAGS) main.cpp
        
update.o: main.hpp update.cpp
	@echo compiling update; $(CC) -c $(CFLAGS) update.cpp
        
draw.o: main.hpp draw.cpp
	@echo compiling draw; $(CC) -c $(CFLAGS) draw.cpp

input.o: main.hpp input.cpp
	@echo compiling input; $(CC) -c $(CFLAGS) input.cpp
        
textureFont.o: main.hpp textureFont.cpp
	@echo compiling textureFont; $(CC) -c $(CFLAGS) textureFont.cpp
        
menu.o: main.hpp menu.cpp
	@echo compiling menu; $(CC) -c $(CFLAGS) menu.cpp

create.o: main.hpp create.cpp
	@echo compiling create; $(CC) -c $(CFLAGS) create.cpp
        
collision.o: main.hpp objecttype.hpp collision.cpp
	@echo compiling collision; $(CC) -c $(CFLAGS) collision.cpp       

net.o: main.hpp net.cpp
	@echo compiling net; $(CC) -c $(CFLAGS) net.cpp
        
particles.o: main.hpp particles.cpp
	@echo compiling particles; $(CC) -c $(CFLAGS) particles.cpp

#--------------------------------------------------------------------------
# class files

objecttype.o: main.hpp objecttype.hpp objecttype.cpp
	@echo compiling objecttype; $(CC) -c $(CFLAGS) objecttype.cpp
        
shiptype.o: main.hpp objecttype.hpp shiptype.hpp shiptype.cpp
	@echo compiling shiptype; $(CC) -c $(CFLAGS) shiptype.cpp 
        
roidtype.o: main.hpp objecttype.hpp roidtype.hpp roidtype.cpp
	@echo compiling roidtype; $(CC) -c $(CFLAGS) roidtype.cpp
        
shottype.o: main.hpp objecttype.hpp shottype.hpp shottype.cpp
	@echo compiling shottype; $(CC) -c $(CFLAGS) shottype.cpp    

startype.o: main.hpp objecttype.hpp startype.hpp startype.cpp
	@echo compiling startype; $(CC) -c $(CFLAGS) startype.cpp

starfieldtype.o: main.hpp starfieldtype.hpp starfieldtype.cpp
	@echo compiling starfieldtype; $(CC) -c $(CFLAGS) starfieldtype.cpp

playertype.o: main.hpp playertype.hpp playertype.cpp
	@echo compiling playertype; $(CC) -c $(CFLAGS) playertype.cpp

alientype.o: main.hpp objecttype.hpp alientype.hpp alientype.cpp
	@echo compiling alientype; $(CC) -c $(CFLAGS) alientype.cpp
        
alienfightertype.o: main.hpp objecttype.hpp alientype.hpp alienfightertype.hpp alienfightertype.cpp
	@echo compiling alienfightertype; $(CC) -c $(CFLAGS) alienfightertype.cpp
        
alienrambotype.o: main.hpp objecttype.hpp alientype.hpp alienrambotype.hpp alienrambotype.cpp
	@echo compiling alienrambotype; $(CC) -c $(CFLAGS) alienrambotype.cpp

alienwandertype.o: main.hpp objecttype.hpp alientype.hpp alienwandertype.hpp alienwandertype.cpp
	@echo compiling alienwandertype; $(CC) -c $(CFLAGS) alienwandertype.cpp

alienluatype.o: main.hpp objecttype.hpp alientype.hpp alienluatype.hpp alienluatype.cpp
	@echo compiling alienluatype; $(CC) -c $(CFLAGS) alienluatype.cpp

#--------------------------------------------------------------------------

clear:
	clear
	rm -f core

clean:
	rm -f core *.o $(S_EXE) $(EXE)
