/****************************************************************************/
/*
	MPRINTF.C  *dg*  11/2026 for MI-C compiler and CP/M

	PRINTF using Monitor
*/

extern mputch();

/****************************************************************************/

mprintf(fmt, args)
	char *fmt;
	int *args;
{
	/* aufruf von print mit zeiger auf die parameterliste args
	   zeichenausgabe ueber CONIO funtion putch() */
	print(mputch, fmt, &args);
}

/****************************************************************************/
