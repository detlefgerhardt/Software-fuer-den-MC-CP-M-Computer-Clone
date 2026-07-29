/****************************************************************************/
/*
	SKEWTEST.C  *dg*  07/2026 for MI-C compiler and MC CP/M clone
	
	Writes n 32K Blocks to disk and reads it back.
	Speedtest for skew/interleave check.
*/
/****************************************************************************/
/* SKEWTEST version history

 24.07.2026 *dg* First version
 
 */

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

char inpbuf[81];

#define BLOCKSIZE 32 /* blocksize in kb */
char *block;

#ASM
BIOS:	DW	0
#ENDASM

/****************************************************************************/

int BD_CoIn()
{
	return BDOS(0xFF, 6);
}

/****************************************************************************/

BD_CoOut(ch)
	char ch;
{
	BDOS(ch, 6);
}

/****************************************************************************/

int GetChr()
{
	return BD_CoIn();
}

/****************************************************************************/

PutChr(ch)
	char ch;
{
	BD_CoOut(ch);
}

/****************************************************************************/

PutStr(s)
	char *s;
{
	while(*s)
		BD_CoOut(*s++);
}		

/****************************************************************************/

BOOL WrBlock(filename, blocks)
	char *filename;
	int blocks;
{
	FILE *fp;
	int b, s, ch;

	fp = fopen(filename, "w");
	if (fp == NULL) return FALSE;

	printf("Writing:");
	
	for (b = 0; b < blocks; b++)
	{
		ch = GetChr();
		if (ch==CTRLC) return FALSE;
		
		printf(" %d", b + 1);
		s = fwrite(block, 1024, BLOCKSIZE, fp);
		if (s != BLOCKSIZE) return FALSE;
	}
	printf("\r\n%d blocks written\r\n", blocks);

	fclose(fp);
	return TRUE;
}

/****************************************************************************/

BOOL WrBl2(filename, blocks)
	char *filename;
	int blocks;
{
	int fd;
	int b, s, ch;

	fd = creat(filename, 1); /* open for writing */
	if (fd == -1) return FALSE;

	printf("Writing:");
	
	for (b = 0; b < blocks; b++)
	{
		ch = GetChr();
		if (ch==CTRLC) return FALSE;
		
		printf(" %d", b + 1);
		s = write(fd, block, 512 * BLOCKSIZE);
		if (s != 512 * BLOCKSIZE) return FALSE;
		s = write(fd, block, 512 * BLOCKSIZE);
		if (s != 512 * BLOCKSIZE) return FALSE;
	}
	printf("\r\n%d blocks written\r\n", blocks);

	close(fd);
	return TRUE;
}

/****************************************************************************/

BOOL RdBlocks(filename, blocks)
	char *filename;
	int blocks;
{
	FILE *fp;
	int b, s, ch;

	fp = fopen(filename, "r");
	if (fp == NULL) return FALSE;
	
	printf("Reading:");
	
	for (b = 0; b < blocks; b++)
	{
		ch = GetChr();
		if (ch==CTRLC) return FALSE;
		
		printf(" %d", b + 1);
		s = fread(block, 1024, BLOCKSIZE, fp);
		if (s != BLOCKSIZE) return FALSE;
	}
	printf("\r\n%d blocks read\r\n", blocks);

	close(fp);
	return TRUE;
}

/****************************************************************************/

BOOL RdBl2(filename, blocks)
	char *filename;
	int blocks;
{
	int fd;
	int b, s, ch;

	fd = open(filename, 0);
	if (fd == -1) return FALSE;
	
	printf("Reading:");
	
	for (b = 0; b < blocks; b++)
	{
		ch = GetChr();
		if (ch==CTRLC) return FALSE;
		
		printf(" %d", b + 1);
		s = read(fd, block, 512 * BLOCKSIZE);
		if (s != 512 * BLOCKSIZE) return FALSE;
		s = read(fd, block, 512 * BLOCKSIZE);
		if (s != 512 * BLOCKSIZE) return FALSE;
	}
	printf("\r\n%d blocks read\r\n", blocks);

	close(fd);
	return TRUE;
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	int n, i, blocks, ch;
	char drive;
	char *filename;
	long filesize;
	BOOL success;

	PutStr("\r\nSKEWTEST V1.1 for MC CP/M\r\n\n");

	printf("How many %dkb blocks to write? ", BLOCKSIZE);
	n = gets(inpbuf);
	if (n == 0) return; /* exit */
	blocks = atoi(inpbuf);
	if (blocks <= 0 || blocks > 25) return; /* exit */
	
	printf("Destination drive [A..P]? ");
	n = gets(inpbuf);
	if (n == 0) return; /* exit */
	drive = toupper(inpbuf[0]);
	if (drive < 'A' || drive > 'P') return; /* exit */

	filename = "X:SKEWTEXT.DAT";
	filename[0] = drive;

	/* fill block */
	block = calloc(BLOCKSIZE, 1024);
	if (block == NULL)
	{
		PutStr("Error: not enough memory\r\n");
		return FALSE;
	}
	
	
	for (n = 0; n < BLOCKSIZE; n++)
		for (i = 0; i < BLOCKSIZE * 1024; i++)
			block[n * BLOCKSIZE + i] = 0x01;

	filesize = (long)blocks * (long)BLOCKSIZE;

	/* testdatei loeschen */
	unlink(filename);

	printf("Write %d blocks (%ld KBytes) to drive %c [Y]? ", blocks, filesize, drive);
	while(TRUE)
	{
		ch = GetChr();
		if (ch == 0) continue;
		if (ch == 'y' || ch == 'Y') break;
		return;
	}
	PutStr("\r\n");

	/*success = WrBlocks(filename, blocks);*/
	success = WrBl2(filename, blocks);
	PutStr("\007\007\007");
	if (!success)
	{
		puts("Error\r\n");
		return;
	}

	printf("\r\nRead %d blocks from drive %c [Y]? ", blocks, drive);
	while(TRUE)
	{
		ch = GetChr();
		if (ch == 0) continue;
		if (ch == 'y' || ch == 'Y') break;
		return;
	}
	PutStr("\r\n");
	
	/*success = RdBlocks(filename, blocks);*/
	success = RdBl2(filename, blocks);
	PutStr("\007\007\007");
	if (!success)
	{
		puts("Error\r\n");
		return;
	}
	
	return;
}

/****************************************************************************/
