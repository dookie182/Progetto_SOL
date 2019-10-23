CC		=  gcc
AR              =  ar
CFLAGS	        += -std=c99 -Wall -g -Werror
ARFLAGS         =  rvs
INCLUDES	= -I.
LDFLAGS 	= -L.
OPTFLAGS	= -O3 
LIBS            = -pthread

# aggiungere qui altri targets
TARGETS		= server   \
		  client   \
		  supervisor   

.PHONY: all clean cleanall
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)

test 		: $(TARGETS)
	make clean
	./test.sh

clean		: 
	rm -f *.o *~ *.a *.log OOB-server-*
	
cleanall	: clean
	rm -f $(TARGETS)
	
	

