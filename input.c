
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _WIN32
# include <unistd.h>
# include <sys/mman.h>
#endif  // _WIN32

#include "input.h"


void* open_infile(const _TCHAR* fn)
{
	void* file;

#ifdef _WIN32
	file = CreateFile(
		fn,
		FILE_GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL );
#else  // _WIN32
	file = (void*)(intptr_t)open(fn, O_RDONLY, 0);
#endif  // _WIN32

	if((long int)file == -1)
	{
		wprintf(L"ERROR: Unable to open file \"%s\"\n", fn);
		return NULL;
	}

	return file;
}

struct mapping* init_input(void* fp, unsigned long size)
{
	struct mapping* res = (struct mapping*)malloc(sizeof(struct mapping));
	if(!res)
	{
		wprintf(L"ERROR: Unable to allocate memory\n");
		return NULL;
	}
	res->_mapping = res->map = NULL;
	res->size = size;

#ifdef _WIN32
	{
		HANDLE file = fp;
		res->_mapping = CreateFileMapping(
			file, 
			NULL, 
			PAGE_READONLY, 
			0, 
			size, 
			NULL );
		if(!res->_mapping)
		{
			CloseHandle(file);
			wprintf(L"ERROR: Unable to create file mapping\n");
			return NULL;
		}
		res->map = MapViewOfFile(
			res->_mapping, 
			FILE_MAP_READ, 
			0, 
			0, 
			size );
		if(!res->map)
		{
			CloseHandle(res->_mapping);
			CloseHandle(file);
			wprintf(L"ERROR: Unable to map file to memory\n");
			return NULL;
		}
	}
#else  // _WIN32
	res->map = mmap(0, size, PROT_READ, MAP_PRIVATE,
			(int)fp, 0);
	if(res->map == MAP_FAILED)
	{
		wprintf(L"ERROR: Unable to map file to memory\n");
		return NULL;
	}
#endif  // _WIN32

	return res;
}

void finalize_input(struct mapping* m, void* fp)
{
#ifdef _WIN32
	if(m)
	{
		UnmapViewOfFile(m->map);
		CloseHandle(m->_mapping);
		free(m);
	}
	CloseHandle(fp);
#else  // _WIN32
	if(m)
		munmap(m->map, m->size);
	close((int)fp);
#endif  // _WIN32
}
