/*
//
// Copyright (C) 2006-2022 Jean-François DEL NERO
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

#include "hfe_qd.h"
#include "trk_utils.h"

#include "utils.h"

#include "wave.h"

#define DEFAULT_NUMBER_OF_CHANNELS 1
#define DEFAULT_SAMPLERATE 30000

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

int check_roland_qd(char * filename)
{
	unsigned int i;
	int block,m;
	FILE *f;
	FILE *f2;
	FILE *f3;
	int found;
	int filesize;
	char tmpfilename[64];
	unsigned char * file;
	unsigned char * test_buf;
	unsigned char syncword[32];
	int position,offset;
	unsigned short val1,val2;
	unsigned short * wave;
	wav_hdr wavhdr;
	qdhfefileformatheader * header_ptr;
	qdtrack * track_ptr;
	unsigned char internalname[64];

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

			if(check_and_fix_qd_header(header_ptr, filesize) < 0)
			{
				free(file);
				return -1;
			}

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

			for(block=0;block < 3;block++)
			{
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

				if(block>0)
					found = searchBitStream(&file[track_ptr->offset],track_ptr->track_len*8,track_ptr->track_len*8,syncword,8*16,found+(32*8));

				if(block>1)
					found = searchBitStream(&file[track_ptr->offset],track_ptr->track_len*8,track_ptr->track_len*8,syncword,8*16,found+(32*8));

				if(found>=0)
				{
					mfmtobin(&file[track_ptr->offset],track_ptr->track_len*8,test_buf,80*1024,found,0 );
					for(i=0;i<80*1024;i++)
					{
						test_buf[i] = LUT_ByteBitsInverter[test_buf[i]];
					}

					sprintf(tmpfilename,"roland_block_%d.bin",block);
					printf("\nSaving MFM block file %s ...\n",tmpfilename);
					f3 = fopen(tmpfilename,"wb");
					if(f3)
					{
						fwrite(test_buf,80*1024,1,f3);
						fclose(f3);
					}

					switch(block)
					{
						case 0:
							printf("\nBlock 1 : cell offset %d (0x%X offset)(%f ms)\n",found,found/8,((float)found/(float)header_ptr->bitRate)*1000);
							printbuf(test_buf,0x40);
							if(checkcrc(&test_buf[7],4))
							{
								printf(">>>>>> BAD CRC :( !\n");
							}
							else
							{
								printf(">>>>>> Valid CRC :) !\n");
							}
						break;

						case 1:
							printf("\nBlock 2 : cell offset %d (0x%X offset)(%f ms)\n",found,found/8,((float)found/(float)header_ptr->bitRate)*1000);
							printbuf(test_buf,0x60);
							if(checkcrc(&test_buf[7],0x46))
							{
								printf(">>>>>> BAD CRC :( !\n");
							}
							else
							{
								printf(">>>>>> Valid CRC :) !\n");
							}

							memset(internalname,0,sizeof(internalname));
							i = 0;
							while( i < 16 && test_buf[ 0xC + i ]!=0xD)
							{
								if(is_printable_char(test_buf[ 0xC + i ]))
									internalname[i] = test_buf[ 0xC + i ];
								else
									internalname[i] = '_';
								i++;
							}
							i = strlen((char*)internalname) - 1;
							while(i>0 && internalname[i] == '_')
							{
								internalname[i] = 0;
								i--;
							}
							printf("Internal Name :%s\n",internalname);

						break;

						case 2:
							printf("\nBlock 3 : cell offset %d (0x%X offset)(%f ms)\n",found,found/8,((float)found/(float)header_ptr->bitRate)*1000);

							printbuf(test_buf,0xA0);
							if(checkcrc(&test_buf[7],0xC0A6))
							{
								printf(">>>>>> BAD CRC :( !\n");
							}
							else
							{
								printf(">>>>>> Valid CRC :) !\n");
							}

							wave = malloc(64*1024*2);
							memset(wave,0,64*1024*2);

							m = 0;
							offset = 0xEF;
							for(i=0;i< (32722/2);i++)
							{
								// Note : Unsure about the low bits order, to be checked with a low frequency ramp sound.
								position = i * 3;
								val1 = ((unsigned short)test_buf[offset+position + 0 ] << 4) | ((test_buf[offset+position + 3 ] >>0) & 0xF);
								if(val1 & 0x0800)
									val1 |= 0xF000;

								val2 = ((unsigned short)test_buf[offset+position + 1 ] << 4) | ((test_buf[offset+position + 3 ] >>4)& 0xF);
								if(val2 & 0x0800)
									val2 |= 0xF000;

								wave[m++] = val1<<4;
								wave[m++] = val2<<4;
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
								wavhdr.SamplesPerSec = DEFAULT_SAMPLERATE;
								wavhdr.bitsPerSample = 16;
								wavhdr.bytesPerSec = ((DEFAULT_SAMPLERATE*wavhdr.bitsPerSample)/8);
								wavhdr.blockAlign = (wavhdr.bitsPerSample/8);
								memcpy((char*)&wavhdr.Subchunk2ID,"data",4);

								wavhdr.ChunkSize = m*2 + sizeof(wav_hdr) - 8;
								wavhdr.Subchunk2Size = m*2;

								fwrite((void*)&wavhdr,sizeof(wav_hdr),1,f3);

								fwrite(wave,m*2,1,f3);

								fclose(f3);
							}

							free(wave);
						break;
					}
				}
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
