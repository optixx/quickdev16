

bin = fftest
src = ff.c main.c diskio.c
objs=$(src:.c=.o)


all: $(src) $(bin)

$(bin): $(objs)
	gcc $(ldflags) $(objs) -o $@

%.o : %.c
	gcc $(cflags) -c $< 


clean:
	rm $(bin) *.o
