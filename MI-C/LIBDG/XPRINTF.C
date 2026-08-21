/****************************************************************************/
/*
	XPRINTF.C  *dg*  08/2026 for MI-C compiler and CP/M
	
	Beispiel für ein selbstdefinertes printf unter Verwendung der
	internen Funktion print()
	
*/
/****************************************************************************/
/* selbstdefinierte Zeichenausgabe */

xputc(c)
	int c;
{
	putchar('/');
	putchar(c);
	putchar('/');
}		

/****************************************************************************/
/* selbstdefiniertes printf mit variabler parameterliste args */

xprintf(fmt, args)
	char *fmt;
	int *args;
{
	/* aufruf von print mit zeiger auf die parameterliste args */
	print(xputc, fmt, &args);
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	xprintf("%d %s\r\n", 1, "abc");
}

/****************************************************************************/
