#! /bin/bash
# Yes, Bash, because we use features specific to it.

# Make it possible to specify another location (DESTDIR=/usr/bin ./install.sh).
if [ -z $DESTDIR ]; then
DESTDIR=/usr/local/bin
elif [ ! -e $DESTDIR ]; then
echo "$DESTDIR does not exist, installing to /usr/local/bin"
DESTDIR=/usr/local/bin
fi

echo "Give root's password:"
# The version of su on Mac OS X requires the user name to be specified
su root -c "
echo Continuing installation.
chown root ucon64
chmod 4775 ucon64
cp -p ucon64 $DESTDIR
"
if [ ! -e $HOME/.ucon64 ]; then
mkdir $HOME/.ucon64
fi
if [ ! -e $HOME/.ucon64/dat ]; then
mkdir $HOME/.ucon64/dat
echo "You can copy/move your DAT file collection to $HOME/.ucon64/dat"
fi

if [ ${OSTYPE:0:6} == darwin ]; then
LIBSUFFIX=.dylib
elif [ $OSTYPE == cygwin ]; then
LIBSUFFIX=.dll
elif [ $OSTYPE == msys ]; then
LIBSUFFIX=.dll
else
LIBSUFFIX=.so
fi

if [ -f libdiscmage/discmage$LIBSUFFIX ]; then
cp libdiscmage/discmage$LIBSUFFIX $HOME/.ucon64
elif [ -f discmage$LIBSUFFIX ]; then
cp discmage$LIBSUFFIX $HOME/.ucon64
fi
echo "Be sure to check $HOME/.ucon64rc for some options after"
echo "you've run uCON64 once."
echo
