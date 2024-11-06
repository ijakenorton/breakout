#include "raylib.h"
#include "types.h"
#include "collision.h"
#include <stdio.h>
#include <stdlib.h>
#define ARENA_IMPLEMENTATION
#include "arena.h"

float screen_width = 1920.0f;
float screen_height = 1080.0f;

static Arena default_arena = { 0 };
/* static Arena temporary_arena = { 0 }; */
static Arena *context_arena = &default_arena;

void *context_alloc(size_t size)
{
	assert(context_arena);
	return arena_alloc(context_arena, size);
}

void assert_with_message(bool condition, char *message)
{
	print_int(condition);
	if (!condition) {
		fprintf(stderr, "%s", message);
		exit(1);
	}
}

void init_level_two(Level *level_two, State *state)
{
	if (level_two->ball == NULL) {
		level_two->ball = context_alloc(sizeof(Ball));
	}

	*level_two->ball = (Ball){
		.centre.x = screen_width / 2.0,
		.centre.y = screen_height - (screen_height / 4),
		.velocity.x = 1.0,
		.velocity.y = 1.0,
		.radius = 20.0,
		.color = BLUE,
	};

	if (level_two->paddle == NULL) {
		level_two->paddle = context_alloc(sizeof(Paddle));
		level_two->paddle->rec = context_alloc(sizeof(Rectangle));
		level_two->paddle->origin = context_alloc(sizeof(Vector2));
	}

	float rec_width = screen_width / 5.0;
	*level_two->paddle->rec = (Rectangle){ .x = screen_width / 2.0,
					       .y = screen_height - 100.0,
					       .width = rec_width,
					       .height = 20.0 };
	level_two->paddle->rotation = 0;
	*level_two->paddle->origin =
		(Vector2){ level_two->paddle->rec->width / 2.0f,
			   level_two->paddle->rec->height / 2.0f };

	float num_row = 10.0;
	float block_width = screen_width / num_row;
	float block_height = block_width / 2;

	if (level_two->blocks_state == NULL) {
		level_two->blocks_state = context_alloc(sizeof(BlocksState));
		level_two->blocks_state->blocks =
			context_alloc(sizeof(Block) * NUM_ROW);
	}

	// init blocks
	int block_count = 0;
	for (int i = block_width; i < screen_width - block_width;
	     i += block_width) {
		level_two->blocks_state->blocks[block_count] = (Block){
			.rec = (Rectangle){ .x = i,
					    .y = screen_height / 3.0,
					    .width = block_width,
					    .height = block_height },
			.alive = true,
			.main_color = RED,
			.outline_color = WHITE,
		};

		block_count++;
	}

	for (int i = block_width + block_height;
	     i < screen_width - block_width * 2; i += block_width) {
		level_two->blocks_state->blocks[block_count] = (Block){
			.rec = (Rectangle){ .x = i,
					    .y = screen_height / 3.0 +
						 block_height,
					    .width = block_width,
					    .height = block_height },
			.alive = true,
			.main_color = YELLOW,
			.outline_color = BLACK,
		};

		block_count++;
	}
	level_two->blocks_state->num_alive = block_count;
	level_two->blocks_state->num_blocks = block_count;

	*state = (State){ .win = false,
			  .loss = false,
			  .paused = true,
			  .cl = level_two,
			  .level_no = 1 };
}

// TODO cleanup memory
// Redundent passing the level and state as I'm currently using it but unsure if thats correct
// So leaving the option
void init_level_one(Level *level_one, State *state)
{
	if (level_one->ball == NULL) {
		level_one->ball = context_alloc(sizeof(Ball));
	}

	*level_one->ball = (Ball){
		.centre.x = screen_width / 2.0,
		.centre.y = screen_height - (screen_height / 4),
		.velocity.x = 0.0f,
		.velocity.y = 1.0f,
		.radius = 20.0f,
		.color = BLUE,
	};

	if (level_one->paddle == NULL) {
		level_one->paddle = context_alloc(sizeof(Paddle));
		level_one->paddle->rec = context_alloc(sizeof(Rectangle));
		level_one->paddle->origin = context_alloc(sizeof(Vector2));
	}

	float rec_width = screen_width / 5.0;
	*level_one->paddle->rec = (Rectangle){ .x = screen_width / 2.0,
					       .y = screen_height - 100.0,
					       .width = rec_width,
					       .height = 20.0 };
	level_one->paddle->rotation = 0;
	*level_one->paddle->origin =
		(Vector2){ level_one->paddle->rec->width / 2.0f,
			   level_one->paddle->rec->height / 2.0f };

	float num_row = 10.0;
	float block_width = screen_width / num_row;

	if (level_one->blocks_state == NULL) {
		level_one->blocks_state = context_alloc(sizeof(BlocksState));
		level_one->blocks_state->blocks =
			context_alloc(sizeof(Block) * NUM_ROW);
	}

	// init blocks
	int block_count = 0;
	for (int i = block_width; i < screen_width - block_width;
	     i += block_width) {
		level_one->blocks_state->blocks[block_count] = (Block){
			.rec = (Rectangle){ .x = i,
					    .y = screen_height / 3.0 - 10.0,
					    .width = block_width,
					    .height = block_width / 2 },
			.alive = true,
			.main_color = RED,
			.outline_color = WHITE,
		};

		block_count++;
	}
	level_one->blocks_state->num_alive = block_count;
	level_one->blocks_state->num_blocks = block_count;

	*state = (State){ .win = false,
			  .loss = false,
			  .paused = true,
			  .cl = level_one,
			  .level_no = 0 };
}

int main(void)
{
	char *title = "BREAKOUT";
	char *reset = "Reset: R";
	char *level_win = "You Won!";
	char *level_loss = "You Lost!";
	char *paused = "Press Space to continue";
	int title_font_size = 30;
	int big_font_size = 40;
	bool debug = false;

	bool paddle_collision = false;
	bool block_collision = false;

	Line ruler[RULER_INC] = { 0 };

	Line horizontal = (Line){
		.startPosX = 0,
		.startPosY = screen_height / 2.0,
		.endPosX = screen_width,
		.endPosY = screen_height / 2.0,
		.color = WHITE,
	};

	Line vertical = (Line){
		.startPosX = screen_width / 2.0,
		.startPosY = screen_height,
		.endPosX = screen_width / 2.0,
		.endPosY = 0,
		.color = WHITE,
	};

	int ruler_count = 0;

	// init ruler
	for (int i = 0; i <= screen_width + 1; i += screen_width / 10) {
		ruler[ruler_count] = (Line){
			.startPosX = i,
			.startPosY = screen_height,
			.endPosX = i,
			.endPosY = screen_height - 20,
			.color = WHITE,
		};
		ruler_count++;
	}

	State state = { 0 };
	LevelFunc levels[2] = { 0 };

	state.cl = context_alloc(sizeof(Level));
	state.cl->blocks_state = NULL;
	state.cl->ball = NULL;
	state.cl->paddle = NULL;
	levels[0] = init_level_one;
	levels[1] = init_level_two;
	levels[state.level_no](state.cl, &state);

	int end_counter = 1200;

	SetTargetFPS(144);

	InitWindow((int)screen_width, (int)screen_height,
		   "raylib [core] example - basic window");

	int paused_cooldown = 1;
	int initial_timer = 1;

	while (!WindowShouldClose()) {
		// update
		// ------------------------------------------------
		// Bit of a hack to fix space being triggered multiple times
		// Handles pause
		if (IsKeyReleased(KEY_SPACE)) {
			if (paused_cooldown == initial_timer) {
				state.paused = !state.paused;
			} else {
				if (paused_cooldown == 0) {
					paused_cooldown = initial_timer;
				} else {
					paused_cooldown--;
				}
			}
		}

		if (IsKeyReleased(KEY_R)) {
			levels[state.level_no](state.cl, &state);
		}

		if (IsKeyReleased(KEY_ENTER)) {
			//currently levels can be changed without winning
			//as a dev door to help testing
			/* if (state.win) { */
			state.level_no++;
			char state_assert_msg[100] = { 0 };

			sprintf(state_assert_msg,
				"State.level_no is: %d, must be less than %d\n",
				state.level_no, MAX_LEVEL);

			assert_with_message(state.level_no < MAX_LEVEL,
					    state_assert_msg);

			levels[state.level_no](state.cl, &state);
			/* } */
		}

		for (int i = 0; i < state.cl->blocks_state->num_blocks; ++i) {
			Block current_block = state.cl->blocks_state->blocks[i];

			if (!state.cl->blocks_state->blocks[i].alive) {
				continue;
			}

			block_collision =
				circle_rect(*state.cl->ball, current_block.rec);

			if (block_collision) {
				state.cl->blocks_state->blocks[i].alive = false;
				state.cl->blocks_state->num_alive--;
				if (state.cl->blocks_state->num_alive == 0) {
					state.win = true;
				}
				Line_id line = { 0 };
				which_line(*state.cl->ball, current_block.rec,
					   &line);
				print_int(line.d);
				switch (line.d) {
				case TOP:
					printf("top\n");
					state.cl->ball->velocity.y *= -1.0;
					break;
				case BOTTOM:
					printf("bottom\n");
					state.cl->ball->velocity.y *= -1.0;
					break;
				case LEFT:
					printf("left\n");
					state.cl->ball->velocity.x *= -1.0;
					break;
				case RIGHT:
					printf("right\n");
					state.cl->ball->velocity.x *= -1.0;
					break;
				}

				break;
			}
		}

		paddle_collision =
			circle_paddle(*state.cl->ball, *state.cl->paddle);

		if (paddle_collision) {
			Vector2 paddle_center = { state.cl->paddle->rec->x,
						  state.cl->paddle->rec->y };

			// Convert ball's position relative to paddle center
			Vector2 relative_pos = Vector2Subtract(
				state.cl->ball->centre, paddle_center);
			Vector2 local_pos = Vector2Rotate(
				relative_pos,
				-state.cl->paddle->rotation * DEG2RAD);

			// If ball is more to the side than top/bottom, treat as side collision
			if (fabsf(local_pos.x) >
			    state.cl->paddle->rec->width / 2) {
				// Side collision - just reverse x velocity
				state.cl->ball->velocity.y *= -1.0f;
			} else {
				// Top/bottom collision - use normal reflection
				Vector2 paddle_normal = Vector2Rotate(
					(Vector2){ 0, -1 },
					state.cl->paddle->rotation * DEG2RAD);
				paddle_normal = Vector2Normalize(paddle_normal);
				state.cl->ball->velocity =
					Vector2Reflect(state.cl->ball->velocity,
						       paddle_normal);
			}
			/* Vector2 paddle_normal = Vector2Rotate( */
			/* 	(Vector2){ 0, -1 }, */
			/* 	state.cl->paddle->rotation * DEG2RAD); */

			/* paddle_normal = Vector2Normalize(paddle_normal); */

			/* state.cl->ball->velocity = Vector2Reflect( */
			/* 	state.cl->ball->velocity, paddle_normal); */
		}

		// Handle left and right walls
		if ((state.cl->ball->centre.x >=
		     (screen_width - state.cl->ball->radius)) ||
		    (state.cl->ball->centre.x <= state.cl->ball->radius)) {
			state.cl->ball->velocity.x *= -1.0f;
		}

		// Handle top wall
		if ((state.cl->ball->centre.y <= state.cl->ball->radius)) {
			state.cl->ball->velocity.y *= -1.0f;
		}

		// Handle bottom wall
		if (state.cl->ball->centre.y >=
		    (screen_height - state.cl->ball->radius)) {
			state.loss = true;
		}

		if (!state.paused) {
			state.cl->ball->centre =
				Vector2Add(state.cl->ball->centre,
					   state.cl->ball->velocity);
		}

		if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
			state.cl->paddle->rec->x += 10.0f;
		}
		if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
			state.cl->paddle->rec->x -= 10.0f;
		}

		if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
			state.cl->paddle->rotation += 1.0f;
		}
		if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
			state.cl->paddle->rotation -= 1.0f;
		}

		if (end_counter != 0) {
			if (state.win) {
				state.cl->ball->velocity.x -= 0.01f;
				state.cl->ball->velocity.y -= 0.01f;
				end_counter--;
			}
		} else {
			state.cl->ball->velocity.x = 0.0f;
			state.cl->ball->velocity.y = 0.0f;
		}

		// ----------------------------------------------
		BeginDrawing();
		ClearBackground(BLACK);

		DrawFPS(10, 10);
		for (int row = 0; row < state.cl->blocks_state->num_blocks;
		     ++row) {
			Block current_block =
				state.cl->blocks_state->blocks[row];

			if (current_block.alive) {
				DrawRectangleRec(current_block.rec,
						 current_block.main_color);
				DrawRectangleLinesRec(
					current_block.rec,
					current_block.outline_color);
			}
		}

		if (debug) {
			for (int line = 0; line < ruler_count; ++line) {
				DrawLine(ruler[line].startPosX,
					 ruler[line].startPosY,
					 ruler[line].endPosX,
					 ruler[line].endPosY,
					 ruler[line].color);
			}

			DrawLine(vertical.startPosX, vertical.startPosY,
				 vertical.endPosX, vertical.endPosY,
				 vertical.color);

			DrawLine(horizontal.startPosX, horizontal.startPosY,
				 horizontal.endPosX, horizontal.endPosY,
				 horizontal.color);
		}

		DrawRectanglePro(*state.cl->paddle->rec,
				 *state.cl->paddle->origin,
				 state.cl->paddle->rotation, YELLOW);

		DrawCircle(state.cl->ball->centre.x, state.cl->ball->centre.y,
			   state.cl->ball->radius, state.cl->ball->color);

		if (state.paused) {
			DrawText(paused,
				 screen_width / 2 -
					 ((float)MeasureText(paused,
							     big_font_size) /
					  2),
				 screen_height / 2, big_font_size, LIGHTGRAY);

			DrawText(title,
				 screen_width / 2 -
					 ((float)MeasureText(title,
							     title_font_size) /
					  2),
				 screen_height / 7, title_font_size, GREEN);

			DrawText(reset,
				 screen_width - screen_width / 10 -
					 ((float)MeasureText(title,
							     title_font_size) /
					  2),
				 screen_height / 7, title_font_size, GREEN);
		}
		if (state.win) {
			DrawText(level_win,
				 screen_width / 2 -
					 ((float)MeasureText(level_win,
							     big_font_size) /
					  2),
				 screen_height / 2, big_font_size, YELLOW);
		}

		if (state.loss && !state.win) {
			DrawText(level_loss,
				 screen_width / 2 -
					 ((float)MeasureText(level_loss,
							     big_font_size) /
					  2),
				 screen_height / 2, big_font_size, ORANGE);
		}
		EndDrawing();
	}

	CloseWindow();
	arena_free(&default_arena);

	return 0;
}
