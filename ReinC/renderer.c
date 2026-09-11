#include "renderer.h"
#include "raylib.h"

#define HUD_HEIGHT 130
#define GRID_MARGIN 20
#define NORMAL_FPS 20

static u32 s_cell_size;
static b32 s_active;

void renderer_init(u32 grid_cols, u32 grid_rows, u32 cell_size) {
  s_cell_size = cell_size;

  i32 screen_w = (i32)(grid_cols * cell_size) + 2 * GRID_MARGIN;
  i32 screen_h = (i32)(grid_rows * cell_size) + 2 * GRID_MARGIN + HUD_HEIGHT;

  InitWindow(screen_w, screen_h, "Snake RL - Live Training");
  SetTargetFPS(NORMAL_FPS);
  s_active = true;
}

b32 renderer_should_render(void) {
  if (!s_active) {
    return false;
  }

  if (WindowShouldClose()) {
    CloseWindow();
    s_active = false;
    return false;
  }

  return true;
}

void renderer_shutdown(void) {
  if (s_active) {
    CloseWindow();
    s_active = false;
  }
}

static i32 grid_to_screen_x(i32 gx) {
  return GRID_MARGIN + gx * (i32)s_cell_size;
}

static i32 grid_to_screen_y(i32 gy_from_top) {
  return GRID_MARGIN + gy_from_top * (i32)s_cell_size;
}

void renderer_draw_frame(RPoint snake, RPoint food, u32 pov, u32 grid_cols,
                         u32 grid_rows, const RenderStats *stats) {
  if (!s_active) {
    return;
  }

  // Hold SPACE to fast-forward the watched episode; release to go back to
  // normal watching speed.
  SetTargetFPS(IsKeyDown(KEY_SPACE) ? 0 : NORMAL_FPS);

  BeginDrawing();
  ClearBackground((Color){24, 26, 32, 255});

  i32 grid_px_w = (i32)(grid_cols * s_cell_size);
  i32 grid_px_h = (i32)(grid_rows * s_cell_size);

  for (u32 gx = 0; gx <= grid_cols; gx++) {
    i32 x = grid_to_screen_x((i32)gx);
    DrawLine(x, GRID_MARGIN, x, GRID_MARGIN + grid_px_h,
             (Color){60, 63, 74, 255});
  }
  for (u32 gy = 0; gy <= grid_rows; gy++) {
    i32 y = grid_to_screen_y((i32)gy);
    DrawLine(GRID_MARGIN, y, GRID_MARGIN + grid_px_w, y,
             (Color){60, 63, 74, 255});
  }

  // Flip row so that env.c's UP (y increasing) reads as visually "up".
  i32 food_row = (i32)grid_rows - 1 - food.y;
  i32 snake_row = (i32)grid_rows - 1 - snake.y;

  DrawRectangle(grid_to_screen_x(food.x) + 3, grid_to_screen_y(food_row) + 3,
                (i32)s_cell_size - 6, (i32)s_cell_size - 6,
                (Color){220, 70, 70, 255});

  i32 sx = grid_to_screen_x(snake.x);
  i32 sy = grid_to_screen_y(snake_row);
  DrawRectangle(sx + 2, sy + 2, (i32)s_cell_size - 4, (i32)s_cell_size - 4,
                (Color){90, 200, 110, 255});

  i32 cx = sx + (i32)s_cell_size / 2;
  i32 cy = sy + (i32)s_cell_size / 2;
  i32 r = (i32)s_cell_size / 4;
  Vector2 p1, p2, p3;
  switch (pov) {
  case 0: // LEFT
    p1 = (Vector2){cx - r, cy};
    p2 = (Vector2){cx + r / 2.0f, cy - r};
    p3 = (Vector2){cx + r / 2.0f, cy + r};
    break;
  case 2: // UP
    p1 = (Vector2){cx, cy - r};
    p2 = (Vector2){cx - r, cy + r / 2.0f};
    p3 = (Vector2){cx + r, cy + r / 2.0f};
    break;
  case 3: // DOWN
    p1 = (Vector2){cx, cy + r};
    p2 = (Vector2){cx - r, cy - r / 2.0f};
    p3 = (Vector2){cx + r, cy - r / 2.0f};
    break;
  case 1: // RIGHT
  default:
    p1 = (Vector2){cx + r, cy};
    p2 = (Vector2){cx - r / 2.0f, cy - r};
    p3 = (Vector2){cx - r / 2.0f, cy + r};
    break;
  }
  DrawTriangle(p1, p2, p3, (Color){20, 40, 25, 255});

  i32 hud_x = GRID_MARGIN;
  i32 hud_y = GRID_MARGIN + grid_px_h + 16;

  DrawText(TextFormat("Epoch %u / %u", stats->epoch, stats->epoch_total), hud_x,
           hud_y, 20, RAYWHITE);
  DrawText(TextFormat("Episode %u / %u   Step %u", stats->episode,
                      stats->episode_total, stats->step),
           hud_x, hud_y + 24, 18, LIGHTGRAY);
  DrawText(TextFormat("Foods eaten this episode: %u   Episode score: %.1f",
                      stats->foods_eaten, stats->episode_score),
           hud_x, hud_y + 48, 18, LIGHTGRAY);
  DrawText(TextFormat("Last epoch avg return: %.3f   std: %.3f",
                      stats->avg_return, stats->return_std),
           hud_x, hud_y + 72, 18, LIGHTGRAY);
  DrawText("Hold SPACE to fast-forward   |   ESC to stop watching (training "
           "keeps going)",
           hud_x, hud_y + 96, 14, GRAY);

  EndDrawing();
}
