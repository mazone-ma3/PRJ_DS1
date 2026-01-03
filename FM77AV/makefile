# ds1.d77 makefile for FM77AV GCC (gcc6809 4.3.6 branch)

TARGET = ds1

#マクロ
CC = m6809-unknown-none-gcc-4.3.6

CFLAGS = -c
#-S

.SUFFIXES: .rel .s
.s.rel:
	as6809 -l -o $*.rel  $*.s

.SUFFIXES: .rel .c
.c.rel:
	$(CC) $(CFLAGS) $< -o $*.rel
#	as6809 -l -o $*.rel  $*.s

.SUFFIXES: .bin .s
.s.bin:
	as6809 -l -o $*.rel  $*.s
	aslink -b .text=0x0100 -m $*.map  $*.rel -s
	objcopy -I srec -O binary $*.s19 $*.bin

.SUFFIXES: .exe .c
.c.exe:
	gcc $*.c -o $*.exe

#生成手順
OBJNAME = $(TARGET)

all:$(OBJNAME).d77

clean:
	rm $(TARGET).rel $(TARGET).d77 ipl.bin

OBJFILES = crt0.rel $(OBJNAME).rel
# /usr/local/lib/gcc/m6809-unknown-none/4.3.6/crt0.o
LIBPATH = /usr/local/lib/gcc/m6809-unknown-none/4.3.6/

$(TARGET).d77 : $(OBJFILES) ipl.bin create.exe
#fm7conv.exe 
#	aslink -b .text=0x1600 -m $(OBJNAME).map  $(OBJFILES) -s $(OBJNAME).s -l $(LIBPATH)libgcc.a
	aslink -b .text=0x1600 -m $(OBJNAME).map  $(OBJFILES) -s -l $(LIBPATH)libgcc.a
	objcopy -I srec -O binary $(OBJNAME).s19 $(OBJNAME).bin
#	fm7conv $(OBJNAME).bin $(OBJNAME).b77
	create $(OBJNAME).d77 ipl.bin 1 ds1.bin 2

$(TARGET).rel : $(TARGET).c common.h inkey.h ds1_grp.h font.h
ipl.bin : ipl.s
crt0.rel : crt0.s

#fm7conv.exe : fm7conv.c
create.exe : create.c
