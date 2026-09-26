/* ndiskdef NBIOS11.C created 26.09.26 12:23 by NDiskDef 1.0 */
/* format F800KB 800KB  (1024/5/80/2) */
drvprm F800KB =
{
	"F800KB",
	{
		40,	/* spt */
		4,	/* bsh */
		15,	/* blm */
		0,	/* exm */
		389,	/* dsm */
		255,	/* drm */
		240,	/* al0 */
		0,	/* al1 */
		64,	/* cks */
		4,	/* ofs */
		/* blocking/deblocking and FLO reg */
		3,	/* psh */
		7,	/* phm */
		32,	/* FLO reg */
		/* physical params */
		1024,	/* phylen */
		4,	/* physec */
		79,	/* phytrk */
		2,	/* sides */
	}
};

/* format F144MB 1440KB  (1024/9/80/2) */
drvprm F144MB =
{
	"F144MB",
	{
		72,	/* spt */
		5,	/* bsh */
		31,	/* blm */
		1,	/* exm */
		354,	/* dsm */
		255,	/* drm */
		192,	/* al0 */
		0,	/* al1 */
		64,	/* cks */
		2,	/* ofs */
		/* blocking/deblocking and FLO reg */
		3,	/* psh */
		7,	/* phm */
		0,	/* FLO reg */
		/* physical params */
		1024,	/* phylen */
		8,	/* physec */
		79,	/* phytrk */
		2,	/* sides */
	}
};

/* format F140MB 1280KB  (1024/8/80/2) */
drvprm F140MB =
{
	"F140MB",
	{
		64,	/* spt */
		5,	/* bsh */
		31,	/* blm */
		1,	/* exm */
		315,	/* dsm */
		255,	/* drm */
		192,	/* al0 */
		0,	/* al1 */
		64,	/* cks */
		2,	/* ofs */
		/* blocking/deblocking and FLO reg */
		3,	/* psh */
		7,	/* phm */
		0,	/* FLO reg */
		/* physical params */
		1024,	/* phylen */
		7,	/* physec */
		79,	/* phytrk */
		2,	/* sides */
	}
};

/* format F120MB 1200KB  (512/15/80/2) */
drvprm F120MB =
{
	"F120MB",
	{
		60,	/* spt */
		5,	/* bsh */
		31,	/* blm */
		1,	/* exm */
		295,	/* dsm */
		255,	/* drm */
		192,	/* al0 */
		0,	/* al1 */
		64,	/* cks */
		2,	/* ofs */
		/* blocking/deblocking and FLO reg */
		2,	/* psh */
		3,	/* phm */
		0,	/* FLO reg */
		/* physical params */
		512,	/* phylen */
		14,	/* physec */
		79,	/* phytrk */
		2,	/* sides */
	}
};

/* format IBMSD 250KB  (128/26/77/1) */
drvprm IBMSD =
{
	"IBMSD",
	{
		26,	/* spt */
		3,	/* bsh */
		7,	/* blm */
		0,	/* exm */
		242,	/* dsm */
		63,	/* drm */
		192,	/* al0 */
		0,	/* al1 */
		16,	/* cks */
		2,	/* ofs */
		/* blocking/deblocking and FLO reg */
		0,	/* psh */
		0,	/* phm */
		48,	/* FLO reg */
		/* physical params */
		128,	/* phylen */
		25,	/* physec */
		76,	/* phytrk */
		1,	/* sides */
	}
};

/* format CF2MB 2048KB  (64/128/256/1) */
drvprm CF2MB =
{
	"CF2MB",
	{
		0,	/* spt */
		5,	/* bsh */
		31,	/* blm */
		1,	/* exm */
		511,	/* dsm */
		255,	/* drm */
		192,	/* al0 */
		0,	/* al1 */
		0,	/* cks */
		0,	/* ofs */
		/* blocking/deblocking and FLO reg */
		-2147483648,	/* psh */
		-1,	/* phm */
		4080,	/* FLO reg */
		/* physical params */
		64,	/* phylen */
		127,	/* physec */
		255,	/* phytrk */
		1,	/* sides */
	}
};

/* format CF8MB 8192KB  (256/128/256/1) */
drvprm CF8MB =
{
	"CF8MB",
	{
		256,	/* spt */
		6,	/* bsh */
		63,	/* blm */
		3,	/* exm */
		1023,	/* dsm */
		255,	/* drm */
		128,	/* al0 */
		0,	/* al1 */
		0,	/* cks */
		0,	/* ofs */
		/* blocking/deblocking and FLO reg */
		1,	/* psh */
		1,	/* phm */
		4080,	/* FLO reg */
		/* physical params */
		256,	/* phylen */
		127,	/* physec */
		255,	/* phytrk */
		1,	/* sides */
	}
};


/* end of NBIOS11.C */ 
