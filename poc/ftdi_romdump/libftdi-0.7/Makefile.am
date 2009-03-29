# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

SUBDIRS = src

EXTRA_DIST = libftdi.spec COPYING.LIB README ChangeLog libftdi-config.in

bin_SCRIPTS = libftdi-config

# Install the pkg-config file:
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libftdi.pc
