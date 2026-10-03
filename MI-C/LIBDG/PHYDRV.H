/* PHYDRV.H *dg* 02.10.2026 */

typedef struct
{
	int drive;
	int byts;	/* bytes / sector */
	int secs;	/* sectors / track */
	int trks;	/* tracks / side */
	char sids;	/* sides */
	char side; /* side to use if only one side */
	char floreg; /* FLO register */
	int size; /* size in KB */
} phyprm;

/* end of PHYDRV.H */
