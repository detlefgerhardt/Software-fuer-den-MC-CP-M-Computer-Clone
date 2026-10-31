/* STRUPR.C *dg* 31.10.2026 */

strupr(ptr)
	char *ptr;
{
	char *ret;

	ret = ptr;
	while(*ptr)
	{
		*ptr = touppe(*ptr);
		ptr++;
	}
	return ret;
}
