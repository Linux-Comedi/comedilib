/*
    lib/buffer.c
    functions for manipulating buffers

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

#include <comedi.h>

#include <libinternal.h>

int comedi_set_buffer_size(comedi_t *it, unsigned int subdev, unsigned int size)
{
	int ret;
	comedi_bufconfig bc;

	memset(&bc, 0, sizeof(bc));
	bc.subdevice = subdev;
	bc.size = size;
	ret = ioctl(it->fd, COMEDI_BUFCONFIG, &bc);
	__comedi_errno = errno;
	if(ret < 0) return ret;

	return bc.size;
}

int comedi_set_max_buffer_size(comedi_t *it, unsigned int subdev, unsigned int max_size)
{
	int ret;
	comedi_bufconfig bc;

	memset(&bc, 0, sizeof(bc));
	bc.subdevice = subdev;
	bc.maximum_size = max_size;
	ret = ioctl(it->fd, COMEDI_BUFCONFIG, &bc);
	__comedi_errno = errno;
	if(ret < 0) return ret;

	return bc.maximum_size;
}

int comedi_get_max_buffer_size(comedi_t *it, unsigned int subdevice)
{
	return comedi_set_max_buffer_size(it, subdevice, 0);
}

int comedi_get_buffer_size(comedi_t *it, unsigned int subdev)
{
	return comedi_set_buffer_size(it, subdev, 0);
}

int comedi_get_buffer_contents(comedi_t *it, unsigned int subdev)
{
	return comedi_mark_buffer_read(it, subdev, 0);
}

int comedi_mark_buffer_read(comedi_t *it, unsigned int subdev, unsigned int bytes)
{
	int ret;
	comedi_bufinfo bi;

	memset(&bi, 0, sizeof(bi));
	bi.bytes_read = bytes;
	ret = ioctl(it->fd, COMEDI_BUFINFO, &bi);
	__comedi_errno = errno;
	if(__comedi_errno == EINVAL)__comedi_errno = EBUF_OVR;
	return bi.buf_int_count - bi.buf_user_count;
}

int comedi_get_buffer_offset(comedi_t *it, unsigned int subdev)
{
	int ret;
	comedi_bufinfo bi;

	memset(&bi, 0, sizeof(bi));
	ret = ioctl(it->fd, COMEDI_BUFINFO, &bi);
	if(ret < 0) return ret;
	return bi.buf_user_ptr;
}

int comedi_get_front_count(comedi_t *it, unsigned int subdev)
{
	int ret;
	comedi_bufinfo bi;

	memset(&bi, 0, sizeof(bi));
	ret = ioctl(it->fd, COMEDI_BUFINFO, &bi);
	if(ret < 0) return ret;
	return bi.buf_int_count;
}

