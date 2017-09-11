CFLAGS=-g -Wall -W -I../../..

run: run.o net.o noerr.o

run-bind: run-bind.o net.o norr.o

net.o: ../net.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

noerr.o: ../../noerr/noerr.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<
