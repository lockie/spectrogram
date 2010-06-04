
#ifndef OUTPUT_H
#define OUTPUT_H

#include "config.h"

#include <stdio.h>
#include <wchar.h>


#define HEIGHT 256

extern FILE* open_outfile(const _TCHAR* filename);
extern void* init_output(FILE* fp, unsigned long fft_size);
extern int finalize_output(void* png_ptr, FILE* fp);

#endif  // OUTPUT_H
