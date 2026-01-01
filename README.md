パズルRPG Dragon Sword 1 リメイク版作成実験 読み物 By m@3

This software includes code generated with the assistance of [Gemini & Grok], an AI developed by [Google & xAI].

・MSX1版/X1版/PC-8801版

z88dk(Ver. 2.4で検証)でコンパイルしてください。実行にはX1/PC-8801はLSX-Dodgers(Gaku氏作)が必要です。別途用意してください。

・FM77AV版

gcc6809(4.3.6 branch)でコンパイルしてください。実行にはF-BASIC V3.3が必要です。別途用意してください。

データファイルは読みませんので実行バイナリ(とFM77AV版はBASICファイル)で動作します。MSX1版以外は起動するディスクイメージにツールで書き込んでください。

各機種には互換ROMが出ていますのでそちらで試せます(場所は探してください)。実機での動作は今のところ未検証です。

画像を差し替える場合は付属の各コンバータ(コンパイルしてください)でバイナリ生成後(ダミーでマスクデータを指定してください)、適当なツールでCのchar型配列形式に変換してください。

各コンバータの説明はコンバータのソースを参照してください。

↓WebMSXでMSX1版を実行

https://webmsx.org/cbios/?MACHINE=MSX1J&rom=https://github.com/mazone-ma3/PRJ_DS1/raw/refs/heads/main/MSX1/ds1.rom

<img width="544" height="480" alt="ds1_0002" src="https://github.com/user-attachments/assets/9b58d333-bcbf-452c-8e2d-79db4af87ca5" />
