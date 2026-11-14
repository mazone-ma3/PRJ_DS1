# --- 設定 ---
TARGET      = ds1_amg
CC          = m68k-amigaos-gcc
AS          = m68k-amigaos-as
#SHELL = cmd.exe

# オプション: -Os (サイズ最適化), -m68000, -msoft-float (FPUなし)
CFLAGS      = -Os -m68000 -msoft-float
# リンク: -noixemul (標準ライブラリ依存を避ける), マップファイル出力
LDFLAGS     = -noixemul -Wl,-Map,$(TARGET).map -m68000

# リソースファイル（adf.batに渡すもの）
RESOURCES   = ds1_8bit.sc5 FONTYOKO.SC5
# se1.raw

# 中間ファイル
OBJS        = $(TARGET).o
# muldiv.o

# --- ルール ---

all: $(TARGET)

# 1. パターンルール (.c -> .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 2. パターンルール (.s -> .o)
%.o: %.s
	$(AS) -o $@ $<

# 3. リンク（実行ファイルの生成）
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
#	@echo "--- Creating Amiga Disk Image (ADF) ---"
	adf.bat $@ $(RESOURCES)

# --- 依存関係 ---
$(TARGET).o: $(TARGET).c
#muldiv.o: muldiv.s

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).map
