
#ifndef INPUT_H
#define INPUT_H

#include "config.h"

#include <wchar.h>


struct mapping
{
	void* map;
	void* _mapping;
	unsigned long size;
};

extern void* open_infile(const _TCHAR* filename);
extern struct mapping* init_input(void* fp, unsigned long size);
extern void finalize_input(struct mapping* m, void* fp);

#endif  // INPUT_H
