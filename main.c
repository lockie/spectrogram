/*
 Spectrogram - simple C program that builds spectrogram picture of
 mp3 file using libmad, libfftw and libpng.
 Copyright (C) 2010  Andrew Kravchuk <awkravchuk@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef _WIN32
# define _CRT_SECURE_NO_WARNINGS
#endif  // _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h> 
#ifndef _WIN32
# include <sys/types.h>
# include <string.h>
# include <unistd.h>
# include <getopt.h>
# define _stat stat
# define _wstat stat
# define _wtoi atoi
# define _tcslen strlen
# define _tcscpy strcpy
# define _tcscat strcat
# define wgetopt_long getopt_long
# define woption option
# define woptarg optarg
#else  // _WIN32


#include "wgetopt.h"
#endif  // _WIN32

#include "input.h"
#include "output.h"
#include "analyze.h"


// Constants
//
#ifdef _WIN32
# define WAIT_KBHIT 1
#endif  // _WIN32

// Prototypes
//
static void wait_any_key();
static void print_usage(const _TCHAR* progname);

// Entrypoint
//
#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
#else  // _WIN32
int main(int argc, char** argv)
#endif  // _WIN32
{
	// vars
	 
	_TCHAR* input = NULL;
	_TCHAR* output = NULL;
	unsigned long fft_size = 0;
	struct _stat st;
	void* infile, * out_ptr;
	FILE* outfile;
	struct mapping* map;

	// Get args
	while(1)
	{           
		static struct woption long_options[] =
		{
			{_T("help"),         no_argument,       NULL, 'h'},
			{_T("version"),      no_argument,       NULL, 'v'},
			{_T("input-file"),   required_argument, NULL, 'i'},
			{_T("output-file"),  required_argument, NULL, 'o'},
			{_T("size"),         required_argument, NULL, 's'},
			{NULL,            0,                 NULL,  0 }
		};
		int c = wgetopt_long(argc, argv, _T("hvi:os"), long_options, NULL);

		// Detect the end of the options.
		if (c == -1)
			break;

		switch (c)
		{
		case 'v':
			wprintf(L"Spectrogram 1.0 Copyright (C) 2010  Andrew Kravchuk \
<awkravchuk@gmail.com>\n");
			wprintf(L"This is free software; see the source for copying \
conditions. There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\
PARTICULAR PURPOSE.\n");
			wait_any_key();
			return EXIT_SUCCESS;

		case 'h':
			print_usage(argv[0]);
			return EXIT_SUCCESS;

		case 'i':
			input = woptarg;
			break;

		case 'o':
			output = woptarg;
			break;

		case 's':
			if(woptarg)
				fft_size = _wtoi(woptarg);
			break;

		case '?':
			// wgetopt_long already printed an error message.
			print_usage(argv[0]);
			return EXIT_FAILURE;	

		default:
			abort();
		}
	}
	if(!input)
	{
		wprintf(L"ERROR: Missig required parameter - input file name\n");
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	if(!output)
	{
		// set output fn to input with "png" extension
		output = (_TCHAR*)malloc(sizeof(_TCHAR) * (_tcslen(input)+5));
		if(!output)
		{
			wprintf(L"ERROR: Unable to allocate memory\n");
			wait_any_key();
			return EXIT_FAILURE;
		}
		_tcscpy(output, input);
		_tcscat(output, _T(".png"));
	}
	if(fft_size == 0)
		fft_size = 1024;

	// Stat input file
	if(_wstat(input, &st) != 0)
	{
		wprintf(L"ERROR: Unable to stat \"%s\"\n", input);
		wait_any_key();
		return EXIT_FAILURE;
	}
	if(st.st_size == 0)
	{
		wprintf(L"ERROR: File \"%s\" is empty\n", input);
		wait_any_key();
		return EXIT_FAILURE;
	}

	// Init input
	infile = open_infile(input);
	if(!infile)
	{
		wait_any_key();
		return EXIT_FAILURE;
	}
	map = init_input(infile, st.st_size);
	if(!map)
	{
		finalize_input(NULL, infile);
		wait_any_key();
		return EXIT_FAILURE;
	}

	// Init output
	outfile = open_outfile(output);
	if(!outfile)
	{
		finalize_input(map, infile);
		wait_any_key();
		return EXIT_FAILURE;
	}
	out_ptr = init_output(outfile, fft_size);
	if(!out_ptr)
	{
		finalize_output(NULL, outfile);
		finalize_input(map, infile);
		wait_any_key();
		return EXIT_FAILURE;
	}

	// Do work
	if(analyze(map, out_ptr, fft_size) != 0)
	{
		finalize_output(out_ptr, outfile);
		finalize_input(map, infile);
		wprintf(L"ERROR: Failed to analyze\n");
		wait_any_key();
		return EXIT_FAILURE;
	}

	// Cleanup
	finalize_output(out_ptr, outfile);
	finalize_input(map, infile);
	
	wprintf(L"Created file blah\n");
	wait_any_key();
	return EXIT_SUCCESS;
}

static void wait_any_key()
{
#if WAIT_KBHIT
	wprintf(L"\nHit any key to quit\n");
	getchar();
#endif
}

static void print_usage(const _TCHAR* progname)
{
	wprintf(L"USAGE: %s [-h|--help] [-v|--version] -i|--input-file=<input\
file> [-o|--output-file=<output file>] [-s|--size=fft-size]\n", progname);
	wait_any_key();
}
