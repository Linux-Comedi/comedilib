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
#include <stdio.h>
#include <string.h>
#include <comedilib.h>
#include "libinternal.h"

static int check_cal_file( comedi_t *dev, struct calibration_file_contents *parsed_file )
{
	if( strcmp( comedi_get_driver_name( dev ), parsed_file->driver_name ) )
	{
		COMEDILIB_DEBUG( 3, "driver name does not match '%s' from calibration file\n",
			parsed_file->driver_name );
		return -1;
	}

	if( strcmp( comedi_get_board_name( dev ), parsed_file->board_name ) )
	{
		COMEDILIB_DEBUG( 3, "board name does not match '%s' from calibration file\n",
			parsed_file->board_name );
		return -1;
	}

	return 0;
}

static inline int valid_channel( struct calibration_file_contents *parsed_file,
	unsigned int cal_index, unsigned int channel )
{
	int num_channels, i;

	num_channels = parsed_file->calibrations[ cal_index ].num_channels;
	if( num_channels == 0 ) return 1;
	for( i = 0; i < num_channels; i++ )
	{
		if( parsed_file->calibrations[ cal_index ].channels[ i ] == channel )
			return 1;
	}

	return 0;
}

static inline int valid_range( struct calibration_file_contents *parsed_file,
	unsigned int cal_index, unsigned int range )
{
	int num_ranges, i;

	num_ranges = parsed_file->calibrations[ cal_index ].num_ranges;
	if( num_ranges == 0 ) return 1;
	for( i = 0; i < num_ranges; i++ )
	{
		if( parsed_file->calibrations[ cal_index ].ranges[ i ] == range )
			return 1;
	}

	return 0;
}

static inline int valid_aref( struct calibration_file_contents *parsed_file,
	unsigned int cal_index, unsigned int aref )
{
	int num_arefs, i;

	num_arefs = parsed_file->calibrations[ cal_index ].num_arefs;
	if( num_arefs == 0 ) return 1;
	for( i = 0; i < num_arefs; i++ )
	{
		if( parsed_file->calibrations[ cal_index ].arefs[ i ] == aref )
			return 1;
	}

	return 0;
}

static int find_calibration( struct calibration_file_contents *parsed_file,
	unsigned int subdev, unsigned int channel, unsigned int range, unsigned int aref )
{
	int num_cals, i;

	num_cals = parsed_file->num_calibrations;

	for( i = 0; i < num_cals; i++ )
	{
		if( parsed_file->calibrations[ i ].subdevice != subdev ) continue;
		if( valid_range( parsed_file, i, range ) == 0 ) continue;
		if( valid_channel( parsed_file, i, channel ) == 0 ) continue;
		if( valid_aref( parsed_file, i, aref ) == 0 ) continue;
		break;
	}
	if( i == num_cals )
	{
		COMEDILIB_DEBUG( 3, "failed to find matching calibration\n" );
		return -1;
	}

	return i;
}

static int set_calibration( comedi_t *dev, struct calibration_file_contents *parsed_file,
	unsigned int cal_index )
{
	int i, retval, num_caldacs;

	num_caldacs = parsed_file->calibrations[ cal_index ].num_caldacs;
	COMEDILIB_DEBUG( 4, "num_caldacs %i\n", num_caldacs );

	for( i = 0; i < num_caldacs; i++ )
	{
		struct caldac_setting caldac;

		caldac = parsed_file->calibrations[ cal_index ].caldacs[ i ];
		COMEDILIB_DEBUG( 4, "subdev %i, ch %i, val %i\n", caldac.subdevice,
			caldac.channel,caldac.value);
		retval = comedi_data_write( dev, caldac.subdevice, caldac.channel,
			0, 0, caldac.value );
		if( retval < 0 ) return retval;
	}

	return 0;
}

EXPORT_SYMBOL(comedi_apply_calibration,0.7.20);
int comedi_apply_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const char *cal_file_path )
{
	struct stat file_stats;
	char file_path[ 1024 ];
	int retval;
	int cal_index;
	FILE *cal_file;
	struct calibration_file_contents *parsed_file;

	if( cal_file_path )
	{
		strncpy( file_path, cal_file_path, sizeof( file_path ) );
	}else
	{
		if( fstat( comedi_fileno( dev ), &file_stats ) < 0 )
		{
			COMEDILIB_DEBUG( 3, "failed to get file stats of comedi device file\n" );
			return -1;
		}

		snprintf( file_path, sizeof( file_path ), "/etc/comedi/calibrations/%s_0x%lx",
			comedi_get_board_name( dev ),
			( unsigned long ) file_stats.st_ino );
	}

	cal_file = fopen( file_path, "r" );
	if( cal_file == NULL )
	{
		COMEDILIB_DEBUG( 3, "failed to open file\n" );
		return -1;
	}

	parsed_file = parse_calibration_file( cal_file );
	if( parsed_file == NULL )
	{
		COMEDILIB_DEBUG( 3, "failed to parse calibration file\n" );
		return -1;
	}

	fclose( cal_file );

	retval = check_cal_file( dev, parsed_file );
	if( retval < 0 )
	{
		cleanup_calibration_parse( parsed_file );
		return retval;
	}

	cal_index = find_calibration( parsed_file, subdev, channel, range, aref );
	if( cal_index < 0 )
	{
		cleanup_calibration_parse( parsed_file );
		return cal_index;
	}

	retval = set_calibration( dev, parsed_file, cal_index );
	if( retval < 0 )
	{
		cleanup_calibration_parse( parsed_file );
		return retval;
	}

	return 0;
}
