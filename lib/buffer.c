#include <comedi.h>

#include <libinternal.h>

int comedi_set_buffer_size(comedi_t *it, unsigned int subdev, unsigned int size)
{
	int ret;
	comedi_bufconfig bc;

	memset(&bc, 0, sizeof(bc));
	bc.subdevice = subdev;
	bc.size = size;
	ret = ioctl_bufconfig(it->fd, &bc);
	if(ret < 0) return ret;

	return bc.size;
}

int comedi_set_buffer_max_size(comedi_t *it, unsigned int subdev, unsigned int max_size)
{
	int ret;
	comedi_bufconfig bc;

	memset(&bc, 0, sizeof(bc));
	bc.subdevice = subdev;
	bc.maximum_size = max_size;
	ret = ioctl_bufconfig(it->fd, &bc);
	if(ret < 0) return ret;

	return bc.maximum_size;
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
	ret = ioctl_bufinfo(it->fd, &bi);
	if(ret < 0) return ret;
	return bi.buf_int_count - bi.buf_user_count;
}

int comedi_get_buffer_offset(comedi_t *it, unsigned int subdev)
{
	int ret;
	comedi_bufinfo bi;

	memset(&bi, 0, sizeof(bi));
	ret = ioctl_bufinfo(it->fd, &bi);
	if(ret < 0) return ret;
	return bi.buf_user_ptr;
}
