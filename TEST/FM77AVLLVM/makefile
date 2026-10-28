# --- 設定 ---
TARGET  = ds1

BINPATH=/home/user/mc6809-toolchain-1.0-rc2-linux-x86_64/bin/

CC      = $(BINPATH)mc6809-clang
AS      = $(BINPATH)mc6809-clang
LD      = $(BINPATH)mc6809-ld
OBJCOPY = $(BINPATH)llvm-objcopy

CFLAGS  = -Os -c -fno-builtin-memcpy
LIBPATH=/home/user/mc6809-toolchain-1.0-rc2-linux-x86_64/lib/clang/24/lib/mc6809-unknown-unknown

# 生成される中間ファイルなどをまとめて定義
OBJFILES = crt0.rel $(TARGET).rel

# --- ルール ---

all: $(TARGET).d77

# 1. パターンルールによる簡略化
%.rel: %.c
	$(CC) $(CFLAGS) $< -o $@

%.rel: %.s
	$(AS) -c -o $@ $<

# 2. ホストPC（gcc）用ビルドのルール
%: %.c
	gcc $< -o $@

# 3. ipl.bin の生成ルール
ipl.bin: ipl.s
	$(AS) -c -o ipl.rel $<
	$(LD)  --image-base=0 -Ttext=0x100 -o ipl.o ipl.rel
	$(OBJCOPY) -I elf -O binary ipl.o $@

# 4. メインターゲット
$(TARGET).d77: $(OBJFILES) ipl.bin create
	$(LD) -T link.ld  -L$(LIBPATH) -lclang_rt.builtins -o $(TARGET).o $(OBJFILES) -Map=output.map
	$(OBJCOPY) -I elf -O binary $(TARGET).o $(TARGET).bin
	./create $(TARGET).d77 ipl.bin 1 $(TARGET).bin 2

# 依存関係
$(TARGET).rel: $(TARGET).c common.h inkey.h ds1_grp.h font.h
crt0.rel: crt0.s

clean:
	rm -f *.rel *.bin *.d77 *.map *.s19 *.exe
