/*
    lib/calib.c
    functions for setting calibration

    Copyright (C) 2003 Frank Mori Hess <fmhess@users.sourceforge.net

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, version 2.1
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA.
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <comedilib.h>
#include <libinternal.h>

static int ph_extract_element( const char *file_path, const char *element,
	char *result, unsigned int result_size )
{
	char perl_prog[ 1024 ];
	FILE *perl_stdout;
	int retval;

	snprintf( perl_prog, sizeof( perl_prog ),
		"perl -e '"
		"use strict;"
		"use warnings;"
		"my $hash;"
		"my $cal;"
		"$hash = `cat %s`;"
		"eval \"\\$cal = $hash;\";"
		"print $cal->%s;"
		"'",
		file_path, element );

	perl_stdout = popen( perl_prog, "r");
	if( perl_stdout == NULL )
	{
		fprintf( stderr, "popen() failed in ph_extract_element()\n" );
		return -1;
	}

	if( fgets( result, result_size, perl_stdout ) == NULL )
	{
		fprintf( stderr, "fgets() returned NULL in ph_extract_element()\n" );
		return -1;
	}

	retval = pclose( perl_stdout );
	if( retval )
	{
		fprintf( stderr, "perl returned error %i\n in ph_extract_element()", retval );
		return -1;
	}

	return 0;
}

//EXPORT_SYMBOL(comedi_set_calibration,0.7.20);
int comedi_set_calibration( comedi_t *dev, const char *cal_file_path,
	unsigned int subdev, unsigned int channel, unsigned int range, unsigned int aref )
{
	struct stat file_stats;
	FILE *cal_file;

	if( cal_file_path )
		cal_file = fopen( cal_file_path, "r" );
	else
	{
		char *file_path;

		if( fstat( comedi_fileno( dev ), &file_stats ) < 0 )
		{
			fprintf( stderr, "failed to get file stats of comedi device file\n" );
			return -1;
		}

		asprintf( &file_path, "/etc/comedi/calibrations/%s_0x%lx",
			comedi_get_board_name( dev ),
			( unsigned long ) file_stats.st_ino );
		cal_file = fopen( file_path, "r" );
		free( file_path );
	}

	if( cal_file == NULL )
	{
		fprintf( stderr, "failed to open calibration file\n" );
		return -1;
	}

	return 0;
}
