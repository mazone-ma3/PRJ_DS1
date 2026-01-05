# --- 設定 ---
TARGET   = ds1
CC       = zcc

# コンパイルオプション（SDCCを使用）
CFLAGS   = +msx -compiler sdcc --opt-code-speed --sdcccall0
# リンクオプション（subtypeで切り替え）
LDFLAGS  = +msx -create-app -lmsxbios

# 中間ファイル
OBJS     = $(TARGET).o

# --- ルール ---

all: $(TARGET).rom

# 1. パターンルール（.c から .o を作成）
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# 2. ROM生成ルール
$(TARGET).rom: $(OBJS)
	$(CC) $(LDFLAGS) -subtype=rom -o $(TARGET) $(OBJS) -m -v

# 3. (おまけ) DOS用COMファイルやBINも作りたい場合
com: $(OBJS)
	$(CC) $(LDFLAGS) -subtype=msxdos -lndos -o $(TARGET).com $(OBJS)

bin: $(OBJS)
	$(CC) $(LDFLAGS) -subtype=disk -zorg=0x83c0 -o $(TARGET).bin $(OBJS)

# 依存関係
$(TARGET).o: $(TARGET).c inkey.h common.h

clean:
	rm -f *.o *.rom *.map *.com *.bin *.map *.ihx *.cas
