** General info on the swig-generated wrappers for Comedilib **

0) Installing required tools
  The wrapper are made with SWIG. Any of the swig-1.3.x series should
  work.  Run
    $ swig -version
  to check the version you have installed, and upgrade if necessary.

1) Building the wrappers
  Note: the following applies when building wrappers separately from
  the comedilib library, using Python's distutils procedures instead
  of Comedilib's `make`.  In this case, Comedilib should be configured
  with the `--disable-python-binding` option to prevent it building
  and installing its own copy of the wrappers.

  After building the main comedilib library (running `make` in the
  base directory), just follow standard distutils procedures
    $ python setup.py build
    $ python setup.py install

  If you want to test the wrappers before installing them, you will
  need to set the `PYTHONPATH` environment variable so Python can find
  the compiled modules.  On my system, that looks like
    $ PYTHONPATH=build/lib.linux-i686-2.7/ ../../demo/python/info.py

2) Using the module
  All the comedilib functions are translated directly to python
  function. The various comedi structs are now available as python
  classes (e.g. comedi_cmd_struct). The members of each struct are now
  attributes of the class and can be set and retrieved in the usual
  way. Comedilib functions which take a pointer to a comedilib struct
  as an argument (in C) now, in python, accept the appropriate struct
  python object.

  For a multichannel acquisition, a C-array containing the channel
  list, gains and referencing is required. This can be created using a
  swig-generated helper class: chanlist(n). This creates a C-array of
  length n and type Unsigned Int. Individual members of the array can
  be accessed/set using pythons indexing syntax:
    mylist = chanlist(3)   #creates a chanlist array of length 3
    mylist[0] = 100 #set some values
    mylist[1] = 200
    mylist[2] = 300

  The chanlist object can then be passed to a comedi_cmd_struct
  object, for example. N.B. The chanlist object contains *no*
  length-checking or other error protection so use with care! Don't
  try to get/set indexes outside the array bounds.

  All the comedilib macros (e.g. CR_PACK) are now available as python
  functions (e.g. `comedi.cr_pack`).

  Look at the examples in demo/python to clarify the above.
