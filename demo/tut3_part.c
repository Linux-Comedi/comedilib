	/* open the device */
	dev = comedi_open(options.filename);
	if(!dev){
		comedi_perror(options.filename);
		exit(1);
	}

	// Print numbers for clipped inputs
	comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

	/* Set up channel list */
	for(i = 0; i < options.n_chan; i++){
		chanlist[i] = CR_PACK(options.channel + i,
				      options.range, 
				      options.aref);
		range_info[i] = comedi_get_range(dev, 
						 options.subdevice, 
						 options.channel, options.range);
		maxdata[i] = comedi_get_maxdata(dev, 
						options.subdevice, 
						options.channel);
	}

	/* prepare_cmd_lib() uses a Comedilib routine to find a
	 * good command for the device.  prepare_cmd() explicitly
	 * creates a command, which may not work for your device. */
	prepare_cmd_lib(dev, 
			options.subdevice, 
			options.n_scan, 
			options.n_chan, 
			1e9 / options.freq, cmd);

	/* comedi_command_test() tests a command to see if the
	 * trigger sources and arguments are valid for the subdevice.
	 * If a trigger source is invalid, it will be logically ANDed
	 * with valid values (trigger sources are actually bitmasks),
	 * which may or may not result in a valid trigger source.
	 * If an argument is invalid, it will be adjusted to the
	 * nearest valid value.  In this way, for many commands, you
	 * can test it multiple times until it passes.  Typically,
	 * if you can't get a valid command in two tests, the original
	 * command wasn't specified very well. */
	ret = comedi_command_test(dev, cmd);
	if(ret < 0){
		comedi_perror("comedi_command_test");
		exit(1);
	}
	ret = comedi_command_test(dev, cmd);
	if(ret < 0){
		comedi_perror("comedi_command_test");
		exit(1);
	}
	fprintf(stderr,"second test returned %d (%s)\n", ret,
			cmdtest_messages[ret]);
	if(ret!=0){
		fprintf(stderr, "Error preparing command\n");
		exit(1);
	}

	/* start the command */
	ret = comedi_command(dev, cmd);
	if(ret < 0){
		comedi_perror("comedi_command");
		exit(1);
	}
	subdev_flags = comedi_get_subdevice_flags(dev, options.subdevice);
	while(1){
		ret = read(comedi_fileno(dev),buf,BUFSZ);
		if(ret < 0){
			/* some error occurred */
			perror("read");
			break;
		}else if(ret == 0){
			/* reached stop condition */
			break;
		}else{
			static int col = 0;
			int bytes_per_sample;
			total += ret;
			if(options.verbose)fprintf(stderr, "read %d %d\n", ret, total);
			if(subdev_flags & SDF_LSAMPL)
				bytes_per_sample = sizeof(lsampl_t);
			else
				bytes_per_sample = sizeof(sampl_t);
			for(i = 0; i < ret / bytes_per_sample; i++){
				if(subdev_flags & SDF_LSAMPL) {
					raw = ((lsampl_t *)buf)[i];
				} else {
					raw = ((sampl_t *)buf)[i];
				}
				print_datum(raw, col);
				col++;
				if(col == options.n_chan){
					printf("\n");
					col=0;
				}
			}
		}
	}

}

/*
 * This prepares a command in a pretty generic way.  We ask the
 * library to create a stock command that supports periodic
 * sampling of data, then modify the parts we want. */
int prepare_cmd_lib(comedi_t *dev, int subdevice, int n_scan, int n_chan, unsigned scan_period_nanosec, comedi_cmd *cmd)
{
	int ret;

	memset(cmd,0,sizeof(*cmd));

	/* This comedilib function will get us a generic timed
	 * command for a particular board.  If it returns -1,
	 * that's bad. */
	ret = comedi_get_cmd_generic_timed(dev, subdevice, cmd, n_chan, scan_period_nanosec);
	if(ret<0){
		printf("comedi_get_cmd_generic_timed failed\n");
		return ret;
	}

	/* Modify parts of the command */
	cmd->chanlist = chanlist;
	cmd->chanlist_len = n_chan;
	if(cmd->stop_src == TRIG_COUNT) cmd->stop_arg = n_scan;

	return 0;
}
