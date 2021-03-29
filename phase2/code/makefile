final: admin.exe clean

admin.exe: adminUI.o system.o admin.o submit.o process.o cpu.o\
           loader.o memory.o paging.o idle.o swap.o term.o clock.o
	gcc -g -o admin.exe adminUI.o system.o admin.o submit.o process.o cpu.o\
           loader.o memory.o paging.o idle.o swap.o term.o clock.o -lpthread

adminUI.o: adminUI.c simos.h
	gcc -g -c adminUI.c -w

system.o: system.c simos.h
	gcc -g -c system.c
# Initlize the system and start admin, client, term processing

admin.o: admin.c simos.h
	gcc -g -c admin.c
# Read admin commands and process them, drive the whole system
 
submit.o: submit.c simos.h
	gcc -g -c submit.c
# Interface with the client to accept client submissions.
# Call submit process in process.c.

process.o: process.c simos.h
	gcc -g -c process.c
# Manage the processes in the system.
# Accept client submissions and prepare the process, call memory to allocate
# memory pages and load program to memory, call cpu to execute processes,
# interact with clock to advance clock and set timers
cpu.o: cpu.c simos.h
	gcc -g -c cpu.c


memory.o: memory.c simos.h
	gcc -g -c memory.c
# Simulate physical memory and access operations


idle.o: idle.c simos.h
	gcc -g -c idle.c
# init, load, and execute the idle process

clock.o: clock.c simos.h
	gcc -g -c clock.c
# Simulate the clock. Host the advance_clock function for clock.
# The remaining functions are for timers. Users can set different timers
# and when time is up there will  timer interrupt.

clean: 
	rm cpu.o process.o admin.o system.o submit.o memory.o clock.o idle.o adminUI.o

