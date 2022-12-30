all: obj1 obj2
obj1: manager.c
		gcc -o manager manager.c
obj2: agent.c
		gcc -o agent agent.c
clean:
	rm -f *.o
	rm -f manager
	rm -f agent
	rm -f *.out