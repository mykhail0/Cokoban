#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants for memory reallocation utilities.
#define MULTIPLIER 3
#define DIVISOR 2

// Number of letters in the Latin alphabet.
#define MAX_BOX_NUM 26

#define NOBOX (-1)

// Object's position on the board.
typedef struct {
  int line;
  int col;
} Pos;

// Board's state.
typedef struct {
  Pos player;
  Pos box[MAX_BOX_NUM];
} State;

// Assumes that name is a character in the Latin alphabet.
int box_name_to_index(int name) {
  return name - ('A' <= name && name <= 'Z' ? 'A' : 'a');
}

/**
 * Description of one row of the board.
 * `len` - number of fields in the row,
 * `line` - specification for each field in the row.
 */
typedef struct {
  int len;
  char* line;
} Line;

/**
 * Description of the board.
 * `lines` - number of rows on the board,
 * `board` - describes contents of each row.
 */
typedef struct {
  int lines;
  Line* board;
} Board;

// History of board states as a stack for undo purposes.
struct list {
  State state;
  struct list* next;
};
typedef struct list Stack;

void init(Stack** s) { *s = NULL; }

bool empty(Stack* s) { return s == NULL; }

void push(Stack** s, State x) {
  Stack* tmp;
  tmp = malloc(sizeof(*tmp));
  assert(tmp != NULL);
  tmp->next = *s;
  tmp->state = x;
  *s = tmp;
}

void pop(Stack** s, State* x) {
  *x = (*s)->state;
  Stack* tmp = *s;
  *s = (*s)->next;
  free(tmp);
}

void top(Stack** s, State* x) {
  pop(s, x);
  push(s, *x);
}

void clear(Stack** s) {
  while (!empty(*s)) {
    State x;
    pop(s, &x);
  }
}

int more(int n) { return 1 + n * MULTIPLIER / DIVISOR; }

// Reallocate the board if needed.
void realloc_board(Board* board, int* size) {
  if (board->lines == *size) {
    *size = more(*size);
    board->board = realloc(board->board, (*size) * sizeof *(board->board));
    assert(board->board != NULL);
    for (int i = board->lines; i < *size; ++i) {
      board->board[i].len = 0;
      board->board[i].line = NULL;
    }
  }
}

// Reallocate the row if needed.
void realloc_line(Line* line, int* size) {
  if (line->len == *size) {
    *size = more(*size);
    line->line = realloc(line->line, (*size) * sizeof *(line->line));
    assert(line->line != NULL);
  }
}

static Board board;
static Stack* states;

// Free the memory.
void clean_up(Board* board, Stack** states) {
  for (int i = 0; i < board->lines; ++i) free(board->board[i].line);
  free(board->board);
  clear(states);
}

void init_state(State* state) {
  assert(state != NULL);
  for (int i = 0; i < MAX_BOX_NUM; ++i) {
    state->box[i].line = NOBOX;
    state->box[i].col = NOBOX;
  }
}

int my_getchar() {
  int c = getchar();
  if (c == EOF) {
    perror("getchar()");
    clean_up(&board, &states);
    exit(EXIT_FAILURE);
  }
  return c;
}

int read_board_getchar() {
  static char ALLOWED_CHARS[] = {'-', '+', '#', '@', '*', '\n', '\0'};
  int c = my_getchar();
  if (strchr(ALLOWED_CHARS, c) == NULL && (c < 'a' || 'z' < c) &&
      (c < 'A' || 'Z' < c)) {
    perror(
        "Invalid character, valid input for the board is only "
        "Latin alphabet letters, new line and -+#@*");
    clean_up(&board, &states);
    exit(EXIT_FAILURE);
  }

  return c;
}

void init_board(Board* board, int* board_size) {
  assert(board != NULL);
  board->board = NULL;
  *board_size = board->lines = 0;
  realloc_board(board, board_size);
}

// Add the next character from input onto the board and state.
void add_ch_to_board(Board* board, State* state, int c) {
  if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
    board->board[board->lines].line[board->board[board->lines].len] =
        'A' <= c && c <= 'Z' ? '+' : '-';
    c = box_name_to_index(c);
    state->box[c].line = board->lines;
    state->box[c].col = board->board[board->lines].len;
  } else {
    if (c == '-' || c == '+' || c == '#') {
      board->board[board->lines].line[board->board[board->lines].len] = c;
    } else {
      state->player.line = board->lines;
      state->player.col = board->board[board->lines].len;
      board->board[board->lines].line[board->board[board->lines].len] =
          c == '@' ? '-' : '+';
    }
  }
}

// Read the board's initial state.
void read_board(Board* board, State* state) {
  int c = read_board_getchar(), line_size = 0, board_size;
  init_board(board, &board_size);

  while (c != '\n') {
    realloc_line(&(board->board[board->lines]), &line_size);
    add_ch_to_board(board, state, c);
    ++(board->board[board->lines].len);
    c = read_board_getchar();

    if (c == '\n') {
      ++(board->lines);
      realloc_board(board, &board_size);
      line_size = 0;
      c = read_board_getchar();
    }
  }
}

// Draws appropriate boxes and player as specified by `state` onto `board`.
void apply_state_to_board(State state, Board* board) {
  for (int i = 0; i < MAX_BOX_NUM; ++i) {
    if (state.box[i].line != NOBOX) {
      board->board[state.box[i].line].line[state.box[i].col] =
          i + (board->board[state.box[i].line].line[state.box[i].col] == '-'
                   ? 'a'
                   : 'A');
    }
  }
  board->board[state.player.line].line[state.player.col] =
      board->board[state.player.line].line[state.player.col] == '-' ? '@' : '*';
}

// Assuming `state` correctly describes `board`'s state, remove boxes and the
// player's character from the `board`.
void make_default(State state, Board* board) {
  for (int i = 0; i < MAX_BOX_NUM; ++i) {
    if (state.box[i].line != NOBOX) {
      board->board[state.box[i].line].line[state.box[i].col] =
          ('a' <= board->board[state.box[i].line].line[state.box[i].col] &&
           board->board[state.box[i].line].line[state.box[i].col] <= 'z')
              ? '-'
              : '+';
    }
  }
  board->board[state.player.line].line[state.player.col] =
      board->board[state.player.line].line[state.player.col] == '@' ? '-' : '+';
}

// Prepare for the game.
void init_sequence(Stack** states, Board* board) {
  State state;
  init_state(&state);
  read_board(board, &state);
  init(states);
  push(states, state);
  apply_state_to_board(state, board);
}

typedef enum { DOWN, LEFT, RIGHT, UP } Direction;
const char ALLOWED_DIRECTIONS[] = {'2', '4', '6', '8', '\0'};

Direction direction(int ch) { return (ch - '0') / 2 - 1; }
Direction opposite(Direction dir) { return 3 - dir; }

// Convert direction to the change in coordinates on the board.
const int DIR_TO_COORD_CHANGE[4][2] = {{1, 0}, {0, -1}, {0, 1}, {-1, 0}};

// Writes to `*dest` coordinates of the `box` after a push in direction `dir`.
void move_to_dest_pos(Pos* dest, State state, int box, Direction dir) {
  if (state.box[box].line == NOBOX || state.box[box].col == NOBOX) {
    perror("Trying to move a box that does not exist on the board.");
    clean_up(&board, &states);
    exit(EXIT_FAILURE);
  }
  dest->line = state.box[box].line + DIR_TO_COORD_CHANGE[dir][0];
  dest->col = state.box[box].col + DIR_TO_COORD_CHANGE[dir][1];
}

// Check if it's possible to put the character or a box in position `pos`.
bool is_valid_and_free(Board board, Pos pos) {
  bool is_valid = false;
  if (-1 < pos.line && pos.line < board.lines && -1 < pos.col &&
      pos.col < board.board[pos.line].len) {
    char field = board.board[pos.line].line[pos.col];
    if ((field < 'a' || 'z' < field) && (field < 'A' || 'Z' < field) &&
        field != '#')
      is_valid = true;
  }
  return is_valid;
}

// Queue data structure for purposes of BFS.
struct qlist {
  Pos pos;
  struct qlist* next;
};
typedef struct qlist Qlist;

typedef struct {
  Qlist* front;
  Qlist* rear;
} Queue;

void initq(Queue* q) {
  q->front = NULL;
  q->rear = q->front;
}

bool emptyq(Queue q) { return q.front == NULL; }

void pushq(Queue* q, Pos x) {
  if (q->front) {
    q->rear->next = malloc(sizeof *(q->rear->next));
    assert(q->rear->next != NULL);
    q->rear->next->pos = x;
    q->rear = q->rear->next;
  } else {
    q->rear = malloc(sizeof *(q->rear));
    assert(q->rear != NULL);
    q->rear->pos = x;
    q->front = q->rear;
  }
  q->rear->next = NULL;
}

void popq(Queue* q, Pos* x) {
  *x = q->front->pos;
  Qlist* tmp = q->front;
  q->front = q->front->next;
  free(tmp);
}

void clearq(Queue* q) {
  while (!emptyq(*q)) {
    Pos x;
    popq(q, &x);
  }
}

// Initialize array of cells that have been processed in the BFS.
void init_seen(bool*** seen, Board board) {
  *seen = malloc(board.lines * sizeof(bool*));
  assert(*seen != NULL);
  for (int i = 0; i < board.lines; ++i) {
    (*seen)[i] = malloc(board.board[i].len * sizeof *((*seen)[i]));
    assert((*seen)[i] != NULL);
  }
  for (int i = 0; i < board.lines; ++i) {
    for (int j = 0; j < board.board[i].len; ++j) (*seen)[i][j] = false;
  }
}

void clear_seen(bool*** seen, Board board) {
  for (int i = 0; i < board.lines; ++i) free((*seen)[i]);
  free(*seen);
}

// Writes neighbour candidates of `x` into `t`.
void get_neighbour_candidates(Pos t[4], Pos x) {
  for (int i = 0; i < 4; ++i) {
    t[i].line = x.line + DIR_TO_COORD_CHANGE[i][0];
    t[i].col = x.col + DIR_TO_COORD_CHANGE[i][1];
  }
}

// Adds valid neighbours from neighbour candidates to the queue.
void add_neighbours(Pos t[4], Board board, Queue* q, bool** seen) {
  for (int i = 0; i < 4; ++i) {
    if (is_valid_and_free(board, t[i]) && !seen[t[i].line][t[i].col]) {
      seen[t[i].line][t[i].col] = true;
      pushq(q, t[i]);
    }
  }
}

bool path_exists(Board board, Pos player, Pos dest) {
  bool **seen, found_path = false;
  init_seen(&seen, board);
  Queue q;
  initq(&q);
  pushq(&q, player);
  seen[player.line][player.col] = true;
  while (!emptyq(q) && !found_path) {
    Pos current, neighbours[4];
    popq(&q, &current);
    found_path = (current.line == dest.line && current.col == dest.col);
    get_neighbour_candidates(neighbours, current);
    add_neighbours(neighbours, board, &q, seen);
  }
  clear_seen(&seen, board);
  clearq(&q);
  return found_path;
}

void undo(Stack** states, Board* board) {
  State state;
  pop(states, &state);
  if (!empty(*states)) {
    make_default(state, board);
    top(states, &state);
    apply_state_to_board(state, board);
  } else {
    push(states, state);
  }
}

void change_state(State* state, int box, Pos box_new_pos) {
  state->player.line = state->box[box].line;
  state->player.col = state->box[box].col;
  state->box[box] = box_new_pos;
}

void make_move(Stack** states, Board* board, int box, Pos box_new_pos) {
  State current_state;
  top(states, &current_state);
  State new_state = current_state;
  change_state(&new_state, box, box_new_pos);
  push(states, new_state);
  make_default(current_state, board);
  apply_state_to_board(new_state, board);
}

void print_board(Board board) {
  for (int i = 0; i < board.lines; ++i) {
    for (int j = 0; j < board.board[i].len; ++j)
      putchar(board.board[i].line[j]);
    putchar('\n');
  }
}

// Process the player's commands in a loop and print board states.
void sokoban(Stack** states, Board* board) {
  for (int c = my_getchar(); c != '.'; c = my_getchar()) {
    if (c == '0') {
      undo(states, board);
    } else if ('a' <= c && c <= 'z') {
      State state;
      top(states, &state);
      c = box_name_to_index(c);
      int ch = my_getchar();
      if (strchr(ALLOWED_DIRECTIONS, ch) == NULL) {
        perror("Expected a direction in the form of 2 | 4 | 6 | 8.");
        clean_up(board, states);
        exit(EXIT_FAILURE);
      }
      Direction dir = direction(ch);
      Pos box_new_pos, char_pos_before_push;
      move_to_dest_pos(&box_new_pos, state, c, dir);
      move_to_dest_pos(&char_pos_before_push, state, c, opposite(dir));
      if (is_valid_and_free(*board, box_new_pos) &&
          is_valid_and_free(*board, char_pos_before_push) &&
          path_exists(*board, state.player, char_pos_before_push))
        make_move(states, board, c, box_new_pos);
    } else {
      perror("Expected a name of the box to move or 0.");
      clean_up(board, states);
      exit(EXIT_FAILURE);
    }
    print_board(*board);
    if (my_getchar() != '\n') {
      perror("Expected newline character.");
      clean_up(board, states);
      exit(EXIT_FAILURE);
    }
  }
}

int main() {
  init_sequence(&states, &board);
  print_board(board);
  sokoban(&states, &board);
  clean_up(&board, &states);
  return 0;
}
