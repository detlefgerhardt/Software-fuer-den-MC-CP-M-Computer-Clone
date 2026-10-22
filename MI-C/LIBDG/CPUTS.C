/* CPUTS.C *dg* 30.08.2026 */

cputs(ptr)
	char *ptr;
{
	while(*ptr)
		putch(*ptr++);
}
