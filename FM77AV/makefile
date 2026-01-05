# --- 設定 ---
TARGET  = ds1
CC      = m6809-unknown-none-gcc-4.3.6
AS      = as6809
LD      = aslink
OBJCOPY = objcopy

CFLAGS  = -c
LIBPATH = /usr/local/lib/gcc/m6809-unknown-none/4.3.6/
LDFLAGS = -b .text=0x1600 -s -l $(LIBPATH)libgcc.a

# 生成される中間ファイルなどをまとめて定義
OBJFILES = crt0.rel $(TARGET).rel

# --- ルール ---

all: $(TARGET).d77

# 1. パターンルールによる簡略化
%.rel: %.c
	$(CC) $(CFLAGS) $< -o $@

%.rel: %.s
	$(AS) -l -o $@ $<

# 2. ホストPC（gcc）用ビルドのルール
%.exe: %.c
	gcc $< -o $@

# 3. ipl.bin の生成ルール
ipl.bin: ipl.s
	$(AS) -l -o ipl.rel $<
	$(LD) -b .text=0x0100 -m ipl.map ipl.rel -s
	$(OBJCOPY) -I srec -O binary ipl.s19 $@

# 4. メインターゲット
$(TARGET).d77: $(OBJFILES) ipl.bin create.exe
	$(LD) -m $(TARGET).map $(OBJFILES) $(LDFLAGS)
	$(OBJCOPY) -I srec -O binary $(TARGET).s19 $(TARGET).bin
	./create $(TARGET).d77 ipl.bin 1 $(TARGET).bin 2

# 依存関係
$(TARGET).rel: $(TARGET).c common.h inkey.h ds1_grp.h font.h
crt0.rel: crt0.s

clean:
	rm -f *.rel *.bin *.d77 *.map *.s19 *.exe
