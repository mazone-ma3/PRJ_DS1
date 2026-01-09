# --- 設定 ---
TARGET      = ds1_68e
CC          = m68k-xelf-gcc
AS          = m68k-xelf-as

# X68000(68000)ターゲット、最適化、文字コード変換（SJIS出力）
CFLAGS      = -m68000 -O2 -finput-charset=utf-8 -fexec-charset=cp932
# リンクオプション：0x6800番地開始、シンボル削除、数学ライブラリ
LDFLAGS     = -Ttext=0x6800 -s -lm

# 中間ファイル
XSPLIBFILES =  xsp2lib.o pcm8afnc.o
OBJS        = $(TARGET).o

# --- ルール ---

all: $(TARGET).xdf

# 1. パターンルール
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) -o $@ $<

# 2. リンク工程
# 結合（フラットバイナリ生成）
$(TARGET).xdf: crt0.o $(OBJS) boot.bin
	m68k-xelf-ld -T link.ld crt0.o $(TARGET).o -o $(TARGET).bin
# ディスクイメージの作成
# ddを使う例 (1.2MBの空のイメージを作り、そこに流し込む)
	dd if=/dev/zero of=$(TARGET).xdf bs=1024 count=1232
	dd if=boot.bin of=$(TARGET).xdf bs=1024 conv=notrunc seek=0
	dd if=$(TARGET).bin of=$(TARGET).xdf bs=1024 conv=notrunc seek=1

#$(TARGET).X: $(OBJS)
#	$(CC) $(LDFLAGS) -o $@ $(OBJS)

# 3. (おまけ) .XやXSP対応にする場合
x: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET).X $(OBJS)

xsp:  $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET)_XSP.X $(OBJS) $(XSPLIBFILES)

# --- 依存関係 ---
$(TARGET).o: $(TARGET).c
# inkey.h

crt0.o: crt0.s

boot.bin : boot_sector.s
# IPLローダーのビルド (フラットバイナリとして出力)
	m68k-xelf-gcc -c boot_sector.s -o boot_sector.o
	m68k-xelf-ld -Ttext=0x00E80000 --oformat binary boot_sector.o -o boot.bin

clean:
	rm -f $(TARGET).o $(TARGET).X crt0.o
