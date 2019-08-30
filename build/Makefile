CC=gcc

UNAME := $(shell uname)

DEBUG ?= 0

ifeq ($(DEBUG), 1)
	CFLAGS=-O0 $(INCLUDES) -Wall -g -DDEBUG
	LDFLAGS= -lc -lm -ldl -lpthread -lasan
else
	CFLAGS=-O3 $(INCLUDES) -Wall
	LDFLAGS= -lc -lm -ldl -s
endif

ifeq ($(UNAME), Darwin)
CFLAGS += -arch x86_64 -mmacosx-version-min=10.9
LDFLAGS += -arch x86_64 -mmacosx-version-min=10.9
endif

ifeq ($(UNAME), Linux)
LDFLAGS += -Wl,-rpath=.
endif

ifeq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
endif

EXEC=qdhxcfe

all: $(EXEC)

qdhxcfe:  qdhxcfe.o utils.o hfe_qd.o trk_utils.o qd_mo5.o qd_roland.o crc.o
	$(CC) -o $@    $^ $(LDFLAGS)

qdhxcfe.o: ../src/qdhxcfe.c
	$(CC) -o $@ -c $< $(CFLAGS)

trk_utils.o: ../src/trk_utils.c
	$(CC) -o $@ -c $< $(CFLAGS)

utils.o: ../src/utils.c
	$(CC) -o $@ -c $< $(CFLAGS)

hfe_qd.o: ../src/hfe_qd.c
	$(CC) -o $@ -c $< $(CFLAGS)

qd_mo5.o: ../src/qd_mo5.c
	$(CC) -o $@ -c $< $(CFLAGS)

qd_roland.o: ../src/qd_roland.c
	$(CC) -o $@ -c $< $(CFLAGS)

crc.o: ../src/crc.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf *.so

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper

