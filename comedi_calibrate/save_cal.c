/*
comedi_calibrate/save_cal.c - stuff for saving calibration info to
a file.

Copyright (C) 2003  Frank Mori Hess <fmhess@users.sourceforge.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
license, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include <comedilib.h>

#include "calib.h"

void write_caldac( FILE *file, caldac_t caldac )
{
	static const char *indent = "\t\t\t\t";

	fprintf( file, "%s", indent );
	fprintf( file, "{\n" );
	fprintf( file, "%s", indent );
	fprintf( file, "\tsubdevice => %i,\n", caldac.subdev );
	fprintf( file, "%s", indent );
	fprintf( file, "\tchannel => %i,\n", caldac.chan );
	fprintf( file, "%s", indent );
	fprintf( file, "\tvalue => %i,\n", caldac.current );
	fprintf( file, "%s", indent );
	fprintf( file, "}" );
}

void write_calibration_setting( FILE *file, saved_calibration_t setting )
{
	static const char *indent = "\t\t";
	int i;

	fprintf( file, "%s", indent );
	fprintf( file, "{\n" );
	fprintf( file, "%s", indent );
	fprintf( file, "\tsubdevice => %i,\n", setting.subdevice );
	fprintf( file, "%s", indent );
	fprintf( file, "\tchannels => [" );
	for( i = 0; i < setting.channels_length; i++ )
		fprintf( file, "%i,", setting.channels[ i ] );
	fprintf( file, "],\n" );
	fprintf( file, "%s", indent );
	fprintf( file, "\tranges => [" );
	for( i = 0; i < setting.ranges_length; i++ )
		fprintf( file, "%i,", setting.ranges[ i ] );
	fprintf( file, "],\n" );
	fprintf( file, "%s", indent );
	fprintf( file, "\tarefs => [" );
	for( i = 0; i < setting.arefs_length; i++ )
		fprintf( file, "%i,", setting.arefs[ i ] );
	fprintf( file, "],\n" );
	fprintf( file, "%s", indent );
	fprintf( file, "\tcaldacs =>\n" );
	fprintf( file, "%s", indent );
	fprintf( file, "\t[\n" );
	for( i = 0; i < setting.caldacs_length; i++ )
	{
		write_caldac( file, setting.caldacs[ i ] );
		fprintf( file, ",\n" );
	}
	fprintf( file, "%s", indent );
	fprintf( file, "\t],\n" );


	fprintf( file, "%s", indent );
	fprintf( file, "}" );
}

int write_calibration_perl_hash( FILE *file, comedi_t *dev,
	saved_calibration_t settings[], unsigned int num_settings )
{
	int i;

	fprintf( file, "{\n" );
	fprintf( file, "\tdriver_name => \"%s\",\n", comedi_get_driver_name( dev ) );
	fprintf( file, "\tboard_name => \"%s\",\n", comedi_get_board_name( dev ) );
	fprintf( file, "\tcalibrations =>\n"
		"\t[\n" );
	for( i = 0; i < num_settings; i++ )
	{
		write_calibration_setting( file, settings[ i ] );
		fprintf( file, ",\n" );
	}
	fprintf( file, "\t],\n"
		"}\n");

	return 0;
}

int write_calibration_file( calibration_setup_t *setup, saved_calibration_t settings[],
	unsigned int num_settings )
{
	FILE *file;
	int retval;
	static const char *save_dir = "/etc/comedi/calibrations";
	char command[ 100 ];
	struct stat file_stats;
	comedi_t *dev = setup->dev;

	if( setup->cal_save_file_path == NULL )
	{
		if( fstat( comedi_fileno( dev ), &file_stats ) < 0 )
		{
			fprintf( stderr, "failed to get file stats of comedi device file\n" );
			return -1;
		}

		snprintf( command, sizeof( command ), "install -d %s", save_dir );
		if( system( command ) )
		{
			fprintf( stderr, "failed to create directory %s\n", save_dir );
			return -1;
		}

		asprintf( &setup->cal_save_file_path, "%s/%s_0x%lx",
			save_dir, comedi_get_board_name( dev ),
			( unsigned long ) file_stats.st_ino );
	}
	file = fopen( setup->cal_save_file_path, "w" );
	if( file == NULL )
	{
		fprintf( stderr, "failed to open file %s for writing\n",
			setup->cal_save_file_path );
		return -1;
	}

	retval = write_calibration_perl_hash( file, dev, settings, num_settings );

	fclose( file );

	return retval;
}

void sc_push_caldac( saved_calibration_t *saved_cal, caldac_t caldac )
{
	saved_cal->caldacs = realloc( saved_cal->caldacs,
		( saved_cal->caldacs_length + 1 ) * sizeof( caldac_t ) );
	if( saved_cal->caldacs == NULL )
	{
		fprintf( stderr, "memory allocation failure\n" );
		abort();
	}
	saved_cal->caldacs[ saved_cal->caldacs_length++ ] = caldac;
}

void sc_push_channel( saved_calibration_t *saved_cal, int channel )
{
	if( channel == SC_ALL_CHANNELS )
	{
		saved_cal->channels_length = 0;
		if( saved_cal->channels )
		{
			free( saved_cal->channels );
			saved_cal->channels = NULL;
		}
	}else
	{
		saved_cal->channels = realloc( saved_cal->channels,
			( saved_cal->channels_length + 1 ) * sizeof( int ) );
		if( saved_cal->channels == NULL )
		{
			fprintf( stderr, "memory allocation failure\n" );
			abort();
		}
		saved_cal->channels[ saved_cal->channels_length++ ] = channel;
	}
}

void sc_push_range( saved_calibration_t *saved_cal, int range )
{
	if( range == SC_ALL_RANGES )
	{
		saved_cal->ranges_length = 0;
		if( saved_cal->ranges )
		{
			free( saved_cal->ranges );
			saved_cal->ranges = NULL;
		}
	}
	else
	{
		saved_cal->ranges = realloc( saved_cal->ranges,
			( saved_cal->ranges_length + 1 ) * sizeof( int ) );
		if( saved_cal->ranges == NULL )
		{
			fprintf( stderr, "memory allocation failure\n" );
			abort();
		}
		saved_cal->ranges[ saved_cal->ranges_length++ ] = range;
	}

}

void sc_push_aref( saved_calibration_t *saved_cal, int aref )
{
	assert( saved_cal->arefs_length < SC_MAX_AREFS_LENGTH );

	if( aref == SC_ALL_AREFS )
		saved_cal->arefs_length = 0;
	else
		saved_cal->arefs[ saved_cal->arefs_length++ ] = aref;
}

void clear_saved_calibration( saved_calibration_t *saved_cal )
{
	if( saved_cal->caldacs )
	{
		free( saved_cal->caldacs );
		saved_cal->caldacs = NULL;
		saved_cal->caldacs_length = 0;
	}
	if( saved_cal->channels )
	{
		free( saved_cal->channels );
		saved_cal->channels = NULL;
		saved_cal->channels_length = 0;
	}
	if( saved_cal->ranges )
	{
		free( saved_cal->ranges );
		saved_cal->ranges = NULL;
		saved_cal->ranges_length = 0;
	}
}
