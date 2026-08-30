/* RANDOM.C *dg* 30.08.2026 */

/* random.c pseudo random number generator */
/* erzeugte mit OpenAI Codex */

/* static makes it local */
static long rndsta;

/****************************************************************************/
/*
 * 16-bit PRNG:
 * Multiplicative congruential generator modulo 32749.
 *
 * state = (23 * state) mod 32749
 *
 * 32749 is prime and 23 is a primitive root modulo 32749, so all seeds
 * from 1 to 32748 produce a cycle with period 32748.
 */

int rndsed(seed)
	int seed;
{
    seed %= 32749;
    if (seed <= 0)
        seed += 32748;
	rndsta = seed;
    return rndsta;
}

/****************************************************************************/
/* get Z80 refresh counter, can be used as seed */

int rndrfh()
{
#ASM
	LD A,R
	LD L,A
	LD H,0
#ENDASM	
}	

/****************************************************************************/

#define RND_A 23
#define RND_M 32749
#define RND_Q 1423  /* m / a */
#define RND_R 20 /* m % a */

/* get next random number from "from" to "to-1" */
int rndnxt(from, to)
	int from, to;
{
	int hi, lo, t;
	
    hi = rndsta / RND_Q;
    lo = rndsta % RND_Q;
    rndsta = RND_A * lo - RND_Q * hi;
    if (rndsta <= 0)
        rndsta += RND_M;

	return rndsta * (to - from) / RND_M + from;
}

/****************************************************************************/
