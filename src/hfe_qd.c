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

#include "hfe_qd.h"

#define CPUFREQ 72000000

unsigned int time_to_bitofs(unsigned int cellseconds, unsigned int time)
{
	return  (unsigned int)(((float)cellseconds) * (float)((float)time/(float)1000));
}

// cellseconds -> cell/s
// tracklen -> ms
// start_sw_pos -> ms
// ready_len -> ms

unsigned char * generate_hfe_qd(unsigned int cellseconds, unsigned int tracklen, unsigned int start_sw_pos, unsigned int ready_len, unsigned int *filesize)
{
	unsigned int file_len;
	unsigned int realbitrate;
	unsigned int tracksize,realtracksize;
	unsigned int start_sw,real_start_sw_pos;
	unsigned int rdy_len, real_ready_len;
	unsigned char * outbuf;
	qdhfefileformatheader * header_ptr;
	qdtrack * track_ptr;

	realbitrate = ((CPUFREQ/2) / ((CPUFREQ/2) / cellseconds));

 	printf("generate_hfe_qd : Asked cellsrate -> %d cells/s, Real cellsrate -> %d cells/s\n",cellseconds,realbitrate);

	tracksize = time_to_bitofs(realbitrate,tracklen)/8;

	if(tracksize & 0x1FF)
		realtracksize = (tracksize & ~0x1FF) + 0x200;
	else
		realtracksize = tracksize & ~0x1FF;

 	printf("generate_hfe_qd : Asked tracklen -> %d ms, Real tracklen %f ms (0x%.8X Bytes)\n",tracklen,(float)(realtracksize*8*1000) / (float)realbitrate,realtracksize );

	start_sw = time_to_bitofs(realbitrate,start_sw_pos)/8;

	if(start_sw & 0x1FF)
		real_start_sw_pos = (start_sw & ~0x1FF) + 0x200;
	else
		real_start_sw_pos = start_sw & ~0x1FF;

 	printf("generate_hfe_qd : Asked start sw pos -> %d ms, Real start sw pos %f ms (Track Offset 0x%.8X Bytes)\n",start_sw_pos,(float)(real_start_sw_pos*8*1000) / (float)realbitrate,real_start_sw_pos );

	rdy_len = time_to_bitofs(realbitrate,ready_len)/8;

	if(rdy_len & 0x1FF)
		real_ready_len = (rdy_len & ~0x1FF) + 0x200;
	else
		real_ready_len = rdy_len & ~0x1FF;

 	printf("generate_hfe_qd : Asked ready length -> %d ms, Real ready length %f ms (End Track Offset 0x%.8X Bytes)\n",ready_len,(float)(real_ready_len*8*1000) / (float)realbitrate,real_ready_len + real_start_sw_pos );

	file_len = 512 /*header*/ + 512 /*track list*/ +  realtracksize;

	printf("generate_hfe_qd : Total final file length : 0x%.8X bytes\n",file_len);

	*filesize = 0;

	outbuf = malloc(file_len);
	if(outbuf)
	{
		memset(outbuf,0x01,file_len);
		memset(outbuf,0x00,512*2);
		header_ptr = (qdhfefileformatheader *)outbuf;

		memcpy(header_ptr->HEADERSIGNATURE,"HXCQDDRV",8);
		header_ptr->formatrevision =  0x00000000;
		header_ptr->number_of_track = 0x00000001;
		header_ptr->number_of_side  = 0x00000001;
		header_ptr->track_encoding  = 0x00000000;
		header_ptr->write_protected = 0x00000000;
		header_ptr->bitRate = realbitrate;
		header_ptr->flags = 0x00000000;
		header_ptr->track_list_offset = 0x200;

		track_ptr = (qdtrack *)&outbuf[header_ptr->track_list_offset];

		track_ptr->offset = 0x400;
		track_ptr->track_len = realtracksize;
		track_ptr->start_sw_position = real_start_sw_pos;
		track_ptr->stop_sw_position = real_start_sw_pos + real_ready_len;

		*filesize = file_len;

	 	printf("generate_hfe_qd : Track offset : 0x%.8X, start sw file offset: 0x%.8X, stop sw file offset: 0x%.8X  )\n", track_ptr->offset,track_ptr->start_sw_position + track_ptr->offset, track_ptr->stop_sw_position + track_ptr->offset);
	}

	return outbuf;
}
