パズルRPG Dragon Sword 1 リメイク版作成実験 読み物 By m@3

This software includes code generated with the assistance of [Gemini & Grok], an AI developed by [Google & xAI].

スプライトの無い機種向けに作成したゲームです。過去にMSX1でMSX-BASICで作ったゲームを元にしています。重力のついたパネルを、出てくる敵を倒したり逃げたりしつつ、ゴールまで運んでください。

MSX1版は全体のプロトタイプとして作成したので今のところ他と画面が違ったりしてます。→寄せてみました。

・MSX1版/X1版/PC-8801版

z88dk(Ver. 2.4で検証)でコンパイルしてください。実行にはX1/PC-8801はLSX-Dodgers(Gaku氏作)が必要です。別途用意してください。

もしMSX1版が起動しない場合はスロット2に入れてみてください。

・FM77AV版

gcc6809(4.3.6 branch)でコンパイルしてください。実行にはF-BASIC V3.3が必要です。別途用意してください。

データファイルは読みませんので実行バイナリ(とFM77AV版はBASICファイル)で動作します。MSX1版以外は起動するディスクイメージにツールで書き込んでください。

各機種には互換ROMが出ていますのでそちらで試せます(場所は探してください)。実機での動作は今のところ未検証です。

画像を差し替える場合は付属の各コンバータ(コンパイルしてください)でバイナリ生成後(ダミーでマスクデータを指定してください)、適当なツールでCのchar型配列形式に変換してください。

各コンバータの説明はコンバータのソースを参照してください。

↓WebMSXでMSX1版を実行

https://webmsx.org/cbios/?MACHINE=MSX1J&rom=https://github.com/mazone-ma3/PRJ_DS1/raw/refs/heads/main/MSX1/BIN/ds1.rom

↓PC-8801版のスクリーンショット画像

<img width="640" height="400" alt="ds1test" src="https://github.com/user-attachments/assets/6c9270ea-ff4c-40f0-b95b-c6be3b23b770" />
