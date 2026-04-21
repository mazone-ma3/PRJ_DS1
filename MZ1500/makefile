# --- 設定 ---
TARGET   = ds1mz
CC       = zcc

# MZ-1500用フラグ (SDCC, 速度優先, 引数渡しをsdcccall0に固定)
CFLAGS   = +mz -compiler sdcc --opt-code-speed --sdcccall0
LDFLAGS  = +mz -create-app -lm

# 中間ファイル
OBJS     = $(TARGET).o

# --- ルール ---

all: $(TARGET).d88

# 1. パターンルール
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# 2. メインターゲット (D88ファイル生成)
$(TARGET).d88: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) -m -v

# 依存関係
$(TARGET).o: $(TARGET).c inkey.h common.h ds1_pcg.h

# 3. おまけ：CPM版もテストしたい場合
cpm: $(OBJS)
	$(CC) +cpm -create-app -o $(TARGET).COM $(OBJS) -lm

clean:
	rm -f *.o *.d88 *.map *.COM *.ihx
