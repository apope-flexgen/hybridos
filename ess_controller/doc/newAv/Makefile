
CFLAGS += -g -Og
CFLAGS += -Wall -Werror -std=c++11
CPP = g++

all : newAv

clean:
	rm -f *.o newAv

newAv: main.o newAv2.o newAvCj.o newUtils.o Sched.o
	${CPP} ${CFLAGS} -o $@ $^ -lcjson -lfims

main.o: main.cpp newAv2.h newUtils.h
	${CPP} ${CFLAGS} -std=c++11 -c $< 

newAv2.o: newAv2.cpp newAv2.h
	${CPP} ${CFLAGS} -c $< 

newAvCj.o: newAvCj.cpp newAv2.h
	${CPP} ${CFLAGS} -c $< 

newUtils.o: newUtils.cpp newUtils.h
	${CPP} ${CFLAGS} -c $< 

Sched.o: Sched.cpp Sched.h
	${CPP} ${CFLAGS} -c $< 
