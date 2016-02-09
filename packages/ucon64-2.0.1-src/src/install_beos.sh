#!/bin/sh
# BeOS R5 install script for uCON64
cd $(dirname "$0")

areply=$(alert "This will start installation of uCON64 in a BeOS system.

uCON64 will be installed in $HOME/config/bin.

Do you want to continue?" "Cancel" "Install")
if [ "$areply" ==  "Install" ]; then
  cp ucon64 "$HOME/config/bin"
  if [ ! -e "$HOME/.ucon64" ]; then
    mkdir "$HOME/.ucon64"
  fi
  if [ ! -e "$HOME/.ucon64/dat" ]; then
    mkdir "$HOME/.ucon64/dat"
  fi
  if [ -f libdiscmage/discmage.so ]; then
    cp libdiscmage/discmage.so "$HOME/.ucon64"
  elif [ -f discmage.so ]; then
    cp discmage.so "$HOME/.ucon64"
  fi
  # ask for ioport driver installation
  drreply=$(alert "In order to communicate with a backup device (\"copier\") uCON64 needs the BeOS ioport driver by Caz Jones. You needn't install it if you don't want to use uCON64 for communicating with a copier.

Would you like to do install it?" "No" "Yes, install driver")
  if [ "$drreply" ==  "Yes, install driver" ]; then
    if [ -e "$HOME/ioport" ]; then
      cd "$HOME/ioport/driver"
      install_ioport
    elif [ -e ioport.zip ]; then
      unzip ioport.zip -d "$HOME"
      cd "$HOME/ioport/driver"
      install_ioport
    elif [ -e "$HOME/ioport.zip" ]; then
      unzip "$HOME/ioport.zip" -d "$HOME"
      cd "$HOME/ioport/driver"
      install_ioport
    else
      alert "Please download the latest version of the ioport driver from either
http://www.infernal.currantbun.com or
http://ucon64.sourceforge.net
and copy it to $HOME. Then retry to install the driver."
      # exit
      # Don't exit as uCON64 itself has been successfully installed
    fi
  fi

  alert "Done.
You can use uCON64 from the terminal now."
fi
