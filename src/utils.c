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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void get_filename(char * path,char * filename)
{
	int i,done;

	i=strlen(path);
	done=0;
	while(i && !done)
	{
		i--;

		if(path[i]=='/')
		{
			done=1;
			i++;
		}
	}

	sprintf(filename,"%s",&path[i]);

	i=0;
	while(filename[i])
	{
		if(filename[i]=='.')
		{
			filename[i]='_';
		}

		i++;
	}

	return;
}

int is_printable_char(unsigned char c)
{
	int i;
	unsigned char specialchar[]={"&#{}()|_@=$!?;+*-"};

	if( (c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		(c >= '0' && c <= '9') )
	{
		return 1;
	}

	i = 0;
	while(specialchar[i])
	{
		if(specialchar[i] == c)
		{
			return 1;
		}

		i++;
	}

	return 0;
}

void printbuf(void * buf,int size)
{
	#define PRINTBUF_HEXPERLINE 16
	#define PRINTBUF_MAXLINE_SIZE ((3*PRINTBUF_HEXPERLINE)+1+PRINTBUF_HEXPERLINE+2)

	int i,j;
	unsigned char *ptr = buf;
	char tmp[8];
	char str[PRINTBUF_MAXLINE_SIZE];

	memset(str, ' ', PRINTBUF_MAXLINE_SIZE);
	str[PRINTBUF_MAXLINE_SIZE-1] = 0;

	j = 0;
	for(i=0;i<size;i++)
	{
		if(!(i&(PRINTBUF_HEXPERLINE-1)) && i)
		{
			printf("%s\n", str);
			memset(str, ' ', PRINTBUF_MAXLINE_SIZE);
			str[PRINTBUF_MAXLINE_SIZE-1] = 0;
			j = 0;
		}

		sprintf(tmp, "%02X", ptr[i]);
		memcpy(&str[j*3],tmp,2);

		if( is_printable_char(ptr[i]) )
			str[3*PRINTBUF_HEXPERLINE + 1 + j] = ptr[i];
		else
			str[3*PRINTBUF_HEXPERLINE + 1 + j] = '.';

		j++;
	}

	printf("%s\n", str);
}
