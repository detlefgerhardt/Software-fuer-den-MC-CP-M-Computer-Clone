/* HEXDMP.C *dg* 22.10.2026 */

/* dump memory from address p, count bytes
   use dspadr as the displayed address, 0xFFFF = actual memory address
*/

hexdmp(p, count, dspadr)
	char *p;
	int count;
	unsigned dspadr;
{
	int cnt, col, ch;
	char s[5];
	int cols = 16;

	if (dspadr == 0XFFFF) dspadr = p;

	cnt = 0;
	while(cnt < count)
	{
		utohex(dspadr, s);
		cputs(s);
		for (col = 0; col < cols; col++)
		{
			putch(' ');
			if (cnt + col < count)
			{
				ctohex(*(p+col), s);
				cputs(s);
			}
			else
				cputs("  ");
			if (col == 7)
				putch(' ');
		}
		putch(' ');
		for (col = 0; col < cols; col++)
		{
			if (cnt + col >= count) break;
			ch = *(p+col);
			if (ch >= 0x20 && ch <= 0x7E)
				putch(ch);
			else
				putch('.');
			if (col == 7)
				putch(' ');
		}
		cputs("\r\n");
		cnt += cols;
		p += cols;
		dspadr += cols;
	}
}

 /* end of file */