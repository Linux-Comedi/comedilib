require 'mkmf'
dir_config('comedilib')
find_header('comedilib_version.h', '../../../include')
have_library('comedi')
create_makefile("comedi")
