
number=0xffaaee
size = 6
print hex(number)

for i in range(0,size):
    n = number >> 4;
    print size-i-1,  hex(number - (n << 4));
    number = n;

