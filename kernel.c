// Shane's Operating System Kernel
// kernel.c

void printString(char*);
void readString(char*);
void readSector(char*, int);
void handleInterrupt(int, char*, int, int);
void readFile(char*, char*, int*);
void executeProgram(char*);
void terminate();
void writeSector(char*, int);
void deleteFile(char*);
void writeFile(char*, char*, int);
void handleTimerInterrupt(int, int);
typedef struct 
{
	int processActive, processStackPointer;
} 
tableEntry;
tableEntry processTable[8];
int currentProcess;
void initializeProgram(int segment);
int setKernelDataSegment();
void restoreDataSegment(int segment);
void enableInterrupts();

void main() {

//	// ProjectA
//	int startVidMem = 0xb800;
//	int vidMemOffset = 0x0;
//	int white = 0x7;
//	char* letters = "Hello World\0";
//
//	while(*letters != 0x0) {
//		putInMemory(startVidMem, vidMemOffset, *letters);
//		vidMemOffset++;
//		putInMemory(startVidMem, vidMemOffset, white);
//		vidMemOffset++;
//		// advance letters pointer
//		letters++;
//	}
//
//  while(1);

//	// ProjectB
//	//Step 1: Printing to the Screen - Interrupt 0x10
//	//printString("Hello World");
//
//	// Step 2: Reading from the keyboard - Interrupt 0x16
//	//char line[80];
//	//printString("Enter a line: ");
//	//readString(line);
//	//printString(line);
//
//	// Step 3 - Read a sector from the disk - Interrupt 0x13
//	//char buffer[512];
//	//readSector(buffer, 30);
//	//printString(buffer);
//
//	// Step 4 - Create your own interrupt
//	//makeInterrupt21();
//	//interrupt(0x21, 0, 0, 0, 0);
//
//	// Step 5 - Make printString, readString, and readSector interrupt calls
//	char line[80];
//	makeInterrupt21();
//	interrupt(0x21, 1, line, 0, 0);
//	interrupt(0x21, 0, line, 0, 0);
//
//	while(1);

//    // ProjectC
//    // Step 1 - Read a file and print it
//    //char buffer[13312];   /*this is the maximum size of a file*/
//    //int sectorsRead;
//    //makeInterrupt21();
//    //interrupt(0x21, 3, "messag", buffer, &sectorsRead);   /*read the file into buffer*/
//    //if (sectorsRead>0)
//    //    {
//    //	interrupt(0x21, 0, buffer, 0, 0);   /*print out the file*/
//    //} else {
//    //	interrupt(0x21, 0, "messag not found\r\n", 0, 0);  /*no sectors read? then print an error*/
//    //}
//    //while(1);   /*hang up*/
//
//    // Step 2 - Load a Program and Execute it
//    //makeInterrupt21();
//    //interrupt(0x21, 4, "tstpr1", 0, 0);
//    //while(1);
//
//    // Step 3 - Terminate program system call
//    //makeInterrupt21();
//    //interrupt(0x21, 4, "tstpr2", 0, 0);
//    //while(1);
//
//    // Step 4 - The Shell - making your own user program
//    makeInterrupt21();
//    interrupt(0x21, 6, "shell", 0, 0);
//    while(1);
//
//    // Project D
//    // Step 4 - Writing a file
//    makeInterrupt21();
//    interrupt(0x21, 8, "this is a test message", "testmg", 3);
//    while(1);

    // ProjectE
    // Step 2 - Process Table
    int entry_number;
    for (entry_number = 0; entry_number < 8; entry_number += 1)
    {
		processTable[entry_number].processActive = 0;
		processTable[entry_number].processStackPointer = 0xff00;
	}
    currentProcess = -1;
    // Step 5 - some additional adjustments, and testing
    executeProgram();
    // Step 1 - Timer Interrupt
    makeTimerInterrupt();
    while(1);
}

// ProjectB
// Step 1: Printing to the Screen - Interrupt 0x10
void printString(char* chars)
{
	int i=0;
	while(chars[i] != '\0') {
		//interrupt(0x10, 0xe*256+'Q', 0, 0, 0);
		interrupt(0x10, 0xe*256+chars[i], 0, 0, 0);
		i++;
	}
}

// Step 2: Reading from the keyboard - Interrupt 0x16
void readString(char* chars)
{
	int i=0;
	while(1) {
		if(i > 80) {
			return;
		}
		chars[i] = interrupt(0x16, 0, 0, 0, 0);
		if(chars[i] == 0xd) {
			interrupt(0x10, 0xe*256+0xd, 0, 0, 0);
			interrupt(0x10, 0xe*256+0xa, 0, 0, 0);
			chars[i+1] = 0xa;
			chars[i+2] = 0x0;
			return;
		}
		else if(chars[i] == 0x8) {
			if(i > 0) {
				interrupt(0x10, 0xe*256+chars[i], 0, 0, 0);
				i--;
				chars[i] = ' ';
				interrupt(0x10, 0xe*256+chars[i], 0, 0, 0);
				interrupt(0x10, 0xe*256+0x8, 0, 0, 0);
			}
		}
		else if(chars[i] != ' ') {
			interrupt(0x10, 0xe*256+chars[i], 0, 0, 0);
			i++;
		}
	}
}

// Step 3 - Read a sector from the disk - Interrupt 0x13
void readSector(char* buffer, int sector)
{
	int relativeSector;
	int head;
	int track;
	relativeSector = mod(sector, 18) + 1;
	head = mod(div(sector, 18), 2);
	track = div(sector, 36);
	interrupt(0x13, 0x2*256+0x1, buffer, track*256+relativeSector, head*256+0);
}

int mod(int a, int b)
{
	while(a>=b) {
		a=a-b;
	}
	return a;
}

int div(int a, int b)
{
	int c=0;
	while((c+1)*b<=a) {
		c++;
	}
	return c;
}

// Step 4 - Create your own interrupt & Step 5 - Make printString, readString, and readSector interrupt calls
void handleInterrupt21(int ax, char* bx, int cx, int dx)
{
	if(ax=0) {
		printString(bx);
	}
	if(ax=1) {
                readString(bx);
        }
	if(ax=2) {
                readSector(bx, cx);
        }
	else {
		printString("error message");
	}
    // ProjectE
    // Step 5 - some additional adjustments, and testing
    setKernelDataSegment();
    for(i=0; i<8; i++)
    {
        putInMemory(0xb800,60*2+i*4,i+0x30);
        if(processActive[i]==1)
        {
            putInMemory(0xb800,60*2+i*4+1,0x20);
        }
        else
        {
            putInMemory(0xb800,60*2+i*4+1,0);
        }
    }
}

// ProjectC
// Step 1 - Read a file and print it
void readFile(char* buffer, char* filename, int* sectorsRead)
{
	char dir[512];
	readSector(dir, 2);
	int fileentry;
	int letter;
	int fileExists;
	for (fileentry = 0; fileentry < 512; fileentry += 32)
	{
		for (letter = 0; letter < 6; letter += 1)
		{
			if (filename[letter] == dir[letter + fileentry])
			{
				fileExists = 0;
				break;
			}
		}
		fileExists = 1;
		if (fileExists == 1)
		{
			break;
		}
	}
	if (fileExists == 0)
	{
		*sectorsRead = 0;
		return;
	}
	int sector_number;
	int sector;
	for (sector_number = 6; sector_number < 26; sector_number += 1)
	{
		sector = dir[entry+sector_number];
		if (sector = 0)
		{
			break;
		} else {
			readSector(sector, buffer);
			buffer = buffer+512;
			*sectorsRead = *sectorsRead + 1;
			return;
		}
	}
	return;
}

// Step 2 - Load a Program and Execute i
void executeProgram(char* name) 
{
	char buffer[13312];
    int b;
	readFile(name, buffer);
    // ProjectE
    // Step 3 - Execute Program, revisited
    dataseg = setKernelDataSegment();
    int entry_number;
    for (entry_number = 0; entry_number < 8; entry_number += 1) 
    {
		if (processTable[entry_number].processActive == 0)
			break;
	}
    dataseg = (entry_number + 2) * 0x1000;
    for (b = 0; b < 13312; b += 1) 
    {
		putInMemory(0x2000, b, buffer[b]);
	}
    initializeProgram(dataseg);    
    processTable[entry_number].processActive = 1;
	processTable[entry_number].processStackPointer = 0xff00;
    restoreDataSegment(dataseg);
	//launchProgram(0x2000);
}

// Step 3 - Terminate program system call
void terminate()
{
	//while(1);
    // Step 4 - The Shell - making your own user program
    //interrupt(0x21, 6, "shell", 0, 0);
    //char shellname[6];
    //shellname[0] = 's';
    //shellname[1] = 'h';
    //shellname[2] = 'e';
    //shellname[3] = 'l';
    //shellname[4] = 'l';
    //shellname[5] = '\0';
    //executeProgram(shellname);
    // ProjectE
    // Step 5 - some additional adjustments, and testing
    setKernelDataSegment();
    processTable[currentProcess].processActive = 0;
    while(1);
}

// Project D
// Step 1:Write Sector
void writeSector(char* buffer, int sector) 
{
	int relativeSector;
    int head;
    int track;
	relativeSector = modw(sector, 18) + 1;
	head = modw(divw(sector, 18), 2);
	track = divw(sector, 36);
	interrupt(0x13, 0x3*256+0x1, buffer, track*256+relativeSector, head*256+0);
}

int modw(int a, int b)
{
	while(a>=b) {
		a=a-b;
	}
	return a;
}

int divw(int a, int b)
{
	int c=0;
	while((c+1)*b<=a) {
		c++;
	}
	return c;
}

// Step 3:Delete File
void deleteFile(char* filename) 
{
	char dir[512];
    char map[512]:
	int entry;
    int fileExists;
    int letter;
	interrupt(0x21, 2, dir, 2, 0);
	interrupt(0x21, 2, map, 1, 0);
	for (entry = 0; entry < 512; entry += 32) 
    {
		fileExists = 1;	
		for (letter = 0; letter < 6; letter += 1) 
        {			
			if (dir[entry + letter] != filename[letter]) 
            {
				fileExists = 0;
				break;
			}
		}
		if (fileExists == 1) 
        {
			break;
		}
	}
    int sector_number;
    int sector;
	for (sector = 6; sector < 32; sector += 1) 
    {
		sector_number = (int) dir[entry + sector];
		if (sector_number != 0) 
        {
			writeSector(sector_number);
			map[sector_number] = 0x0;
		}
	}
	dir[entry] = 0x0;
	interrupt(0x21, 6, dir, 2, 0);
	interrupt(0x21, 6, map, 1, 0);
	return;

// Step 4 - Writing a file
void writeFile(char* buffer, char* filename, int numberOfSectors) 
{
	char map[512];
    char dir[512];
	int sector[256];
	int entry;
    interrupt(0x21, 2, map, 1, 0);
	interrupt(0x21, 2, dir, 2, 0);
	for (entry = 0; entry < 512; entry += 32) 
    {
		if (dir[entry] == 0x0) 
        {
            break;
        }
	}
    int i;
	for (i = 0; i < 6; i += 1) 
    {
		dir[entry + i] = filename[i];
	}
    int j;
	for (i = 0; i < numberOfSectors; i++) 
    {
		for (j = 3; j < 512; j += 1) 
        {
			if (map[j] == 0) 
            {
				map[j] = 0xFF;
				sector[i] = j;			
				dir[entry + 6 + i] = sector[i];
				writeSector(buffer + 512*i, sector[i]);
				break;
			}
		}
	}
	for (i = entry + numberOfSectors + 6; i < 32; i += 1) 
    {
		dir[i] = 0x0;
	}
    interrupt(0x21, 6, map, 1, 0);
	interrupt(0x21, 6, dir, 2, 0);
	return;
}

// Project E
// Step 1 - Timer Interrupt
void handleTimerInterrupt(int segment, int sp)
{
    //printChar[0] = 'T';
    //printChar[1] = 'i';
    //printChar[2] = 'c';
    // Step 4 - Scheduling
    dataseg = setKernelDataSegment();
    if (currentProcess != -1)
    {
		processTable[currentProcess].processStackPointer = sp;
    }
    while (processActive = 0)
    {
        currentProcess = currentProcess + 1;
        if (currentProcess = 8)
        {
            currentProcess = 0;
        }
        if (processTable[currenProcess].processActive == 1)
        {
            break;
        }   
    }
    segment = (currentProcess + 2) * 0x1000;
	newSP = processTable[currentProcess].processStackPointer;
    restoreDataSegment(dataseg);   
    returnFromTimer(int segment, int sp);
}
}


