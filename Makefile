CC = gcc
CFLAGS = -Wall -O2 -lgroonga -I/usr/include/groonga

all: query_sample db_sample kvs_sample setop_sample

query_sample : src/query_sample.c
	$(CC) src/query_sample.c -o query_sample.o $(CFLAGS)

db_sample : src/db_sample.c
	$(CC) src/db_sample.c -o db_sample.o $(CFLAGS)

kvs_sample : src/kvs_sample.c
	$(CC) src/kvs_sample.c -o kvs_sample.o $(CFLAGS)

setop_sample : src/setop_sample.c
	$(CC) src/setop_sample.c -o setop_sample.o $(CFLAGS)

clean:
	rm -rf query_sample.o db_sample.o kvs_sample.o setop_sample.o

