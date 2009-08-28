export CFLAGS="-I/opt/local/include" 
export LDFLAGS="-L/opt/local/lib" 
./configure --with-libusb --disable-discmage  
make -j 3 clean all
