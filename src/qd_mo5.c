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

#include "utils.h"

#include "hfe_qd.h"
#include "trk_utils.h"

#define PREAMBULE_SIZE 2500
#define ENDTRACK_SIZE 128
#define NUM_OF_SECTORS 400
#define SECTOR_SIZE 128

//Table des secteurs logiques en fonction des secteurs physiques du Quick Disk
//(Thanks Mr Daniel Coulon :) )!
int qdsector[400] =
{
 321, 33,225,129,322, 34,226,130,323, 35,227,131,324, 36,228,132, //p20,2,14,8
 325, 37,229,133,326, 38,230,134,327, 39,231,135,328, 40,232,136,
 329, 41,233,137,330, 42,234,138,331, 43,235,139,332, 44,236,140,
 333, 45,237,141,334, 46,238,142,335, 47,239,143,336, 48,240,144,
 337,305,209,113,338,306,210,114,339,307,211,115,340,308,212,116, //p21,19,13,3
 341,309,213,117,342,310,214,118,343,311,215,119,344,312,216,120,
 345,313,217,121,346,314,218,122,347,315,219,123,348,316,220,124,
 349,317,221,125,350,318,222,126,351,319,223,127,352,320,224,128,
 353,289,193, 97,354,290,194, 98,355,291,195, 99,356,292,196,100, //p22,18,12,6
 357,293,197,101,358,294,198,102,359,295,199,103,360,296,200,104,
 361,297,201,105,362,298,202,106,363,299,203,107,364,300,204,108,
 365,301,205,109,366,302,206,110,367,303,207,111,368,304,208,112,
 369,273,177, 81,370,274,178, 82,371,275,179, 83,372,276,180, 84, //p23,17,11,5
 373,277,181, 85,374,278,182, 86,375,279,183, 87,376,280,184, 88,
 377,281,185, 89,378,282,186, 90,379,283,187, 91,380,284,188, 92,
 381,285,189, 93,382,286,190, 94,383,287,191, 95,384,288,192, 96,
 385,257,161, 65,386,258,162, 66,387,259,163, 67,388,260,164, 68, //p24,16,10,4
 389,261,165, 69,390,262,166, 70,391,263,167, 71,392,264,168, 72,
 393,265,169, 73,394,266,170, 74,395,267,171, 75,396,268,172, 76,
 397,269,173, 77,398,270,174, 78,399,271,175, 79,400,272,176, 80,
  17,241,145, 49, 18,242,146, 50, 19,243,147, 51, 20,244,148, 52, //p1,15,9,3
  21,245,149, 53, 22,246,150, 54, 23,247,151, 55, 24,248,152, 56,
  25,249,153, 57, 26,250,154, 58, 27,251,155, 59, 28,252,156, 60,
  29,253,157, 61, 30,254,158, 62, 31,255,159, 63, 32,256,160, 64,
   1,  9,  5, 13,  2, 10,  6, 14,  3, 11,  7, 15,  4, 12,  8, 16  //p0
};

unsigned char * load_mo5_trk(char * infile, unsigned int * outbuffersize)
{
	unsigned int i,j,k;
	FILE *f;
	unsigned char * outbuffer,sum;
	unsigned char * mfmbuffer;
	unsigned char chksum;
	unsigned int mfmoutsize;

	printf("Loading MO5 QD file %s...\n",infile);

	mfmbuffer = NULL;

	f=fopen(infile,"rb");
	if(f)
	{
		outbuffer = malloc((NUM_OF_SECTORS * 161) + PREAMBULE_SIZE + ENDTRACK_SIZE);
		if(!outbuffer)
			return 0;

		memset(outbuffer, 0, (NUM_OF_SECTORS * 161) + PREAMBULE_SIZE + ENDTRACK_SIZE);

		k = 0;

		for(j=0;j<PREAMBULE_SIZE;j++)
		{
			outbuffer[k++] = 0x16;
		}

		for(i=0;i<NUM_OF_SECTORS;i++)
		{
			for(j=0;j<17;j++)
			{
				outbuffer[k++] = 0x16;
			}

			outbuffer[k++] = 0xA5;
			outbuffer[k++] = ((i+1)>>8);
			outbuffer[k++] = ((i+1)&0xFF);
			chksum = outbuffer[k - 1] + outbuffer[k - 2] + outbuffer[k - 3];
			outbuffer[k++] = chksum;

			for(j=0;j<10;j++)
			{
				outbuffer[k++] = 0x16;
			}

			sum = 0;
			outbuffer[k++] = 0x5A;

			//fseek(f, get_phys_sector(i+1) * SECTOR_SIZE, SEEK_SET);

			fseek(f, (qdsector[i]-1) * SECTOR_SIZE, SEEK_SET);
			if(fread(&outbuffer[k],SECTOR_SIZE,1,f)!=1)
			{
				printf("Error while source file !?\n");
			}
			k += SECTOR_SIZE;

			for(j=0;j<(SECTOR_SIZE+1);j++)
			{
				sum += outbuffer[ (k - (SECTOR_SIZE + 1)) + j];
			}
			outbuffer[k++] = sum;
		}

		for(j=0;j<ENDTRACK_SIZE;j++)
		{
			outbuffer[k++] = 0x16;
		}

		mfmoutsize = k * 2;

		mfmbuffer = malloc(mfmoutsize);
		if(!mfmbuffer)
		{
			free(outbuffer);
			return 0;
		}

		memset(mfmbuffer,0,mfmoutsize);

		bintomfm(mfmbuffer,mfmoutsize*8,outbuffer,(NUM_OF_SECTORS * 161) + PREAMBULE_SIZE + ENDTRACK_SIZE,0);

		for(i=0;i<mfmoutsize;i++)
		{
			mfmbuffer[i] = LUT_ByteBitsInverter[mfmbuffer[i]];
		}

		*outbuffersize = mfmoutsize;
	}

	return mfmbuffer;
}


int check_mo5_qd(char * filename)
{
	int i,k,lastsector,sector;
	FILE *f;
	FILE *f2;
	int lastfound,found,founddat;
	int filesize;
	unsigned char * file;
	unsigned char * test_buf;
	unsigned char syncword[16],checksum;

	printf("Checking MO5 %s...\n",filename);
	
	f = fopen(filename,"rb");
	if(f)
	{
		fseek(f,0,SEEK_END);
		filesize = ftell(f);
		fseek(f,0,SEEK_SET);

		file = malloc(filesize);
		if( file)
		{
			test_buf = malloc(1024*80);
		
			if(fread(file,filesize,1,f) != 1)
				printf("!!!!! Error while reading %s !!!!!!\n",filename);

			fclose(f);

			for(i=0;i<filesize;i++)
			{
				file[i] = LUT_ByteBitsInverter[file[i]];
			}

			f2 = fopen("inverted.INV","wb");
			if(f2)
			{
				fwrite(&file[512*2],filesize - (512*2),1,f2);
				fclose(f2);
			}

			syncword[0] = 0x44;
			syncword[1] = 0x91;

			lastsector = 0;
			sector = 0;

			found = 0x400*8;
			lastfound = found;
			while(sector != 400)
			{
				syncword[0] = 0xA9;
				syncword[1] = 0x14;
				syncword[2] = 0xA9;
				syncword[3] = 0x14;
				syncword[4] = 0xA9;
				syncword[5] = 0x14;
				syncword[6] = 0xA9;
				syncword[7] = 0x14;
				syncword[8] = 0xA9;
				syncword[9] = 0x14;
				syncword[10] = 0x44;
				syncword[11] = 0x91;
				found = searchBitStream(file,filesize*8,filesize*8,syncword,8*12,found);
				if(found>=0)
				{
					found += (10*8);
					mfmtobin(file,filesize*8,test_buf,0xAF,found,0 );

					sector = (test_buf[1]<<8) + test_buf[2];
					syncword[10] = 0x91;
					syncword[11] = 0x44;
					founddat = searchBitStream(file,filesize*8,filesize*8,syncword,8*12,found + ((4)*8*2));
					if(founddat >= 0 && ((founddat -found) < 128*8))
					{
						founddat += (10*8);
						mfmtobin(file,filesize*8,test_buf,1+128+1,founddat,0 );
						checksum = 0;
						for(k=0;k<1+128;k++)
						{
							checksum += test_buf[k];
						}
					
						if(checksum == test_buf[1+128])
						{
							printf("found : Sector %d (%d) - %d - (delta sect :%d delta pos : %d)\n",sector,(founddat -found),found,sector-lastsector,found-lastfound);
						}
						else
							printf("found BAD DATA CHECKSUM: Sector %d (%d) - %d - (delta sect :%d delta pos : %d)\n",sector,(founddat -found),found,sector-lastsector,found-lastfound);
					}

					found += (128*8);
					lastsector = sector;
					lastfound = found ;
				}

				found += (0xAF*2) - 0x8;
			}
			
			free(test_buf);
			free(file);
		}
	}
	else
	{
		printf("Error can't open %s !...\n",filename);
	}	
	

	return 0;
}
