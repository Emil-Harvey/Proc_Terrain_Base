// Copyright(c) 2019 Nick Klingensmith (@koujaku). All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy of this license, see < https://opensource.org/licenses/MIT >

#pragma once

#include <stdio.h>
#include <stdint.h>

/// <summary> Writes an uncompressed 24 or 32 bit (8 bit per channel) depth .tga image to the indicated file! </summary>
/// <param name='filename'>I'd recommended you add a '.tga' to the end of this filename.</param>
/// <param name='dataBGRA'>A chunk of color data, one channel per byte, ordered as BGRA. Size should be width*height*dataChannels.</param>
/// <param name='dataChannels'>The number of channels in the color data. Use 1 for grayscale, 3 for BGR, and 4 for BGRA.</param>
/// <param name='fileChannels'>The number of color channels to write to file. Must be 3 for BGR, or 4 for BGRA. Does NOT need to match dataChannels.</param>
static void tga_write(const char* filename, uint32_t width, uint32_t height, uint8_t* dataBGRA, uint8_t dataChannels = 4, uint8_t fileChannels = 3)
{
	FILE* fp = NULL;
	// MSVC prefers fopen_s, but it's not portable
	//fp = fopen(filename, "wb");
	fopen_s(&fp, filename, "wb");
	if (fp == NULL) return;

	// You can find details about TGA headers here: http://www.paulbourke.net/dataformats/tga/
	uint8_t header[18] = { 0,0,2,0,0,0,0,0,0,0,0,0, (uint8_t)(width % 256), (uint8_t)(width / 256), (uint8_t)(height % 256), (uint8_t)(height / 256), (uint8_t)(fileChannels * 8), 0x20 };
	fwrite(&header, 18, 1, fp);

	for (uint32_t i = 0; i < width * height; i++)
	{
		for (uint32_t b = 0; b < fileChannels; b++)
		{
			fputc(dataBGRA[(i * dataChannels) + (b % dataChannels)], fp);
		}
	}
	fclose(fp);
}

/// Writes an uncompressed 32 bit grayscale .raw image (no header) to the indicated file.
/// This file will be identical to the data array in memory.
/// 
/// Add a '.raw' to the end of the filename.
/// Width & height are interchangable, just used for more readibility [the dimensions are assumed to be known by whoever uses the file]. i.e. 100 * 40 data is be no different to 2 * 200
/// data: An array of float values, [width * height] size.
static void create_raw(const char* filename, uint32_t width, uint32_t height, float* data)// one channel.
{
	FILE* fp = NULL;
	// MSVC prefers fopen_s, but it's not portable
	//fp = fopen(filename, "wb");
	fopen_s(&fp, filename, "wb");
	if (fp == NULL) return;

	// no header

	for (unsigned int i = 0; i < width * height; i++)
	{
		fputc(data[i], fp);		
	}
	fclose(fp);
}