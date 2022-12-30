all: obj1 obj2
obj1: manager.o
		gcc -g manager.o
prog1.o: manager.c
		gcc -Wall manager.c
obj2: agent.o
		gcc -g agent.o
prog2.o: agent.c
		gcc -Wall agent.c
clean:
	rm -f *.o
	rm -f manager
	rm -f agent