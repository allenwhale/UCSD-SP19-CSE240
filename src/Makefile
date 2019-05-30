CC=g++
OPTS=-g -std=c++14 -O2 -Werror -Wall -Wshadow

all: main.o predictor.o tage.o tage.h
	$(CC) $(OPTS) -lm -o predictor main.o predictor.o tage.o

main.o: main.cpp predictor.h tage.h
	$(CC) $(OPTS) -c main.cpp

predictor.o: predictor.h predictor.cpp tage.h
	$(CC) $(OPTS) -c predictor.cpp

tage.o: tage.h tage.cpp
	$(CC) $(OPTS) -c tage.cpp

clean:
	rm -f *.o predictor
