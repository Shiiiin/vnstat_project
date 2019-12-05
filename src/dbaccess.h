#ifndef DBACCESS_H
#define DBACCESS_H

/* legacy database version */
/* import is supported only from version 3 */
/* 1 = 1.0, 2 = 1.1-1.2, 3 = 1.3-1.8 */
#define LEGACYDBVERSION 3

/* all structs below are used for supporting data import */
/* from the legacy database format => don't modify  */

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif
typedef struct {
	time_t date;
	uint64_t rx, tx;
} HOUR;

typedef struct {
	time_t date;
	uint64_t rx, tx;
	int rxk, txk;
	int used;
} DAY;

typedef struct {
	time_t month;
	uint64_t rx, tx;
	int rxk, txk;
	int used;
} MONTH;

/* legacy database structure */
typedef struct {
	int version;
	char interface[32];
	char nick[32];
	int active;
	uint64_t totalrx, totaltx, currx, curtx;
	int totalrxk, totaltxk;
	time_t lastupdated, created;
	DAY day[30];
	MONTH month[12];
	DAY top10[10];
	HOUR hour[24];
	uint64_t btime;
} DATA;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

int importlegacydb(const char *iface, const char *dirname);
int insertlegacydata(DATA *data, const char *iface);
int readdb(DATA *data, const char *iface, const char *dirname, const int force);
void initdb(DATA *data);
int validatedb(DATA *data);

#endif
