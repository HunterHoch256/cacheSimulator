#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>

#define BUS_SIZE 32 //We assume a logical bus size of 32 bits
#define COST_MULTIPLIER 0.09 //This cost multiplier is constant


typedef struct {
   int valid;//0 for invlaid, 1 for valid
   unsigned long tag;//each block gets its own tag
   
   //data. I'm not technically interested in data for the sake of this assignment
} Block;

typedef struct {
  Block *blocks;
  int roundRobinPosition;
  //I'm not going to store the index, since it's accessible by position
  
} CacheLine;//A cache line is made of blocks, depending on associativity

typedef struct {
  CacheLine *lines;//Creates numEntries lines
  int size;//This should be the nnumber of entries
  int blockOffsetBits;//How many bits makeup the block offset
  int Random; //Set to 1 if Random replacement, set to 0 if Round-Robin
  int blockSize;//The block size in bytes
  int associativity;//Can be 1, 2, 4, 8, 16 
} Cache;

int cacheAccesses = 0;
int cacheHits = 0;
int cacheMisses = 0;
int compulsoryMisses = 0;
int conflictMisses = 0;
int robinReplace = 0;
  
void initializeCache(Cache *cache) {
    //Running through what this does
    //This function takes in a cache struct that already has its lines mallloced (The number of lines should be equal to the number of rows)
    //It then runs a loop through
    int i, j;
    for (i = 0; i < cache->size; i++) {
      cache->lines[i].roundRobinPosition = 0;
      //myCache.lines = malloc(myCache.size * sizeof(CacheLine));
      cache->lines[i].blocks = malloc(cache->associativity * sizeof(Block));
      for (j = 0; j < cache->associativity; j++){
        
        cache->lines[i].blocks[j].valid = 0;
        cache->lines[i].blocks[j].tag = 0;
        // Data? Don't think we care about it
        //cache->lines[i].data = (char *)malloc(cache->blockSize * sizeof(char));
        /*
        if (cache->lines[i].data == NULL) {
          perror("Memory allocation error");
        }
        */
        }
    }
}  
  
void simulateMemoryAccess(Cache *cache, unsigned long address) {
    cacheAccesses++;
    int numValid = 0;
    //int tagPos = -1;//Will record the Pos of a matched tag with invalid bit
    // Extract tag and set index from the address
    //printf("address is %x\n", address);
    unsigned long tag = address >> cache->blockOffsetBits;
    //printf("tag is %x\n", tag);
    int setIndex = (address >> cache->blockOffsetBits) % cache->size;
    //printf("setIndex is %d\n\n", setIndex);
    
    // Search for the tag in the set
    int i, j;
    //for (i = 0; i < cache->size; i++) {//Why does this even exist?
      for (j = 0; j < cache->associativity; j++){
        //cacheAccesses++;
        if (cache->lines[setIndex].blocks[j].valid && cache->lines[setIndex].blocks[j].tag == tag) {
            // Cache hit
            //printf("Cache Hit!\n");
            cacheHits++;
            return; // Exit early
            }
        
        cacheMisses++;
        //}
    }

    // Cache miss
    //If we get to this point, that means that there was no matching tags found in any of the blocks at that index.
    //That means that we need to replace a block in that index, which will be done using either round robin or random depending on the command line input by the user. 
    //Random will choose a random number between 0 and the associativity-1, which will replace a random block in the set
    //Round Robin will replace the block indicated by the roundRobinPosition variable, which is initialized to 0 for every set (cache line) and is incremented after a replace is performed.  
    
    //Check if all valid bits are set
    for (i = 0; i < cache->associativity; i++){
      if (cache->lines[setIndex].blocks[i].valid == 1){
        numValid++;
      }
    }
    //if numValid is anything less than associativity, then there is a block in the set that is not valid. 
    //if numValid < associativity, there must be a compulsory miss
    //if numValid == associativity, then it's a conflict miss
    
    
    //cacheMisses++;
    if (numValid < cache->associativity-1) {
        // Conflict miss (valid bit was 1 but tag didn't match)
        //printf("Conflict Miss - Replacing Block in Cache\n");
        compulsoryMisses++;
    } else {
        // Compulsory miss (valid bit was 0)
        //printf("Compulsory Miss - Loading Block into Cache\n");
        conflictMisses++;
    }
    
    
    
    //In the case of a compulsory miss, we're just filling the set up. But because we can choose our replacement policy we must refer to it to fill the block
    
    int replacementBlockIndex;
    
    //int randomValue = rand() % (maxValue + 1);
    
    if (cache->Random == 1){
      //Use Random Policy
      //MUST BE BETWEEN 0 AND ASSOCIATIVITY - 1
      replacementBlockIndex = rand() % (cache->associativity + 1);;
    }
    else {
      //Use Round Robin Policy
      //replacementIndex = robinReplace % cache->size;
      
      replacementBlockIndex = cache->lines[setIndex].roundRobinPosition;
      
    }

    // Update cache line information
    
    //USE replacementIndex here to implement RR AND RND!!!!
    /*
    for (i = 0; i < cache->blockSize; i++) {
      cache->lines[setIndex * cache->associativity + replacementIndex].data[i] = 
    }
    */
    
    
    
    cache->lines[setIndex].blocks[replacementBlockIndex].valid = 1;
    cache->lines[setIndex].blocks[replacementBlockIndex].tag = tag;
    // Load the block into the cache, you might fetch it from memory here
    cache->lines[setIndex].roundRobinPosition++;
    if (cache->lines[setIndex].roundRobinPosition >= cache->associativity){
      cache->lines[setIndex].roundRobinPosition = 0;
    }
    //printf("robinReplace: %d\n", cache->lines[setIndex].roundRobinPosition);
}
 
  

int main(int argc, char *argv[]){

  srand(time(NULL));
  
  
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
  int offset;//Might need to make a double
  int tagSize;
  int indexSize;
  int totalRows;
  int overhead;
  double implementationMemory; 
  //double cost; Currently unused
  int numTraces;//This will be used to determine how many cache cycles we go through
  
  
  int i;
  for (i = 1; i < argc; i++){
    
    //NOTE TO SELF: Change this to a switch statement after verifying it works
    if (strncmp(argv[i], "-f", 2) == 0){
      //Found a trace file indicator! Need to determine the trace to put it in
      if(trace1Found==0){
        //No trace files found yet. Put in trace1Name
        trace1Name = argv[i+1];
        trace1Found = 1;
        //Tell the loop we'll have at least one trace file
        numTraces = 1;
      }
      else if(trace2Found==0){
        //Trace 1 has been accounted for, must put in Trace 2
        trace2Name = argv[i+1];
        trace2Found = 1;
        //Tell the loop we'll have at least two trace files
        numTraces = 2;
      }
      else{
        //Trace 1 and 2 are both determined. Must put in Trace 3 (last possible trace)
        trace3Name = argv[i+1];
        trace3Found = 1;
        //Tell the loop we'll have at least three trace files
        numTraces = 3;
      }
    }
    else if(strncmp(argv[i], "-s", 2)==0){
      //Found a cacheSize indicator!
      cacheSize = atoi(argv[i+1]);
    }
    else if(strncmp(argv[i], "-b", 2)==0){
      //Found a blockSize indicator!
      blockSize = atoi(argv[i+1]);
    }
    else if(strncmp(argv[i], "-a", 2)==0){
      //Found an associativity indicator!
      associativity = atoi(argv[i+1]);
    }
    else if(strncmp(argv[i], "-r", 2)==0){
      //Found a replacePolicy indicator!
      replacePolicy = argv[i+1];
    }
    else if(strncmp(argv[i], "-p", 2)==0){
      //Found a physicalSize indicator!
      physicalSize = atoi(argv[i+1]);
    }
    else {
      //printf("No matching if\n");//DEBUG
    }
    
    //argv[0] is just the executable, and the current system simply won't trip any of the if statements.
    
  }
  
  
  
  printf("\nCache Simulator CS 3853 Fall 2023 - Group #11\n\n");
  
  //Start printing cache info here
  int numLoops = 0;
  while(numLoops<numTraces){
    //printf("\nCache Simulator CS 3853 Fall 2023 - Group #11\n\n");
    
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
    
    printf("Implementation Memory Size:      %.2lf KB (%.0lf) bytes\n", (implementationMemory/1024), ceil(implementationMemory));
    
    printf("Cost                             $%.2lf @ ($0.09 / KB)\n\n\n", ((implementationMemory/1024)*COST_MULTIPLIER));
    
    
    //The following regards parsing the trace file (first trace only)
    
    if(numLoops==0){
      //printf("Printing the first 20 addresses and lengths of cache operations in %s:\n\n", trace1Name);
      int whileLoops = 0;
      //Only need to print for first trace file
      FILE *file = fopen(trace1Name, "r");
      //Error checking is good for the environment
      if (file == NULL) {
        perror("Error opening file");
        return 1;
      }

      char line[100];

      while (fgets(line, sizeof(line), file) && whileLoops<60) {//This is 60 because the loop also has to count through the other lines (3 lines per instruction)
        char prefix[19];
        char address_str[10];  
        int length;

        // Copy the first 18 characters into prefix
        strncpy(prefix, line, 18);
        prefix[18] = '\0';//Null-terminating to tell it to stop
        
        //sscanf length and adress
        if( sscanf(prefix, "EIP (%d): %s", &length, address_str) == 2){
          //printf("0x%s (0%d)\n", address_str, length);
        }
        whileLoops++;
      }
      fclose(file);

    }

    //I'm gonna try to start getting the important stuff here
    printf("***** *****  CACHE SIMULATION RESULTS  ***** *****\n");
    
    //THE CACHE STILL NEEDS TO BE FORMALLY INITIALIZED
    Cache myCache;
    myCache.blockSize = blockSize;
    myCache.associativity = associativity;
    //Get numEntries for the cache. This determines the cache "size"
    
    int numEntries;
    numEntries = (cacheSize * 1024) / blockSize;  // Adjust for block size
    numEntries = numEntries / associativity;       // Adjust for associativity
    
    myCache.size = numEntries;
    myCache.lines = malloc(myCache.size * sizeof(CacheLine));

    myCache.blockOffsetBits = offset;//This should work, if not I need to check it
    
    
    
    if (strcmp(replacePolicy, "RR") == 0){
      //Round Robin found
      myCache.Random = 0;
    }
    else {
      myCache.Random = 1;
    }
    
    if (myCache.lines == NULL) {
        perror("Memory allocation error");
        return 1;
    }

    // Initialize the cache
    initializeCache(&myCache);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //For our cache sims purposes, this is all that matters
    FILE *file = fopen(trace1Name, "r");//THIS MUST BE UPDATED TO REFLECT WHICH TRACE TO OPEN. TRACE1 IS A TEMP SOLUTION
    char line[100];//Use as my buffer
    int bytesRead;
    unsigned long memAddress;
    unsigned long destAddress;
    unsigned long srcAddress;

    
    //use fgets to parse data and sscanf to store
    //For the first line, pos line[5,6] gets the length
    //on that same line[10,17] gets the memAddress
    //On the NEXT line, line[6,13] gets the destAddress
    //on that same line, line[33,40] gets the srcAddress
    
    int lineCounter = 0; //Used to determine the type of line fgets is on
    //Since the file is very strictly organize, we can use this variable to loop between 0, 1, and 2 until EOF
    bool EOF_notReached = true;
    
    while (EOF_notReached != false){//Will loop until EOF
      
      while (fgets(line, sizeof(line), file)){
        
      
        if (lineCounter == 0){//First line
          //bytesRead = atoi(line[5,6]);
          //memAddress = line[10,17]
          sscanf(line, "EIP (%d): %lx", &bytesRead, &memAddress);
          //printf("bytesRead: %d, Address: 0x%lx\n", bytesRead, memAddress);//Just to make sure it's working
        }
        else if (lineCounter == 1){//Second line
          sscanf(line, "dstM: %lx %*s srcM: %lx", &destAddress, &srcAddress);
          //printf("dstM Address: 0x%lx, srcM Address: 0x%lx\n", destAddress, srcAddress);
          
        }
        else {//Must be the empty line
          lineCounter = -1;//Reset line counter for next set
          //It's possible an extra line in the trace file might break this code. If this happens, the program may crash. This will need to be handled
          
          //This is also probably where all the actual cache stuff will happen
          //Since the pieces are in place to perform a cache operation, everything will be done here in order to affect the cache. This will be a very VERY intensive else statment.
          
          //I should start by converting the hex addresses (where applicable) to binary. (Maybe not necessary)
          //int z;
          //for (z = 0; z < bytesRead; z++){
          simulateMemoryAccess(&myCache, memAddress);
          
          //}
          if (destAddress != 0){simulateMemoryAccess(&myCache, destAddress);}
          if (srcAddress != 0){simulateMemoryAccess(&myCache, srcAddress);}
          
          
          
          
          
          
          
          
          
        }
        
        //printf("lineCounter: %d\n\n", lineCounter);
        
        lineCounter++;
      }
      EOF_notReached = false;
    }
    int i, j;
    //int validLooper;
    int numInvalid = 0;
    for (i = 0; i < myCache.size; i++){
      for (j = 0; j < myCache.associativity; j++){
        if (myCache.lines[i].blocks[j].valid == 0){
          numInvalid++;
        }
      }
    } 
    
    
    
    //printf("cacheHits: %d\n", cacheHits);
    //cacheAccesses = cacheHits+cacheMisses;
    printf("Total Cache Accesses:    %d\n", cacheAccesses);
    printf("Cache Hits:              %d\n", cacheHits);
    printf("Cache Misses:            %d\n", cacheMisses);
    printf("--- Compulsory Misses       %d\n", compulsoryMisses);
    printf("--- Conflict Misses:        %d\n\n", conflictMisses);
    
    printf("***** *****  CACHE HIT & MISS RATE:  ***** *****\n\n");
    
    double hitRate = (cacheHits*100.0)/cacheAccesses;
    printf("Hit Rate:                %f%%\n", hitRate);
    double missRate = 100 - hitRate;
    printf("Miss Rate:               %f%%\n", missRate);
    //Have not included CPI implementation
    printf("CPI:                     UNDETERMINED Cycles/Instruction\n\n");
    double unusedKB = ((totalBlocks-compulsoryMisses) * (blockSize+overhead/1024)) / 1024;
    double waste = COST_MULTIPLIER * unusedKB;
    printf("Unused Cache Space:      %.2f KB / %.2f KB = %.2f\%% Waste: $%.2f\n", unusedKB, implementationMemory/1024, unusedKB/implementationMemory/1024,waste);
    printf("Unused Cache Blocks:     %d / %d\n", numInvalid, totalBlocks);
    printf("\n\n");
    numLoops++;
  }
   
  
    
  
  
  return 0;
}