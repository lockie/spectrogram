
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "mad.h"
#include "fftw3.h"
#include "png.h"

#include "analyze.h"
#include "output.h"


#define UNUSED_ARG(x) (void)x
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define SQR(X) ((X)*(X))


/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */
static inline
signed int scale(mad_fixed_t sample)
{
	double s = mad_f_todouble(sample);

	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
	sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
	sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static int* do_fft(unsigned int nsamples, signed long* buf)
{
	// vars
	double* in;
	fftw_complex* out;
	fftw_plan plan;
	unsigned int i;

	// Allocate arrays
	int* res = (int*)malloc(nsamples * sizeof(int));
	if(!res)
	{
		wprintf(L"ERROR: Unable to allocate memory\n");
		return NULL;
	}
	in = 
		(double*)fftw_malloc(nsamples * sizeof(double));
	if(!in)
	{
		free(res);
		wprintf(L"ERROR: Unable to allocate memory\n");
		return NULL;
	}
	out = 
		(fftw_complex*)fftw_malloc(nsamples * sizeof(fftw_complex));
	if(!out)
	{
		fftw_free(in);
		free(res);
		wprintf(L"ERROR: Unable to allocate memory\n");
		return NULL;
	}

	// Plan FFT
	plan = 
		fftw_plan_dft_r2c_1d(nsamples, in, out, FFTW_ESTIMATE);
	if(!plan)
	{
		fftw_free(out);
		fftw_free(in);
		free(res);
		wprintf(L"ERROR: Unable to create fftw plan\n");
		return NULL;
	}

	// Fill input
	for(i = 0; i < nsamples; i++)
		in[i] = scale(buf[i]);

	// Do FFT
	fftw_execute(plan);

	// Get result
	for(i = 0; i < nsamples; i++)
	{
		res[i] = (int)(sqrt(SQR(out[i][0]) + SQR(out[i][1])) / nsamples);
	}

	// Cleanup
	fftw_free(out);
	fftw_free(in);

	return res;
}

// This is a private message structure. A generic pointer to this structure
//  is passed to each of the callback functions. Put here any data you need
//  to access from within the callbacks.
struct buffer {
	unsigned char const* start;
	unsigned long length;
	unsigned long fft_size;
	unsigned long nsamples;
	signed long* current_buffer;
	unsigned long current_buffer_size;
	png_bytep row_pointers[HEIGHT];
	int current_row;
};

// The purpose of this input callback is to (re)fill
//  the stream buffer which is to be decoded. An entire file
//  has been mapped into memory, so we just call mad_stream_buffer() with the
//  address and length of the mapping. When this callback is called a second
//  time, we are finished decoding.
static enum mad_flow input(void* data, struct mad_stream* stream)
{
	struct buffer* buff = (struct buffer*)data;

	if (!buff->length)
		return MAD_FLOW_STOP;

	mad_stream_buffer(stream, buff->start, buff->length);

	buff->length = 0;

	return MAD_FLOW_CONTINUE;
}

// This is the output callback function. It is called after each frame of
//  MPEG audio data has been completely decoded. The purpose of this callback
//  is to output (or play) the decoded PCM audio.
static enum mad_flow output(void* data, struct mad_header const* header,
	struct mad_pcm* pcm)
{
	// vars
	int size;
	unsigned int i;
	int* fft;

	struct buffer* buff = data;

	if(!buff->nsamples)
		buff->nsamples = pcm->samplerate;  // 1s window length
	if(!buff->current_buffer)
	{
		buff->current_buffer =
			(signed long*)malloc(buff->nsamples * sizeof(signed long));
		if(!buff->current_buffer)
		{
			wprintf(L"ERROR: Unable to allocate memory\n");
			return MAD_FLOW_BREAK;
		}
	}

	size = MAX(buff->nsamples - buff->current_buffer_size, pcm->length);
	memcpy(&buff->current_buffer[buff->current_buffer_size],
			pcm->samples[0], size);
	buff->current_buffer_size += size;

	if(buff->current_buffer_size == buff->nsamples)
	{
		fft = do_fft(buff->nsamples, buff->current_buffer);
		if(!fft)
			return MAD_FLOW_BREAK;

		// Fill output image row
		if(buff->current_row < HEIGHT)
		{
			png_bytep row = buff->row_pointers[buff->current_row] = 
				(png_bytep)malloc(sizeof(png_byte) * buff->fft_size);
			memset(row, 0, buff->fft_size * sizeof(png_byte));
			for(i = 0; i < MIN(buff->fft_size, buff->nsamples/2); i++)
				row[i] = fft[i];

			buff->current_row++;
		}
		free(fft);

		buff->current_buffer_size = 0;
		if(size < pcm->length)
		{
			buff->current_buffer_size = pcm->length - size;
			memcpy(buff->current_buffer, &pcm->samples[0][size],
					buff->current_buffer_size);
		}
	}

	UNUSED_ARG(header);

	return MAD_FLOW_CONTINUE;
}

// Error callback function. The list of possible MAD_ERROR_* errors
//  can be found in the mad.h (or stream.h) header file.
static enum mad_flow error(void* data, struct mad_stream* stream,
	struct mad_frame* frame)
{
	struct buffer* buff = (struct buffer*)data;

	wprintf( L"WARNING: libmad decoding error %#x (%s) at byte offset %#x\n",
		stream->error, 
		mad_stream_errorstr(stream), 
		stream->this_frame - buff->start );

	UNUSED_ARG(frame);

	// return MAD_FLOW_BREAK here to stop decoding and issue error
	return MAD_FLOW_CONTINUE;
}

// Do analyzing work
int analyze(struct mapping* in_buffer, void* out_buffer, unsigned long fft_size)
{
	struct buffer buff;
	struct mad_decoder decoder;
	int result;

	// Initialize our private message structure
	memset(&buff, 0, sizeof(buff));
	buff.start  = in_buffer->map;
	buff.length = in_buffer->size;
	buff.fft_size = fft_size;

	// Configure input, output, and error functions
	mad_decoder_init(&decoder, &buff,
		input, 0 /* header */, 0 /* filter */, output,
		error, 0 /* message */);

	// Start decoding
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	// Write result
	png_write_image((png_structp)out_buffer, buff.row_pointers);

	// Cleanup
	mad_decoder_finish(&decoder);

	return result;
}
