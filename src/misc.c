#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__APPLE__) && !defined(__FreeBSD_kernel__)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#define _XOPEN_SOURCE 600
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif
/* enable wcswidth on kFreeBSD */
#if defined(__FreeBSD_kernel__) && defined(__GLIBC__)
#define __USE_XOPEN
#define _XOPEN_SOURCE
#endif
#include "common.h"
#include "misc.h"
#include <wchar.h>

int spacecheck(const char *path)
{
	struct statvfs buf;
	uint64_t free;

	/* do space check only when configured for it */
	if (!cfg.spacecheck) {
		return 1;
	}

	if (statvfs(path, &buf)) {
		if (noexit) {
			return 0;
		} else {
			snprintf(errorstring, 1024, "Free diskspace check failed: %s", strerror(errno));
			printe(PT_Error);
			exit(EXIT_FAILURE);
		}
	}

	free = (uint64_t)(buf.f_bavail / (float)1024) * buf.f_bsize;

	if (debug) {
		printf("bsize %d\n", (int)buf.f_bsize);
		printf("blocks %lu\n", (unsigned long int)buf.f_blocks);
		printf("bfree %lu\n", (unsigned long int)buf.f_bfree);
		printf("bavail %lu\n", (unsigned long int)buf.f_bavail);
		printf("ffree %lu\n", (unsigned long int)buf.f_ffree);
		printf("%" PRIu64 " free space left\n", free);
	}

	/* the database is likely to be less than 200 kiB but let's require */
	/* 1 MiB to be on the safe side, anyway, the filesystem should */
	/* always have more free space than that */
	if (free <= 1024) {
		return 0;
	} else {
		return 1;
	}
}

void sighandler(int sig)
{
	/* set signal */
	intsignal = sig;

	if (debug) {
		switch (sig) {

			case SIGHUP:
				snprintf(errorstring, 1024, "DEBUG: SIGHUP (%d)", sig);
				break;

			case SIGTERM:
				snprintf(errorstring, 1024, "DEBUG: SIGTERM (%d)", sig);
				break;

			case SIGINT:
				snprintf(errorstring, 1024, "DEBUG: SIGINT (%d)", sig);
				break;

			default:
				snprintf(errorstring, 1024, "DEBUG: Unknown signal %d", sig);
				break;
		}
		printe(PT_Info);
	}
}

uint64_t getbtime(void)
{
	uint64_t result = 0;
#if defined(__linux__)
	FILE *fp;
	int check;
	char temp[64], statline[128];

	if ((fp = fopen("/proc/stat", "r")) == NULL) {
		snprintf(errorstring, 1024, "Unable to read /proc/stat: %s", strerror(errno));
		printe(PT_Error);
		if (noexit) {
			return 0;
		} else {
			exit(1);
		}
	}

	check = 0;
	while (fgets(statline, 128, fp) != NULL) {
		sscanf(statline, "%63s", temp);
		if (strcmp(temp, "btime") == 0) {
			/* if (debug)
				printf("\n%s\n",statline); */
			check = 1;
			break;
		}
	}
	fclose(fp);

	if (check == 0) {
		snprintf(errorstring, 1024, "btime missing from /proc/stat.");
		printe(PT_Error);
		if (noexit) {
			return 0;
		} else {
			exit(1);
		}
	}

	result = strtoull(statline + 6, (char **)NULL, 0);

#elif defined(BSD_VNSTAT)
	struct timeval btm;
	size_t len = sizeof(btm);
	int mib[2] = {CTL_KERN, KERN_BOOTTIME};

	if (sysctl(mib, 2, &btm, &len, NULL, 0) < 0) {
		if (debug)
			printf("sysctl(kern.boottime) failed.\n");
		return 0;
	}

	result = (uint64_t)btm.tv_sec;
#endif

	return result;
}

char *getvalue(const uint64_t bytes, const int len, const RequestType type)
{
	static char buffer[64];
	int i, declen = cfg.defaultdecimals, p = 1024;
	uint64_t limit;

	if (type == RT_ImageScale) {
		declen = 0;
	}

	if (cfg.unitmode == 2) {
		p = 1000;
	}

	if ((type == RT_Estimate) && (bytes == 0)) {
		declen = len - (int)strlen(getunitprefix(2)) - 2;
		if (declen < 2) {
			declen = 2;
		}
		snprintf(buffer, 64, "%*s  %*s", declen, "--", (int)strlen(getunitprefix(2)), " ");
	} else {
		for (i = UNITPREFIXCOUNT - 1; i > 0; i--) {
			limit = (uint64_t)(pow(p, i - 1)) * 1000;
			if (bytes >= limit) {
				if (i > 1) {
					snprintf(buffer, 64, "%" DECCONV "*.*f %s", getunitspacing(len, 5), declen, bytes / (double)(getunitdivisor(cfg.unitmode, i + 1)), getunitprefix(i + 1));
				} else {
					if (type == RT_Estimate) {
						declen = 0;
					}
					snprintf(buffer, 64, "%" DECCONV "*.*f %s", getunitspacing(len, 2), declen, bytes / (double)(getunitdivisor(cfg.unitmode, i + 1)), getunitprefix(i + 1));
				}
				return buffer;
			}
		}
		snprintf(buffer, 64, "%" DECCONV "*" PRIu64 " %s", getunitspacing(len, 1), bytes, getunitprefix(1));
	}

	return buffer;
}

int getunitspacing(const int len, const int index)
{
	int l = len;

	/* tune spacing according to unit */
	/* +1 for space between number and unit */
	l -= (int)strlen(getunitprefix(index)) + 1;
	if (l < 0) {
		l = 1;
	}

	return l;
}

char *gettrafficrate(const uint64_t bytes, const time_t interval, const int len)
{
	static char buffer[64];
	int declen = cfg.defaultdecimals;
	uint64_t b = bytes;

	if (interval == 0) {
		snprintf(buffer, 64, "%*s", len, "n/a");
		return buffer;
	}

	/* convert to proper unit */
	if (cfg.rateunit == 1) {
		b *= 8;
	}

	return getratestring(b / (uint64_t)interval, len, declen);
}

const char *getunitprefix(const int index)
{
	/* clang-format off */
    static const char *unitprefix[] = { "na",
        "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB",  /* IEC   - 1024^n */
        "B", "KB",  "MB",  "GB",  "TB",  "PB",  "EB",   /* JEDEC - 1024^n */
        "B", "kB",  "MB",  "GB",  "TB",  "PB",  "EB" }; /* SI    - 1000^n */
	/* clang-format on */

	if (index > UNITPREFIXCOUNT) {
		return unitprefix[0];
	} else {
		return unitprefix[(cfg.unitmode * UNITPREFIXCOUNT) + index];
	}
}

const char *getrateunitprefix(const int unitmode, const int index)
{
	/* clang-format off */
    static const char *rateunitprefix[] = { "na",
        "B/s",     "KiB/s",   "MiB/s",   "GiB/s",   "TiB/s",   "PiB/s",   "EiB/s",    /* IEC   - 1024^n */
        "B/s",     "KB/s",    "MB/s",    "GB/s",    "TB/s",    "PB/s",    "EB/s",     /* JEDEC - 1024^n */
        "B/s",     "kB/s",    "MB/s",    "GB/s",    "TB/s",    "PB/s",    "EB/s",     /* SI    - 1000^n */
        "bit/s",   "Kibit/s", "Mibit/s", "Gibit/s", "Tibit/s", "Pibit/s", "Eibit/s",  /* IEC   - 1024^n */
        "bit/s",   "kbit/s",  "Mbit/s",  "Gbit/s",  "Tbit/s",  "Pbit/s",  "Ebit/s" }; /* SI    - 1000^n */
	/* clang-format on */

	if (index > UNITPREFIXCOUNT) {
		return rateunitprefix[0];
	} else {
		return rateunitprefix[(unitmode * UNITPREFIXCOUNT) + index];
	}
}

uint64_t getunitdivisor(const int unitmode, const int index)
{
	if (index > UNITPREFIXCOUNT) {
		return 1;
	} else {
		if (unitmode == 2 || unitmode == 4) {
			return (uint64_t)(pow(1000, index - 1));
		} else {
			return (uint64_t)(pow(1024, index - 1));
		}
	}
}

int getunit(void)
{
	int unit;

	if (cfg.rateunit == 0) {
		unit = cfg.unitmode;
	} else {
		unit = 3 + cfg.rateunitmode;
	}

	return unit;
}

char *getratestring(const uint64_t rate, const int len, const int declen)
{
	int l, i, unit, p = 1024;
	static char buffer[64];
	uint64_t limit;

	unit = getunit();

	if (unit == 2 || unit == 4) {
		p = 1000;
	}

	for (i = UNITPREFIXCOUNT - 1; i > 0; i--) {
		limit = (uint64_t)(pow(p, i - 1)) * 1000;
		if (rate >= limit) {
			l = getratespacing(len, unit, i + 1);
			snprintf(buffer, 64, "%" DECCONV "*.*f %s", l, declen, rate / (double)(getunitdivisor(unit, i + 1)), getrateunitprefix(unit, i + 1));
			return buffer;
		}
	}

	l = getratespacing(len, unit, 1);
	snprintf(buffer, 64, "%" DECCONV "*.0f %s", l, rate / (double)(getunitdivisor(unit, 1)), getrateunitprefix(unit, 1));
	return buffer;
}

int getratespacing(const int len, const int unitmode, const int unitindex)
{
	int l = len;

	l -= (int)strlen(getrateunitprefix(unitmode, unitindex)) + 1;
	if (l < 0) {
		l = 1;
	}

	return l;
}

int getpadding(const int len, const char *str)
{
#if defined(HAVE_MBSTOWCS) && defined(HAVE_WCSWIDTH)
	wchar_t wbuffer[64];
	if (!cfg.utflocale) {
		return len;
	}
	if ((int)mbstowcs(wbuffer, str, 64) < 0) {
		return len;
	}
	return len + ((int)strlen(str) - wcswidth(wbuffer, 64));
#else
	return len;
#endif
}

void cursortocolumn(const int column)
{
	printf("\033[%dG", column);
}

void cursorhide(void)
{
	printf("\033[?25l");
}

void cursorshow(void)
{
	printf("\033[?25h");
}

void eraseline(void)
{
	printf("\033[2K");
}

/* validity of date or time itself isn't checked here as sqlite handles that */
int validatedatetime(const char *str)
{
	short valid;
	unsigned int len, i, t;
	const char *templates[] = {"dddd-dd-dd dd:dd", "dddd-dd-dd"};

	len = (unsigned int)strlen(str);
	if (len > strlen(templates[0])) {
		return 0;
	}

	for (t = 0; t < 2; t++) {
		if (len != strlen(templates[t])) {
			continue;
		}
		valid = 1;
		for (i = 0; i < strlen(templates[t]); i++) {
			switch (templates[t][i]) {
				case 'd':
					if (!isdigit(str[i])) {
						valid = 0;
					}
					break;
				default:
					if (str[i] != templates[t][i]) {
						valid = 0;
					}
					break;
			}
			if (!valid) {
				break;
			}
		}
		if (!valid) {
			continue;
		}
		return 1;
	}

	return 0;
}

int issametimeslot(const ListType listtype, const time_t entry, const time_t updated)
{
	struct tm e, u;

	if (updated < entry) {
		return 0;
	}

	if (entry == updated) {
		return 1;
	}

	if (localtime_r(&entry, &e) == NULL || localtime_r(&updated, &u) == NULL) {
		return 0;
	}

	switch (listtype) {
		case LT_5min:
			if ((entry - (entry % 300)) == (updated - (updated % 300))) {
				return 1;
			}
			break;
		case LT_Hour:
			if (e.tm_year == u.tm_year && e.tm_yday == u.tm_yday && e.tm_hour == u.tm_hour) {
				return 1;
			}
			break;
		case LT_Top:
		case LT_Day:
			if (e.tm_year == u.tm_year && e.tm_yday == u.tm_yday) {
				return 1;
			}
			break;
		case LT_Month:
			if (e.tm_year == u.tm_year && e.tm_mon == u.tm_mon) {
				return 1;
			}
			break;
		case LT_Year:
			if (e.tm_year == u.tm_year) {
				return 1;
			}
			break;
		case LT_None:
			return 0;
	}

	return 0;
}
