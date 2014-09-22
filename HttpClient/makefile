CHTTP_OBJS =  	HttpClient.c

CC = g++


CFLAGS = -g --std=c99 -Wextra


PROGNAME = HttpClient


$(PROGNAME) : $(CHTTP_OBJS) 
		$(CC) $(CHTTP_OBJS) -o $@

.SUFFIXES : .o .c

.cpp.o :
		$(CC) $(CPPFLAGS) -c -o $@ $(CFLAGS) -I $(INCLUDEDIR) $<

.c.o :
		$(CC) $(CPPFLAGS) -c -o $@ $(CFLAGS) -I $(INCLUDEDIR) $<

clean :
	rm *.o $(PROGNAME)

