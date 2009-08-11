################################################################################
###                      EFSL - Embedded Filesystems Library                 ###
###                      -----------------------------------                 ###
###                                                                          ###
################################################################################

# This is the configuration file for EFSL. This file will enable your to build
# the library if you have GNU make, or compatible, on your system.
# If you do not have a make utility on your system, or it cannot be used in this
# fashion (when using IDE's, like MSVC or Code composer), please refer to the
# documentation on how to build EFSL. It is possible to build EFSL with any C
# compiler although it will be a bit more work.

# C compiler
# ----------
#
# Here you select with what binary the sourcefiles must be compiled

CC=avr-gcc

# AR archiver
# -----------
#
# This variable controls what archiver is to be used. This utility is optional,
# if you don't have GNU make, you probably need to link differently as well.

AR=avr-ar

# Objcopy
# --------
#
# This variable controls what objcopy is to be used. This utility will be used
# when the program is converted to an format that your uC understands.

OBJCOPY=avr-objcopy

# C compiler options
# ------------------
#
# Here you can configure several options about the compilation.

DEBUGGING=-g3
VERIFY=-Wall
ARCHITECTURE=-mmcu=atmega128
OPTIMISE=-Os
GCFLAGS=$(DEBUGGING) $(VERIFY) $(ARCHITECTURE) $(OPTIMISE)
