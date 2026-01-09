# --- 設定 ---
TARGET   = ds1_98
CC       = wcl
AS       = wasm
LD       = wlink

# CFLAGS: -ms: Smallモデル, -0: 8086用, -os: サイズ最適化, -zl: ライブラリ参照削除, -fpc: 数学ライブラリ不要
CFLAGS   = -ms -0 -os -zp1 -zl -fpc -s -zc
ASFLAGS  = -ms -0

# 中間ファイル
OBJS     = $(TARGET).obj

# --- ルール ---

all: $(TARGET).xdf

# 1. パターンルール (.c -> .obj)
# wcl は -c オプションを付けないとリンクまで行こうとするので注意
%.obj: %.c
	$(CC) -c -fo=$@ $(CFLAGS) $<

# 2. パターンルール (.asm -> .obj)
%.obj: %.asm
	$(AS) $(ASFLAGS) -fo=$@ $<

# 3. iplのルール (.asm -> .bin)
ipl.bin : ipl.asm
	$(AS) $(ASFLAG) -fo=ipl.obj ipl.asm
	$(LD) file ipl.obj name ipl.bin form raw

# 4. リンク工程
# wlink の独自記法を整理
$(TARGET).xdf: start.obj $(OBJS) ipl.bin
	$(LD) file { start.obj $(OBJS) } name $(TARGET).bin option Map form raw
# 空の1.2MBイメージ作成
	dd if=/dev/zero of=$@ bs=1024 count=1232
# IPLを0セクタ目に
	dd if=ipl.bin of=$@ bs=1024 conv=notrunc seek=0
# メインを1セクタ目に
	dd if=$(TARGET).bin of=$@ bs=1024 conv=notrunc seek=1

# 5. おまけ .exeを作る場合(ソースの#define DEBUG2を削除しておく)
exe: $(TARGET).c
	$(CC) -c -fo=$(TARGET).obj -ms -bt=dos -ox -0 $<
	$(LD) system Dos option quiet name $(TARGET).exe file { $(OBJS) }

# --- 依存関係 ---
$(TARGET).obj: $(TARGET).c common.h inkey.h
start.obj : start.asm

clean:
	rm -f *.obj *.bin *.err *.map *.exe
