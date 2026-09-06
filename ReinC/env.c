// env will define game dynamics and add reward structure

#include <stdlib.h>

#include "arena.h"
#include "autograd.h"
#include "base.h"
#include "mat.h"
#include "model.c"
#include "prng.h"

typedef enum { LEFT = 0, RIGHT = 1, UP = 2, DOWN = 3, NONE = 4 } ACTION;

typedef struct {
  i32 x;
  i32 y;
} State;

typedef struct {
  State snake;
  State food;
  f32 score;
  u32 foods_eaten;

  u32 rows;
  u32 cols;
  u32 grid_size;
  u64 steps;
  ACTION pov;
} SnakeENV;

#define BUFFER_SIZE 1024
#define EPISODE_LEN 100

typedef struct {
  State states[EPISODE_LEN];
  State food_states[EPISODE_LEN];
  ACTION povs[EPISODE_LEN];
  ACTION actions[EPISODE_LEN];
  f32 rewards[EPISODE_LEN];
  f32 returns[EPISODE_LEN];
  State next_states[EPISODE_LEN];
  b32 dones[EPISODE_LEN];

  u32 len;

} Trajactory;

typedef struct {
  Trajactory trajactories[BUFFER_SIZE];
  u32 count;
} ReplayBuffer;
