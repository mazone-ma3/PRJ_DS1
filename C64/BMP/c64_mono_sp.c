#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <16x16_16color.bmp>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("File open failed");
        return 1;
    }

    // BMPヘッダ読み込み (54バイト)
    unsigned char header[54];
    if (fread(header, 1, 54, fp) != 54) {
        printf("Error: Invalid BMP file.\n");
        fclose(fp);
        return 1;
    }

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bpp = *(unsigned short*)&header[28];
    int data_offset = *(int*)&header[10];

    if (bpp != 4 || width != 16 || (height != 16 && height != -16)) {
        printf("Error: Must be a 16x16 pixel, 4-bit (16-color) BMP file.\n");
        fclose(fp);
        return 1;
    }

    // カラーパレット読み込み (16色分 × 4バイト)
    unsigned char palette[16][4];
    fread(palette, 4, 16, fp);

    // ピクセルデータへ移動
    fseek(fp, data_offset, SEEK_SET);

    unsigned char bmp_data[16][8];
    int is_bottom_up = (height > 0);
    int abs_height = (height < 0) ? -height : height;

    for (int y = 0; y < abs_height; y++) {
        int target_y = is_bottom_up ? (abs_height - 1 - y) : y;
        fread(bmp_data[target_y], 1, 8, fp);
    }
    fclose(fp);

    // パレットインデックスを展開 (16x16)
    int pixel_index[16][16];
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 8; x++) {
            unsigned char b = bmp_data[y][x];
            pixel_index[y][x * 2]     = (b >> 4) & 0x0F;
            pixel_index[y][x * 2 + 1] = b & 0x0F;
        }
    }

    // 画像内で使われている「黒以外」の色をすべて自動検出する
    // （※黒 = パレット上でRGBが全て0のインデックス、またはインデックス0）
    int colors[16];
    int color_count = 0;

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int idx = pixel_index[y][x];
            
            // 黒(RGB=0,0,0)は背景扱いとして除外する場合
            if (palette[idx][0] == 0 && palette[idx][1] == 0 && palette[idx][2] == 0) {
                // 必要に応じて黒も抽出したい場合はこのif文を外してください
                continue; 
            }

            // 重複チェックしてリストに追加
            int exists = 0;
            for (int i = 0; i < color_count; i++) {
                if (colors[i] == idx) {
                    exists = 1;
                    break;
                }
            }
            if (!exists) {
                colors[color_count++] = idx;
            }
        }
    }

    printf("// Detected %d unique colors (excluding black) in image.\n\n", color_count);

    // 検出された色ごとにC64モノクロスプライトデータ(63バイト)を生成
    for (int c = 0; c < color_count; c++) {
        int target_color = colors[c];
        unsigned char sprite_data[63] = {0}; // 24x21 = 63バイト初期化
        int byte_idx = 0;

    // C64スプライトは 24x21ピクセル (幅3バイト × 高さ21行)
    for (int y = 0; y < 21; y++) {
        for (int bx = 0; bx < 3; bx++) {
            unsigned char row_byte = 0;
            
            // 1バイトあたり8ピクセル (bx * 8 から bx * 8 + 7)
            for (int bit = 0; bit < 8; bit++) {
                int px = bx * 8 + bit;
                int py = y;

                // 16x16の範囲内であり、かつ対象の色である場合にビットを立てる
                if (px < 16 && py < 16) {
                    if (pixel_index[py][px] == target_color) {
                        row_byte |= (1 << (7 - bit));
                    }
                }
            }
            sprite_data[byte_idx++] = row_byte;
        }
    }

        // 結果を出力 (C言語配列)
        printf("// Color Index %d (Palette B=%d, G=%d, R=%d)\n", 
               target_color, palette[target_color][0], palette[target_color][1], palette[target_color][2]);
        printf("unsigned char sprite_color_%d[63] = {\n", target_color);
        for (int i = 0; i < 63; i++) {
            if (i % 8 == 0) printf("    ");
            printf("0x%02x, ", sprite_data[i]);
            if ((i + 1) % 8 == 0) printf("// Row %d\n", i / 3);
        }
        printf("};\n\n");
    }

    return 0;
}