
#include "config.h"

#include "png.h"

#include "output.h"


FILE* open_outfile(const _TCHAR* filename)
{
	FILE* fp = fopen((const char*)filename, "wb");
	if(!fp)
	{
		wprintf(L"ERROR: Unable to open \"%s\" for writing\n", filename);
		return NULL;
	}
	return fp;
}

void* init_output(FILE* fp, unsigned long fft_size)
{
	png_infop info_ptr;
	png_structp png_ptr;

	// Create png writing struct
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
	{
		wprintf(L"ERROR: Unable to create png write struct\n");
		return NULL;
	}

	// Create png info struct
	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(fp);
		wprintf(L"ERROR: Unable to create png info struct\n");
		return NULL;
	}

	// Init IO
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		wprintf(L"ERROR: Unable to init libpng io\n");
		return NULL;
	}
	png_init_io(png_ptr, fp);

	// Write header
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		wprintf(L"ERROR: Unable to write png header\n");
		return NULL;
	}
	png_set_IHDR(png_ptr, info_ptr, fft_size, HEIGHT,
		8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	// Return png struct
	return png_ptr;
}

int finalize_output(void* p, FILE* fp)
{
	png_structp png_ptr = p;

	if(p)
	{
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			wprintf(L"ERROR: Unable to write end of png file\n");
			return 1;
		}
		png_write_end(png_ptr, NULL);
	}
	fclose(fp);
	return 0;
}
