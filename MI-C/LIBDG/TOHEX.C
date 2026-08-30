/* TOHEX.C *dg* 30.08.2026 */
/* int/long to hex string */

char hexdig(d)
	int d;
{
	if (d < 10) return d + '0';
	return d - 10 + 'A';
}


/* unsigned char -> 2 char hex string */
char *ctohex(i, s)
	int i;
	char *s;
{
	s[0] = hexdig(i >> 4);
	s[1] = hexdig(i & 0x0F);
	s[2] = 0;
	return s;
}

/* unsigned int -> 4 char hex string */
char *utohex(u, s)
	unsigned u;
	char *s;
{
	ctohex(u >> 8, s);
	ctohex(u & 0xFF, s + 2);
	s[4] = 0;
	return s;
}

/* unsigned long -> 8 char hex string */
char *ltohex(l, s)
	long l;
	char *s;
{
	utohex((int)(l >> 16), s);
	utohex((int)(l & 0xFFFF), s + 4);
	s[8] = 0;
	return s;
}
