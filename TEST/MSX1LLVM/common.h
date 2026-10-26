// common.h 各機種共通ルーチン
//#define DEBUG

// 定数
#define MAP_W 12
#define MAP_H 12
//#define MAX_PANELS 10
#define MAX_STAGES 12

// 仮想VRAM（マップの可変状態を保持）
static char vram[MAP_H][MAP_W];  // 書き換え可能なマップ

const int level_experience[10] = {0, 20, 50, 90, 140, 200, 270, 350, 440, 540};

// ゲームモード
enum {
	MODE_PUZZLE,
	MODE_BATTLE,
	MODE_RETURN,
	MODE_TITLE,
	MODE_CLEAR
};

// 複数ステージ
/*
	"############"
	"#....PB....#"
	"#..........#"
	"#..........#"
	"#..........#"
	"#..........#"
	"#..........#"
	"#..........#"
	"#..........#"
	"#..........#"
	"#..........#"
	"######G#####",
*/

const char *levels[MAX_STAGES] = {
	"############"	// 1
	"#....PB....#"
	"##SSS.SHS.##"
	"##..S.S.#.##"
	"##....S.#.##"
	"##....S.#.##"
	"##.SS.S.#.##"
	"##..S.S.#.##"
	"##..S.S.#..#"
	"##SSS.SHS.##"
	"#.........##"
	"######G#####",

	"############"	// 2
	"#.#..PB.#..#"
	"#####.#.#..#"
	"#.S.S...#..#"
	"#.S.SSSH...#"
	"#..........#"
	"#..##...#.S#"
	"#.S.#.S.#.H#"
	"#.SS#...#.##"
	"#S..#.##...#"
	"#..S......##"
	"######G#####",

	"############"	// 3
	"###..PB....#"
	"##.........#"
	"#.S.#.#.####"
	"##S..##.#.##"
	"#..........#"
	"###...S.#.##"
	"##.#.S.#####"
	"##.S.###.S.#"
	"##S..#.#.S.#"
	"#..........#"
	"######G#####",

	"############"	// 4
	"#....PB....#"
	"#....##....#"
	"#........S##"
	"#.........##"
	"#....#...S##"
	"#...SSS....#"
	"#..S...S...#"
	"#..SS.SS...#"
	"#.#...S.#..#"
	"###.##HHH###"
	"######G#####",

	"############"	// 5
	"#....PB....#"
	"#..SSSSSS..#"
	"#.S......S.#"
	"#.S..SS..S.#"
	"#.S.S..S.S.#"
	"#.S.S..S.S.#"
	"#.S..SS..S.#"
	"#.S......S.#"
	"#..SSSSSS..#"
	"#..........#"
	"######G#####",

	"############"	// 6
	"#....PB....#"
	"#..........#"
	"#..........#"
	"#.....S...H#"
	"#...S.#.S.H#"
	"#.S.S.#.S.H#"
	"#.S.S.#.S.H#"
	"#.S.S###S.H#"
	"#.SSSSSSSSH#"
	"#........###"
	"######G#####",

	"############" // 7
	"#.###PB###.#"
	"###...H...##"
	"##..#SS#S.##"
	"##SS....S.##"
	"#.....#S..##"
	"#.....S..H.#"
	"#....SHS#..#"
	"#..........#"
	"#....#H#...#"
	"#....#H#...#"
	"######G#####",

	"############"	// 8
	"###..PB..#.#"
	"#..#S#S..#.#"
	"#.......#.S#"
	"#..#..##...#"
	"#...##.....#"
	"##....S.S..#"
	"#.S###.S.S##"
	"#..........#"
	"###H###SSSS#"
	"#..........#"
	"######G#####",

	"############"	// 9
	"#....PB....#"
	"#.S.S.H.S.S#"
	"#.S.S.S.S.S#"
	"#..S.S.#.S.#"
	"#..S.#.S.S.#"
	"#...S.S.S..#"
	"#...S.#.S..#"
	"#....S.S...#"
	"#....S.S...#"
	"#.....H....#"
	"######G#####",

	"############"	// 10
	"#....PBH...#"
	"#...SSHSS..#"
	"#..SS.H.SS.#"
	"#.S.S.H.S.S#"
	"#.SSSHSHSSS#"
	"#.SS.S.S.SS#"
	"#..SS...SS.#"
	"#...SS.SS..#"
	"#....SSS...#"
	"#..........#"
	"######G#####",

	"############"	// 11
	"#....PB....#"
	"#.....H....#"
	"#...#.H.#..#"
	"#...#.S.#.H#"
	"#...S.S.#.H#"
	"#.H.#.S.#.H#"
	"#.HHSH#SSSH#"
	"#.HH......H#"
	"#.HHS.SSSHH#"
	"#..##H...H.#"
	"######G#####",

	"############"	// 12
	"#.S..PB##..#"
	"#.S...H.H..#"
	"#.S...SSS..#"
	"#.H##.#.#..#"
	"#..........#"
	"##SH.#####.#"
	"##.....#...#"
	"####...S...#"
	"#..#...S...#"
	"####...S...#"
	"######G#####",

};

// グローバル変数
int player_x = -1, player_y = -1;
int gravity_x = -1, gravity_y = -1;
int panel_count = 0;
int goal_x, goal_y;

int player_hp = 20;
int player_atk = 5;
int experience = 0;
int level = 1;
int game_mode = MODE_TITLE;  // 0:パズル, 1:バトル
char battle_msg[40];
char *pbattle_msg;
int current_stage = 0;

int enemy_hp, enemy_atk;
int old_player_x = -1, old_player_y = -1;
int old_gravity_x = -1, old_gravity_y = -1;

char c;

// 関数
void parse_map(void);
int can_move(int x, int y);
int try_move(int dx, int dy);
void gravity_fall(void);
void draw_background(void);
void update_objects(void);
void start_battle(void);
void update_battle(void);

int quotient = 0;
int rem = 0;

int divideBy10(int n) {
	quotient = 0;
	rem = n;

	while (rem >= 10) {
		rem -= 10;
		quotient++;
	}
	return quotient;
}

int res;

int get_mod10(int n) {
//	int res;
	res = n - divideBy10(n) * 10;
	return (res >= 10) ? res - 10 : res;
}


int itoa2(int value, char *str) {
	char *p = str;
	int tmp = value;
	int size = 0;
	char *start;
	char t;

	if (value <= 0) { *p++ = '0'; *p = '\0'; return 0; }
	while (tmp) { *p++ = '0' + get_mod10(tmp); tmp = divideBy10(tmp); }
	*p-- = '\0';
	// 逆転
	start = str;
	while (start < p) {
		size++;
		t = *start; *start++ = *p; *p-- = t;
	}
	return size;
}

unsigned char simple_rnd(void) {
/*	static unsigned char r = 1;
	r = r * 37 + 41;  // 適当な定数
	return r;*/
	static unsigned char seed = 1;
	seed = (seed * 5) + 1;
	return seed;  // 0-255
}

int size;

int strcpy2(char *dst, char *src)
{
	size = 0;
	while(*src != '\0'){
		size++;
		*(dst++) = *(src++);
	}
	*dst = '\0';
	return size;
}

void init_status(void)
{
	player_hp = 20;
	player_atk = 5;
	experience = 0;
	level = 1;
	current_stage = 0;
}

void main2(void) {
	define_tiles();
//	fill_vram(0x01, 32*24);  // 初回クリア（床で埋める）
start:
//	parse_map();
//	draw_background();
//	update_objects();

	for (;;) {
		simple_rnd();
//		print_at(PRINT_MUL * 24, 10,"012345678");
//		print_at(PRINT_MUL * 24, 11,"ABCDEFG");
		if((game_mode != MODE_TITLE) && (game_mode != MODE_CLEAR)){
			pbattle_msg = battle_msg;
			pbattle_msg += strcpy2(pbattle_msg, "STAGE ");
			pbattle_msg += itoa2(current_stage+1, pbattle_msg);
			print_at(PRINT_MUL * 24, 0, battle_msg);

			pbattle_msg = battle_msg;
			pbattle_msg += strcpy2(pbattle_msg, "LEVEL ");
			pbattle_msg += itoa2(level, pbattle_msg);
			print_at(PRINT_MUL * 24, 1, battle_msg);

			pbattle_msg = battle_msg;
			pbattle_msg += strcpy2(pbattle_msg, "HP ");
			pbattle_msg += itoa2(player_hp, pbattle_msg);
			print_at(PRINT_MUL * 24, 2, battle_msg);

			pbattle_msg = battle_msg;
			pbattle_msg += strcpy2(pbattle_msg, "EXP ");
			pbattle_msg += itoa2(experience, pbattle_msg);
			print_at(PRINT_MUL * 24, 3, battle_msg);
		}

		switch(game_mode){
		case MODE_PUZZLE:
			print_at(PRINT_MUL * 24, 4, "GIVE UP");
			print_at(PRINT_MUL * 24, 5,  A_KEY " KEY" );
			print_at(PRINT_MUL * 24, 6, "TITLE");
			print_at(PRINT_MUL * 24, 7,  B_KEY " KEY" );
			gravity_fall();
			vsync();
			keycode = keyscan();
			if(!keycode){
				continue;
			}
			if (keycode & KEY_B){
#ifndef DEBUG
				game_mode = MODE_RETURN;
				cls();
#else
				gravity_x = goal_x; gravity_y = goal_y-1;
#endif
			}
			if (keycode & KEY_A) {
				print_at(PRINT_MUL * 0, 23, "Give Up");
				wait(60);
				cls();
				parse_map();
				draw_background();
				update_objects();
			}
			else if (keycode && (simple_rnd() < 6)) {  // 約2% (6/256)
#ifndef DEBUG
				start_battle();
#endif
			}else{
				if ((keycode == KEY_RIGHT1) && try_move(1, 0)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode == KEY_LEFT1) && try_move(-1, 0)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode == KEY_DOWN1) && try_move(0, 1)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode == KEY_UP1) && try_move(0, -1)) { wait(4); update_objects(); play_sound_effect(); }
			}
			break;
		case MODE_BATTLE:
			update_battle();
			break;

		case MODE_RETURN:
			print_at(PRINT_MUL * 2, 10, "HIT " A_KEY " KEY TO TITLE");
			print_at(PRINT_MUL * 2, 12, "HIT " B_KEY " KEY TO GAME");
			while(keyscan());
			for(;;){
				keycode = keyscan();
				if(keycode == KEY_A){
					game_mode = MODE_TITLE;
					cls();
					break;
				}else if(keycode == KEY_B){
					game_mode = MODE_PUZZLE;
					draw_background();
					update_objects();
					break;
				}
			}
			break;

		case MODE_TITLE:
			print_at(PRINT_MUL * 6, 5, "DRAGON SWORD PART 1");
			put_logo(PRINT_MUL * 8, 15);
			print_at(PRINT_MUL * 6, 18, "HIT " A_KEY " KEY TO START");
#ifndef DEBUG2
			print_at(PRINT_MUL * 6, 20, "HIT " B_KEY " KEY TO EXIT");
#endif
			while(keyscan());
			for(;;){
				keycode = keyscan();
				if(keycode == KEY_A){
					init_status();
					game_mode = MODE_PUZZLE;
					cls();
					parse_map();
					draw_background();
					update_objects();
					break;
				}else if(keycode == KEY_B){
					goto end;
				}
			}
			while(keyscan());
			break;

		case MODE_CLEAR:
			print_at(PRINT_MUL * 3, 10, "ALL CLEAR CONGRATULATIONS");
			wait(600);
			game_mode = MODE_TITLE;
			cls();
			break;
		}
	}
end:;
}

void parse_map(void) {
	static int x, y;
	const char *lvl = levels[current_stage];
	panel_count = 0;
	for (y = 0; y < MAP_H; y++) {
		for (x = 0; x < MAP_W; x++) {
			char c = lvl[y * MAP_W + x];
			vram[y][x] = c;  // 仮想VRAMにコピー
			if (c == 'P') { old_player_x = player_x = x; old_player_y = player_y = y; vram[y][x] = '.'; }
			if (c == 'B') { old_gravity_x = gravity_x = x; old_gravity_y = gravity_y = y; vram[y][x] = '.'; }

			if (c == 'G') { goal_x = x; goal_y = y; }
		}
	}
}

int can_move(int x, int y) {
	if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
	if (vram[y][x] == '#') return 0;
	if (vram[y][x] == 'H') return 0;
	if (vram[y][x] == 'S') return 0;
	if (gravity_x == x && gravity_y == y) return 0;

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
			return 1;
		}
		return 0;
	}
	if(vram[ny][nx] == 'S'){
		int pnx = nx + dx;
		int pny = ny + dy;
		if (can_move(pnx, pny)) {
			vram[ny][nx] = '.';
			vram[pny][pnx] = 'S';
			put_chr16(nx, ny, TILE_FLOOR);
			put_chr16(pnx, pny, TILE_NORMAL);
			return 1;
		}
	}
	else if(vram[ny][nx] != 'G'){
		if (can_move(nx, ny)) {
			player_x = nx; player_y = ny;
			return 1;
		}
	}
	return 0;
}

int nx,ny;
char below;

void gravity_fall(void) {
	if (gravity_x == -1) return;

	for(;;){
	if (gravity_y + 1 < MAP_H) {
		nx = gravity_x;
		ny = gravity_y + 1;
		below = vram[ny][nx];

		if (below != '#') {
			if (below == 'H') {
				vram[ny][nx] = '.';  // Hを消す
			}
			if (can_move(nx, ny)) {
				gravity_y++;
				play_sound_effect();

				if (gravity_x == goal_x && gravity_y == goal_y) {
					update_objects();
					print_at(PRINT_MUL * 6, 10, "STAGE CLEAR");
					wait(60);
					cls();
					player_hp = 20 + 5 * level;
					current_stage++;
					if (current_stage >= MAX_STAGES){
						game_mode = MODE_CLEAR;//current_stage = 0;
						return;
					}
					parse_map();
					draw_background();
					update_objects();
				} else {
					update_objects();  // 落ちた位置を更新
					wait(2);
				}
			}else{
				break;
			}
		}else{
			break;
		}
	}else{
		break;
	}
	}
}

void draw_background(void) {
	static int x, y;
	for (y = 0; y < MAP_H; y++) {
		for (x = 0; x < MAP_W; x++) {
			char c = vram[y][x];
			if (c == '#') put_chr16(x, y, TILE_WALL);
			else if (c == 'G') put_chr16(x, y, TILE_GOAL);
			else if (c == 'H') put_chr16(x, y, TILE_HOLLOW);
			else if (c == 'S') put_chr16(x, y, TILE_NORMAL);
			else put_chr16(x, y, TILE_FLOOR);
		}
	}
}

void update_objects(void) {
	static int i;

	// 前の位置を床に戻す
	if ((old_player_x != player_x) || (old_player_y != player_y)) put_chr16(old_player_x , old_player_y, TILE_FLOOR);
	if ((old_gravity_x != gravity_x) || (old_gravity_y != gravity_y)) put_chr16(old_gravity_x, old_gravity_y, TILE_FLOOR);

	// 新しい位置に描画
	if (player_x >= 0)
		put_chr16(player_x, player_y, TILE_PLAYER);  // プレイヤー

	if (gravity_x >= 0) put_chr16(gravity_x, gravity_y, TILE_GRAVITY);  // 重力パネル（B）

	// 位置保存
	old_player_x = player_x; old_player_y = player_y;
	old_gravity_x = gravity_x; old_gravity_y = gravity_y;
}

void start_battle(void) {
	game_mode = MODE_BATTLE;
	enemy_hp = 10 + (level - 1) * 5;
	enemy_atk = 3 + ((level - 1) * 3 / 2);
	strcpy2(battle_msg, "Slime appeared");
	cls();
	// スライム表示（中央上部に）
	put_chr16(8, 2, TILE_SLIME);  // スライムキャラコード6
	print_at(PRINT_MUL * 5, 8, battle_msg);
	print_at(PRINT_MUL * 5, 12, "Press  " A_KEY "  to attack");
	print_at(PRINT_MUL * 5, 14, "Press  " B_KEY "  to escape");
	wait(60);
}

void damage_battle(void) {
	player_hp -= enemy_atk;
	//sprintf(battle_msg, "You took %d damage! HP:%d", enemy_atk, player_hp);
	pbattle_msg = battle_msg;
	pbattle_msg += strcpy2(pbattle_msg, "You took ");
	pbattle_msg += itoa2(enemy_atk, pbattle_msg);
	pbattle_msg += strcpy2(pbattle_msg, " damage  HP ");
	pbattle_msg += itoa2(player_hp, pbattle_msg);
//	*pbattle_msg = '\0';
	cls();
	// スライム再描画（バトル継続時）
	put_chr16(8, 2, TILE_SLIME);
	print_at(PRINT_MUL * 5, 8, "Enemy defeated  No");
	print_at(PRINT_MUL * 5, 10, battle_msg);
	if(player_hp > 0){
		print_at(PRINT_MUL * 5, 12, "Press  " A_KEY "  to attack");
		print_at(PRINT_MUL * 5, 14, "Press  " B_KEY "  to escape");
	}else{
		wait(60);
		cls();
		print_at(PRINT_MUL * 10, 10, "You dead");
		player_hp = 20 + 5 * level;
		wait(60);
		cls();
		game_mode = MODE_PUZZLE;
		parse_map();
		draw_background();
		update_objects();
	}
}

void update_battle(void) {
	keycode = keyscan();
	if (keycode & KEY_A) {
		enemy_hp -= player_atk;
		if (enemy_hp <= 0) {
			experience += 10;
//			if (experience >= level * 20) {
			if (experience >= level_experience[level]) {
				level++;
				experience = 0;
				player_hp = 20 + 5 * level;
				player_atk += 2;
				print_at(PRINT_MUL * 5, 16, "Level Up");
				wait(30);
			}
			cls();
			strcpy2(battle_msg, "Enemy defeated");
			print_at(PRINT_MUL * 5, 10, battle_msg);
			wait(60);
			cls();
			game_mode = MODE_PUZZLE;
			draw_background();
			update_objects();
		} else {
			damage_battle();
		}
		play_sound_effect();
		wait(10);
	} else if (keycode & KEY_B) {  // 逃げる
		if ((simple_rnd() < 153)) {  // 約60%成功 (153/256)
			strcpy2(battle_msg, "Escaped");
			print_at(PRINT_MUL * 5, 16, battle_msg);
			wait(60);
			cls();
			game_mode = MODE_PUZZLE;
			draw_background();
			update_objects();
		} else {
			player_hp -= enemy_atk;
			strcpy2(battle_msg, "Failed to escape");
			print_at(PRINT_MUL * 5, 16, battle_msg);
			wait(60);
			cls();
			// ... ダメージ表示
			damage_battle();
		}
		play_sound_effect();
		wait(10);
	}
}
