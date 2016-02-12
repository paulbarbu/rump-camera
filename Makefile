TUPLE=
STRIP=strip

# ALT+2, quit
run: camera.bin
	#set PATH /home/paul/localhost/rumpkernel/rumprun/rumprun/bin/ $PATH
	#-W if,inet,static,10.0.120.101/24 \
	rumprun qemu -i \
		-I if,vioif,'-net tap,script=no,ifname=tap0'\
        -W if,inet,static,192.168.0.103/24 \
		-b fs.img,/data \
		-g -curses \
		-- camera.bin 12312
	
camera.bin: camera
	rumprun-bake hw_generic camera.bin camera
	# TODO: use the proper strip commadn before running this
	#$(STRIP) camera.bin

rump:
	$(eval TUPLE=x86_64-rumprun-netbsd-)
	-unlink libs
	ln -s ${@}libs libs
	$(MAKE) camera TUPLE=$(TUPLE)

x86_64:
	-unlink libs
	ln -s ${@}libs libs
	$(MAKE) camera TUPLE=$(TUPLE)
	
camera: main.c video.c manager.c httpd.c credentials.h config.h
	$(eval CC=$(TUPLE)gcc)
	$(eval STRIP=$(TUPLE)strip)
	@echo tuple $(TUPLE)
	@echo cc $(CC)

	$(CC) -g -Wall -o camera -I. ${^} libs/libmicrohttpd.a -Wl,-Bdynamic -lpthread

iface:
	(ip tuntap add tap0 mode tap)
	(ip link set dev tap0 up)
	#(ip addr add 10.0.120.100/24 dev tap0)
	(ip addr add 192.169.0.102/24 dev tap0)
	(ip addr flush eth0)
	(brctl addbr br0)
	(brctl addif br0 eth0 tap0)
	(dhclient br0)

fs:
	# create a 50MB file system
	dd if=/dev/zero of=fs.img bs=512 count=102400
	mkfs -t ext2 fs.img
	chown paul:paul fs.img

mount:
	mkdir -p mnt
	$(eval LOOP=$(shell losetup --show -f fs.img))
	mount $(LOOP) mnt

.PHONY: clean
clean:
	-umount mnt
	-rmdir mnt
	rm -f camera camera.bin fs.img
