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

typedef struct qdhfefileformatheader_
{
	unsigned char HEADERSIGNATURE[8]; //HXCQDDRV
	unsigned int formatrevision;
	unsigned int number_of_track; // 1
	unsigned int number_of_side;  // x
	unsigned int track_encoding;
	unsigned int write_protected;
	unsigned int bitRate;   // (cells rate / s)
	unsigned int flags;
	unsigned int track_list_offset;
}__attribute__ ((__packed__)) qdhfefileformatheader;

typedef struct qdtrack_
{
	unsigned int offset;
	unsigned int track_len;
	unsigned int start_sw_position;
	unsigned int stop_sw_position;
}__attribute__ ((__packed__)) qdtrack;


unsigned char * generate_hfe_qd(unsigned int cellseconds, unsigned int tracklen, unsigned int start_sw_pos, unsigned int ready_len, unsigned int *filesize);
unsigned int time_to_bitofs(unsigned int cellseconds, unsigned int time);
