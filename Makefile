TUPLE=

# ALT+2, quit
run: camera.bin
	fsck.ext2 -y fs.img
	#set PATH /home/paul/localhost/rumpkernel/rumprun/rumprun/bin/ $PATH
	rumprun qemu -i \
		-I if,vioif,'-net tap,script=no,ifname=tap0'\
        -W if,inet,static,192.168.0.103/24 \
		-b fs.img,/data \
		-g -curses \
		-- camera.bin 12312

# gdb -ex 'target remote:1337' camera_debug.bin 
# break rumprun_main2
debug: camera_debug.bin
	fsck.ext2 -y fs.img
	rumprun qemu -i \
		-I if,vioif,'-net tap,script=no,ifname=tap0'\
        -W if,inet,static,192.168.0.103/24 \
		-b fs.img,/data \
		-g -curses \
		-D 1337 \
		-- camera_debug.bin 12312
	
camera.bin: rump
	rumprun-bake hw_generic camera.bin camera
	x86_64-rumprun-netbsd-strip camera.bin
			
camera_debug.bin: rump sleep
	rumprun-bake hw_generic camera_debug.bin sleep camera

x86_64:
	-unlink libs
	ln -s ${@}libs libs
	$(MAKE) camera TUPLE=$(TUPLE)
	
rump:
	$(eval TUPLE=x86_64-rumprun-netbsd-)
	
	-unlink libs
	ln -s ${@}libs libs
	$(MAKE) camera TUPLE=$(TUPLE)
	
camera: main.c video.c manager.c httpd.c credentials.h config.h
	$(eval CC=$(TUPLE)gcc)
	@echo tuple $(TUPLE)
	@echo cc $(CC)

	$(CC) -g -Wall -o camera -I. ${^} libs/libmicrohttpd.a -Wl,-Bdynamic -lpthread
			
sleep: sleep.c
	$(eval CC=$(TUPLE)gcc)
	$(eval STRIP=$(TUPLE)strip)
	@echo tuple $(TUPLE)
	@echo cc $(CC)

	$(CC) -g -Wall -o sleep -I. ${^}
	
 # run as root
iface:
	(ip tuntap add tap0 mode tap)
	(ip link set dev tap0 up)
	(ip addr add 192.169.0.102/24 dev tap0)
	(ip addr flush eth0)
	(brctl addbr br0)
	(brctl addif br0 eth0 tap0)
	(dhclient br0)

# run as root
fs:
	# create a 50MB file system
	dd if=/dev/zero of=fs.img bs=512 count=102400
	mkfs -t ext2 fs.img
	chown paul:paul fs.img

# run as root
mount:
	mkdir -p mnt
	$(eval LOOP=$(shell losetup --show -f fs.img))
	mount $(LOOP) mnt

# run as root if you want to umount
.PHONY: clean
clean:
	-umount mnt
	-rmdir mnt
	rm -f camera camera.bin fs.img camera_debug.bin sleep

.PHONY: check
check:
	cppcheck --language=c --enable=all .