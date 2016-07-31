# Makefile that compiles mike-os

CC=gcc
INCLUDE_DIR:=../include
INCLUDE_FLAG:=-I$(INCLUDE_DIR)

ASM_FLAGS:=-g -f elf32
GCC_FLAGS:=-std=gnu11 -g -ffreestanding -masm=intel -fno-stack-protector 
GCC_FLAGS:= $(GCC_FLAGS) -m32 -c $(INCLUDE_FLAG)

LD_FLAGS:=-m elf_i386 -T 
LD_SCRIPT:=link.ld

KERNEL_NAME:=kernel
KERNEL_OUTPUT_DIR:=../output/
KERNEL_PATH:=$(KERNEL_OUTPUT_DIR)$(KERNEL_NAME)

C_OBJECTS+=$(shell find ../ -name '*.c')
ASM_OBJECTS:=$(shell find . -name '*.asm')

COMPILATION_DIR:=./compilation
VPATH:=drivers/keyboard:drivers/ata:port_io::mm:../libc:logging:multitask

C_OBJECT_FILES:=$(patsubst %.c, $(COMPILATION_DIR)/%.o, $(notdir $(C_OBJECTS)) )
ASM_OBJECT_FILES:=$(patsubst %.asm, $(COMPILATION_DIR)/%.o, $(notdir $(ASM_OBJECTS)))

HARD_DRIVE_PATH:= ../output/hard_drive.img

.PHONY:
	assemble
	compile
	all
	clean
	run

# Main compilation rule
all: link $(HARD_DRIVE_PATH)

$(HARD_DRIVE_PATH):
	dd if=/dev/zero of=$(HARD_DRIVE_PATH) bs=4096 count=1

$(COMPILATION_DIR):
	mkdir $(COMPILATION_DIR)

$(KERNEL_OUTPUT_DIR):
	mkdir $(KERNEL_OUTPUT_DIR)

$(COMPILATION_DIR)/%.o : %.c
	$(CC) $(GCC_FLAGS) -o $@ $<

$(COMPILATION_DIR)/%.o : %.asm
	nasm $(ASM_FLAGS) -o $@ $<

assemble: $(ASM_OBJECT_FILES)
compile: $(C_OBJECT_FILES)

# Link and remove object files
link: $(KERNEL_OUTPUT_DIR) $(COMPILATION_DIR) compile assemble
	ld $(LD_FLAGS) $(LD_SCRIPT) -o $(KERNEL_PATH) $(C_OBJECT_FILES) $(ASM_OBJECT_FILES)
	rm -rf $(COMPILATION_DIR)

clean: 
	rm $(KERNEL_PATH)

run:
	qemu-system-i386 -kernel $(KERNEL_PATH) -hda $(HARD_DRIVE_PATH)

run_debug: 
	qemu-system-i386 -kernel $(KERNEL_PATH) -monitor telnet:127.0.0.1:1234,server,nowait -d cpu_reset -hda $(HARD_DRIVE_PATH)
