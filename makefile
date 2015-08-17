CC=g++
CFLAGS=-c -Wall -std=c++11 -DDEBUG
#add -DDEBUG to CFLAGS for debugging messages
LDFLAGS=-lncurses
SUBFOLDERS=src
OBJFOLDER=obj
EXECUTABLE=test.exe

#shouldn't need to change these
VPATH=$(SUBFOLDERS)
SOURCES=$(wildcard *.cpp $(addsuffix /*.cpp,$(SUBFOLDERS)))
OBJECTS=$(patsubst %.cpp,$(OBJFOLDER)/%.o,$(notdir $(SOURCES) ) )

all: $(SOURCES) $(OBJFOLDER)/$(EXECUTABLE)

$(OBJFOLDER)/$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJFOLDER)/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY : clean
clean:
	rm -f $(OBJFOLDER)/*.o $(OBJFOLDER)/$(EXECUTABLE)

.PHONY : rebuild
rebuild: clean all
