// ds1_full.c - Dragon Sword 1 Complete Prototype (MSX1 / z88dk)
// 2025/12/26 Compiled

#include <msx.h>
#include <msx/gfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_ADDRESS 0x1800

void copy_to_vram(char *src, char *dst, int size)
{
	while(size--)
		vpoke(dst++, *(src++));
}

void fill_vram(int size, char pattern)
{
	char *adr = BASE_ADDRESS;
	while(size--)
		vpoke(adr++, pattern);
}

void beep(void)
{
__asm
	push	af
	push	hl
	push	bc
	push	de
	push	ix
	push	iy
	call 00c0h
	pop	iy
	pop	ix
	pop	de
	pop	bc
	pop	hl
	pop	af
__endasm;
}

void put_chr(int x, int y, char chr) {
	vpoke(BASE_ADDRESS + x + y * 32, chr);
}

// VRAM直書き
void print_at(int x, int y, char *str) {
	char chr;
	if((x < 0) || (y < 0))
		return;
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
			put_chr(x++, y, chr);
	}
}

// vsync (jiffy使用)
unsigned char *jiffy = (unsigned char *)0xfc9e, old_jiffy;
void vsync(void) {
	old_jiffy = *jiffy;
	while (*jiffy == old_jiffy);
}

void wait(int j) {
	int i;
	for (i = 0; i < j; ++i)
		vsync();
}

void cls(void) {
	int i,j;
	for(j = 0l; j < 24; j++)
		for(i = 0; i < 32; ++i)
			put_chr(i, j, ' ');
}

// タイルパターン定義 (8x8)
unsigned char tile_wall[8]	= {0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF};   // 0:壁
unsigned char tile_floor[8]   = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};   // 1:床
unsigned char tile_player[8]  = {0x18,0x3C,0x7E,0xDB,0xFF,0x66,0x66,0xFF};   // 2:プレイヤー
unsigned char tile_normal[8]  = {0xFF,0xFF,0xDB,0xDB,0xDB,0xDB,0xFF,0xFF};   // 3:通常パネル（S） - シンプルブロック
unsigned char tile_gravity[8] = {0xFF,0xFF,0xDB,0xDB,0x7E,0x3C,0x18,0xFF};   // 4:重力パネル（B） - 下向き矢印付き
unsigned char tile_goal[8]	= {0x3C,0x42,0x81,0x81,0x81,0x81,0x42,0x3C};   // 5:ゴール
unsigned char tile_slime[8]   = {0x18,0x3C,0x7E,0x99,0x7E,0x3C,0x18,0x00};   // 6:スライム

void define_tiles() {
	copy_to_vram(tile_wall,	0x0000, 8);  // 0
	copy_to_vram(tile_floor,   0x0008, 8);  // 1
	copy_to_vram(tile_player,  0x0010, 8);  // 2
	copy_to_vram(tile_normal,  0x0018, 8);  // 3:通常パネル
	copy_to_vram(tile_gravity, 0x0020, 8);  // 4:重力パネル
	copy_to_vram(tile_goal,	0x0028, 8);  // 5
	copy_to_vram(tile_slime,   0x0030, 8);  // 6
}

// 定数
#define MAP_W 10
#define MAP_H 10
#define MAX_PANELS 10
#define MAX_STAGES 3

// 複数ステージ
const char *levels[MAX_STAGES] = {
	"##########"
	"#P.......#"
	"#.###S...#"
	"#.S...B..#"
	"#.###.####"
	"#.S......#"
	"#.###.####"
	"#........#"
	"#........#"
	"#####G####",

	"##########"
	"#P..S....#"
	"#.###....#"
	"#..B.S...#"
	"#.###.####"
	"#....S...#"
	"#.###.####"
	"#........#"
	"#....,...#"
	"#####G####",

	"##########"
	"#P.......#"
	"#.###S####"
	"#..B.....#"
	"#.### ####"
	"#........#"
	"#.### ####"
	"#....S...#"
	"#........#"
	"#####G####"
};

// グローバル変数
int player_x = -1, player_y = -1;
int gravity_x = -1, gravity_y = -1;
int panels_x[MAX_PANELS], panels_y[MAX_PANELS];
int panel_count = 0;
int goal_x, goal_y;

int player_hp = 20;
int player_atk = 5;
int exp = 0;
int level = 1;
int game_mode = 0;  // 0:パズル, 1:バトル
char battle_msg[40];
int current_stage = 0;

int enemy_hp, enemy_atk;
int old_player_x = -1, old_player_y = -1;
int old_gravity_x = -1, old_gravity_y = -1;
int old_panels_x[MAX_PANELS], old_panels_y[MAX_PANELS];

// 関数
void parse_map();
int can_move(int x, int y);
int try_move(int dx, int dy);
void gravity_fall();
void draw_background();
void update_objects();
void start_battle();
void update_battle();
void play_sound_effect();

void main() {
	int i;
	for(i = 0; i < MAX_PANELS; ++i){
		old_panels_x[i] = old_panels_y[i] = -1;
	}
	msx_color(15, 1, 1);
	set_mode(1);
	define_tiles();
	fill_vram(0x01, 32*24);  // 初回クリア（床で埋める）

	parse_map();
	draw_background();
	update_objects();

	while (1) {
		if (game_mode == 0) {
			gravity_fall();

			if ((get_stick(0) == 3) && try_move(1, 0)) { wait(4); update_objects(); play_sound_effect(); }
			if ((get_stick(0) == 7) && try_move(-1, 0)) { wait(4); update_objects(); play_sound_effect(); }
			if ((get_stick(0) == 5) && try_move(0, 1)) { wait(4); update_objects(); play_sound_effect(); }
			if ((get_stick(0) == 1) && try_move(0, -1)) { wait(4); update_objects(); play_sound_effect(); }

			if (get_stick(0) && ((rand() % 100) < 5)) {
				start_battle();
			}
		} else {
			update_battle();
		}

		vsync();
	}
}

void parse_map() {
	static int x, y;
	panel_count = 0;
	const char *current_level = levels[current_stage];
	for (y = 0; y < MAP_H; y++) {
		for (x = 0; x < MAP_W; x++) {
			char c = current_level[y * MAP_W + x];
			if (c == 'P') { player_x = x; player_y = y; }
			if (c == 'B') { gravity_x = x; gravity_y = y; }
			if (c == 'S') {
				if (panel_count < MAX_PANELS) {
					panels_x[panel_count] = x;
					panels_y[panel_count] = y;
					panel_count++;
				}
			}
			if (c == 'G') { goal_x = x; goal_y = y; }
		}
	}
}

int can_move(int x, int y) {
	if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
	if (levels[current_stage][y * MAP_W + x] == '#') return 0;
	if (gravity_x == x && gravity_y == y) return 0;

	static int i;
	for (i = 0; i < panel_count; i++) {
		if (panels_x[i] == x && panels_y[i] == y) return 0;
	}
	return 1;
}

int try_move(int dx, int dy) {
	static int nx, ny;
	nx = player_x + dx;
	ny = player_y + dy;

	if (gravity_x == nx && gravity_y == ny) {
		int gnx = nx + dx;
		int gny = ny + dy;
		if (can_move(gnx, gny)) {
			gravity_x = gnx;
			gravity_y = gny;
			player_x = nx; player_y = ny;
			return 1;
		}
		return 0;
	}

	static int i;
	for (i = 0; i < panel_count; i++) {
		if (panels_x[i] == nx && panels_y[i] == ny) {
			int pnx = nx + dx;
			int pny = ny + dy;
			if (can_move(pnx, pny)) {
				panels_x[i] = pnx;
				panels_y[i] = pny;
				player_x = nx; player_y = ny;
				return 1;
			}
			return 0;
		}
	}

	if (can_move(nx, ny)) {
		player_x = nx; player_y = ny;
		return 1;
	}
	return 0;
}

void gravity_fall() {
	if (gravity_x == -1) return;

	if (gravity_y + 1 < MAP_H && can_move(gravity_x, gravity_y + 1)) {
		gravity_y++;
		play_sound_effect();

		if (gravity_x == goal_x && gravity_y == goal_y) {
			update_objects();
			print_at(10, 10, "STAGE CLEAR!!");
			wait(60);
			cls();
			current_stage++;
			if (current_stage >= MAX_STAGES) current_stage = 0;  // ループ
			parse_map();
			draw_background();
		}
			update_objects();
	}
}

void draw_background() {
	static int x, y;
	for (y = 0; y < MAP_H; y++) {
		for (x = 0; x < MAP_W; x++) {
			char c = levels[current_stage][y * MAP_W + x];
			if (c == '#') put_chr(x * 2 , y, 0);
			else if (c == 'G') put_chr(x * 2, y, 5);
			else put_chr(x * 2, y, 1);
		}
	}
}

void update_objects() {
	static int i;

	// 前の位置を床に戻す
	if (old_player_x >= 0) put_chr(old_player_x * 2 , old_player_y, 1);
	if (old_gravity_x >= 0) put_chr(old_gravity_x * 2, old_gravity_y, 1);
	for (i = 0; i < panel_count; i++) {
		if (old_panels_x[i] >= 0) put_chr(old_panels_x[i] * 2, old_panels_y[i], 1);
	}

	// 新しい位置に描画
	if (player_x >= 0)
		put_chr(player_x * 2, player_y, 2);  // プレイヤー

	if (gravity_x >= 0) put_chr(gravity_x * 2, gravity_y, 4);  // 重力パネル（B）

	for (i = 0; i < panel_count; i++) {
		put_chr(panels_x[i] * 2, panels_y[i], 3);  // 通常パネル（S）
	}

	// 位置保存
	old_player_x = player_x; old_player_y = player_y;
	old_gravity_x = gravity_x; old_gravity_y = gravity_y;
	for (i = 0; i < panel_count; i++) {
		old_panels_x[i] = panels_x[i];
		old_panels_y[i] = panels_y[i];
	}
}

void start_battle() {
	game_mode = 1;
	enemy_hp = 10 + (level - 1) * 5;
	enemy_atk = 3 + (level - 1) * 2;
	strcpy(battle_msg, "Slime appeared!");
	cls();
	// スライム表示（中央上部に）
	put_chr(16, 4, 6);  // スライムキャラコード6
	print_at(5, 8, battle_msg);
	print_at(5, 12, "Press SPACE to attack");
}

void update_battle() {
	if (get_trigger(0)) {
		enemy_hp -= player_atk;
		if (enemy_hp <= 0) {
			exp += 10;
			if (exp >= level * 20) {
				level++;
				exp = 0;
				player_hp += 5;
				player_atk += 2;
				print_at(5, 14, "Level Up!");
				wait(30);
			}
			cls();
			strcpy(battle_msg, "Enemy defeated!");
			print_at(5, 10, battle_msg);
			wait(60);
			cls();
			game_mode = 0;
			draw_background();
			update_objects();
		} else {
			player_hp -= enemy_atk;
			sprintf(battle_msg, "You took %d damage! HP:%d", enemy_atk, player_hp);
			cls();
			// スライム再描画（バトル継続時）
			put_chr(16, 4, 6);
			print_at(5, 8, "Enemy defeated? No!");
			print_at(5, 10, battle_msg);
			print_at(5, 12, "Press SPACE to attack");
		}
		play_sound_effect();
		wait(10);
	}
}

void play_sound_effect() {
	beep();  // シンプルなビープ音
}