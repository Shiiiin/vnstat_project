/*
 * iostat: report CPU and I/O statistics
 * (C) 1999-2018 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _IOSTAT_H
#define _IOSTAT_H

#include "disk.h"

/* I_: iostat - D_: Display - F_: Flag */
#define I_D_CPU			0x000001
#define I_D_DISK		0x000002
#define I_D_TIMESTAMP		0x000004
#define I_D_EXTENDED		0x000008
#define I_D_PART_ALL		0x000010
#define I_D_KILOBYTES		0x000020
#define I_F_HAS_SYSFS		0x000040
#define I_D_DEBUG		0x000080
#define I_D_UNFILTERED		0x000100
#define I_D_MEGABYTES		0x000200
#define I_D_PARTITIONS		0x000400
#define I_F_HAS_DISKSTATS	0x000800
#define I_D_HUMAN_READ		0x001000
#define I_D_PERSIST_NAME	0x002000
#define I_D_OMIT_SINCE_BOOT	0x004000
#define I_D_JSON_OUTPUT		0x008000
#define I_D_DEVMAP_NAME		0x010000
#define I_D_ISO			0x020000
#define I_D_GROUP_TOTAL_ONLY	0x040000
#define I_D_ZERO_OMIT		0x080000
#define I_D_UNIT		0x100000
#define I_D_SHORT_OUTPUT	0x200000

#define DISPLAY_CPU(m)			(((m) & I_D_CPU)              == I_D_CPU)
#define DISPLAY_DISK(m)			(((m) & I_D_DISK)             == I_D_DISK)
#define DISPLAY_TIMESTAMP(m)		(((m) & I_D_TIMESTAMP)        == I_D_TIMESTAMP)
#define DISPLAY_EXTENDED(m)		(((m) & I_D_EXTENDED)         == I_D_EXTENDED)
#define DISPLAY_PART_ALL(m)		(((m) & I_D_PART_ALL)         == I_D_PART_ALL)
#define DISPLAY_KILOBYTES(m)		(((m) & I_D_KILOBYTES)        == I_D_KILOBYTES)
#define DISPLAY_MEGABYTES(m)		(((m) & I_D_MEGABYTES)        == I_D_MEGABYTES)
#define HAS_SYSFS(m)			(((m) & I_F_HAS_SYSFS)        == I_F_HAS_SYSFS)
#define DISPLAY_DEBUG(m)		(((m) & I_D_DEBUG)            == I_D_DEBUG)
#define DISPLAY_UNFILTERED(m)		(((m) & I_D_UNFILTERED)       == I_D_UNFILTERED)
#define DISPLAY_PARTITIONS(m)		(((m) & I_D_PARTITIONS)       == I_D_PARTITIONS)
#define HAS_DISKSTATS(m)		(((m) & I_F_HAS_DISKSTATS)    == I_F_HAS_DISKSTATS)
#define DISPLAY_HUMAN_READ(m)		(((m) & I_D_HUMAN_READ)       == I_D_HUMAN_READ)
#define DISPLAY_PERSIST_NAME_I(m)	(((m) & I_D_PERSIST_NAME)     == I_D_PERSIST_NAME)
#define DISPLAY_OMIT_SINCE_BOOT(m)	(((m) & I_D_OMIT_SINCE_BOOT)  == I_D_OMIT_SINCE_BOOT)
#define DISPLAY_DEVMAP_NAME(m)		(((m) & I_D_DEVMAP_NAME)      == I_D_DEVMAP_NAME)
#define DISPLAY_ISO(m)			(((m) & I_D_ISO)              == I_D_ISO)
#define DISPLAY_GROUP_TOTAL_ONLY(m)	(((m) & I_D_GROUP_TOTAL_ONLY) == I_D_GROUP_TOTAL_ONLY)
#define DISPLAY_ZERO_OMIT(m)		(((m) & I_D_ZERO_OMIT)        == I_D_ZERO_OMIT)
#define DISPLAY_JSON_OUTPUT(m)		(((m) & I_D_JSON_OUTPUT)      == I_D_JSON_OUTPUT)
#define DISPLAY_UNIT(m)			(((m) & I_D_UNIT)	      == I_D_UNIT)
#define DISPLAY_SHORT_OUTPUT(m)		(((m) & I_D_SHORT_OUTPUT)     == I_D_SHORT_OUTPUT)

/* Preallocation constants */
#define NR_DEV_PREALLOC		4

/* Environment variable */
#define ENV_POSIXLY_CORRECT	"POSIXLY_CORRECT"

/*
 * Structures for I/O stats.
 * The number of structures allocated corresponds to the number of devices
 * present in the system, plus a preallocation number to handle those
 * that can be registered dynamically.
 * The number of devices is found by using /sys filesystem (if mounted).
 * For each io_stats structure allocated corresponds a io_hdr_stats structure.
 * A io_stats structure is considered as unused or "free" (containing no stats
 * for a particular device) if the 'major' field of the io_hdr_stats
 * structure is set to 0.
 */
struct io_stats {
	/* # of sectors read */
	unsigned long rd_sectors	__attribute__ ((aligned (8)));
	/* # of sectors written */
	unsigned long wr_sectors	__attribute__ ((packed));
	/* # of sectors discarded */
	unsigned long dc_sectors	__attribute__ ((packed));
	/* # of read operations issued to the device */
	unsigned long rd_ios		__attribute__ ((packed));
	/* # of read requests merged */
	unsigned long rd_merges		__attribute__ ((packed));
	/* # of write operations issued to the device */
	unsigned long wr_ios		__attribute__ ((packed));
	/* # of write requests merged */
	unsigned long wr_merges		__attribute__ ((packed));
	/* # of discard operations issued to the device */
	unsigned long dc_ios		__attribute__ ((packed));
	/* # of discard requests merged */
	unsigned long dc_merges		__attribute__ ((packed));
	/* Time of read requests in queue */
	unsigned int  rd_ticks		__attribute__ ((packed));
	/* Time of write requests in queue */
	unsigned int  wr_ticks		__attribute__ ((packed));
	/* Time of discard requests in queue */
	unsigned int  dc_ticks		__attribute__ ((packed));
	/* # of I/Os in progress */
	unsigned int  ios_pgr		__attribute__ ((packed));
	/* # of ticks total (for this device) for I/O */
	unsigned int  tot_ticks		__attribute__ ((packed));
	/* # of ticks requests spent in queue */
	unsigned int  rq_ticks		__attribute__ ((packed));
};

#define IO_STATS_SIZE	(sizeof(struct io_stats))

struct ext_io_stats {
	/* r_await */
	double r_await;
	/* w_await */
	double w_await;
	/* d_await */
	double d_await;
	/* rsec/s */
	double rsectors;
	/* wsec/s */
	double wsectors;
	/* dsec/s */
	double dsectors;
	/* sec/s */
	double sectors;
	/* %rrqm */
	double rrqm_pc;
	/* %wrqm */
	double wrqm_pc;
	/* %drqm */
	double drqm_pc;
	/* rareq-sz */
	double rarqsz;
	/* wareq-sz */
	double warqsz;
	/* dareq-sz */
	double darqsz;
};

/* Possible values for field "status" in io_hdr_stats structure */
#define DISK_UNREGISTERED	0
#define DISK_REGISTERED		1
#define DISK_GROUP		2

/*
 * Each io_stats structure has an associated io_hdr_stats structure.
 * An io_hdr_stats structure tells if the corresponding device has been
 * unregistered or not (status field) and also indicates the device name.
 */
struct io_hdr_stats {
	unsigned int status		__attribute__ ((aligned (4)));
	unsigned int used		__attribute__ ((packed));
	char name[MAX_NAME_LEN];
};

#define IO_HDR_STATS_SIZE	(sizeof(struct io_hdr_stats))

/* List of devices entered on the command line */
struct io_dlist {
	/* Indicate whether its partitions are to be displayed or not */
	int disp_part			__attribute__ ((aligned (4)));
	/* Device name */
	char dev_name[MAX_NAME_LEN];
};

#define IO_DLIST_SIZE	(sizeof(struct io_dlist))

#endif  /* _IOSTAT_H */


