CC=g++
CFLAGS=-c -std=c++0x -I/usr/local/include/mysql++/ -I/usr/include/mysql/
LDFLAGS=-L/usr/lib/mysql -lmysqlclient -lmysqlpp -lwiringPi
SOURCES=lnglat.cpp db.cpp map.cpp controller.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o main