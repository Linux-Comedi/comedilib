/***************************************************************************
	cal_common.c  -  shared calibration routines
                             -------------------

    begin                : Fri May 2, 2003
    copyright            : (C) 2003 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "calib.h"
#include <assert.h>
#include <stdlib.h>

void generic_do_cal( calibration_setup_t *setup,
	saved_calibration_t *saved_cal, int observable, int caldac )
{
	if( caldac < 0 || observable < 0 ) return;

	cal_binary( setup, observable, caldac );
	sc_push_caldac( saved_cal, setup->caldacs[ caldac ] );
}

void generic_do_relative( calibration_setup_t *setup,
	saved_calibration_t *saved_cal, int observable1, int observable2, int caldac )
{
	if( caldac < 0 || observable1 < 0 || observable2 < 0 ) return;

	cal_relative_binary( setup, observable1, observable2, caldac );
	sc_push_caldac( saved_cal, setup->caldacs[ caldac ] );
}

void generic_do_linearity( calibration_setup_t *setup,
	saved_calibration_t *saved_cal, int observable1, int observable2,
	int observable3, int caldac )
{
	if( caldac < 0 || observable1 < 0 || observable2 < 0 || observable3 < 0 )
		return;

	cal_linearity_binary( setup, observable1, observable2, observable3, caldac );
	sc_push_caldac( saved_cal, setup->caldacs[ caldac ] );
}

void generic_prep_adc_caldacs( calibration_setup_t *setup,
	const generic_layout_t *layout, unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->ad_subdev < 0 ) return;

	if( setup->do_reset )
	{
		reset_caldac( setup, layout->adc_offset( channel ) );
		reset_caldac( setup, layout->adc_gain( channel ) );
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->ad_subdev,
			channel, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			DPRINT( 0, "Failed to apply existing calibration, reseting dac caldacs.\n" );
			reset_caldac( setup, layout->adc_offset( channel ) );
			reset_caldac( setup, layout->adc_gain( channel ) );
		}
	}
}

void generic_prep_dac_caldacs( calibration_setup_t *setup,
	const generic_layout_t *layout, unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->da_subdev < 0 ) return;

	if( setup->do_reset )
	{
		reset_caldac( setup, layout->dac_offset( channel ) );
		reset_caldac( setup, layout->dac_gain( channel ) );
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->da_subdev,
			channel, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			DPRINT( 0, "Failed to apply existing calibration, reseting dac caldacs.\n" );
			reset_caldac( setup, layout->dac_offset( channel ) );
			reset_caldac( setup, layout->dac_gain( channel ) );
		}
	}
}

int generic_cal_by_channel_and_range( calibration_setup_t *setup,
	const generic_layout_t *layout  )
{
	int range, channel, num_ai_ranges, num_ai_channels, num_ao_ranges,
		num_ao_channels, retval, num_calibrations, i;
	saved_calibration_t *saved_cals, *current_cal;

	assert( comedi_range_is_chan_specific( setup->dev, setup->ad_subdev ) == 0 );

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 ) return -1;

	num_ai_channels = comedi_get_n_channels( setup->dev, setup->ad_subdev );
	if( num_ai_channels < 0 ) return -1;

	if( setup->da_subdev && setup->do_output )
	{
		assert( comedi_range_is_chan_specific( setup->dev, setup->da_subdev ) == 0 );

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		num_ao_channels = comedi_get_n_channels( setup->dev, setup->da_subdev );
		if( num_ao_channels < 0 ) return -1;
	}else
		num_ao_ranges = num_ao_channels = 0;

	num_calibrations = num_ai_ranges * num_ai_channels + num_ao_ranges * num_ao_channels;

	saved_cals = malloc( num_calibrations * sizeof( saved_calibration_t ) );
	if( saved_cals == NULL ) return -1;

	current_cal = saved_cals;

	for( channel = 0; channel < num_ai_channels; channel++ )
	{
		for( range = 0; range < num_ai_ranges; range++ )
		{
			generic_prep_adc_caldacs( setup, layout, channel, range );

			generic_do_cal( setup, current_cal, layout->adc_ground_observable( channel, range ),
				layout->adc_offset( channel ) );

			generic_do_cal( setup, current_cal, layout->adc_high_observable( channel, range ),
				layout->adc_gain( channel ) );

			current_cal->subdevice = setup->ad_subdev;
			sc_push_caldac( current_cal, setup->caldacs[ layout->adc_gain( channel ) ] );
			sc_push_caldac( current_cal, setup->caldacs[ layout->adc_offset( channel ) ] );
			sc_push_channel( current_cal, channel );
			sc_push_range( current_cal, range );
			sc_push_aref( current_cal, SC_ALL_AREFS );

			current_cal++;
		}
	}
	for( channel = 0; channel < num_ao_channels; channel++ )
	{
		for( range = 0; range < num_ao_ranges; range++ )
		{
			generic_prep_dac_caldacs( setup, layout, channel, range );

			generic_do_cal( setup, current_cal, layout->dac_ground_observable( channel, range ),
				layout->dac_offset( channel ) );

			generic_do_cal( setup, current_cal, layout->dac_high_observable( channel, range ),
				layout->dac_gain( channel ) );

			current_cal->subdevice = setup->da_subdev;
			sc_push_caldac( current_cal, setup->caldacs[ layout->dac_gain( channel ) ] );
			sc_push_caldac( current_cal, setup->caldacs[ layout->dac_offset( channel ) ] );
			sc_push_channel( current_cal, channel );
			sc_push_range( current_cal, range );
			sc_push_aref( current_cal, SC_ALL_AREFS );

			current_cal++;
		}
	}

	retval = write_calibration_file( setup, saved_cals, num_calibrations );
	for( i = 0; i < num_calibrations; i++ )
		clear_saved_calibration( &saved_cals[ i ] );
	free( saved_cals );
	return retval;
}

static int dummy_caldac( unsigned int channel )
{
	return -1;
}
static int dummy_observable( unsigned int channel, unsigned int range )
{
	return -1;
}
void init_generic_layout( generic_layout_t *layout )
{
	layout->adc_offset = dummy_caldac;
	layout->adc_gain = dummy_caldac;
	layout->dac_offset = dummy_caldac;
	layout->dac_gain = dummy_caldac;
	layout->adc_high_observable = dummy_observable;
	layout->adc_ground_observable = dummy_observable;
	layout->dac_high_observable = dummy_observable;
	layout->dac_ground_observable = dummy_observable;
}
