CC=g++
ROOTFLAG=`root-config --cflags --glibs`

all: EventBuilder

EventBuilder: EventBuilder.cpp SolReader.h
	$(CC) EventBuilder.cpp -o EventBuilder ${ROOTFLAG}


clean:
	-rm EventBuilder