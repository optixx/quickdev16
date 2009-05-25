

bin = fftest
src = ff.c main.c diskio.c 
#cc950.c
objs=$(src:.c=.o)


all: $(src) $(bin)

$(bin): $(objs)
	gcc $(ldflags) $(objs) -o $@

%.o : %.c
	gcc $(cflags) -c $< 


clean:
	rm -rf $(bin) *.o


image32:
	dd if=/dev/zero of=disk00.vfat bs=1M count=64
	sudo mkfs.vfat -F 32 -v disk00.vfat


image_parted:
	dd if=/dev/zero of=disk00.vfat bs=1M count=256
	fdisk -H 32 -S 32 -C 500 disk00.vfat
	# create one partion
	sudo losetup -o 16384  /dev/loop0 disk00.vfat
	sudo mkfs.vfat -F 32 -v  /dev/loop0
	sudo mount /dev/loop0  disk
	sudo cp -v /var/log/*log disk/
	sudo ls -al disk
	sudo umount disk
	sudo losetup -d /dev/loop0


image_not_parted:
	dd if=/dev/zero of=disk00.vfat bs=1M count=256
	fdisk -H 32 -S 32 -C 500 disk00.vfat
	# no one partion
	sudo losetup /dev/loop0 disk00.vfat
	sudo mkfs.vfat -f 2 -F 16 -v  /dev/loop0
	sudo mount /dev/loop0  disk
	sudo cp /var/log/*log disk/
	sudo umount disk
	sudo losetup -d /dev/loop0


image_no_fdisk:
	dd if=/dev/zero of=disk00.vfat bs=1M count=256
	sudo mkfs.vfat -I -f 2 -F 16 -v disk00.vfat
	sudo mount -o loop disk00.vfat disk
	sudo cp /var/log/*log disk/
	sudo umount disk

image_no_dd:
	rm -fv disk00.vfat
	mkfs.vfat   -F 32 -v -C disk00.vfat 256000 
	sudo mount -o loop,check=s disk00.vfat disk
	sudo cp -v /var/log/*log disk/
	sudo umount disk

mount:
	sudo losetup /dev/loop0 disk00.vfat
	sudo mount /dev/loop0  disk

umount:
	sudo umount disk
	sudo losetup -d /dev/loop0

