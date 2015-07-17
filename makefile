CC=g++
CFLAGS=-c -Wall -std=c++11 -lncurses
LDFLAGS=-lncurses
SUBFOLDERS=Components Systems
OBJFOLDER=obj
EXECUTABLE=test.exe

#shouldn't need to change these
VPATH=$(SUBFOLDERS)
SOURCES=$(wildcard *.cpp $(addsuffix /*.cpp,$(SUBFOLDERS)))
OBJECTS=$(patsubst %.cpp,$(OBJFOLDER)/%.o,$(notdir $(SOURCES) ) )

all: $(SOURCES) $(OBJFOLDER)/$(EXECUTABLE)
	
$(OBJFOLDER)/$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJFOLDER)/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY : clean
clean:
	rm -f obj/*.o $(EXECUTABLE)

.PHONY : rebuild
rebuild: clean all