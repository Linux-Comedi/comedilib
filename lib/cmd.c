/*
    lib/cmd.c
    support functions for commands

    COMEDILIB - Linux Control and Measurement Device Interface Library
    Copyright (C) 1997-2001 David A. Schleef <ds@schleef.org>

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

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include <libinternal.h>


int comedi_get_cmd_src_mask(comedi_t *it,unsigned int subd,comedi_cmd *cmd)
{
	subdevice *s;
	int ret;

	if(!valid_subd(it,subd))return -1;

	s=it->subdevices+subd;

	if(s->cmd_mask_errno){
		errno = s->cmd_mask_errno;
		return -1;
	}

	if(!s->cmd_mask){
		comedi_cmd *mask;

		mask = malloc(sizeof(comedi_cmd));

		memset(mask,0,sizeof(*cmd));

		mask->subdev = subd;
		mask->flags = 0;
		mask->start_src = TRIG_ANY;
		mask->scan_begin_src = TRIG_ANY;
		mask->convert_src = TRIG_ANY;
		mask->scan_end_src = TRIG_ANY;
		mask->stop_src = TRIG_ANY;

		s->cmd_mask = mask;

		ret = comedi_command_test(it,mask);
		if(ret<0){
			s->cmd_mask_errno = errno;
			return -1;
		}
	}
	*cmd=*s->cmd_mask;
	return 0;
}

static int __generic_timed(comedi_t *it,unsigned int s,
	comedi_cmd *cmd, unsigned int ns)
{
	int ret;

	ret = comedi_get_cmd_src_mask(it,s,cmd);
	if(ret<0)return ret;

	__comedi_errno = ENOTSUPPORTED;

	if(cmd->start_src&TRIG_NOW){
		cmd->start_src=TRIG_NOW;
		cmd->start_arg=0;
	}else if(cmd->start_src&TRIG_INT){
		cmd->start_src=TRIG_INT;
		cmd->start_arg=0;
	}else{
		DEBUG(3,"can't find good start_src\n");
		return -1;
	}

	/* Potential bug: there is a possibility that the source mask may
	 * have * TRIG_TIMER set for both convert_src and scan_begin_src,
	 * but they may not be supported together. */
	if(cmd->convert_src&TRIG_TIMER){
		if(cmd->scan_begin_src&TRIG_FOLLOW){
			cmd->convert_src = TRIG_TIMER;
			cmd->convert_arg = ns;
			cmd->scan_begin_src = TRIG_FOLLOW;
			cmd->scan_begin_arg = 0;
		}else{
			cmd->convert_src = TRIG_TIMER;
			cmd->convert_arg = ns;
			cmd->scan_begin_src = TRIG_TIMER;
			cmd->scan_begin_arg = ns;
		}
	}else if(cmd->convert_src & TRIG_NOW &&
		cmd->scan_begin_src & TRIG_TIMER)
	{
		cmd->convert_src = TRIG_NOW;
		cmd->convert_arg = 0;
		cmd->scan_begin_src = TRIG_TIMER;
		cmd->scan_begin_arg = ns;
	}else{
		DEBUG(3,"comedi_get_cmd_generic_timed: can't do timed?\n");
		return -1;
	}

	cmd->scan_end_src = TRIG_COUNT;
	cmd->scan_end_arg = 1;

	if(cmd->stop_src&TRIG_COUNT){
		cmd->stop_src=TRIG_COUNT;
		cmd->stop_arg=2;
	}else if(cmd->stop_src&TRIG_NONE){
		cmd->stop_src=TRIG_NONE;
		cmd->stop_arg=0;
	}else{
		DEBUG(3,"comedi_get_cmd_generic_timed: can't find a good stop_src\n");
		return -1;
	}

	cmd->chanlist_len = 1;

	ret=comedi_command_test(it,cmd);
	DEBUG(3,"comedi_get_cmd_generic_timed: test 1 returned %d\n",ret);
	if(ret==3){
		/* good */
		ret=comedi_command_test(it,cmd);
		DEBUG(3,"comedi_get_cmd_generic_timed: test 2 returned %d\n",ret);
	}
	if(ret==4 || ret==0){
		__comedi_errno = 0;
		return 0;
	}
	return -1;
}

int comedi_get_cmd_generic_timed(comedi_t *it,unsigned int subd,comedi_cmd *cmd,
	unsigned int ns)
{
	subdevice *s;
	int ret;

	if(!valid_subd(it,subd))return -1;

	s=it->subdevices+subd;

	if(s->cmd_timed_errno){
		errno = s->cmd_mask_errno;
		return -1;
	}

	if(!s->cmd_timed){
		s->cmd_timed = malloc(sizeof(comedi_cmd));

		ret = __generic_timed(it,subd,s->cmd_timed,ns);
		if(ret<0){
			s->cmd_mask_errno = errno;
			return -1;
		}
	}
	*cmd=*s->cmd_timed;
	return 0;
}

