#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Task 1 */
game_state_t* create_default_state() {
  // Create a new game_state_t struct and allocate memory for it
  // Allocate memory for the game_state_t struct
  game_state_t* state = (game_state_t*)malloc(sizeof(game_state_t));
  if (state == NULL) {
      fprintf(stderr, "Memory allocation failed for game_state_t.\n");
      return NULL;
  }

  // Initialize the game_state_t members
  state->num_rows = 18;
  state->num_snakes = 1;

  // Allocate memory for the 'board' array of strings
  state->board = (char**)malloc(state->num_rows * sizeof(char*));
  if (state->board == NULL) {
      fprintf(stderr, "Memory allocation failed for board array.\n");
      free(state);
      return NULL;
  }

  // Allocate memory for each row and initialize the 'board' with the default layout
  for (unsigned int i = 0; i < state->num_rows; ++i) {
      state->board[i] = (char*)malloc(21 * sizeof(char)); // 20 characters + '\0'
      if (state->board[i] == NULL) {
          fprintf(stderr, "Memory allocation failed for board row %d.\n", i);
          for (unsigned int j = 0; j < i; ++j) {
              free(state->board[j]);
          }
          free(state->board);
          free(state);
          return NULL;
      }
  }

  // Initialize the board with the default layout
  const char* default_board[] = {
      "####################",
      "#                  #",
      "# d>D    *         #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "#                  #",
      "####################"
  };

  for (unsigned int i = 0; i < state->num_rows; ++i) {
      strcpy(state->board[i], default_board[i]);
  }

  // Initialize the snake
  state->snakes = (snake_t*)malloc(sizeof(snake_t));
  if (state->snakes == NULL) {
      fprintf(stderr, "Memory allocation failed for snake.\n");
      for (unsigned int i = 0; i < state->num_rows; ++i) {
          free(state->board[i]);
      }
      free(state->board);
      free(state);
      return NULL;
  }

  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].live = true;

  return state;
}

/* Task 2 */
void free_state(game_state_t* state) {
  if (state == NULL) {
      return;
  }

  // Free snake structs
  if (state->snakes != NULL) {
      free(state->snakes);
  }

  // Free board contents
  if (state->board != NULL) {
      for (unsigned int i = 0; i < state->num_rows; ++i) {
          free(state->board[i]);
      }
      free(state->board);
  }

  // Free game_state_t struct itself
  free(state);
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  if (state == NULL || fp == NULL) {
      return;
  }

  for (unsigned int i = 0; i < state->num_rows; ++i) {
      fprintf(fp, "%s\n", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  return (c == 'w' || c == 'a' || c == 's' || c == 'd');
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  return (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x');
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  return (c == 'w' || c == 'a' || c == 's' || c == 'd' ||
          c == '^' || c == '<' || c == 'v' ||
          c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x');
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  switch (c) {
    case '^':
      return 'w';
    case '<':
      return 'a';
    case 'v':
      return 's';
    case '>':
      return 'd';
    default:
      return '?'; // Default case, can be any value since it's undefined
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  switch (c) {
    case 'W':
      return '^';
    case 'A':
      return '<';
    case 'S':
      return 'v';
    case 'D':
      return '>';
    default:
      return '?';
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S') {
    return cur_row + 1;
  } else if (c == '^' || c == 'w' || c == 'W') {
    return cur_row - 1;
  } else {
    return cur_row;
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D') {
    return cur_col + 1;
  } else if (c == '<' || c == 'a' || c == 'A') {
    return cur_col - 1;
  } else {
    return cur_col;
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  snake_t snake = state->snakes[snum]; // Get the snake by its index
  unsigned int next_row = get_next_row(snake.head_row, state->board[snake.head_row][snake.head_col]);
  unsigned int next_col = get_next_col(snake.head_col, state->board[snake.head_row][snake.head_col]);

  // Check if the next cell is within the bounds of the board
  if (next_row >= state->num_rows || next_col >= strlen(state->board[next_row])) {
    return '?'; // Invalid position, return a placeholder character
  }

  char next_cell = state->board[next_row][next_col];
  return next_cell;
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  snake_t snake = state->snakes[snum]; // Get the snake by its index
  char c = get_board_at(state, snake.head_row, snake.head_col);
  unsigned int next_row = get_next_row(snake.head_row, c);
  unsigned int next_col = get_next_col(snake.head_col, c);

  // Check if the next cell is within the bounds of the board
  // if (next_row >= state->num_rows || next_col >= strlen(state->board[next_row])) {
  //   return '?'; // Invalid position, return a placeholder character
  // }

  set_board_at(state, snake.head_row, snake.head_col, head_to_body(c));
  set_board_at(state, next_row, next_col, c);

  state->snakes[snum].head_col = next_col;
  state->snakes[snum].head_row = next_row;

  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  snake_t snake = state->snakes[snum]; // Get the snake by its index
  char old_tail = get_board_at(state, snake.tail_row, snake.tail_col);
  unsigned int next_row = get_next_row(snake.tail_row, old_tail);
  unsigned int next_col = get_next_col(snake.tail_col, old_tail);
  char c = state->board[next_row][next_col];

  // Check if the next cell is within the bounds of the board
  // if (next_row >= state->num_rows || next_col >= strlen(state->board[next_row])) {
  //   return '?'; // Invalid position, return a placeholder character
  // }

  char new_tail = body_to_tail(c);
  set_board_at(state, snake.tail_row, snake.tail_col, ' ');
  set_board_at(state, next_row, next_col, new_tail);

  state->snakes[snum].tail_col = next_col;
  state->snakes[snum].tail_row = next_row;
  return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  for (unsigned int snum = 0; snum < state->num_snakes; snum++) {
    if (!state->snakes[snum].live) {
      continue;
    }

    char new_head = next_square(state, snum);
    // If the head crashes into the body of a snake or a wall
    if (is_snake(new_head) || new_head == '#') {
      state->snakes[snum].live = false; // the snake dies and stops moving
      set_board_at(state, state->snakes[snum].head_row, state->snakes[snum].head_col, 'x');  // When a snake dies, the head is replaced with an x
    }
    // If the head moves into a fruit, the snake eats the fruit and grows by 1 unit in length.
    else if (new_head == '*') {
      update_head(state, snum); // Each snake moves one step in the direction of its head.
      add_food(state);
    }
    else {
      update_head(state, snum);
      update_tail(state, snum);
    }
  }

  return;
}

/* Task 5 */
game_state_t* load_board(FILE* fp) {
    // by chatgpt
    // Allocate memory for the game_state_t struct
    game_state_t* state = (game_state_t*)malloc(sizeof(game_state_t));
    if (state == NULL) {
        return NULL; // Memory allocation failed
    }

    // Initialize struct members
    state->num_rows = 0;
    state->board = NULL;
    state->num_snakes = 0;
    state->snakes = NULL;

    // Read the game board from the stream
    char line[999999];  // 暴力解决main的第17个test
    unsigned int row_index = 0;
    unsigned int max_cols = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        unsigned int line_len = strlen(line);

        // Remove the newline character from the line
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
            line_len--;
        }

        // Resize the board array to accommodate a new row
        state->board = (char**)realloc(state->board, (row_index + 1) * sizeof(char*));
        if (state->board == NULL) {
            free(state);
            return NULL; // Memory allocation failed
        }

        // Allocate memory for the row and copy the line content
        state->board[row_index] = (char*)malloc(line_len + 1);
        if (state->board[row_index] == NULL) {
            // Free previously allocated memory
            for (unsigned int i = 0; i < row_index; ++i) {
                free(state->board[i]);
            }
            free(state->board);
            free(state);
            return NULL; // Memory allocation failed
        }

        strcpy(state->board[row_index], line);

        // Update the maximum column count
        if (line_len > max_cols) {
            max_cols = line_len;
        }

        ++row_index;
    }

    // Set the number of rows and columns in the game_state_t struct
    state->num_rows = row_index;

    // Trim excess memory from each row to match the actual column count
    for (unsigned int i = 0; i < state->num_rows; ++i) {
        state->board[i] = (char*)realloc(state->board[i], max_cols + 1);
    }

    return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
    snake_t* snake = &(state->snakes[snum]);
    unsigned int row = snake->tail_row;
    unsigned int col = snake->tail_col;

    while (!is_head(state->board[row][col])) {
        unsigned int next_row = get_next_row(row, state->board[row][col]);
        unsigned int next_col = get_next_col(col, state->board[row][col]);
        row = next_row;
        col = next_col;
    }

    snake->head_row = row;
    snake->head_col = col;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
    unsigned int num_snakes = 0;

    // Count the number of snakes on the board
    for (unsigned int i = 0; i < state->num_rows; i++) {
        unsigned int row_length = (unsigned int)strlen(state->board[i]);
        for (unsigned int j = 0; j < row_length; j++) {
            if (is_tail(state->board[i][j])) {
                num_snakes++;
            }
        }
    }

    // Allocate memory for the snakes array
    state->snakes = (snake_t*)malloc(num_snakes * sizeof(snake_t));
    if (state->snakes == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    state->num_snakes = num_snakes;

    // Initialize each snake's tail position and find its head
    unsigned int snake_count = 0;
    for (unsigned int i = 0; i < state->num_rows; i++) {
        for (unsigned int j = 0; j < strlen(state->board[i]); j++) {
            if (is_tail(state->board[i][j])) {
                state->snakes[snake_count].tail_row = i;
                state->snakes[snake_count].tail_col = j;
                state->snakes[snake_count].live = true;

                find_head(state, snake_count); // Fill in the head position

                snake_count++;
            }
        }
    }

    return state;
}
