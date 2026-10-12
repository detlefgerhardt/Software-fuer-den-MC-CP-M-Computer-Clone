/* STRICMP.C *dg* 12.10.2026 */

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

int stricmp(s1, s2)
	char *s1,*s2;
{
	int ch1, ch2;

	while(TRUE)
	{
		ch1 = toupper(*s1);
		ch2 = toupper(*s2);
		if (ch1 == 0 && ch2 == 0) return 0; /* equal */
		if (ch1 == 0 && ch2 != 0) return -ch2; /* s2 is greater */
		if (ch1 != 0 && ch2 == 0) return ch1; /* s1 is greater */
		if (ch1 != ch2) return ch1 - ch2;
		s1++;
		s2++;
    }
}


int strincmp(s1, s2, n)
	char *s1,*s2;
	int n;
{
	int i, ch1, ch2;
	
    for (i = 0; i < n; i++)
	{
		ch1 = toupper(*s1);
		ch2 = toupper(*s2);
		if (ch1 == 0 && ch2 == 0) return 0; /* equal */
		if (ch1 == 0 && ch2 != 0) return -ch2; /* s2 is greater */
		if (ch1 != 0 && ch2 == 0) return ch1; /* s1 is greater */
		if (ch1 != ch2) return ch1 - ch2;
		s1++;
		s2++;
	}
	/* equal */
	return 0;
}

/* end of file */
