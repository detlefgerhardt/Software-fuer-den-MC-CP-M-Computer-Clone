/* cputs() */

/* gets max characters from console till cr/lf, max is specified in ptr[0] */
cgets(ptr)
	char *ptr;
{
	register int max;
	register int c;
	register int cr;
	max = *ptr++;
	cr = 0;
	while(1)
	{
		c = getch();
		if (c==13)
		{
			cr = c;
			continue;
		}
		if (c==10 && cr==13)
		{
			*ptr = '\0';
			return;
		}
		cr = 0;
		*ptr++ = c;
		max--;
		if (max == 0)
		{
			*ptr++ = '\0';
			return;
		}
	}
}
 /* end of file */