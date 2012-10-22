
CSRC_ =	$(CSRC:.c=.c_)
ASRC_ =	$(ASRC:.s=.s_)

OBJ = $(CSRC:.c=.o) $(ASRC:.s=.o)
LST = $(CSRC:.c=.lst) $(ASRC:.s=.lst)

OPT = s

DEBUG =	dwarf-2

EXTRAINCDIRS =

CSTANDARD = -std=gnu99

CDEFS =
CINCS =

GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

CFLAGS = -mmcu=$(MCU) -I.
CFLAGS += -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall	-Wstrict-prototypes
#CFLAGS	+= -mcall-prologues
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)

ifdef SECTION_INITPORT

CFLAGS += -Wl,--section-start=.initport=$(SECTION_INITPORT)

endif

ifdef SECTION_BL

CFLAGS += -Wl,--section-start=.bl=$(SECTION_BL)

endif

ifdef SECTION_BLC

CFLAGS += -Wl,--section-start=.blc=$(SECTION_BLC)

endif

CFLAGS += $(GENDEPFLAGS)

ASFLAGS	= -mmcu=$(MCU) -I. -x assembler-with-cpp

MATH_LIB = -lm

EXTMEMOPTS =

LDFLAGS	= -Wl,-Map=$(TARGET).map,--cref
LDFLAGS	+= $(EXTMEMOPTS)
LDFLAGS	+= $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)

REMOVE = rm -f
CRC = ../../TOOLS/crc_time
CC = avr-gcc
DD = dd
OBJDUMP	= avr-objdump
OBJCOPY	= avr-objcopy
SIZE = avr-size
NM = avr-nm
SHELL =	sh
AWK = ../../TOOLS/mawk32
PERL = ../perl
PROG = avrdude
BIN2HEX	= ../../TOOLS/bin2hex
HEX2BIN	= ../../TOOLS/hex2bin

help:
	@echo
	@echo Available	options:
	@echo all	- compile changed files
	@echo ALL	- recompile all	files
	@echo flash	- program microcontroller
	@echo fuse_zq	- set fuses for	CRYSTAL
	@echo fuse_ext	- set fuses for	EXTERNAL CLOCK
	@echo clean	- remove temp files
	@echo WRITE FUSES BEFORE FLASH!!!!
	@echo

ALL: clean all

all: sizebefore	build sizeafter

ELFSIZE	= $(SIZE) -A $(TARGET).elf

sizebefore:
	@if test -f $(TARGET).elf; then	$(ELFSIZE); echo; fi

sizeafter:
	@if test -f $(TARGET).elf; then	$(ELFSIZE); echo; fi

build: c_ s_ c s o elf lss bin

bin: $(TARGET).bin
lss: $(TARGET).lss
elf: $(TARGET).elf
c_ : $(CSRC_)
s_ : $(ASRC_)
c: $(CSRC)
s: $(ASRC)
o: $(OBJ)

%.bin: %.elf
	$(OBJCOPY) -O binary -R	.eeprom	--gap-fill 0xff	--pad-to $(FLASHSIZE) $< 1.bin
ifdef	FLASHMAIN
	$(CRC) 0 $(FLASHMAIN) 1.bin 2.bin
	$(CRC) $(FLASHMAIN) $(FLASHSIZE) 2.bin $@
	$(REMOVE) 1.bin	2.bin
endif
ifndef	FLASHMAIN
	$(CRC) 0 $(FLASHSIZE) 1.bin $@
	$(REMOVE) 1.bin
endif
	$(BIN2HEX) $@ main.hex

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.c_: %.c
	$(AWK) -f ../scp.awk $<	> $@

%.s_: %.s
	$(AWK) -f ../scp.awk $<	> $@

%.s_ : %.c_
	$(CC) -S -x c $(CFLAGS)	$< -o $@

%.o : %.s_
	$(PERL)	../call-ret.pl $<
	$(CC) -c $(ASFLAGS) $< -o $@

%.elf: $(OBJ)
	$(CC) $(CFLAGS)	$^ --output $@ $(LDFLAGS)

clean:
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(ASRC_) $(CSRC:.c=.s_)
	$(REMOVE) $(CSRC_)
	$(REMOVE) *.[Bb][Aa][Kk]
	$(REMOVE) .dep/*

flash:
	$(PROG)	-c usbasp -p $(MCU_) -U	flash:w:main.hex
lock:
	$(PROG)	-c usbasp -p $(MCU_) -U	lock:w:0xfc:m

fuse_zq:
	$(PROG)	-c usbasp -p $(MCU_) -U	hfuse:w:0xc1:m
	$(PROG)	-c usbasp -p $(MCU_) -U	lfuse:w:0xAf:m

fuse_rc8:
fuse:
	$(PROG)	-c usbasp -p $(MCU_) -U	lfuse:w:0xA4:m
	$(PROG)	-c usbasp -p $(MCU_) -U	hfuse:w:0xd9:m

fuse_ext:
	$(PROG)	-c usbasp -p $(MCU_) -U	lfuse:w:0xA0:m
	$(PROG)	-c usbasp -p $(MCU_) -U	hfuse:w:0xd1:m

read:
	$(PROG)	-c usbasp -p $(MCU_) -U	flash:r:r_flash.hex:i
	$(PROG)	-c usbasp -p $(MCU_) -U	eeprom:r:r_eeprom.hex:i
	$(PROG)	-c usbasp -p $(MCU_) -U	eeprom:r:r_eeprom.bin:r

-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY:	all clean s c s_ c_ build elf o	lss sizebefore sizeafter flash bin fuse_zq fuse_rc8 fuse_ext help lock fuse
.SUFFIXES:
