# ds1.rom makefile for MSX1 z88dk

TARGET = ds1

#マクロ
CC= zcc

#CFLAGS = +msx -compiler sccz80 --opt-code-speed
CFLAGS = +msx -compiler sdcc --opt-code-speed --sdcccall0
# --opt-code-size
# --sdcccall0
# -DNODLAY -lm

LINKOPTS = +msx -create-app -subtype=msxdos -lndos -lmsxbios -DMALLOC
#LINKOPTS = +cpm -create-app
LINKOPTS2 = +msx -create-app -subtype=disk -zorg=0x83c0  -lmsxbios
LINKOPTS3 = +msx -create-app -subtype=rom -lmsxbios
# -vn -lm 

# Cソ－スからobj作成

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $< -c

#生成手順

all:$(TARGET).rom
#msx

clean:
	rm $(TARGET).o $(TARGET).rom

OBJFILES = $(TARGET).o

$(TARGET).rom : $(OBJFILES)
#	$(CC) $(LINKOPTS) $(OBJFILES)  -o $(TARGET).COM -m -v
#	$(CC) $(LINKOPTS2) $(OBJFILES) -o $(TARGET).bin -m
	$(CC) $(LINKOPTS3) $(OBJFILES) -o $(TARGET).rom -m

$(TARGET).o : $(TARGET).c inkey.h common.h
