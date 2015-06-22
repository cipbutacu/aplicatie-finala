CC=gcc
CFLAGS=-c -Wall  -I/usr/local/include  -I/usr/include/mysql -DBIG_JOINS=1  -fno-strict-aliasing  -g
LDFLAGS=-L/usr/local/lib  -lwiringPi  -lwiringPiDev -L/usr/lib -lwebsockets  -L/usr/lib/arm-linux-gnueabihf -lmysqlclient -lpthread -lz -lm -lrt -ldl
SOURCES=main.c homey.c out.c shmem.c db.c WebConnect.c Motors.c GPS.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=adas

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
	
clean:
	rm -rf *o homey
