
#ifndef ANALYZE_H
#define ANALYZE_H

#include "config.h"

#include "input.h"


// Analyze buffer of given length
extern int analyze(struct mapping* in_buffer, void* out_buffer, 
	unsigned long fft_size);

#endif  // ANALYZE_H
