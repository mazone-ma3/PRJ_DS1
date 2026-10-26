# --- 設定 ---
TARGET   = ds1

BINPATH=C:\WINAP\LLVM-Z80-23.1.3-r1-Windows-X64\bin

CC       = $(BINPATH)\clang
AS       = $(BINPATH)\llvm-mc
LD       = $(BINPATH)\ld.lld
OBJCOPY  = $(BINPATH)\llvm-objcopy

# コンパイルオプション
CFLAGS   =  --target=z80 -O1 -c

ASFLAGS  =  -filetype=obj -triple=z80

# リンクオプション（subtypeで切り替え）
LDFLAGS  = +msx -create-app -lmsxbios

LIBPATH= C:\WINAP\LLVM-Z80-23.1.3-r1-Windows-X64\lib\z80

# 中間ファイル
OBJS     = rom.o $(TARGET).o

# --- ルール ---

all: $(TARGET).rom

# 1. パターンルール（.c から .o を作成）
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS)  $< -o $@

# 2. ROM生成ルール
$(TARGET).rom: $(OBJS)
	$(LD) -T rom.ld $(LIBPATH)\z80_rt.a $(OBJS) -o $(TARGET).elf -Map=output.map
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).rom
	truncate -s 16384 $(TARGET).rom

# 依存関係
$(TARGET).o: $(TARGET).c inkey.h common.h
rom.o : rom.s

clean:
	rm -f *.o *.rom *.com *.bin *.map *.ihx *.cas
