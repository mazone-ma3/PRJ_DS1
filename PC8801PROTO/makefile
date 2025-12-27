# ds1_88.com makefile for PC-8801 z88dk

TARGET = ds1_88

#マクロ
CC= zcc

#CFLAGS = +cpm -compiler sccz80 --opt-code-speed
CFLAGS = +cpm -compiler sdcc --opt-code-speed --sdcccall0
# --opt-code-size
# --sdcccall0
# -DNODLAY -lm

LINKOPTS = +cpm -create-app -lm
# -vn

# Cソ－スからobj作成

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $< -c

#生成手順

all:$(TARGET).COM

clean:
	rm $(TARGET).o $(TARGET).COM

OBJFILES = $(TARGET).o

$(TARGET).COM : $(OBJFILES)
	$(CC) $(LINKOPTS) $(OBJFILES)  -o $(TARGET).COM -m -v

$(TARGET).o : $(TARGET).c inkey.h common.h ds1_grp.h font.h
