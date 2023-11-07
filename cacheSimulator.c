#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>

/*
int log2(int numIn){
  int num;
  num = (int)(log10(numIn) / log10(2));
  return num;
}
*/

int main(int argc, char *argv[]){
  
  //printf("HelloWorld!\n");
  
  //Let's look at the following execution example:
  //Sim.exe –f trace1.trc –f trace2.trc –s 512 –b 16 –a 2 –r RR –p 1GBs
  
  //These program initializers can be as short as:
  //Sim.exe –f trace1.trc –s 512 –b 16 –a 2 –r RR –p 1GBs
  //OR be as long as: 
  //Sim.exe –f trace1.trc –f trace2.trc –f trace3.trc –s 512 –b 16 –a 2 –r RR –p 1GBs
  
  //This means we will have as few as 13 command line args (0-12) or as many as 17 (0-16).
  //This can be used to fill out all the info we need
  
  //Most importantly, we need to be able to identify what elements belong to each section of the command line
  //Example: if [command-line-arg]=="-f", then [command-line-arg++]==traceFileName
    
  //There's also many variations that the command line args can have, but we don't have to immediately worry about those until the args are processed.
  //We could assume that 1MB could just be put in as 1024 for now (all physical memory numbers are in bytes/KB. 64KB would just be 64).
  //Physical memory is the only command-line-arg that used a mem type in the args itself. Even the prof said this is unnecessary
  
  //The information printed/calculated for Milestone 1 must be done for each given trace file (each trace file is fully evaluated before the next)
  
  //Some quick error checking
  if (argc < 2) {
        printf("Usage: %s -f trace1.trc [-f trace2.trc ...] -s cacheSize -b blockSize -a associativity -r replacePolicy -p physicalSize\n", argv[0]);
        return 1;
    }
  
  
  int count = 0;//Use this to track number if times looped
  char* trace1Name;
  int trace1Found = 0;
  char* trace2Name;
  int trace2Found = 0;
  char* trace3Name;
  int trace3Found = 0;
  //Note to self: Could make an array of char arrays for trace files.
  int cacheSize = 0; //Stored in bytes/KB
  int blockSize = 0;
  int associativity = 0;
  char* replacePolicy;
  int physicalSize = 0; //Im going to convert the values to bytes/KB on the command line, thus it being an int. int luckily has enough room for the max size value 
  //The following values all require calculation
  int totalBlocks;
  int BUS_SIZE = 32; //THIS IS TECHNICALLY CONSTANT
  double COST_MULTIPLIER = 0.09; //ALSO CONSTANT, and in 0.09/KB
  int offset;//Might need to make a double
  int tagSize;
  int indexSize;
  int totalRows;
  int overhead;
  int implementationMemory; 
  double cost;
  
  
  int i;
  for (i = 1; i < argc; i++){
    //printf("%s\n", argv[i]);
    
    //printf("Comparing %s to -f. Result is: %d\n", argv[i], strcmp(argv[i],"-f"));
    
    //NOTE TO SELF: Change this to a switch statement after verifying it works
    if (strncmp(argv[i], "-f", 2) == 0){
      //printf("Found trace file\n");
      //Found a trace file indicator! Need to determine the trace to put it in
      if(trace1Found==0){
        //No trace files found yet. Put in trace1Name
        trace1Name = argv[i+1];
        trace1Found = 1;
      }
      else if(trace2Found==0){
        //Trace 1 has been accounted for, must put in Trace 2
        trace2Name = argv[i+1];
        trace2Found = 1;
      }
      else{
        //Trace 1 and 2 are both determined. Must put in Trace 3 (last possible trace)
        trace3Name = argv[i+1];
        trace3Found = 1;
      }
    }
    else if(strncmp(argv[i], "-s", 2)==0){
      //Found a cacheSize indicator!
      //printf("Found cacheSize\n");
      cacheSize = atoi(argv[i+1]);
    }
    else if(strncmp(argv[i], "-b", 2)==0){
      //Found a blockSize indicator!
      //printf("Found blockSize\n");
      blockSize = atoi(argv[i+1]);
    }
    else if(strncmp(argv[i], "-a", 2)==0){
      //Found an associativity indicator!
      //printf("Found associativity\n");
      associativity = atoi(argv[i+1]);
    }
    else if(strncmp(argv[i], "-r", 2)==0){
      //Found a replacePolicy indicator!
      //printf("Found replacePolicy\n");
      replacePolicy = argv[i+1];
    }
    else if(strncmp(argv[i], "-p", 2)==0){
      //Found a physicalSize indicator!
      //printf("Found physSize\n");
      physicalSize = atoi(argv[i+1]);
    }
    else {
      //printf("No matching if\n");//DEBUG
    }
    
    //argv[0] is just the executable, and the current system simply wont trip any of the if statements.
    
  }
  
 
  
  //Calculate number of trace files (i.e # of loops the cache will experience)
  //Note to self: This could go in with the actual command line processing
  int numTraces;
  if (trace1Found==1){
    numTraces = 1;
    if(trace2Found==1){
      numTraces = 2;
      if(trace3Found==1){
        numTraces = 3;
      }
    }
  }
  
  printf("numTraces is %d\n", numTraces);

  

  //Start printing cache info here
  int numLoops = 0;
  while(numLoops<numTraces){
    printf("Cache Simulator CS 3853 Fall 2023 – Group #11\n\n");
    
    //Admittedly this series of ifs is gross. It can be rectified by putting the traces in an array and referencing that
    if(numLoops==0){
      //Start with trace 1
      printf("Trace File: %s\n\n", trace1Name);
    }
    if(numLoops==1){
      //Then trace 2 (if it exists)
      printf("Trace File: %s\n\n", trace2Name);
    }
    if(numLoops==2){
      //Finally trace 3 (if it exists)
      printf("Trace File: %s\n\n", trace3Name);
    } 
    
    printf("***** Cache Input Parameters *****\n");
    printf("Cache Size:                      %d KB\n", cacheSize);//22 spaces
    printf("Block Size:                      %d bytes\n", blockSize);//22 spaces
    printf("Associativity:                   %d\n", associativity);//19 spaces
    printf("Replacement Policy               %s\n\n\n", replacePolicy);//15 spaces
    
    
    printf("***** Cache Calculated Values *****\n");
    
    totalBlocks = (cacheSize*1024)/blockSize;
    printf("Total # Blocks:                  %d\n", totalBlocks);
    
    offset = (int)((log10(blockSize) / log10(2)));
    indexSize = (int)((log10(cacheSize*1024/(blockSize*associativity)) / log10(2)));
    tagSize = (BUS_SIZE-indexSize-offset);
    
    printf("Tag size:                        %d bits\n", tagSize);
    printf("Index size:                      %d bits\n", indexSize);
    
    totalRows = (totalBlocks/associativity);
    printf("Total # Rows:                    %d\n", totalRows);
    
    overhead = ((cacheSize*blockSize*associativity)+totalRows);
    printf("Overhead Size:                   %d bytes\n", overhead);
    
    implementationMemory = (cacheSize*1024) + overhead; //This is in bytes 
    
    printf("Implementation Memory Size:      %d KB (%d) bytes\n", (implementationMemory/1024), (implementationMemory));
    
    printf("Cost                             $%.2lf @ ($0.09 / KB)", ((implementationMemory/1024)*COST_MULTIPLIER));
    
    
    
    
    
    
    
    printf("\n\n");
    numLoops++;
  }
   
    
    

  
  return 0;
}