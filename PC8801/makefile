# ds1_88.d88 makefile for PC-8801 z88dk

TARGET = ds1_88

#マクロ
CC= zcc

#CFLAGS = +cpm -compiler sccz80 --opt-code-speed
#CFLAGS = +cpm -compiler sdcc --opt-code-speed --sdcccall0
CFLAGS = +pc88 -compiler sdcc --opt-code-speed --sdcccall0
# --opt-code-size
# --sdcccall0
# -DNODLAY -lm

LINKOPTS = +cpm -create-app -lm
LINKOPTS2 = +pc88 -create-app -subtype=disk -lm
# -vn

# Cソ－スからobj作成

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $< -c

#生成手順

all:$(TARGET).d88
#COM

clean:
	rm $(TARGET).o $(TARGET).d88
# $(TARGET).COM

OBJFILES = $(TARGET).o

#$(TARGET).COM : $(OBJFILES)
$(TARGET).d88 : $(OBJFILES)
#	$(CC) $(LINKOPTS) $(OBJFILES)  -o $(TARGET).COM -m -v
	$(CC) $(LINKOPTS2) $(OBJFILES)  -o $(TARGET).d88 -m -v

$(TARGET).o : $(TARGET).c inkey.h common.h ds1_88.h font88.h
