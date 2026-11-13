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

    unsigned char palette[16][4];
    fread(palette, 4, 16, fp);
    fseek(fp, data_offset, SEEK_SET);

    unsigned char bmp_data[16][8];
    int is_bottom_up = (height > 0);
    int abs_height = (height < 0) ? -height : height;

    for (int y = 0; y < abs_height; y++) {
        int target_y = is_bottom_up ? (abs_height - 1 - y) : y;
        fread(bmp_data[target_y], 1, 8, fp);
    }
    fclose(fp);

    int pixel_index[16][16];
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 8; x++) {
            unsigned char b = bmp_data[y][x];
            pixel_index[y][x * 2]     = (b >> 4) & 0x0F;
            pixel_index[y][x * 2 + 1] = b & 0x0F;
        }
    }

    // 1. パレットから「黒」「白」「赤」を自動検出
    int black_idx = -1;
    int white_idx = -1;
    int red_idx = -1;

    for (int i = 0; i < 16; i++) {
        int b = palette[i][0];
        int g = palette[i][1];
        int r = palette[i][2];

        // 黒 (0, 0, 0 付近)
        if (b < 30 && g < 30 && r < 30 && black_idx == -1) {
            black_idx = i;
        }
        // 白 (200以上の高輝度)
        else if (b > 200 && g > 200 && r > 200 && white_idx == -1) {
            white_idx = i;
        }
        // 赤 (赤が高く、緑と青が低い)
        else if (r > 150 && g < 100 && b < 100 && red_idx == -1) {
            red_idx = i;
        }
    }

    // 見つからなかった場合のフォールバック
    if (black_idx == -1) black_idx = 0;
    if (white_idx == -1) white_idx = 1;
    if (red_idx == -1)   red_idx = 2;

    // 2. 黒・白・赤以外の色を左上から走査して「自由色」を1つ自動検出する
    int free_color_idx = -1;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int idx = pixel_index[y][x];
            if (idx != black_idx && idx != white_idx && idx != red_idx) {
                free_color_idx = idx;
                break;
            }
        }
        if (free_color_idx != -1) break;
    }
    // 画像内に別の色がない場合は白で代用
    if (free_color_idx == -1) {
        free_color_idx = white_idx;
    }

    // 3. カラーコードの割り当てマップ作成（C64の仕様に完全一致）
    int color_code_map[16] = {0};
    color_code_map[black_idx]      = 0; // 00: 黒 (背景色0)
    color_code_map[white_idx]      = 1; // 01: 白 (固定色1 / $D022)
    color_code_map[red_idx]        = 2; // 10: 赤 (固定色2 / $D023)
    color_code_map[free_color_idx] = 3; // 11: 自由色 (カラーRAM用)

    printf("// --- C64 Multicolor Strict Mapping ---\n");
    printf("// Code 0 (00): Black    (Palette Index %d) -> Background ($D021)\n", black_idx);
    printf("// Code 1 (01): White    (Palette Index %d) -> Fixed Color 1 ($D022)\n", white_idx);
    printf("// Code 2 (10): Red      (Palette Index %d) -> Fixed Color 2 ($D023)\n", red_idx);
    printf("// Code 3 (11): Free     (Palette Index %d) -> Color RAM ($D800)\n\n", free_color_idx);

    // 4. 2x2のキャラクタデータ（32バイト）の生成
    unsigned char c64_data[32];
    int byte_index = 0;

    for (int block_y = 0; block_y < 2; block_y++) {
        for (int block_x = 0; block_x < 2; block_x++) {
            for (int y = 0; y < 8; y++) {
                unsigned char row_byte = 0;
                
                for (int x = 0; x < 4; x++) {
                    int px_left  = block_x * 8 + (x * 2);
                    int px_right = block_x * 8 + (x * 2 + 1);
                    int py       = block_y * 8 + y;

                    int idx_left  = pixel_index[py][px_left];
                    int idx_right = pixel_index[py][px_right];

                    // ドット潰れ防止の優先選択（背景の黒より色を優先）
                    int chosen_idx = idx_left;
                    if (idx_left == black_idx && idx_right != black_idx) {
                        chosen_idx = idx_right;
                    }

                    int code = color_code_map[chosen_idx];
                    int shift = (3 - x) * 2;
                    row_byte |= (code & 0x03) << shift;
                }
                c64_data[byte_index++] = row_byte;
            }
        }
    }

    // プレビュー表示（. = 黒, W = 白, R = 赤, #: 自由色）
    printf("\n=== Preview (.:Black, W:White, R:Red, #:Free) ===\n");
    for (int block_y = 0; block_y < 2; block_y++) {
        for (int y = 0; y < 8; y++) {
            printf("  ");
            for (int block_x = 0; block_x < 2; block_x++) {
                int char_idx = block_y * 2 + block_x;
                unsigned char row_byte = c64_data[char_idx * 8 + y];
                
                for (int x = 0; x < 4; x++) {
                    int shift = (3 - x) * 2;
                    int code = (row_byte >> shift) & 0x03;
                    
                    char symbol = '.';
                    if (code == 1) symbol = 'W';
                    if (code == 2) symbol = 'R';
                    if (code == 3) symbol = '#';
                    
                    printf("%c%c", symbol, symbol);
                }
                printf(" printf ");
            }
            printf("\n");
        }
        printf("\n");
    }

    // 結果出力（C配列）
    printf("unsigned char c64_multicolor_chars[32] = {\n");
    for (int i = 0; i < 32; i++) {
        if (i % 8 == 0) printf("    ");
        printf("0x%02x, ", c64_data[i]);
        if ((i + 1) % 8 == 0) printf("// Character %d\n", i / 8);
    }
    printf("};\n\n");

    printf("// Color RAM values for 2x2 characters (Free color mapping):\n");
    printf("unsigned char c64_color_ram[4] = { %d, %d, %d, %d };\n", 
           free_color_idx, free_color_idx, free_color_idx, free_color_idx);

    return 0;
}