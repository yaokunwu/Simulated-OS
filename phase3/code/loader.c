#include "simos.h"

// need to be consistent with memory.c: mType and constant definitions
#define opcodeShift 24
#define operandMask 0x00ffffff
#define diskPage -2

FILE *progFd;
int numInstr, numData;

//==========================================
// This module loads program into swap space and memory, called by process.c
// Need to provide pid of the process to be loaded ==> allow access to PCB
//    Note: the pid in CPU register is not the same as this pid
//==========================================


//============= Part 1: from program file to buffer
// Open and read the parameters of the program file
// Read instructions/data from program file, and copy them to a buffer
//   (instruction: opcode and operand are merged into 1 memory word)
//   (each buffer is of the size of a memory page)
// The buffer will be copied to swap space and memory in Part 2

int init_programfile (int pid, char *fname)
{ int ret, msize;

  progFd = fopen (fname, "r");
  if (progFd == NULL)
  { fprintf (infF,
      "\aSubmission Error: Incorrect program file: %s!\n", fname);
    return (progError);
  }
  ret = fscanf (progFd, "%d %d %d\n", &msize, &numInstr, &numData);
  if (ret < 3) 
  { fprintf (infF,
      "\aProgram Error: Missing program parameters in %s\n", fname);
    close (progError); return (progError);
  }
  if (msize != numInstr+numData || msize > maxPpages*pageSize)
  { fprintf (infF,
      "\aProgram Error: Incorrect program parameters in %s\n", fname);
    close (progError); return (progError);
  }
  if (cpuDebug)
    fprintf (bugF, "Program info: %d %d %d\n", msize, numInstr, numData);
  return (progNormal);
}

int load_instruction (mType *buf, int page, int offset, int numInstr)
{ int ret, opcode, operand;
 
  ret = fscanf (progFd, "%d %d\n", &opcode, &operand);
  if (ret < 2) 
  { fprintf (infF, "\aProgram Error: Incorrect instruction\n");
    return (progError);
  }
  opcode = opcode << opcodeShift;
  operand = operand & operandMask;
  // operand = (operand + numInstr) & operandMask;
  buf[offset].mInstr = opcode | operand;
  if (cpuDebug)
    fprintf (bugF, "Load instruction (%x, %d) into M(%d, %d)\n",
                   opcode, operand, page, offset);
  return (progNormal);
}

int load_data (mType *buf, int page, int offset)
{ int ret;
  mdType data;

  ret = fscanf (progFd, mdInFormat"\n", &data);
  if (ret < 1) 
  { fprintf (infF, "\aProgram Error: Incorrect data\n");
    return (progError);
  }
  buf[offset].mData = data;
  if (cpuDebug)
    fprintf (bugF, "Load data: "mdOutFormat" into M(%d, %d)\n",
                   data, page, offset);
  return (progNormal);
}

//=============== Part 2: write buffer to swap space and memory

// load program to swap space, return the number of pages loaded
// or return progError if the program has problems
int load_process_to_swap (int pid)
{ mType *buf;
  int ret, i, page, count;

  init_process_pagetable (pid);

  // Add instruction and data to swap space, starting from page 0
  // continue till everything is loaded
  // Loading may cross multiple pages, each page triggers a swapQ reqest
  page = 0; count = 0;
  buf = (mType *) malloc (pageSize*sizeof(mType));
  for (i=0; i<numInstr; i++)
  { ret = load_instruction (buf, page, count, numInstr);
    if (ret == progError) return (progError);
    count++;
    if (count % pageSize == 0) 
    { update_process_pagetable (pid, page, diskPage);
      insert_swapQ (pid, page, (unsigned *)buf, actWrite, freeBuf);
      page++; count = 0;
      buf = (mType *) malloc (pageSize*sizeof(mType));
         // buf will be freed up in swap.c
  } }

  // *** ADD CODE: load data to swap space
  // should continue to load to the current page till it is filled

  if (count == 0) free(buf);   // malloc'ed an extra buf, which is not used
  else   // the last page has not finished processing
  { for (i=count; i<pageSize; i++)
      buf[i].mData = 0;   // zero the remaining buffer content
    insert_swapQ (pid, page, (unsigned*)buf, actWrite, freeBuf);
    update_process_pagetable (pid, page, diskPage);
    page++;   // this is now the total number of pages loaded
  }

  if (cpuDebug)
  { fprintf (bugF, "Load %d pages for process %d\n", page, pid);
    dump_process_pagetable (bugF, pid);
  }
  return (page);
}

// load pages to memory, #pages to load = min (loadPpages, lastpage)
// after loading last page, ask swap to place process to ready queue
int load_pages_to_memory (int pid, int numpage)
{ int i, lastpage;

  if (pid < idlePid || pid > maxProcess)
  { fprintf (infF, "Process ID for load is out of bound: pid = %d\n", pid);
    exit (-1);
  }
  if (numpage > loadPpages) lastpage = loadPpages - 1;
  else lastpage = numpage - 1;

  // *** ADD CODE: call swap_in_page to swap-in initial pages to memory
  // pages to be swapped-in: page 0 to lastpage
}

// load program to swap space & memory, return the number of pages loaded
// or return progError if the program has problems
int load_process (int pid, char *fname)
{ int ret;

  ret = init_programfile (pid, fname);
  if (ret == progError) return (progError);   // invalid program 

  ret = load_process_to_swap (pid);   // return #pages loaded
  if (ret != progError) load_pages_to_memory (pid, ret); 

  fclose (progFd);
  return (ret);
}

