# DS1.EXP makefile for FM TOWNS GCC (GCCクロスコンパイル開発環境)

# --- 設定 ---
TARGET       = ds1_tw
GCCROOT2     = C:\tgcc

BIN          = c:\winap\llvm\bin
BIN2         = $(GCCROOT2)\bin
LIB_PATH     = $(GCCROOT2)\lib
INC_PATH     = $(GCCROOT2)\include

# ツール定義
CC = $(BIN)\clang -target i386-elf  -nostdlib
LD = $(BIN)\clang -target i386-elf

AS           = $(BIN)\clang -target i386-elf
GENEXP       = $(BIN2)\genexp3

# フラグ定義
CCFLAGS2 =   -O0 -fno-ident -fno-asynchronous-unwind-tables  -IC:\msys64\usr\x86_64-pc-cygwin\include -fno-unwind-tables -fno-addrsig 
LDFLAGS2 =  -nostdlib -T link.ld

# 中間ファイル
OBJS         = $(TARGET).o

CRT0         = $(LIB_PATH)\crt0.o

# --- ルール ---

all: $(TARGET).EXP

# 1. パターンルール（.c -> .o）
# 中間ファイル .i や .s を自動で処理する流れを定義
%.o: %.c
	$(CC) -c $< -o $*.o $(CCFLAGS2)
#	$(CC) -S $< -o $*.s $(CCFLAGS2)

# 2. パターンルール（.s -> .o）
%.o: %.s
	$(AS) -o $@ $<

# 3. リンク（.bin生成）
$(TARGET).bin: $(OBJS)
	$(LD)  $(LDFLAGS2)  $(OBJS) -o $(TARGET).elf
	c:\tgcc\bin\objcopy --remove-section .comment -I elf32-i386 -O a.out-i386 $(TARGET).elf $(TARGET).bin

# 4. EXPファイル変換
$(TARGET).EXP: $(TARGET).bin
	$(GENEXP) $< $@

# --- 依存関係 ---
$(TARGET).o: $(TARGET).c common.h inkey.h

clean:
	rm -f *.o *.bin *.EXP
