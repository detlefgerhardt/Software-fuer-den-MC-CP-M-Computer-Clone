/* VT100.H *dg* 12.10.2026 */
/* VT100 controls using conio */

#define VT_NORM 0x01
#define VT_DIM 0x02
#define VT_EMPH 0x03
#define VT_UNDER 0x04
#define VT_BLINK 0x08
/*#define VT_HIDDEN 0x10*/
#define VT_INV 0x20
#define VT_GRAPH 0x80

#define VT_BLACK 0x00
#define VT_RED 0x01
#define VT_GREEN 0x02
#define VT_YELL 0x03
#define VT_BLUE 0x04
#define VT_MAG 0x05
#define VT_CYAN 0x06
#define VT_LTGRY 0x07
#define VT_DEF 0x09

/* vt_puts controls
 001 = norm
 002 = invers
 003 = emph
 004 = blink
 005 = graph on
 006 = graph off
*/

/* vt100 graphic characters (octal)
 154 161 167 153
 164 170 156 165
 155 161 166 152
*/

/* end of file */
