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

#include <stdint.h>

#include "trk_utils.h"

#include "hfe_qd.h"


unsigned char LUT_ByteBitsInverter[]=
{
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

// -----------------------------------------------------------------------------
// MFM      : Reversal at each '1' or between 2 '0' (at the clock place).
// Data     : 0 c 0 c 1 c 1 c 1 c 0 c 1 c 1 c 1 c 1 c 0 c 0 c 0
//               _____     ___         ___     ___       ___
// Reversal : __|     |___|   |_______|   |___|   |_____|   |___
// Cells      0 1 0 0 1 0 1 0 1 0 0 0 1 0 1 0 1 0 1 0 0 1 0 1 0
// Decoding :  | 0 | 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 0 | 0 | 0 |
// -----------------------------------------------------------------------------

int mfmtobin(unsigned char * input_data,int input_data_size,unsigned char * decod_data,int decod_data_size,int bit_offset,int lastbit)
{
	int i,j;
	unsigned char b,c1,c2;

	i = 0;
	b = 0x80;

	bit_offset = bit_offset%input_data_size;
	j = bit_offset>>3;

	do
	{

		c1 = (unsigned char)( input_data[j] & (0x80>>(bit_offset&7)) );
		bit_offset = (bit_offset+1)%input_data_size;
		j = bit_offset>>3;

		c2 = (unsigned char)( input_data[j] & (0x80>>(bit_offset&7)) );
		bit_offset = (bit_offset+1)%input_data_size;
		j = bit_offset>>3;

		if( !c1 && c2 )
			decod_data[i] = (unsigned char)( decod_data[i] | b );
		else
			decod_data[i] = (unsigned char)( decod_data[i] & ~b );

		b = (unsigned char)( b>>1 );
		if(!b)
		{
			b=0x80;
			i++;
		}

	}while(i<decod_data_size);

	return bit_offset;
}

int getbit(unsigned char * input_data,int bit_offset)
{
	return ( ( input_data[bit_offset>>3] >> ( 0x7 - (bit_offset&0x7) ) ) ) & 0x01;
}

int getbit_inv(unsigned char * input_data,int bit_offset)
{
	return ( ( input_data[bit_offset>>3] >> ( (bit_offset&0x7) ) ) ) & 0x01;
}

void setbit(unsigned char * input_data,int bit_offset,int state)
{
	if(state)
	{
		input_data[bit_offset>>3] = (unsigned char)( input_data[bit_offset>>3] |  (0x80 >> ( bit_offset&0x7 ) ) );
	}
	else
	{
		input_data[bit_offset>>3] = (unsigned char)( input_data[bit_offset>>3] & ~(0x80 >> ( bit_offset&0x7 ) ) );
	}

	return;
}

void setbit_inv(unsigned char * input_data,int bit_offset,int state)
{
	if(state)
	{
		input_data[bit_offset>>3] = (unsigned char)( input_data[bit_offset>>3] |  (0x01 << ( bit_offset&0x7 ) ) );
	}
	else
	{
		input_data[bit_offset>>3] = (unsigned char)( input_data[bit_offset>>3] & ~(0x01 << ( bit_offset&0x7 ) ) );
	}

	return;
}

void setfieldbit(unsigned char * dstbuffer,unsigned char byte,int bitoffset,int size)
{
	int i,j;

	i = bitoffset;

	for(j=0;j<size;j++)
	{
		if(byte&((0x80)>>(j&7)))
			dstbuffer[i>>3] = (unsigned char)( dstbuffer[i>>3] | ( (0x80>>(i&7))) );
		else
			dstbuffer[i>>3] = (unsigned char)( dstbuffer[i>>3] & (~(0x80>>(i&7))) );

		i++;
	}
}

int chgbitptr(int tracklen,int cur_offset,int offset)
{
	if( offset>=0 )
	{
		return (cur_offset + offset) % tracklen;
	}
	else
	{
		if(cur_offset >= -offset)
		{
			return cur_offset + offset;
		}
		else
		{
			return (tracklen - ( ((-offset) - cur_offset) ) );
		}
	}
}

int slowSearchBitStream(unsigned char * input_data,uint32_t input_data_size,int searchlen,unsigned char * chr_data,uint32_t chr_data_size,uint32_t bit_offset)
{
	uint32_t cur_startoffset;
	uint32_t i;
	int tracksearchlen;
	int len;

	cur_startoffset = bit_offset;
	len = 0;

	if(searchlen<=0)
	{
		tracksearchlen = input_data_size;
	}
	else
	{
		tracksearchlen = searchlen;
	}

	while( ( cur_startoffset < input_data_size ) && ( len < tracksearchlen ) )
	{
		i=0;
		while( ( i < chr_data_size) && ( ( getbit(input_data,( (cur_startoffset + i) % input_data_size)) == getbit(chr_data, i % chr_data_size) ) ) )
		{
			i++;
		}

		if(i == chr_data_size)
		{
			return cur_startoffset;
		}

		cur_startoffset++;
		len++;

	}

	// End of track passed ?
	if( (searchlen>=0) && (cur_startoffset == input_data_size ) && ( len < tracksearchlen ) )
	{
		cur_startoffset = 0;
		while( ( cur_startoffset < input_data_size ) && ( len < tracksearchlen ) )
		{
			i=0;
			while( ( i < chr_data_size) && ( ( getbit(input_data,( (cur_startoffset + i) % input_data_size)) == getbit(chr_data, i % chr_data_size) ) ) )
			{
				i++;
			}

			if(i == chr_data_size)
			{
				return cur_startoffset;
			}

			cur_startoffset++;
			len++;

		}
	}

	return -1;
}

int searchBitStream(unsigned char * input_data,uint32_t input_data_size,int searchlen,unsigned char * chr_data,uint32_t chr_data_size,uint32_t bit_offset)
{
	uint32_t i,j,trackoffset,cnt,starti;
	unsigned char stringtosearch[8][128];
	unsigned char prev;
	uint32_t tracksize;
	int searchsize;
	int t;
	int bitoffset;

	cnt=(chr_data_size>>3);
	if(chr_data_size&7)
		cnt++;

	// Prepare strings & mask ( input string shifted 7 times...)
	for(i=0;i<8;i++)
	{
		prev=0;
		for(j=0;j<cnt;j++)
		{
			stringtosearch[i][j]= (unsigned char)(prev | (chr_data[j]>>i));
			prev = (unsigned char)(chr_data[j] << (8-i));
		}
		stringtosearch[i][j]=prev;
	}

	starti = bit_offset & 7;
	trackoffset = bit_offset >> 3;

	tracksize = input_data_size >> 3;
	if( input_data_size & 7 ) tracksize++;

	tracksize= tracksize - ( chr_data_size >> 3 );
	if( chr_data_size & 7 ) tracksize--;

	if(searchlen>0)
	{
		searchsize = searchlen >> 3;
		if( searchlen & 7 ) searchsize++;
	}
	else
	{
		searchsize = tracksize;
	}

	t=0;
	// Scan the track data...
	while( ((trackoffset+(cnt+1))<tracksize) && (t<searchsize) )
	{
		for(i=starti;i<8;i++)
		{
			j=1;
			while( ( j < cnt ) && !( stringtosearch[i][j] ^ input_data[trackoffset + j] ) )
			{
				j++;
			}

			if( j == cnt )
			{	// found!
				if( !( ( stringtosearch[i][0] ^ input_data[trackoffset] ) & (0xFF>>i) ) )
				{
					if( !( ( stringtosearch[i][j] ^ input_data[trackoffset + j] ) & (0xFF<<(8-i)) ) )
					{
						return ( trackoffset << 3 ) + i;
					}
				}
			}
		}

		trackoffset++;
		t++;

		starti=0;
	}

	if(t<searchsize)
	{
		if(searchlen>0)
		{
			if(searchlen - (t*8) > 0)
			{
				bitoffset = slowSearchBitStream(input_data,input_data_size,searchlen - (t*8),chr_data,chr_data_size,trackoffset<<3 | starti);
			}
			else
			{
				bitoffset = -1;
			}
		}
		else
		{
			bitoffset = slowSearchBitStream(input_data,input_data_size,searchlen,chr_data,chr_data_size,trackoffset<<3 | starti);
		}
	}
	else
	{
		bitoffset = -1;
	}

	return bitoffset;
}


int bintomfm(unsigned char * track_data,int track_data_size,unsigned char * bin_data,int bin_data_size,int bit_offset)
{
	int i,lastbit;
	unsigned char b;

	i = 0;
	b = 0x80;

	bit_offset = bit_offset%track_data_size;

	lastbit = 0;
	if(bit_offset)
	{
		if(getbit(track_data,bit_offset-1) )
			lastbit = 1;
	}
	else
	{
		if(getbit(track_data,track_data_size-1) )
			lastbit = 1;
	}

	do
	{
		if(bin_data[i] & b)
		{
			setbit(track_data,bit_offset,0);
			bit_offset = (bit_offset+1)%track_data_size;
			setbit(track_data,bit_offset,1);
			bit_offset = (bit_offset+1)%track_data_size;
			lastbit = 1;
		}
		else
		{
			if(lastbit)
			{
				setbit(track_data,bit_offset,0);
				bit_offset = (bit_offset+1)%track_data_size;
				setbit(track_data,bit_offset,0);
				bit_offset = (bit_offset+1)%track_data_size;
			}
			else
			{
				setbit(track_data,bit_offset,1);
				bit_offset = (bit_offset+1)%track_data_size;
				setbit(track_data,bit_offset,0);
				bit_offset = (bit_offset+1)%track_data_size;
			}
			lastbit = 0;
		}

		b = (unsigned char)( b >> 1 );
		if(!b)
		{
			b = 0x80;
			i++;
		}

	}while(i<bin_data_size);

	return bit_offset;
}
