/* STRLWR.C *dg* 30.08.2026 */

strlwr(ptr)
	char *ptr;
{
	char *ret;

	ret = ptr;
	while(*ptr)
	{
		*ptr = tolower(*ptr);
		ptr++;
	}
	return ret;
}
