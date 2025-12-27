# ds1.b77 makefile for FM77AV GCC (gcc6809 4.3.6 blanch)

TARGET = ds1

#マクロ
CC = m6809-unknown-none-gcc-4.3.6

CFLAGS = -S

.SUFFIXES: .rel .s
.s.rel:
	as6809 -l -o $*.rel  $*.s

.SUFFIXES: .rel .c
.c.rel:
	$(CC) $(CFLAGS) $<
	as6809 -l -o $*.rel  $*.s

#生成手順
OBJNAME = $(TARGET)

all:$(OBJNAME).b77

clean:
	rm $(TARGET).rel $(TARGET).b77

OBJFILES = crt0.rel $(OBJNAME).rel
# /usr/local/lib/gcc/m6809-unknown-none/4.3.6/crt0.o

$(TARGET).b77 : $(OBJFILES)
	aslink -b .text=0x1600 -m $(OBJNAME).map $(OBJFILES) -s $(OBJNAME).s
	objcopy -I srec -O binary $(OBJNAME).s19 $(OBJNAME).bin
	fm7conv $(OBJNAME).bin $(OBJNAME).b77

$(TARGET).rel : $(TARGET).c common.h inkey.h ds1_grp.h font.h
crt0.rel : crt0.s
