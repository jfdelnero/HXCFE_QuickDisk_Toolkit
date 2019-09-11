/*
//
// Copyright (C) 2006-2019 Jean-François DEL NERO
//
// This file is part of HxCFloppyEmulator.
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <stdint.h>

#include "crc.h"
#include "utils.h"

#include "hfe_qd.h"
#include "trk_utils.h"

#include "wave.h"

#define DEFAULT_NUMBER_OF_CHANNELS 1
#define DEFAULT_SAMPLERATE 32000

#define S612_FORMAT 0
#define S700_FORMAT 1

static unsigned short checkcrc(unsigned char * buffer,int size)
{
	unsigned char CRC16_High,CRC16_Low;
	unsigned char crctable[32];
	int i;

	CRC16_Init( &CRC16_High, &CRC16_Low,crctable,0x8005,0x0000);

	for(i=0;i<size;i++)
	{
		CRC16_Update(&CRC16_High, &CRC16_Low, LUT_ByteBitsInverter[buffer[i]],(unsigned char*)&crctable);
	}

	return ((unsigned short)CRC16_High<<8) | CRC16_Low;
}

int check_akai_qd(char * filename)
{
	unsigned int i;
	int m;
	FILE *f;
	FILE *f2;
	FILE *f3;
	int found;
	int filesize;
	char tmpfilename[64];
	unsigned char * file;
	unsigned char * test_buf;
	unsigned char syncword[32];
	int position,offset,subformat,tracksize,samplerate;
	unsigned short val1;
	unsigned short * wave;
	wav_hdr wavhdr;
	qdhfefileformatheader * header_ptr;
	qdtrack * track_ptr;

	f=fopen(filename,"rb");
	if(f)
	{
		fseek(f,0,SEEK_END);
		filesize = ftell(f);
		fseek(f,0,SEEK_SET);

		file = malloc(filesize);
		if( file )
		{
			test_buf = malloc(1024*80);

			if( fread(file,filesize,1,f) != 1)
			{
				printf("!!!!!!!!Error while reading %s !!!!!!!!!!\n",filename);
			}

			fclose(f);

			header_ptr = (qdhfefileformatheader * )file;
			track_ptr = (qdtrack *)&file[header_ptr->track_list_offset];

			for(i=track_ptr->offset;i<track_ptr->track_len;i++)
			{
				file[i] = LUT_ByteBitsInverter[file[i]];
			}

			printf("Saving inverted MFM track (inverted.INV)\n");
			f2 = fopen("inverted.INV","wb");
			if(f2)
			{
				fwrite(&file[track_ptr->offset],track_ptr->track_len,1,f2);
				fclose(f2);
			}

			memset(test_buf,0,80*1024);

			found = track_ptr->offset*8;

			syncword[0] = 0x94;
			syncword[1] = 0x4A;
			syncword[2] = 0x94;
			syncword[3] = 0x4A;
			syncword[4] = 0x94;
			syncword[5] = 0x4A;
			syncword[6] = 0x94;
			syncword[7] = 0x4A;
			syncword[8] = 0x94;
			syncword[9] = 0x4A;
			syncword[10] = 0x94;
			syncword[11] = 0x4A;
			syncword[12] = 0x94;
			syncword[13] = 0x4A;
			syncword[14] = 0x44;
			syncword[15] = 0x91;

			found = searchBitStream(&file[track_ptr->offset],track_ptr->track_len*8,track_ptr->track_len*8,syncword,8*16,found);

			if(found>=0)
			{
				mfmtobin(&file[track_ptr->offset],track_ptr->track_len*8,test_buf,80*1024,found,0 );
				for(i=0;i<80*1024;i++)
				{
					test_buf[i] = LUT_ByteBitsInverter[test_buf[i]];
				}

				sprintf(tmpfilename,"akai_block.bin");
				printf("\nSaving MFM block file %s ...\n",tmpfilename);
				f3 = fopen(tmpfilename,"wb");
				if(f3)
				{
					fwrite(test_buf,80*1024,1,f3);
					fclose(f3);
				}

				printf("\nBlock : cell offset %d (0x%X offset)(%f ms)\n",found,found/8,((float)found/(float)header_ptr->bitRate)*1000);

				if( !memcmp(&test_buf[8],"S700 FORMAT",11) )
				{
					printf("Akai S700 format\n");
					subformat = S700_FORMAT;
					tracksize = 0xC0BD;
					samplerate = 22050;
				}
				else
				{
					printf("Akai S612 format\n");
					subformat = S612_FORMAT;
					tracksize = 0xFC23;
					samplerate = DEFAULT_SAMPLERATE;
				}

				printbuf(test_buf,0xA0);
				if(checkcrc(&test_buf[7],tracksize))
				{
					printf(">>>>>> BAD CRC :( !\n");
				}
				else
				{
					printf(">>>>>> Valid CRC :) !\n");
				}

				wave = malloc(64*1024*2);
				memset(wave,0,64*1024*2);

				switch(subformat)
				{
					case S612_FORMAT:
						m = 0;
						offset = 0x28;
						for(i=0;i< (0xFC00/2);i++)
						{
							position = i * 2;
							val1 = (((unsigned short)test_buf[offset+position + 1 ] << 8) | ((test_buf[offset+position + 0 ] >>0)))>>4;
							if(val1 & 0x0800)
								val1 |= 0xF000;

							wave[m++] = (val1<<4) + 0x8000;
						}
					break;

					case S700_FORMAT:
						m = 0;
						offset = 0xC2;
						for(i=0;i< (0x8000);i++)
						{
							val1 = ((unsigned short)test_buf[offset+ 0x4000 + i ] << 4);

							// Note : Unsure about the low bits order, to be checked with a low frequency ramp sound.
							if(i&1)
								val1 |= ((unsigned short)test_buf[offset + (i>>1) ] >> 4) & 0xF;
							else
								val1 |= ((unsigned short)test_buf[offset + (i>>1) ] >> 0) & 0xF;

							if(val1 & 0x0800)
								val1 |= 0xF000;

							wave[m++] = (val1<<4) + 0x8000;
						}
					break;

				}

				memset(&wavhdr,0,sizeof(wavhdr));
				printf("Saving sample.wav...\n");
				f3 = fopen("sample.wav","wb");
				if(f3)
				{
					memcpy((char*)&wavhdr.RIFF,"RIFF",4);
					memcpy((char*)&wavhdr.WAVE,"WAVE",4);
					memcpy((char*)&wavhdr.fmt,"fmt ",4);
					wavhdr.Subchunk1Size = 16;
					wavhdr.AudioFormat = 1;
					wavhdr.NumOfChan = DEFAULT_NUMBER_OF_CHANNELS;
					wavhdr.SamplesPerSec = samplerate;
					wavhdr.bitsPerSample = 16;
					wavhdr.bytesPerSec = ((samplerate*wavhdr.bitsPerSample)/8);
					wavhdr.blockAlign = (wavhdr.bitsPerSample/8);
					memcpy((char*)&wavhdr.Subchunk2ID,"data",4);

					wavhdr.ChunkSize = m*2 + sizeof(wav_hdr) - 8;
					wavhdr.Subchunk2Size = m*2;

					fwrite((void*)&wavhdr,sizeof(wav_hdr),1,f3);

					fwrite(wave,m*2,1,f3);

					fclose(f3);
				}

				free(wave);
			}

			free(test_buf);
			free(file);
		}
	}
	else
	{
		printf("!!!!!!!!Error while opening %s !!!!!!!!!!\n",filename);
	}

	return 0;
}
