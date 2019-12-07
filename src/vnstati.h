#ifndef VNSTATI_H
#define VNSTATI_H

#include <sys/stat.h>  /* fstat() */

#define YBEGINOFFSET -1
#define YENDOFFSET 6

#define DOUTRAD 49
#define DINRAD 15 

/* global variables for vnstati */
int cbackground, cedge, cheader, cheadertitle, cheaderdate, ctext, cline, clinel, cvnstat;
int crx, crxd, ctx, ctxd, cbgoffset;
time_t current;

#endif
