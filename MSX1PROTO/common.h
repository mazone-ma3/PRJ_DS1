// common.h 各機種共通ルーチン

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
	"#.###H####"
	"#.S......#"
	"#.###H####"
	"#........#"
	"#........#"
	"#####G####",

	"##########"
	"#P..S....#"
	"#.###....#"
	"#..B.S...#"
	"#.###.####"
	"#....S...#"
	"#.###H####"
	"#........#"
	"#....,...#"
	"#####G####",

	"##########"
	"#P.......#"
	"#.###S####"
	"#..B.....#"
	"#.###H####"
	"#........#"
	"#.###H####"
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

void main2(void) {
	int i;
	for(i = 0; i < MAX_PANELS; ++i){
		old_panels_x[i] = old_panels_y[i] = -1;
	}
	define_tiles();
	fill_vram(0x01, 32*24);  // 初回クリア（床で埋める）

	parse_map();
	draw_background();
	update_objects();

	while (1) {
		sprintf(battle_msg, "STAGE:%d", current_stage+1);
		print_at(PRINT_MUL * 0, 20, battle_msg);
		sprintf(battle_msg, "LEVEL:%d", level);
		print_at(PRINT_MUL * 0, 21, battle_msg);
		sprintf(battle_msg, "HP:%d", player_hp);
		print_at(PRINT_MUL * 0, 22, battle_msg);

		if (game_mode == 0) {
			gravity_fall();

			keycode = keyscan();
			if (keycode & KEY_A) {
				print_at(PRINT_MUL * 0, 23, "Give Up");
				wait(60);
				cls();
				parse_map();
				draw_background();
				update_objects();
			}
			else if (keycode && ((rand() % 100) < 5)) {
				start_battle();
			}else{
				if ((keycode & KEY_RIGHT1) && try_move(1, 0)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode & KEY_LEFT1) && try_move(-1, 0)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode & KEY_DOWN1) && try_move(0, 1)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode & KEY_UP1) && try_move(0, -1)) { wait(4); update_objects(); play_sound_effect(); }
			}
		} else {
			update_battle();
		}
		vsync();
	}
}

void parse_map(void) {
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
	char c = levels[current_stage][y * MAP_W + x];
	if (c == '#' || c == 'H') return 0;  // 壁と新ブロックはプレイヤー通れない
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
//			player_x = nx; player_y = ny;
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
//				player_x = nx; player_y = ny;
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

void gravity_fall(void) {
	if (gravity_x == -1) return;

	if (gravity_y + 1 < MAP_H) {
		c = levels[current_stage][(gravity_y + 1) * MAP_W + gravity_x];
		if ((c == 'H') || can_move(gravity_x, gravity_y + 1)) {  // Hも突き抜ける
			gravity_y++;
			play_sound_effect();

			if (gravity_x == goal_x && gravity_y == goal_y) {
				update_objects();
				print_at(PRINT_MUL * 10, 10, "STAGE CLEAR!!");
				wait(60);
				cls();
				player_hp = 20 + 5 * level;
				current_stage++;
				if (current_stage >= MAX_STAGES) current_stage = 0;  // ループ
				parse_map();
				draw_background();
			}
			update_objects();
		}
	}
}

void draw_background(void) {
	static int x, y;
	for (y = 0; y < MAP_H; y++) {
		for (x = 0; x < MAP_W; x++) {
			char c = levels[current_stage][y * MAP_W + x];
			if (c == '#') put_chr16(x , y, TILE_WALL);
			else if (c == 'G') put_chr16(x, y, TILE_GOAL);
			else if (c == 'H') put_chr16(x, y, TILE_HOLLOW);
			else put_chr16(x, y, TILE_FLOOR);
		}
	}
}

void update_objects(void) {
	static int i;

	// 前の位置を床に戻す
	if (old_player_x >= 0) put_chr16(old_player_x , old_player_y, TILE_FLOOR);
	if ((old_gravity_x != gravity_x) || (old_gravity_y != gravity_y)) put_chr16(old_gravity_x, old_gravity_y, TILE_FLOOR);
	for (i = 0; i < panel_count; i++) {
		if ((old_panels_x[i] != panels_x[i]) || old_panels_y[i] != panels_y[i]) put_chr16(old_panels_x[i], old_panels_y[i], TILE_FLOOR);
	}

	// 新しい位置に描画
	if (player_x >= 0)
		put_chr16(player_x, player_y, TILE_PLAYER);  // プレイヤー

	if (gravity_x >= 0) put_chr16(gravity_x, gravity_y, TILE_GRAVITY);  // 重力パネル（B）

	for (i = 0; i < panel_count; i++) {
		put_chr16(panels_x[i], panels_y[i], TILE_NORMAL);  // 通常パネル（S）
	}

	// 位置保存
	old_player_x = player_x; old_player_y = player_y;
	old_gravity_x = gravity_x; old_gravity_y = gravity_y;
	for (i = 0; i < panel_count; i++) {
		old_panels_x[i] = panels_x[i];
		old_panels_y[i] = panels_y[i];
	}
}

void start_battle(void) {
	game_mode = 1;
	enemy_hp = 10 + (level - 1) * 5;
	enemy_atk = 3 + (level - 1) * 2;
	strcpy(battle_msg, "Slime appeared!");
	cls();
	// スライム表示（中央上部に）
	put_chr16(8, 2, TILE_SLIME);  // スライムキャラコード6
	print_at(PRINT_MUL * 5, 8, battle_msg);
	print_at(PRINT_MUL * 5, 12, "Press SPACE to attack");
}

void update_battle(void) {
	if (keyscan() & KEY_A) {
		enemy_hp -= player_atk;
		if (enemy_hp <= 0) {
			exp += 10;
			if (exp >= level * 20) {
				level++;
				exp = 0;
				player_hp = 20 + 5 * level;
				player_atk += 2;
				print_at(PRINT_MUL * 5, 14, "Level Up!");
				wait(30);
			}
			cls();
			strcpy(battle_msg, "Enemy defeated!");
			print_at(PRINT_MUL * 5, 10, battle_msg);
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
			put_chr16(8, 2, TILE_SLIME);
			print_at(PRINT_MUL * 5, 8, "Enemy defeated? No!");
			print_at(PRINT_MUL * 5, 10, battle_msg);
			if(player_hp > 0){
				print_at(PRINT_MUL * 5, 12, "Press SPACE to attack");
			}else{
				wait(60);
				cls();
				print_at(PRINT_MUL * 10, 10, "You dead!");
				player_hp = 20 + 5 * level;
				wait(60);
				cls();
				game_mode = 0;
				parse_map();
				draw_background();
				update_objects();
			}
		}
		play_sound_effect();
		wait(10);
	}
}
