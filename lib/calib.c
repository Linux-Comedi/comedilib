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
#include <libinternal.h>

static int extract_ph_string( const char *file_path, const char *hash_ref,
	const char *element, char *result, unsigned int result_size )
{
	char perl_prog[ 1024 ];
	FILE *perl_stdout;
	int retval;

	snprintf( perl_prog, sizeof( perl_prog ),
		"perl -e '
		use strict;
		use warnings;
		my $hash;
		my $%s;
		$hash = `cat %s`;
		eval \"\\$%s = $hash;\";
		print %s;
		'",
		hash_ref, file_path, hash_ref, element );

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

static int extract_ph_integer( const char *file_path, const char *hash_ref,
	const char *element )
{
	char result[ 100 ];
	int retval;

	retval = extract_ph_string( file_path, hash_ref, element, result, sizeof( result ) );
	if( retval < 0 ) return retval;

	return strtol( result, NULL, 0 );
}

static int check_cal_file( comedi_t *dev, const char *file_path )
{
	char result[ 100 ];
	int retval;

	retval = extract_ph_string( file_path, "cal", "cal->{driver_name}",
		result, sizeof( result ) );
	if( retval < 0 ) return retval;

	if( strcmp( comedi_get_driver_name( dev ), result ) )
	{
		fprintf( stderr, "driver name does not match calibration file\n" );
		return -1;
	}

	retval = extract_ph_string( file_path, "cal", "cal->{board_name}",
		result, sizeof( result ) );
	if( retval < 0 ) return retval;

	if( strcmp( comedi_get_board_name( dev ), result ) )
	{
		fprintf( stderr, "board name does not match calibration file\n" );
		return -1;
	}

	return 0;
}

static inline int num_calibrations( const char *file_path )
{
	return extract_ph_integer( file_path, "cal", "scalar( @{$cal->{calibrations}} )" );
}

static int extract_array_element( const char *file_path, unsigned int cal_index,
	const char *array_name, unsigned int array_index )
{
	char element[ 100 ];

	snprintf( element, sizeof( element ),
		"cal->{ calibrations }[ %i ]->{ %s }[ %i ]", cal_index, array_name, array_index );
	return extract_ph_integer( file_path, "cal", element );
}

static int extract_array_length( const char *file_path, unsigned int cal_index,
	const char *array_name )
{
	char element[ 100 ];

	snprintf( element, sizeof( element ),
		"scalar( @{ cal->{ calibrations }[ %i ]->{ %s } } )", cal_index, array_name );
	return extract_ph_integer( file_path, "cal", element );
}

static int extract_subdevice( const char *file_path, unsigned int cal_index )
{
	char element[ 100 ];

	snprintf( element, sizeof( element ),
		"cal->{ calibrations }[ %i ]->{ subdevice }", cal_index );
	return extract_ph_integer( file_path, "cal", element );
}

static int valid_item( const char *file_path, unsigned int cal_index,
	const char *item_type, unsigned int item )
{
	int num_items, i;

	num_items = extract_array_length( file_path, cal_index, item_type );
	if( num_items < 0 ) return 0;
	if( num_items == 0 ) return 1;
	for( i = 0; i < num_items; i++ )
	{
		if( extract_array_element( file_path, cal_index, item_type, i ) == item )
			return 1;
	}

	return 0;
}

static inline int valid_range( const char *file_path, unsigned int cal_index,
	unsigned int range )
{
	return valid_item( file_path, cal_index, "ranges", range );
}

static inline int valid_channel( const char *file_path, unsigned int cal_index,
	unsigned int channel )
{
	return valid_item( file_path, cal_index, "channels", channel );
}

static inline int valid_aref( const char *file_path, unsigned int cal_index,
	unsigned int aref )
{
	return valid_item( file_path, cal_index, "arefs", aref );
}

static int find_calibration( const char *file_path, unsigned int subdev,
	unsigned int channel, unsigned int range, unsigned int aref )
{
	int num_cals, i;

	num_cals = num_calibrations( file_path );
	if( num_cals < 0 ) return num_cals;

	for( i = 0; i < num_cals; i++ )
	{
		if( extract_subdevice( file_path, i ) != subdev ) continue;
		if( valid_range( file_path, i, range ) == 0 ) continue;
		if( valid_channel( file_path, i, channel ) == 0 ) continue;
		if( valid_aref( file_path, i, aref ) == 0 ) continue;
		break;
	}
	if( i == num_cals ) return -1;

	return i;
}

static int set_calibration( comedi_t *dev, const char *file_path,
	unsigned int cal_index )
{
	int i, retval, num_caldacs;

	num_caldacs = extract_array_length( file_path, cal_index, "caldacs" );
	if( num_caldacs < 0 ) return num_caldacs;

	for( i = 0; i < num_caldacs; i++ )
	{
		int subdev, channel, value;
		char *element;

		asprintf( &element, "cal->{calibrations}[ %i ]->{caldacs}[ %i ]->{subdev}",
			cal_index, i );
		subdev = extract_ph_integer( file_path, "cal", element );
		free( element );
		if( subdev < 0 )
		{
			fprintf( stderr, "failed to extract subdev\n" );
			return subdev;
		}

		asprintf( &element, "cal->{calibrations}[ %i ]->{caldacs}[ %i ]->{channel}",
			cal_index, i );
		channel = extract_ph_integer( file_path, "cal", element );
		free( element );
		if( channel < 0 )
		{
			fprintf( stderr, "failed to extract channel\n" );
			return channel;
		}

		asprintf( &element, "cal->{calibrations}[ %i ]->{caldacs}[ %i ]->{value}",
			cal_index, i );
		value = extract_ph_integer( file_path, "cal", element );
		free( element );
		if( value < 0 )
		{
			fprintf( stderr, "failed to extract value\n" );
			return value;
		}

		retval = comedi_data_write( dev, subdev, channel, 0, 0, value );
		if( retval < 0 ) return retval;
	}
	
	return 0;
}

EXPORT_SYMBOL(comedi_set_calibration,0.7.20);
int comedi_set_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const char *cal_file_path )
{
	struct stat file_stats;
	char file_path[ 1024 ];
	int retval;
	int cal_index;

	if( cal_file_path )
	{
		strncpy( file_path, cal_file_path, sizeof( file_path ) );
	}else
	{
		if( fstat( comedi_fileno( dev ), &file_stats ) < 0 )
		{
			fprintf( stderr, "failed to get file stats of comedi device file\n" );
			return -1;
		}

		snprintf( file_path, sizeof( file_path ), "/etc/comedi/calibrations/%s_0x%lx",
			comedi_get_board_name( dev ),
			( unsigned long ) file_stats.st_ino );
	}

	retval = check_cal_file( dev, file_path );
	if( retval < 0 ) return retval;

	cal_index = find_calibration( file_path, subdev, channel, range, aref );
	retval = set_calibration( dev, file_path, cal_index );
	if( retval < 0 ) return retval;

	return 0;
}
