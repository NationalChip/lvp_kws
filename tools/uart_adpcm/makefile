CFLAGS_WINDOWS := -D WINDOWS
CFLAGS_8002 := -D CHIP_8002

linux-8002:
	gcc $(CFLAGS_8002) -o sample.elf main.c porting/port.c message.c crc/crc32.c adpcm/adpcm.c -lpthread -no-pie
windows-8002:
	gcc $(CFLAGS_WINDOWS) $(CFLAGS_8002) -o sample.exe main.c porting/port.c message.c crc/crc32.c adpcm/adpcm.c -lpthread -no-pie	
clean:
	rm sample.elf sample.exe -f
