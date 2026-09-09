#ifndef RENDERER_H
#define RENDERER_H

#include "base.h"

typedef struct {
  i32 x;
  i32 y;
} RPoint;

typedef struct {
  u32 epoch;
  u32 epoch_total;
  u32 episode;
  u32 episode_total;
  u32 step;
  f32 episode_score;
  f32 avg_return;
  f32 return_std;
  u32 foods_eaten;
} RenderStats;

// opens the window
void render_init(u32 grid_cols, u32 grid_rows, u32 cell_size);

// detects the close request(if user closed the window)
b32 renderer_should_render(void);

void renderer_shutdown(void);

// pov uses the same encoding as env
void renderer_draw_frame(RPoint snake, RPoint food, u32 pov, u32 grid_cols,
                         u32 grid_rows, const RenderStats *stats);

#endif
