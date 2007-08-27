/*
	A C++ binding for comedilib.  Requires the Boost C++ libraries.
	Copyright (C) 2006-2007  Frank Mori Hess <fmhess@users.sourceforge.net>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _COMEDILIB_WRAPPER_HPP
#define _COMEDILIB_WRAPPER_HPP

#include <boost/shared_ptr.hpp>
#include <comedilib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace comedi
{
	class subdevice;

	class device	{
	public:
		device() {}
		device(const std::string &device_file)
		{
			this->open(device_file);
		}
		std::string board_name() const
		{
			const char *name = comedi_get_board_name(comedi_handle());
			if(name == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_board_name() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_board_name");
				throw std::runtime_error(message.str());
			}
			return name;
		}
		int command_test(comedi_cmd *cmd) const
		{
			int retval = comedi_command_test(comedi_handle(), cmd);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_command_test() returned error.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_command_test");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		void command(comedi_cmd *cmd) const
		{
			int retval = comedi_command(comedi_handle(), cmd);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_command() failed, return value=" << retval << " .";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_command");
				throw std::runtime_error(message.str());
			}
		}
		std::string default_calibration_path() const
		{
			return comedi_get_default_calibration_path(comedi_handle());
		}
		void do_insn(comedi_insn *instruction) const
		{
			int retval = comedi_do_insn(comedi_handle(), instruction);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_do_insn() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_do_insn");
				throw std::runtime_error(message.str());
			}
		}
		std::string driver_name() const
		{
			const char *name = comedi_get_driver_name(comedi_handle());
			if(name == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_driver_name() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_driver_name");
				throw std::runtime_error(message.str());
			}
			return name;
		}
		inline subdevice find_subdevice_by_type(int type, const subdevice *start_subdevice = 0) const;
		int fileno() const
		{
			int fd = comedi_fileno(comedi_handle());
			if(fd < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_fileno() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_fileno");
				throw std::runtime_error(message.str());
			}
			return fd;
		}
		unsigned n_subdevices() const
		{
			int retval = comedi_get_n_subdevices(comedi_handle());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_n_subdevices() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_n_subdevices");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		void open(const std::string &device_file)
		{
			comedi_t *dev = comedi_open(device_file.c_str());
			if(dev == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_open() failed, with device file name: \"" << device_file << "\".";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_open");
				throw std::runtime_error(message.str().c_str());
			}
			_comedi_handle.reset(dev, &comedi_close);
		}
	private:
		friend class subdevice;

		comedi_t* comedi_handle() const
		{
			if(_comedi_handle == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": device not open.";
				std::cerr << message.str() << std::endl;
				throw std::invalid_argument(message.str());
			}
			return _comedi_handle.get();
		}

		boost::shared_ptr<comedi_t> _comedi_handle;
	};

	class subdevice
	{
	public:
		subdevice(const device &dev, unsigned subdevice_index):
			_device(dev), _index(subdevice_index)
		{
			if(_index >= _device.n_subdevices())
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": invalid subdevice index.";
				std::cerr << message.str() << std::endl;
				throw std::invalid_argument(message.str());
			}
		}
		unsigned buffer_size() const
		{
			int retval = comedi_get_buffer_size(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_buffer_size() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_buffer_size");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		lsampl_t data_read(unsigned channel, unsigned range, unsigned aref) const
		{
			lsampl_t value;
			int retval = comedi_data_read(comedi_handle(), index(), channel, range, aref, &value);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_read() failed, return value=" << retval << " .";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_data_read");
				throw std::runtime_error(message.str());
			}
			return value;
		}
		std::vector<lsampl_t> data_read_n(unsigned channel, unsigned range, unsigned aref, unsigned num_samples) const
		{
			std::vector<lsampl_t> values(num_samples);
			int retval = comedi_data_read_n(comedi_handle(), index(), channel, range, aref, &values.at(0), values.size());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_read_n() failed, return value=" << retval << " .";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_data_read_n");
				throw std::runtime_error(message.str());
			}
			return values;
		}
		void data_read_hint(unsigned channel, unsigned range, unsigned aref) const
		{
			int ret = comedi_data_read_hint(comedi_handle(), index(), channel, range, aref);
			if(ret < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_read_hint() failed, return value = " << ret << " .";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_data_read_hint");
				throw std::runtime_error(message.str());
			}
		}
		void data_write(unsigned channel, unsigned range, unsigned aref, lsampl_t data) const
		{
			int retval = comedi_data_write(comedi_handle(), index(), channel, range, aref, data);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_data_write() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_data_write");
				throw std::runtime_error(message.str());
			}
		}
		device dev() const {return _device;}
		unsigned flags() const
		{
			int retval = comedi_get_subdevice_flags(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_subdevice_flags() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_subdevice_flags");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		unsigned index() const {return _index;};
		unsigned max_buffer_size() const
		{
			int retval = comedi_get_max_buffer_size(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_max_buffer_size() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_max_buffer_size");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		unsigned n_channels() const
		{
			int retval = comedi_get_n_channels(comedi_handle(), index());
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_n_channels() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_n_channels");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		unsigned n_ranges(unsigned channel = 0) const
		{
			int retval = comedi_get_n_ranges(comedi_handle(), index(), channel);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_n_ranges() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_n_ranges");
				throw std::runtime_error(message.str());
			}
			return retval;
		}
		lsampl_t max_data(unsigned channel = 0) const
		{
			lsampl_t value = comedi_get_maxdata(comedi_handle(), index(), channel);
			if(value == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_maxdata() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_maxdata");
				throw std::runtime_error(message.str());
			}
			return value;
		}
		const comedi_range* range(unsigned channel, unsigned range_index) const
		{
			comedi_range *cRange = comedi_get_range(comedi_handle(), index(), channel, range_index);
			if(cRange == 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_get_range() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_get_range");
				throw std::runtime_error(message.str());
			}
			return cRange;
		}
		void set_buffer_size(unsigned num_bytes) const
		{
			int retval = comedi_set_buffer_size(comedi_handle(), index(), num_bytes);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_buffer_size() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_set_buffer_size");
				throw std::runtime_error(message.str());
			}
		}
		void set_max_buffer_size(unsigned num_bytes) const
		{
			int retval = comedi_set_max_buffer_size(comedi_handle(), index(), num_bytes);
			if(retval < 0)
			{
				std::ostringstream message;
				message << __PRETTY_FUNCTION__ << ": comedi_set_max_buffer_size() failed.";
				std::cerr << message.str() << std::endl;
				comedi_perror("comedi_set_max_buffer_size");
				throw std::runtime_error(message.str());
			}
		}
	private:
		comedi_t* comedi_handle() const {return dev().comedi_handle();}

		device _device;
		unsigned _index;
	};

	subdevice device::find_subdevice_by_type(int type, const subdevice *start_subdevice) const
	{
		unsigned start_index;
		if(start_subdevice) start_index = start_subdevice->index();
		else start_index = 0;
		int subdev = comedi_find_subdevice_by_type(comedi_handle(), type, start_index);
		if(subdev < 0)
		{
			std::ostringstream message;
			message << __PRETTY_FUNCTION__ << ": failed to find subdevice of type " << type << " .";
			std::cerr << message.str() << std::endl;
			comedi_perror("comedi_find_subdevice_by_type");
			throw std::runtime_error(message.str());
		}
		return subdevice(*this, subdev);
	}
};

#endif	// _COMEDILIB_WRAPPER_HPP
