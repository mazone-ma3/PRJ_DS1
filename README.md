パズルRPG Dragon Sword 1 リメイク版作成実験 読み物 By m@3

This software includes code generated with the assistance of [Gemini & Grok], an AI developed by [Google & xAI].

スプライトの無い機種向けに作成したゲームです。過去にMSX1でMSX-BASICで作ったゲームを元にしています。重力のついたパネルを、出てくる敵を倒したり逃げたりしつつ、ゴールまで運んでください。

MSX1版は全体のプロトタイプとして作成したので今のところ他と画面が違ったりしてます。→寄せてみました。

何故かIPL起動実験の続き(実践編)になってます。

・MSX1版/X1版/PC-8801版

z88dk(Ver. 2.4で検証)でコンパイルしてください。X1版/PC-8801版は直接.d88を生成するように修正しました。

もしMSX1版が起動しない場合はスロット2に入れてみてください。

・FM77AV版

gcc6809(4.3.6 branch)でコンパイルしてください。直接.d77を生成するように修正しました。

・PC-9801版

OpenWatcom(Ver. 2.0で検証)でコンパイルしてください。直接.xdfを生成します。

・FM TOWNS版

OpenWatcom(Ver. 2.0で検証)でコンパイルしてください。直接.xdfを生成します。IPL版はうんづではキー入力に対応しません。

・X68000版

elf2x68kでコンパイルしてください。直接.xdfを生成します。



各機種には互換ROMが出ていますのでそちらで試せます(場所は探してください)。実機での動作は今のところ未検証です。

画像を差し替える場合は8bit版は付属の各コンバータ(コンパイルしてください)でバイナリ生成後(ダミーでマスクデータを指定してください)、適当なツール(TOOLにあるBIN2HEADERなど)でCのchar型配列形式に変換してください。

16/32bit版はTOOLのBMPTOOLSのBMP298でバイナリ生成後(PC-98用ですが共通です)、やはりC形式に変換して下さい。(容量を稼ぐため8色にして一部削っています)

各コンバータの説明はコンバータのソースを参照してください。

Pyxel版もWebの方にあります。

↓WebMSXでMSX1版を実行

https://webmsx.org/cbios/?MACHINE=MSX1J&rom=https://github.com/mazone-ma3/PRJ_DS1/raw/refs/heads/main/MSX1/BIN/ds1.rom

↓PC-8801版のスクリーンショット画像

<img width="640" height="400" alt="ds1test" src="https://github.com/user-attachments/assets/6c9270ea-ff4c-40f0-b95b-c6be3b23b770" />
