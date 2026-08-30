/* STRUPR.C *dg* 30.08.2026 */

strupr(ptr)
	char *ptr;
{
	char *ret;

	ret = ptr;
	while(*ptr)
	{
		*ptr = toupper(*ptr);
		ptr++;
	}
	return ret;
}
