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
#include <EXTERN.h>
#include <perl.h>

static int extract_ph_string( PerlInterpreter *my_perl, const char *perl_statement,
	char *result, unsigned int result_size )
{
	SV *perl_retval;
	STRLEN len;

	perl_retval = eval_pv( perl_statement, FALSE );
	strncpy( result, SvPV( perl_retval, len ), result_size );
	return 0;
}

static int extract_ph_integer( PerlInterpreter *my_perl, const char *perl_statement )
{
	SV *perl_retval;
	int result;

	perl_retval = eval_pv( perl_statement, FALSE );
	result = SvIV( perl_retval );
	return result;
}

static int check_cal_file( comedi_t *dev, PerlInterpreter *my_perl )
{
	char result[ 100 ];
	int retval;

	retval = extract_ph_string( my_perl, "$cal->{driver_name};",
		result, sizeof( result ) );
	if( retval < 0 ) return retval;

	if( strcmp( comedi_get_driver_name( dev ), result ) )
	{
		fprintf( stderr, "driver name does not match calibration file\n" );
		return -1;
	}

	retval = extract_ph_string( my_perl, "$cal->{board_name};",
		result, sizeof( result ) );
	if( retval < 0 ) return retval;

	if( strcmp( comedi_get_board_name( dev ), result ) )
	{
		fprintf( stderr, "board name does not match calibration file\n" );
		return -1;
	}

	return 0;
}

static inline int num_calibrations( PerlInterpreter *my_perl )
{
	return extract_ph_integer( my_perl, "scalar( @{$cal->{calibrations}} );" );
}

static int extract_array_element( PerlInterpreter *my_perl, unsigned int cal_index,
	const char *array_name, unsigned int array_index )
{
	char element[ 100 ];

	snprintf( element, sizeof( element ),
		"$cal->{ calibrations }[ %i ]->{ %s }[ %i ];", cal_index, array_name, array_index );
	return extract_ph_integer( my_perl, element );
}

static int extract_array_length( PerlInterpreter *my_perl, unsigned int cal_index,
	const char *array_name )
{
	char element[ 100 ];

	snprintf( element, sizeof( element ),
		"scalar( @{ $cal->{ calibrations }[ %i ]->{ %s } } );", cal_index, array_name );
	return extract_ph_integer( my_perl, element );
}

static int extract_subdevice( PerlInterpreter *my_perl, unsigned int cal_index )
{
	char element[ 100 ];

	snprintf( element, sizeof( element ),
		"$cal->{ calibrations }[ %i ]->{ subdevice };", cal_index );
	return extract_ph_integer( my_perl, element );
}

static int valid_item( PerlInterpreter *my_perl, unsigned int cal_index,
	const char *item_type, unsigned int item )
{
	int num_items, i;

	num_items = extract_array_length( my_perl, cal_index, item_type );
	if( num_items < 0 ) return 0;
	if( num_items == 0 ) return 1;
	for( i = 0; i < num_items; i++ )
	{
		if( extract_array_element( my_perl, cal_index, item_type, i ) == item )
			return 1;
	}

	return 0;
}

static inline int valid_range( PerlInterpreter *my_perl, unsigned int cal_index,
	unsigned int range )
{
	return valid_item( my_perl, cal_index, "ranges", range );
}

static inline int valid_channel( PerlInterpreter *my_perl, unsigned int cal_index,
	unsigned int channel )
{
	return valid_item( my_perl, cal_index, "channels", channel );
}

static inline int valid_aref( PerlInterpreter *my_perl, unsigned int cal_index,
	unsigned int aref )
{
	return valid_item( my_perl, cal_index, "arefs", aref );
}

static int find_calibration( PerlInterpreter *my_perl, unsigned int subdev,
	unsigned int channel, unsigned int range, unsigned int aref )
{
	int num_cals, i;

	num_cals = num_calibrations( my_perl );
	if( num_cals < 0 ) return num_cals;

	for( i = 0; i < num_cals; i++ )
	{
		if( extract_subdevice( my_perl, i ) != subdev ) continue;
		if( valid_range( my_perl, i, range ) == 0 ) continue;
		if( valid_channel( my_perl, i, channel ) == 0 ) continue;
		if( valid_aref( my_perl, i, aref ) == 0 ) continue;
		break;
	}
	if( i == num_cals ) return -1;

	return i;
}

static int set_calibration( comedi_t *dev, PerlInterpreter *my_perl,
	unsigned int cal_index )
{
	int i, retval, num_caldacs;

	num_caldacs = extract_array_length( my_perl, cal_index, "caldacs" );
	if( num_caldacs < 0 ) return num_caldacs;

	for( i = 0; i < num_caldacs; i++ )
	{
		int subdev, channel, value;
		char *element;

		asprintf( &element, "$cal->{calibrations}[ %i ]->{caldacs}[ %i ]->{subdevice};",
			cal_index, i );
		subdev = extract_ph_integer( my_perl, element );
		free( element );
		if( subdev < 0 )
		{
			fprintf( stderr, "failed to extract subdev\n" );
			return subdev;
		}

		asprintf( &element, "$cal->{calibrations}[ %i ]->{caldacs}[ %i ]->{channel};",
			cal_index, i );
		channel = extract_ph_integer( my_perl, element );
		free( element );
		if( channel < 0 )
		{
			fprintf( stderr, "failed to extract channel\n" );
			return channel;
		}

		asprintf( &element, "$cal->{calibrations}[ %i ]->{caldacs}[ %i ]->{value};",
			cal_index, i );
		value = extract_ph_integer( my_perl, element );
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

static PerlInterpreter* alloc_my_perl( void )
{
	PerlInterpreter *my_perl;
	char *embedding[] = { "", "-e", "0" };

	my_perl = perl_alloc();
	if( my_perl == NULL )
	{
		fprintf( stderr, "failed to alloc perl interpreter\n");
		return my_perl;
	}
	perl_construct( my_perl );
	perl_parse(my_perl, NULL, 3, embedding, NULL);

	return my_perl;
}

static int startup_my_perl( PerlInterpreter *my_perl, const char *file_path )
{
	int retval;
	char perl_prog[ 1024 ];

	snprintf( perl_prog, sizeof( perl_prog ),
		"
		my $hash = `cat '%s'`;
		eval \"\\$cal = $hash;\";
		", file_path );

	retval = perl_run( my_perl );
	if( retval )
	{
		fprintf( stderr, "nonzero exit from perl_run\n");
		return -1;
	}
	eval_pv( perl_prog, FALSE );

	return 0;
}

static void cleanup_my_perl( PerlInterpreter *my_perl )
{
	perl_destruct( my_perl );
	perl_free( my_perl );
}

EXPORT_SYMBOL(comedi_apply_calibration,0.7.20);
int comedi_apply_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const char *cal_file_path )
{
	struct stat file_stats;
	char file_path[ 1024 ];
	int retval;
	int cal_index;
	PerlInterpreter *my_perl;

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

	my_perl = alloc_my_perl();
	if( my_perl == NULL )
		return -1;

	retval = startup_my_perl( my_perl, file_path );
	if( retval < 0 )
	{
		cleanup_my_perl( my_perl );
		return retval;
	}

	retval = check_cal_file( dev, my_perl );
	if( retval < 0 )
	{
		cleanup_my_perl( my_perl );
		return retval;
	}

	cal_index = find_calibration( my_perl, subdev, channel, range, aref );
	if( cal_index < 0 )
	{
		cleanup_my_perl( my_perl );
		return cal_index;
	}

	retval = set_calibration( dev, my_perl, cal_index );
	if( retval < 0 );
	{
		cleanup_my_perl( my_perl );
		return retval;
	}

	return 0;
}
