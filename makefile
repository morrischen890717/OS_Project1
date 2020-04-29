all: os_project.c
	gcc os_project.c -o os_project.o
clean:
	rm -f os_project.o