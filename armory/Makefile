CC=g++
CFLAG= -g
ROOTFLAG=`root-config --cflags --glibs`

all: EventBuilder

EventBuilder: EventBuilder.cpp SolReader.h Hit.h
	$(CC) $(CFLAG) EventBuilder.cpp -o EventBuilder ${ROOTFLAG}


clean:
	-rm EventBuilder