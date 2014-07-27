CC = gcc
CFLAGS = -Wall -O2 -lgroonga -I/usr/include/groonga

all: query_sample db_sample kvs_sample

query_sample : src/query_sample.c
	$(CC) src/query_sample.c -o query_sample $(CFLAGS)

db_sample : src/db_sample.c
	$(CC) src/db_sample.c -o db_sample $(CFLAGS)

kvs_sample : src/kvs_sample.c
	$(CC) src/kvs_sample.c -o kvs_sample $(CFLAGS)

clean:
	rm -rf query_sample db_sample kvs_sample

