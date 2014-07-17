CC = gcc
CFLAGS = -Wall -O2 -lgroonga -I/usr/include/groonga

all: ctx_sample table_sample

ctx_sample : src/ctx_sample.c
	$(CC) src/ctx_sample.c -o ctx_sample.o $(CFLAGS)

table_sample : src/table_sample.c
	$(CC) src/table_sample.c -o table_sample.o $(CFLAGS)

clean:
	rm -rf ctx_sample.o table_sample.o

