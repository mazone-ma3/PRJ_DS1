/* X68000 Custom IPL - Loader version */

	.section .text
	.global _start

	/* Simple short branch over "BPB" area - just bra (not bra.l or bra.s) */
	bra	 boot_start			   /* 60 xx - relative branch, assembler calculates offset */

	/* Dummy OEM name: 7 chars + space (mimic real Human68k style) */
	.ascii  "MYX68K "				/* 8 bytes total */

	/* Dummy BPB fields (little-endian manual bytes, X68000 realistic values) */
	.byte   0x00, 0x04			   /* Bytes per sector: 1024 */
	.byte   0x01					 /* Sectors per cluster: 1 */
	.byte   0x01, 0x00			   /* Reserved sectors: 1 */
	.byte   0x02					 /* Number of FATs: 2 */
	.byte   0x70, 0x00			   /* Root entries: 112 (typical for small disks) */
	.byte   0x00, 0x0C			   /* Total sectors (16bit): 3072 (dummy, 0x0C00 LE) */
	.byte   0xF8					 /* Media descriptor: fixed disk-like */
	.byte   0x03, 0x00			   /* Sectors per FAT: 3 */
	.byte   0x08, 0x00			   /* Sectors per track: 8 (X68000!) */
	.byte   0x02, 0x00			   /* Number of heads: 2 */
	.byte   0x00, 0x00, 0x00, 0x00   /* Hidden sectors: 0 */
	.byte   0x00, 0x00, 0x00, 0x00   /* Large sector count: 0 */

	/* Extended BPB */
	.byte   0x00					 /* Drive number */
	.byte   0x00					 /* Reserved */
	.byte   0x29					 /* Extended boot signature */
	.byte   0x78, 0x56, 0x34, 0x12   /* Volume ID */
	.ascii  "X68K FLOPPY"			/* Volume label (11 bytes) */
	.ascii  "FAT12   "			   /* Filesystem type (8 bytes) */

	/* Pad with NOPs */
	.fill   30, 1, 0x90

boot_start:
	/* Stack setup */
|	move.l  #0x00100000, %a7

_start:
	| 1. スタックポインタの初期化
	move.l  #0x002000, %sp
|	/* Stack setup */
|	move.l  #0x00100000, %sp |a7

	| 2. IOCS _B_READ ($42) で第2セクタをロード
	| 引数はスタックではなくレジスタで渡す (X68000 IOCS仕様)
	move.l  #0x0100000, %a1   | 転送先アドレス
	move.w  #0x9070, %d1	 | ドライブ番号 他(PDA & MODE)
	move.l  #0x03000002, %d2		  | 目的読み出し位置
	move.w  #30000, %d3		 | 読み込むバイト数
	
	move.w  #0x46, %d0	   | IOCS _B_READ
	trap	#15			  | 実行

done:
|    bra     done
	
	| 3. 読み込んだメインプログラムへジャンプ
	jmp	 0x0100000

| 256バイトのサイズ調整 (パディング)
.org 1024
