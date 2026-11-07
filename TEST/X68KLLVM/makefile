# --- 設定 ---
TARGET      = ds1_68e
CC          = clang-24 -target m68k
AS          = clang-24 --target=m68k -mcpu=68000 -c
AS2         = $(AS)
#AS2         = m68k-linux-gnu-as

LD          = m68k-linux-gnu-gcc
LD2         = m68k-linux-gnu-ld
LD3         = clang-24

# X68000(68000)ターゲット、最適化、文字コード変換（SJIS出力）
CFLAGS      = -O0 -nostdlib -fno-pic -fno-pie  -mcpu=68000 

# 中間ファイル
OBJS        = $(TARGET).o

# --- ルール ---

all: $(TARGET).xdf

# 1. パターンルール
%.o: %.c
	$(CC) $(CFLAGS) -c  $< -o $*.o
#	$(CC) $(CFLAGS) -S  $< -o $*.asm

%.o: %.s
	$(AS)  $< -o $@

# 2. リンク工程
# 結合（フラットバイナリ生成）
$(TARGET).xdf: crt0.o $(OBJS) boot.bin

	$(LD) -N -T link.ld crt0.o $(TARGET).o -o $(TARGET).bin  -nostdlib  -lgcc  -Wl,-Map=Outmput.map -Wl,--verbose

#	$(LD3) -v --target=m68k -mcpu=68000 -T link.ld   crt0.o $(TARGET).o -o $(TARGET).bin -nostdlib -lgcc -Wl,-Map

# ディスクイメージの作成
# ddを使う例 (1.2MBの空のイメージを作り、そこに流し込む)
	dd if=/dev/zero of=$(TARGET).xdf bs=1024 count=1232
	dd if=boot.bin of=$(TARGET).xdf bs=1024 conv=notrunc seek=0
	dd if=$(TARGET).bin of=$(TARGET).xdf bs=1024 conv=notrunc seek=1

# --- 依存関係 ---
$(TARGET).o: $(TARGET).c common.h inkey.h

crt0.o: crt0.s

boot.bin : boot_sector.s
# IPLローダーのビルド (フラットバイナリとして出力)
	$(AS2) boot_sector.s -o boot_sector.o
	$(LD2) -Ttext=0x00E80000 --oformat binary boot_sector.o -o boot.bin

clean:
	rm -f $(TARGET).o $(TARGET).X crt0.o boot.bin
