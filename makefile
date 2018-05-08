# all: interpreter scheduler p1 p2 p3 p4

scheduler: scheduler.c
	gcc -g -Wall -o scheduler scheduler.c

interpreter: interpreter.c
	gcc -g -Wall -o interpreter interpreter.c

p1: p1.c
	gcc -g -Wall -o p1 p1.c

p2: p2.c
	gcc -g -Wall -o p2 p2.c

p3: p3.c
	gcc -g -Wall -o p3 p3.c

p4: p4.c
	gcc -g -Wall -o p4 p4.c

clean: 
	$(RM) scheduler interpreter p1 p2 p3 p4

run: all
	./interpreter