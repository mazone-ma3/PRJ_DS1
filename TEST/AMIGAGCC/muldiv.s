.align 2
.global _my_mod
_my_mod:
    | --- 入力: d0 = a (dividend), d1 = b (divisor) ---
    | --- 出力: d0 = 余り (remainder) ---
    
    move.l  d2, -(sp)       | 作業用に d2 を保存
    move.l  d0, d2          | 符号判定用に a を退避
    
    | 1. 絶対値をとる
    tst.l   d0
    bpl.s   .pos1
    neg.l   d0
.pos1:
    tst.l   d1
    bpl.s   .pos2
    neg.l   d1
.pos2:
    | 2. 未符号除算を実行
    bsr.s   __udiv_core     | d0/d1 をそのまま使って計算
    
    | --- この時点で d1 = 余り ---
    
    | 3. 被除数 (d2) の符号に合わせて余りの符号を調整
    tst.l   d2
    bpl.s   .done
    neg.l   d1
.done:
    move.l  d1, d0          | 戻り値を d0 にセット
    move.l  (sp)+, d2       | d2 を復帰
    rts

| -----------------------------------------
| 内部用：32bit未符号除算コア
| 入力: d0=被除数, d1=除数
| 出力: d0=商, d1=余り
| -----------------------------------------
.global __udiv_core
__udiv_core:
    move.l  d3, -(sp)       | d3 を一時保存
    move.l  d1, d3          | d3 に除数を退避
    moveq   #0, d1          | 余り (d1) をクリア
    moveq   #31, d2         | ループ回数 (31から0まで32回)

.loop:
    add.l   d0, d0          | d0 を左シフト、溢れたビットが X フラグへ
    addx.l  d1, d1          | X フラグを d1 (余り) の下位ビットへ
    cmp.l   d3, d1          | 余り vs 除数
    bcs.s   .next
    sub.l   d3, d1          | 引けたら引く
    addq.l  #1, d0          | 商のビットを立てる
.next:
    dbra    d2, .loop

    
    move.l  (sp)+, d3       | d3 復帰
    rts


.align 2
.global _my_div
_my_div:
    | --- 入力: d0 = a (被除数), d1 = b (除数) ---
    | --- 出力: d0 = 商 (quotient) ---
    
    move.l  d2, -(sp)       | 作業用に d2 を保存
    
    | 1. 商の符号を決定するために a と b の XOR をとって保存
    move.l  d0, d2
    eor.l   d1, d2          | d2 の最上位ビットが「商の符号」になる
    
    | 2. 絶対値をとる (符号なし除算の準備)
    tst.l   d0
    bpl.s   .pos3
    neg.l   d0
.pos3:
    tst.l   d1
    bpl.s   .pos4
    neg.l   d1
.pos4:
    | 除数0チェック (必要なら)
    tst.l   d1
    beq.s   .div_zero

    | 3. 未符号除算を実行 (先ほど作ったコアを利用)
    bsr.s   __udiv_core     | d0=商, d1=余り が返る
    
    | 4. 保存しておいた XOR 結果 (d2) に基づいて商の符号を調整
    tst.l   d2
    bpl.s   .done2
    neg.l   d0              | 異符号同士の計算なら商をマイナスに
    
.done2:
    move.l  (sp)+, d2       | d2 復帰
    rts

.div_zero:
    moveq   #-1, d0         | 0除算時に返す値 (環境に合わせて)
    move.l  (sp)+, d2
    rts



.align 2
.global _my_mul
_my_mul:
    | --- 入力: d0 = a, d1 = b ---
    | --- 出力: d0 = a * b ---
    
    movem.l d2-d3, -(sp)    | 作業レジスタ退避
    
    | 1. 商の符号を決定 (a XOR b)
    move.l  d0, d2
    eor.l   d1, d2          | d2の最上位ビットに符号を保持
    
    | 2. 絶対値をとる
    tst.l   d0
    bpl.s   .pos5
    neg.l   d0
.pos5:
    tst.l   d1
    bpl.s   .pos6
    neg.l   d1
.pos6:
    | 3. 32bit * 32bit の筆算 (結果は下位32bitのみ)
    | a = (aH << 16) + aL, b = (bH << 16) + bL
    | a*b = (aL*bL) + (aH*bL << 16) + (aL*bH << 16) + ...
    
    move.l  d0, d3          | d3 = a
    swap    d3              | d3.w = aH
    mulu.w  d1, d3          | d3 = aH * bL
    
    move.l  d1, -(sp)       | bを一時保存
    swap    d1              | d1.w = bH
    mulu.w  d0, d1          | d1 = aL * bH
    
    add.w   d1, d3          | (aH*bL + aL*bH)
    swap    d3              | 上位16bitへ移動
    clr.w   d3              | 下位はクリア
    
    move.l  (sp)+, d1       | bを戻す
    mulu.w  d1, d0          | d0 = aL * bL (32bit結果)
    
    add.l   d3, d0          | 全て足し合わせる
    
    | 4. 符号の調整
    tst.l   d2
    bpl.s   .done3
    neg.l   d0
    
.done3:
    movem.l (sp)+, d2-d3    | レジスタ復帰
    rts
