#!/usr/bin/env python
#
# Copyright (C) May 2012  W. Trevor King <wking@drexel.edu>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

"""Emulate the C demo `insn.c`

This example does 3 instructions in one system call.  It does a
`gettimeofday()` call, then reads `N_SAMPLES` samples from an analog
input, and the another `gettimeofday()` call.
"""

import logging as _logging

import comedi as _comedi


MAX_SAMPLES = 128
LOG = _logging.getLogger('comedi-insn')
LOG.addHandler(_logging.StreamHandler())
LOG.setLevel(_logging.ERROR)


def insn_str(insn):
    return ', '.join([
            'insn: {}'.format(insn.insn),
            'subdev: {}'.format(insn.subdev),
            'n: {}'.format(insn.n),
            'data: {!r}'.format(insn.data),
            'chanspec: {}'.format(insn.chanspec),
            ])

def setup_gtod_insn(device, insn):
    insn.insn = _comedi.INSN_GTOD
    insn.subdev = 0
    insn.n = 2
    data = _comedi.lsampl_array(2)
    data[0] = 0
    data[1] = 0
    data.thisown = False
    insn.data = data.cast()
    insn.chanspec = 0
    return insn

def get_time_of_day(insn):
    assert insn.insn == _comedi.INSN_GTOD, insn.insn
    data = _comedi.lsampl_array.frompointer(insn.data)
    seconds = data[0]
    microseconds = data[1]
    return seconds + microseconds/1e6

class SetupReadInsn (object):
    def __init__(self, subdevice, channel, range, aref, n_scan):
        self.subdevice = subdevice
        self.channel = channel
        self.range = range
        self.aref = getattr(_comedi, 'AREF_{}'.format(aref.upper()))
        self.n_scan = n_scan

    def __call__(self, device, insn):
        insn.insn = _comedi.INSN_READ
        insn.n = self.n_scan
        data = _comedi.lsampl_array(self.n_scan)
        data.thisown = False
        insn.data = data.cast()
        insn.subdev = self._get_subdevice(device)
        insn.chanspec = _comedi.cr_pack(self.channel, self.range, self.aref)
        return insn

    def _get_subdevice(self, device):
        if self.subdevice is None:
            return _comedi.comedi_find_subdevice_by_type(
                device, _comedi.COMEDI_SUBD_AI, 0);
        return self.subdevice


def setup_insns(device, insn_setup_functions):
    n = len(insn_setup_functions)
    insns = _comedi.comedi_insnlist_struct()
    insns.n_insns = n
    array = _comedi.insn_array(n)
    array.thisown = False
    for i,setup in enumerate(insn_setup_functions):
        array[i] = setup(device, array[i])
    insns.insns = array.cast()
    return insns

def free_insns(insns):
    array = _comedi.insn_array.frompointer(insns.insns)
    array.thisown = True
    for i in range(insns.n_insns):
        insn = array[i]
        data = _comedi.lsampl_array.frompointer(insn.data)
        data.thisown = True


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        '-f', '--filename', default='/dev/comedi0',
        help='path to comedi device file')
    parser.add_argument(
        '-s', '--subdevice', type=int, help='subdevice for analog input')
    parser.add_argument(
        '-c', '--channel', type=int, default=0,
        help='channel for analog input')
    parser.add_argument(
        '-a', '--analog-reference', dest='aref', default='ground',
        choices=['diff', 'ground', 'other', 'common'],
        help='reference for analog input')
    parser.add_argument(
        '-r', '--range', type=int, default=0, help='range for analog input')
    parser.add_argument(
        '-N', '--num-scans', type=int, default=10,
        help='number of analog input scans')
    parser.add_argument(
        '-v', '--verbose', default=0, action='count')

    args = parser.parse_args()

    if args.verbose >= 3:
        LOG.setLevel(_logging.DEBUG)
    elif args.verbose >= 2:
        LOG.setLevel(_logging.INFO)
    elif args.verbose >= 1:
        LOG.setLevel(_logging.WARN)

    if args.num_scans > MAX_SAMPLES:
        LOG.warn('requested too many samples, reducing to {}'.format(
                MAX_SAMPLES))
        args.num_scans = MAX_SAMPLES

    LOG.info(('measuring device={0.filename} subdevice={0.subdevice} '
              'channel={0.channel} range={0.range} analog reference={0.aref}'
              ).format(args))

    device = _comedi.comedi_open(args.filename)
    if not device:
        raise Exception('error opening Comedi device {}'.format(
                args.filename))

    setup_read_insn = SetupReadInsn(
        subdevice=args.subdevice, channel=args.channel,
        aref=args.aref, range=args.range, n_scan=args.num_scans)

    insns = setup_insns(
        device, [setup_gtod_insn, setup_read_insn, setup_gtod_insn])

    ret = _comedi.comedi_do_insnlist(device, insns)
    if ret != insns.n_insns:
        raise Exception('error running instructions ({})'.format(ret))

    ret = _comedi.comedi_close(device)
    if ret != 0:
        raise Exception('error closing Comedi device {} ({})'.format(
                args.filename, ret))

    array = _comedi.insn_array.frompointer(insns.insns)
    t1 = get_time_of_day(array[0])
    t2 = get_time_of_day(array[2])
    print('initial time: {}'.format(t1))
    print('final time:   {}'.format(t2))
    print('difference:   {}'.format(t2-t1))
    print('data:')
    data = _comedi.lsampl_array.frompointer(array[1].data)
    for i in range(array[1].n):
        print(data[i])

    free_insns(insns)
