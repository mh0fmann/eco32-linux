/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eco32.c -- eco32 partition table support
 */


#include "check.h"
#include "eco32.h"


#define PARTITON_TABLE_SECTOR	1
#define NPE		(SECTOR_SIZE / sizeof(PartEntry))
#define DESCR_SIZE				20
#define PART_MAGIC	0xF5A5F2F9


typedef struct {
	unsigned int type;
	unsigned int start;
	unsigned int size;
	char descr[DESCR_SIZE];
} PartEntry;



int eco32_partition(struct parsed_partitions* state)
{

	int i;
	Sector sect;
	PartEntry* entry;

	entry = (PartEntry*)read_part_sector(state, PARTITON_TABLE_SECTOR, &sect);

	if (!entry)
		return -1;

	if (	entry[NPE-1].type != PART_MAGIC ||
	        entry[NPE-1].start != ~PART_MAGIC ||
	        entry[NPE-1].size != PART_MAGIC) {
		return 0;
	}

	for (i = 0; i < NPE-1; i++) {
		if (entry[i].type) {
			put_partition(state, i+1, entry[i].start, entry[i].size);
		}
	}

	put_dev_sector(sect);
	strlcat(state->pp_buf, "\n", PAGE_SIZE);

	return 1;
}
