CFLAGS = -O2 -Wall

all: pcsensor

pcsensor: pcsensor.c
	${CC} -o $@ $^ -lusb

clean:
	rm -f pcsensor *.o
