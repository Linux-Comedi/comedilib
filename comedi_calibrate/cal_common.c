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
		reset_caldac( setup, layout->adc_offset_fine( channel ) );
		reset_caldac( setup, layout->adc_gain_fine( channel ) );
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->ad_subdev,
			channel, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			DPRINT( 0, "Failed to apply existing calibration, reseting dac caldacs.\n" );
			reset_caldac( setup, layout->adc_offset( channel ) );
			reset_caldac( setup, layout->adc_gain( channel ) );
			reset_caldac( setup, layout->adc_offset_fine( channel ) );
			reset_caldac( setup, layout->adc_gain_fine( channel ) );
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
static void generic_do_dac_channel( calibration_setup_t *setup, const generic_layout_t *layout ,
	saved_calibration_t *current_cal, unsigned int channel, unsigned int range )
{
	generic_prep_dac_caldacs( setup, layout, channel, range );

	generic_do_relative( setup, current_cal, layout->dac_ground_observable( setup, channel, range ),
		layout->dac_high_observable( setup, channel, range ), layout->dac_gain( channel ) );

	generic_do_cal( setup, current_cal, layout->dac_ground_observable( setup, channel, range ),
		layout->dac_offset( channel ) );

	current_cal->subdevice = setup->da_subdev;
	sc_push_channel( current_cal, channel );
	sc_push_range( current_cal, range );
	sc_push_aref( current_cal, SC_ALL_AREFS );
}

static void generic_do_adc_channel( calibration_setup_t *setup, const generic_layout_t *layout ,
	saved_calibration_t *current_cal, unsigned int channel, unsigned int range )
{
	generic_prep_adc_caldacs( setup, layout, channel, range );

	generic_do_relative( setup, current_cal, layout->adc_high_observable( setup, channel, range ),
		layout->adc_ground_observable( setup, channel, range ), layout->adc_gain( channel ) );

	generic_do_cal( setup, current_cal, layout->adc_ground_observable( setup, channel, range ),
		layout->adc_offset( channel ) );

	generic_do_relative( setup, current_cal, layout->adc_high_observable( setup, channel, range ),
		layout->adc_ground_observable( setup, channel, range ), layout->adc_gain_fine( channel ) );

	generic_do_cal( setup, current_cal, layout->adc_ground_observable( setup, channel, range ),
		layout->adc_offset_fine( channel ) );

	current_cal->subdevice = setup->ad_subdev;
	sc_push_channel( current_cal, channel );
	sc_push_range( current_cal, range );
	sc_push_aref( current_cal, SC_ALL_AREFS );
}

static int calibration_is_valid( const saved_calibration_t *saved_cal,
	unsigned int subdevice, unsigned int channel, unsigned int range )
{
	int i;

	if( saved_cal->subdevice != subdevice ) return 0;
	if( saved_cal->channels_length )
	{
		for( i = 0; i < saved_cal->channels_length; i++ )
		{
			if( saved_cal->channels[ i ] == channel )
				break;
		}
		if( i == saved_cal->channels_length ) return 0;
	}
	if( saved_cal->ranges_length )
	{
		for( i = 0; i < saved_cal->ranges_length; i++ )
		{
			if( saved_cal->ranges[ i ] == range )
				break;
		}
		if( i == saved_cal->ranges_length ) return 0;
	}
	return 1;
}

static void apply_calibration(  calibration_setup_t *setup, saved_calibration_t *saved_cals,
	unsigned int saved_cals_length, unsigned int subdevice, unsigned int channel,
	unsigned int range )
{
	int i, retval;

	for( i = 0; i < saved_cals_length; i++ )
	{
		int j;

		if( calibration_is_valid( &saved_cals[ i ], subdevice, channel, range ) )
		{
			for( j = 0; j < saved_cals[ i ].caldacs_length; j++ )
			{
				caldac_t *caldac;

				caldac = &saved_cals[ i ].caldacs[ j ];
				retval = comedi_data_write( setup->dev, caldac->subdev, caldac->chan,
					0, 0, caldac->current );
				assert( retval >= 0 );
			}
		}
	}
}

static void generic_prep_adc_for_dac( calibration_setup_t *setup, const generic_layout_t *layout,
	saved_calibration_t *saved_cals, unsigned int saved_cals_length,
	unsigned int dac_channel, unsigned int dac_range )
{
	unsigned int adc_channel, adc_range;
	int i;
	int chanspec;

	for( i = 0; i < setup->n_observables; i++ )
	{
		chanspec = setup->observables[ i ].preobserve_insn.chanspec;
		if( CR_CHAN( chanspec ) == dac_channel && CR_RANGE( chanspec ) == dac_range )
			break;
	}
	assert( i < setup->n_observables );
	chanspec = setup->observables[ i ].observe_insn.chanspec;
	adc_channel = CR_CHAN( chanspec );
	adc_range = CR_RANGE( chanspec );

	apply_calibration( setup, saved_cals, saved_cals_length, setup->ad_subdev,
		adc_channel, adc_range );
}

int generic_cal_by_channel_and_range( calibration_setup_t *setup,
	const generic_layout_t *layout  )
{
	int range, channel, num_ai_ranges, num_ai_channels, num_ao_ranges,
		num_ao_channels, retval, num_calibrations, num_ai_calibrations, i;
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

	num_ai_calibrations = num_ai_ranges * num_ai_channels;
	num_calibrations = num_ai_calibrations + num_ao_ranges * num_ao_channels;

	saved_cals = malloc( num_calibrations * sizeof( saved_calibration_t ) );
	if( saved_cals == NULL ) return -1;

	current_cal = saved_cals;

	for( channel = 0; channel < num_ai_channels; channel++ )
	{
		for( range = 0; range < num_ai_ranges; range++ )
		{
			generic_do_adc_channel( setup, layout, current_cal, channel, range );
			current_cal++;
		}
	}
	for( channel = 0; channel < num_ao_channels; channel++ )
	{
		for( range = 0; range < num_ao_ranges; range++ )
		{
			generic_prep_adc_for_dac( setup, layout, saved_cals, num_ai_calibrations,
				channel, range );
			generic_do_dac_channel( setup, layout, current_cal, channel, range );
			current_cal++;
		}
	}

	retval = write_calibration_file( setup, saved_cals, num_calibrations );
	for( i = 0; i < num_calibrations; i++ )
		clear_saved_calibration( &saved_cals[ i ] );
	free( saved_cals );
	return retval;
}

int generic_cal_by_range( calibration_setup_t *setup,
	const generic_layout_t *layout  )
{
	int channel, range, num_ai_ranges, num_ao_ranges,
		num_ao_channels, retval, num_calibrations, i;
	saved_calibration_t *saved_cals, *current_cal;

	assert( comedi_range_is_chan_specific( setup->dev, setup->ad_subdev ) == 0 );

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 ) return -1;

	if( setup->da_subdev && setup->do_output )
	{
		assert( comedi_range_is_chan_specific( setup->dev, setup->da_subdev ) == 0 );

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		num_ao_channels = comedi_get_n_channels( setup->dev, setup->da_subdev );
		if( num_ao_channels < 0 ) return -1;
	}else
		num_ao_ranges = num_ao_channels = 0;

	num_calibrations = num_ai_ranges + num_ao_ranges * num_ao_channels;

	saved_cals = malloc( num_calibrations * sizeof( saved_calibration_t ) );
	if( saved_cals == NULL ) return -1;

	current_cal = saved_cals;

	for( range = 0; range < num_ai_ranges; range++ )
	{
		generic_do_adc_channel( setup, layout, current_cal, 0, range );
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
		current_cal++;
	}
	for( channel = 0; channel < num_ao_channels; channel++ )
	{
		for( range = 0; range < num_ao_ranges; range++ )
		{
			generic_prep_adc_for_dac( setup, layout, saved_cals, num_ai_ranges,
				channel, range );
			generic_do_dac_channel( setup, layout, current_cal, channel, range );
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
static int dummy_observable( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return -1;
}
void init_generic_layout( generic_layout_t *layout )
{
	layout->adc_offset = dummy_caldac;
	layout->adc_gain = dummy_caldac;
	layout->adc_offset_fine = dummy_caldac;
	layout->adc_gain_fine = dummy_caldac;
	layout->dac_offset = dummy_caldac;
	layout->dac_gain = dummy_caldac;
	layout->adc_high_observable = dummy_observable;
	layout->adc_ground_observable = dummy_observable;
	layout->dac_high_observable = dummy_observable;
	layout->dac_ground_observable = dummy_observable;
}
