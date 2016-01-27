TUPLE=x86_64-rumprun-netbsd
CC=$(TUPLE)-gcc
STRIP=$(TUPLE)-strip

# ALT+2, quit
run: camera.bin
	rumprun -D qemu -i \
		-I if,vioif,'-net tap,script=no,ifname=tap0'\
        -W if,inet,static,10.0.120.101/24 \
		-g -curses \
		-- camera.bin 12312
	
camera.bin: camera
	rumprun-bake hw_generic camera.bin camera
	$(STRIP) camera.bin
	
camera: main.c
	$(CC) -o camera -I. main.c libmicrohttpd.a -Wl,-Bdynamic -lpthread

iface:
	(ip tuntap add tap0 mode tap)
	(ip addr add 10.0.120.100/24 dev tap0)
	(ip link set dev tap0 up)

.PHONY: clean
clean:
	rm -f camera camera.bin
