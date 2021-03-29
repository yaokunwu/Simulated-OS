#include "simos.h"
#include <signal.h>

int childr;
int childw;
FILE *fchildw;
char act;
char buf[1024];
void sig_handler(int sig_num) {
	read (childr, buf, sizeof (buf));
	act = buf[0];
	set_interrupt(adcmdInterrupt);
}
void execute_process_iteratively (int round, int read, int write)
{ int i;
	childr = read;
	childw = write;
  	fchildw = fdopen(write, "w");
	signal(SIGRTMIN, sig_handler);
  for (i=0; i<round && systemActive==1; i++) execute_process();
  systemActive = 0;
}

void one_admin_command (char act)
{ char fname[100];

  switch (act)
  { case 'T':  // Terminate simOS
      systemActive = 0; break;
    case 's':  // submit a program: should not be admin's command
      program_submission (); break;
    case 'q':  // dump ready queue and list of processes completed IO
      dump_ready_queue (fchildw); dump_endIO_list (fchildw); break;
    case 'r':   // dump the list of available PCBs
      dump_registers (fchildw); break;
    case 'p':   // dump the list of available PCBs
      dump_PCB_list (fchildw); break;
    case 'm':   // dump memory of each process
      dump_PCB_memory (fchildw); break;
    case 'f':   // dump memory frames and free frame list
      dump_memoryframe_info (fchildw); break;
    case 'n':   // dump the content of the entire memory
      dump_memory (fchildw); break;
    case 'e':   // dump events in clock.c
      dump_events (fchildw); break;
    case 't':   // dump terminal IO queue
      dump_termIO_queue (fchildw); break;
    case 'w':   // dump swap queue
      dump_swapQ (fchildw); break;
    default:   // can be used to yield to client submission input
      fprintf (fchildw, "Error: Incorrect command!!!\n");
  }
}

// Admin has been signaled, got an admin command
// signal handler stores the command somewhere (could be multiple commands)
//       and raise the adcmdInterrupt
// The interrupt handler now calls this function to process the commands
// You need to rewrite this function to retrieve the commands for processing
//       instead of reading them from stdin
// You alson need to change system.c to not to call this function


void process_admin_commands ()
{ char action[10];
	
  	one_admin_command(act);
	if (act != 's') {
		fflush(fchildw);		
	}
	usleep(1000);
	if (act == 's') {
		kill(getppid(),SIGINT); 
	} 
}

