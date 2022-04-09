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

#include "utils.h"

#include "hfe_qd.h"

#include "qd_mo5.h"
#include "qd_roland.h"
#include "qd_akai.h"

#include "trk_utils.h"

int verbose;

int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,512);
	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,512);

				j=0;
				i=1;
				while( argv[param][i] && argv[param][i]!=':')
				{
					option[j]=argv[param][i];
					i++;
					j++;
				}

				if( !strcmp(option,paramtosearch) )
				{
					if(argtoparam)
					{
						if(argv[param][i]==':')
						{
							i++;
							j=0;
							while( argv[param][i] )
							{
								argtoparam[j]=argv[param][i];
								i++;
								j++;
							}
							argtoparam[j]=0;
							return 1;
						}
						else
						{
							return -1;
						}
					}
					else
					{
						return 1;
					}
				}
			}
		}
		param++;
	}

	return 0;
}

void printhelp(char* argv[])
{
	printf("Options:\n");
	printf("  -qdtrklen \t\t\t: QD track length (ms)\n");
	printf("  -qdstartsw \t\t\t: QD start switch position (ms)\n");
	printf("  -qdreadylen \t\t\t: QD ready length\n");
	printf("  -qddatadelay \t\t\t: QD ready to data delay\n");
	printf("  -loadraw \t\t\t: Load a raw mfm track\n");
	printf("  -loadmo5qd \t\t\t: Load Thomson MO5 QD file\n");
	printf("  -generate \t\t\t: Generate a HxC QD file\n");
	printf("  -checkmo5qd \t\t\t: Test a MO5 formatted HxC QD file\n");
	printf("  -checkrolandqd \t\t: Test a Roland formatted HxC QD file\n");
	printf("  -checkakaiqd \t\t\t: Test a Akai formatted HxC QD file\n");
	printf("  -help \t\t\t: This help\n");
	printf("\n");
}

int main(int argc, char* argv[])
{
	char filename[512];
	char ofilename[512];
	char tmp[512];

	unsigned int outfilesize;
	unsigned char * outfilebuf;
	unsigned char * inbuffer;
	unsigned int inbuffersize;
	FILE *f1,*f2;
	qdhfefileformatheader * header_ptr;
	qdtrack * track_ptr;

	unsigned int qd_track_len;
	unsigned int qd_cell_rate;
	unsigned int qd_start_sw;
	unsigned int qd_ready_len;
	unsigned int qd_ready_data_delay;
	unsigned int i;
	unsigned int rawld_bitoffset;

	verbose=0;
	inbuffer = NULL;

	printf("HxC Floppy Emulator : QD Floppy image file tool v0.0.2.1\n");
	printf("Copyright (C) 2006-2019 Jean-Francois DEL NERO\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions;\n\n");

	qd_cell_rate = 203128;
	qd_track_len = 8000;
	qd_start_sw = 500;
	qd_ready_len = 4520;
	qd_ready_data_delay = 168;

	rawld_bitoffset = 0;

	// Verbose option...
	if(isOption(argc,argv,"verbose",0)>0)
	{
		printf("verbose mode\n");
		verbose=1;
	}

	// help option...
	if(isOption(argc,argv,"help",0)>0)
	{
		printhelp(argv);
	}

	memset(filename,0,sizeof(filename));

	// Input file name option
	if(isOption(argc,argv,"finput",(char*)&filename)>0)
	{
		printf("Input file : %s\n",filename);
	}

	// Output file name option
	strcpy(ofilename,"DSKA0000.QD");
	isOption(argc,argv,"foutput",(char*)&ofilename);

	if(isOption(argc,argv,"qdtrklen",(char*)&tmp)>0)
	{
		qd_track_len = atoi(tmp);
	}

	if(isOption(argc,argv,"qdrate",(char*)&tmp)>0)
	{
		qd_cell_rate = atoi(tmp);
	}

	if(isOption(argc,argv,"qdstartsw",(char*)&tmp)>0)
	{
		qd_start_sw = atoi(tmp);
	}

	if(isOption(argc,argv,"qdreadylen",(char*)&tmp)>0)
	{
		qd_ready_len = atoi(tmp);
	}

	if(isOption(argc,argv,"qddatadelay",(char*)&tmp)>0)
	{
		qd_ready_data_delay = atoi(tmp);
	}

	if(isOption(argc,argv,"loadmo5qd",(char*)&filename)>0)
	{
		qd_ready_len = 5500;
		inbuffer = load_mo5_trk(filename, &inbuffersize);
	}

	if(isOption(argc,argv,"loadraw",(char*)&filename)>0)
	{
		printf("Load raw input file : %s\n",filename);
		f1 = fopen(filename,"rb");
		if(f1)
		{
			fseek(f1,0,SEEK_END);
			inbuffersize = ftell(f1);
			fseek(f1,0,SEEK_SET);

			inbuffer = malloc(inbuffersize);
			if(inbuffer)
			{
				memset(inbuffer, 0, inbuffersize);
				if(fread(inbuffer,inbuffersize,1,f1) != 1)
				{
					printf("!!!!!!!!! Error while reading %s !!!!!!!!!!\n",filename);
				}
			}
		}
		else
		{
			printf("!!!!!!!!!!! Error while opening %s !!!!!!!!!!!!!\n",filename);
		}
	}

	if(isOption(argc,argv,"raw_ld_bitoffset",(char*)&tmp)>0)
	{
		rawld_bitoffset = atoi(tmp);
	}

	if(isOption(argc,argv,"generate",0)>0)
	{
		printf("qd cells rate : %d cell/s\n",qd_cell_rate);
		printf("qd track length : %d ms\n",qd_track_len);
		printf("qd start switch position : %d ms\n",qd_start_sw);
		printf("qd ready length : %d ms\n",qd_ready_len);
		printf("qd ready - data delay : %d ms\n",qd_ready_data_delay);

		outfilesize = 0;
		outfilebuf = generate_hfe_qd(qd_cell_rate, qd_track_len, qd_start_sw, qd_ready_len,&outfilesize);

		if(outfilebuf && outfilesize)
		{
			header_ptr = (qdhfefileformatheader *)outfilebuf;
			track_ptr = (qdtrack *)&outfilebuf[header_ptr->track_list_offset];

			if( inbuffer )
			{
				printf("Place track data at 0x%.8X - 0x%.8X\n",track_ptr->offset + track_ptr->start_sw_position + (time_to_bitofs(header_ptr->bitRate,qd_ready_data_delay)/8), track_ptr->offset + track_ptr->start_sw_position + inbuffersize + (time_to_bitofs(header_ptr->bitRate,qd_ready_data_delay)/8));
				for(i=0;i<(inbuffersize*8);i++)
				{
					setbit_inv(outfilebuf,((track_ptr->offset + track_ptr->start_sw_position)*8) + time_to_bitofs(header_ptr->bitRate,qd_ready_data_delay) + i + rawld_bitoffset, getbit_inv(inbuffer,i));
				}
				free(inbuffer);
			}

			f2 = fopen(ofilename,"wb");
			if(f2)
			{
				printf("Creating %s...\n", ofilename);
				fwrite(outfilebuf,outfilesize,1,f2);
				fclose(f2);
			}
			else
			{
				printf("!!!!!!!!!!!! Error while creating %s !!!!!!!!!!!!!\n", ofilename);
			}

			free(outfilebuf);
		}
	}
	
	if(isOption(argc,argv,"checkmo5qd",(char*)&filename)>0)
	{
		check_mo5_qd(filename);
	}

	if(isOption(argc,argv,"checkrolandqd",(char*)&filename)>0)
	{
		check_roland_qd(filename);
	}

	if(isOption(argc,argv,"checkakaiqd",(char*)&filename)>0)
	{
		check_akai_qd(filename);
	}

	if( (isOption(argc,argv,"help",0)<=0) &&
		(isOption(argc,argv,"qdtrklen",0)<=0) &&
		(isOption(argc,argv,"qdstartsw",0)<=0) &&
		(isOption(argc,argv,"qdreadylen",0)<=0) &&
		(isOption(argc,argv,"qddatadelay",0)<=0) &&
		(isOption(argc,argv,"loadraw",0)<=0) &&
		(isOption(argc,argv,"loadmo5qd",0)<=0) &&
		(isOption(argc,argv,"checkmo5qd",0)<=0) &&
		(isOption(argc,argv,"checkrolandqd",0)<=0) &&
		(isOption(argc,argv,"checkakaiqd",0)<=0) &&
		(isOption(argc,argv,"generate",0)<=0)
		)
	{
		printhelp(argv);
	}

	return 0;
}


