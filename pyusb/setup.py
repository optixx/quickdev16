#!/usr/bin/env python
#
# PyUSB setup script
#
# Copyright 2005 - 2007 Wander Lairson Costa

from distutils.core import setup, Extension
import sys

extra_link_args = []
extra_compile_args = []
platform = sys.platform.lower()
libraries = ["usb"]

# necessary to work fine in MacOS
# many thanks to Damian Staniforth! :-)
if -1 != platform.find("mac"):
	extra_link_args = ['-framework',
					   'CoreFoundation',
					   '-framework',
					   'IOKit']
elif -1 != platform.find("win32"):
	libraries = ["libusb"]
# necessary to work fine in darwin
# Many thanks to James Barabas!
elif -1 != platform.find("darwin"):
	extra_link_args = ['-framework',
					   'CoreFoundation',
					   '-framework',
					   'IOKit',
					   '-L/opt/local/lib']
	extra_compile_args = ['-I/opt/local/include']
# Juha Torkkel has reported problems compiling on freebsd
# when libusb is in /usr/local tree. I don't know on freebsd, but
# on Linux the paths to usr/local are in $PATH.
# Thanks Juha... ;)
elif -1 != platform.find("freebsd"):
	extra_link_args = ['-L/usr/local/lib']
	extra_compile_args = ['-I/usr/local/include']
																											

usbmodule = Extension(name = 'usb',
					libraries = libraries,
					sources = ['pyusb.c'],
					extra_link_args = extra_link_args,
					extra_compile_args = extra_compile_args,
					depends = ['pyusb.h'])

setup(name = 'pyusb',
	version = '0.4.1',
	description = "USB access extension module",
	long_description =
	"""
	PyUSB provides easy USB access to python.
	The module contains classes and methods to
	support the most USB operations.
	""",
	author = 'Wander Lairson Costa',
	author_email = 'wander.lairson@gmail.com',
	url = 'http://pyusb.berlios.de',
	license = 'BSD',
	ext_modules = [usbmodule])

# vim:ts=4
