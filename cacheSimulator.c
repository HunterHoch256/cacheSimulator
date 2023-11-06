#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <errno.h>

int main(int argc, char* argv[]){
  
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
  
  
  int count = 0;//Use this to track number if times looped
  char* trace1Name;
  int trace1Found = 0;
  char* trace2Name;
  int trace2Found = 0;
  char* trace3Name;
  int trace3Found = 0;
  //Note to self: Could make an array of char arrays for trace files.
  int cacheSize;
  int blockSize;
  int associativity;
  char* replacePolicy;
  int physicalSize; //Im going to convert the values to bytes/KB on the command line, thus it being an int
  
  while (count < argc){
    //NOTE TO SELF: Change this to a switch statement after verifying it works
    if (strcmp("-f",argv[count]) == 0){
      //Found a trace file indicator! Need to determine the trace to put it in
      if(trace1Found==0){
        //No trace files found yet. Put in trace1Name
        trace1Name = argv[count++];
        trace1Found = 1;
      }
      else if(trace2Found==0){
        //Trace 1 has been accounted for, must put in Trace 2
        trace2Name = argv[count++];
        trace2Found = 1;
      }
      else{
        //Trace 1 and 2 are both determined. Must put in Trace 3 (last possible trace)
        trace3Name = argv[count++];
        trace3Found = 1;
      }
    }
    
    if(strcmp("-s",argv[count])==0){
      //Found a cacheSize indicator!
      cacheSize = atoi(argv[count++]);
    }
    
    if(strcmp("-b",argv[count])==0){
      //Found a blockSize indicator!
      blockSize = atoi(argv[count++]);
    }
    
    if(strcmp("-a",argv[count])==0){
      //Found an associativity indicator!
      associativity = atoi(argv[count++]);
    }
    
    if(strcmp("-r",argv[count])==0){
      //Found a replacePolicy indicator!
      replacePolicy = argv[count++];
    }
    
    if(strcmp("-p",argv[count])==0){
      //Found a physicalSize indicator!
      physicalSize = atoi(argv[count++]);
    }
    
    //argv[0] is just the executable, and the current system simply wont trip any of the if statements.
    count++;
  }
  
  
  

  //Start printing cache info here
  printf("Cache Simulator CS 3853 Fall 2023 – Group #11\n\n");
  
  
  return 0;
}