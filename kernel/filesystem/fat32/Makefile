
CFLAGS += -I .


kernel_fs_fat32_objs:= $(shell find ./*.c)


ECHO:
	@echo "$@"


$(kernel_fs_fat32_objs): ECHO
	gcc $(CFLAGS) -c $@ -o $@.o


all: $(kernel_fs_fat32_objs)

