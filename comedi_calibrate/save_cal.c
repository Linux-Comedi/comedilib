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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <comedilib.h>

#include "calib.h"

#define CAL_MAX_CHANNELS_LENGTH 1024
#define CAL_MAX_RANGES_LENGTH 128
#define CAL_MAX_AREFS_LENGTH 16

struct calibration_setting
{
	unsigned int subdevice;
	caldac_t caldacs[ N_CALDACS ];
	unsigned int caldacs_length;
	int channels[ CAL_MAX_CHANNELS_LENGTH ]; /* channels that caldac settings are restricted to */
	unsigned int channels_length; /* number of elements in channels array, 0 means allow all channels */
	int ranges[ CAL_MAX_RANGES_LENGTH ]; /* ranges that caldac settings are restricted to */
	unsigned int ranges_length;/* number of elements in ranges array, 0 means allow all ranges */
	int arefs[ CAL_MAX_AREFS_LENGTH ]; /* arefs that caldac settings are used restricted to */
	unsigned int arefs_length; /* number of elements in arefs array, 0 means allow any aref */
};

int get_inode( comedi_t *dev, ino_t *inode )
{
	struct stat file_stats;
	int retval;

	retval = fstat( comedi_fileno( dev ), &file_stats );
	if( retval < 0 )
		return -1;

	*inode = file_stats.st_ino;
	return 0;
}

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

void write_calibration_setting( FILE *file, struct calibration_setting setting )
{
	static const char *indent = "\t\t";
	int i;

	fprintf( file, "%s", indent );
	fprintf( file, "{\n" );
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
	fprintf( file, "\tcaldacs => [\n" );
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

int write_calibration_file( FILE *file, comedi_t *dev,
	struct calibration_setting settings[], unsigned int num_settings )
{
	ino_t inode;
	int retval;
	int i;

	retval = get_inode( dev, &inode );
	if( retval < 0 ) return retval;

	fprintf( file, "{\n" );
	fprintf( file, "\tboard_name => \"%s\",\n", comedi_get_board_name( dev ) );
	fprintf( file, "\tcalibrations => [\n" );
	for( i = 0; i < num_settings; i++ )
	{
		write_calibration_setting( file, settings[ i ] );
		fprintf( file, ",\n" );
	}
	fprintf( file, "\t],\n"
		"};\n");

	return 0;
}
