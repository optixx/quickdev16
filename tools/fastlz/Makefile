
all: poc pack

poc: poc.c fastlz.c 
	gcc -o poc poc.c fastlz.c -I/usr/local/Cellar/openssl/1.0.2g/include -L/usr/local/Cellar/openssl/1.0.2g/lib -lssl  -lcrypto 

pack: pack.c fastlz.c
	gcc -o pack pack.c fastlz.c -I/usr/local/Cellar/openssl/1.0.2g/include -L/usr/local/Cellar/openssl/1.0.2g/lib -lssl  -lcrypto 
