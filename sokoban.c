#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Constants for memory reallocation utilities.
#define MULTIPLIER 3
#define DIVISOR 2

#define NOBOX (-1)

// Object's position on the board.
struct pos {
  int line;
  int col;
};

// Board's state.
typedef struct {
  struct pos player;
  struct pos box[26];
} Tstate;

/**
 * Rekord opisujacy pojedyncza linijke planszy.
 * `len` to liczba kratek w linijce, `line` opisuje kratki.
 */
struct line {
  int len;
  char* line;
};

/**
 * Plansza.
 * `lines` to liczba linijek na planszy, `board` opisuje linijki.
 */
struct board {
  int lines;
  struct line* board;
};

// Obsluga stosu stanow planszy.
struct list {
  Tstate state;
  struct list* next;
};
typedef struct list Tstack;

void init(Tstack** s) { *s = NULL; }

bool empty(Tstack* s) { return s == NULL; }

void push(Tstack** s, Tstate x) {
  Tstack* tmp;
  tmp = malloc(sizeof(*tmp));
  tmp->next = *s;
  tmp->state = x;
  *s = tmp;
}

void pop(Tstack** s, Tstate* x) {
  *x = (*s)->state;
  Tstack* tmp = *s;
  *s = (*s)->next;
  free(tmp);
}

void top(Tstack** s, Tstate* x) {
  pop(s, x);
  push(s, *x);
}

void clear(Tstack** s) {
  while (!empty(*s)) {
    Tstate x;
    pop(s, &x);
  }
}

int more(int n) { return 1 + n * MULTIPLIER / DIVISOR; }

// Realokuje plansze, jesli trzeba.
void realloc_board(struct board* board, int* size, int n) {
  if (n == *size) {
    *size = more(*size);
    board->board = realloc(board->board, (*size) * sizeof *(board->board));
  }
}

// Realokuje linijke planszy, jesli trzeba.
void realloc_line(struct line* line, int* size, int n) {
  if (n == *size) {
    *size = more(*size);
    line->line = realloc(line->line, (*size) * sizeof *(line->line));
  }
}

void init_state(Tstate* state) {
  for (int i = 0; i < 26; ++i) {
    state->box[i].line = NOBOX;
    state->box[i].col = NOBOX;
  }
}

// Wczytuje plansze i jej poczatkowy stan.
void read_board(struct board* board, Tstate* state) {
  init_state(state);
  board->board = NULL;
  int c = getchar(), line = 0, col = 0, lineSize = 0, boardSize = 0;
  realloc_board(board, &boardSize, line);
  board->board[line].line = NULL;
  while (c != '\n') {
    realloc_line(&(board->board[line]), &lineSize, col);
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
      board->board[line].len = col;
      char ch;
      int fLetter;
      if ('A' <= c && c <= 'Z') {
        fLetter = 'A';
        ch = '+';
      } else {
        fLetter = 'a';
        ch = '-';
      }
      board->board[line].line[col] = ch;
      state->box[c - fLetter].line = line;
      state->box[c - fLetter].col = col;
    } else {
      switch (c) {
        case '-':
          board->board[line].line[col] = '-';
          break;
        case '+':
          board->board[line].line[col] = '+';
          break;
        case '#':
          board->board[line].line[col] = '#';
          break;
        case '@':
          board->board[line].line[col] = '-';
          state->player.line = line;
          state->player.col = col;
          break;
        case '*':
          board->board[line].line[col] = '+';
          state->player.line = line;
          state->player.col = col;
          break;
      }
    }
    col++;
    board->board[line].len = col;
    c = getchar();
    if (c == '\n') {
      line++;
      board->lines = line;
      lineSize = 0;
      realloc_board(board, &boardSize, line);
      col = 0;
      board->board[line].line = NULL;
      c = getchar();
    }
  }
}

// Zamienia pusta plansze na plansze w pewnym stanie.
void apply_state_to_board(Tstate state, struct board* board) {
  for (int i = 0; i < 26; ++i) {
    if (state.box[i].line != NOBOX) {
      if (board->board[state.box[i].line].line[state.box[i].col] == '-')
        board->board[state.box[i].line].line[state.box[i].col] = i + 'a';
      else
        board->board[state.box[i].line].line[state.box[i].col] = i + 'A';
    }
  }
  if (board->board[state.player.line].line[state.player.col] == '-')
    board->board[state.player.line].line[state.player.col] = '@';
  else
    board->board[state.player.line].line[state.player.col] = '*';
}

// Zamienia plansze na pusta plansze.
void make_default(Tstate state, struct board* board) {
  for (int i = 0; i < 26; ++i) {
    if (state.box[i].line != NOBOX) {
      if ('a' <= board->board[state.box[i].line].line[state.box[i].col] &&
          board->board[state.box[i].line].line[state.box[i].col] <= 'z')
        board->board[state.box[i].line].line[state.box[i].col] = '-';
      else
        board->board[state.box[i].line].line[state.box[i].col] = '+';
    }
  }
  if (board->board[state.player.line].line[state.player.col] == '@')
    board->board[state.player.line].line[state.player.col] = '-';
  else
    board->board[state.player.line].line[state.player.col] = '+';
}

// Przygotowuje program do gry.
void init_sequence(Tstack** states, struct board* board) {
  Tstate state;
  read_board(board, &state);
  init(states);
  push(states, state);
  apply_state_to_board(state, board);
}

// Tablica pomocnicza do konwersji kierunku pchniecia na zmiane polozenia.
int Conversion[4][2] = {{1, 0},    // 2
                        {0, -1},   // 4
                        {0, 1},    // 6
                        {-1, 0}};  // 8

// Zamienia nazwe skrzyni na jej indeks w tablicy skrzyn.
int turn_name_to_index(int name) {
  int fLetter;
  if ('A' <= name && name <= 'Z')
    fLetter = 'A';
  else
    fLetter = 'a';
  return name - fLetter;
}

/**
 * Zapisuje w `*dest` wspolrzedne komorki,
 * gdzie ma trafic skrzynia po pchnieciu `box``dir`.
 */
void translate_move_to_pos(struct pos* dest, Tstate state, int box, int dir) {
  dest->line = state.box[box].line + Conversion[dir][0];
  dest->col = state.box[box].col + Conversion[dir][1];
}

// Sprawdza czy da sie postawic gracza czy skrzynie na pozycje `pos`.
bool validate_pos(struct board board, struct pos pos) {
  bool canDoIt = false;
  if (-1 < pos.line && pos.line < board.lines && -1 < pos.col &&
      pos.col < board.board[pos.line].len) {
    char analyzedCh = board.board[pos.line].line[pos.col];
    if ((analyzedCh < 'a' || 'z' < analyzedCh) &&
        (analyzedCh < 'A' || 'Z' < analyzedCh) && (analyzedCh != '#'))
      canDoIt = true;
  }
  return canDoIt;
}

// Obsluga kolejki, ktora jest magazynem dla BFS.
struct qlist {
  struct pos pos;
  struct qlist* next;
};
typedef struct qlist Tqlist;

typedef struct {
  Tqlist* front;
  Tqlist* rear;
} Tqueue;

void initq(Tqueue* q) {
  q->front = NULL;
  q->rear = q->front;
}

bool emptyq(Tqueue q) { return q.front == NULL; }

void pushq(Tqueue* q, struct pos x) {
  if (q->front) {
    q->rear->next = malloc(sizeof *(q->rear->next));
    q->rear->next->pos = x;
    q->rear = q->rear->next;
  } else {
    q->rear = malloc(sizeof *(q->rear));
    q->rear->pos = x;
    q->front = q->rear;
  }
  q->rear->next = NULL;
}

void popq(Tqueue* q, struct pos* x) {
  *x = q->front->pos;
  Tqlist* tmp = q->front;
  q->front = q->front->next;
  free(tmp);
}

void clearq(Tqueue* q) {
  while (!emptyq(*q)) {
    struct pos x;
    popq(q, &x);
  }
}

/**
 * Inicjuje tablice,
 * ktora przechowuje informacje o przetworzonych komorkach planszy.
 */
void init_discovered(bool*** t, struct board board) {
  *t = malloc(board.lines * sizeof(bool*));
  for (int i = 0; i < board.lines; ++i)
    (*t)[i] = malloc(board.board[i].len * sizeof *((*t)[i]));
  for (int i = 0; i < board.lines; ++i) {
    for (int j = 0; j < board.board[i].len; ++j) (*t)[i][j] = false;
  }
}

// Zwalnia pamiec zaalokowana przez tablice przetworzonych komorek planszy.
void clear_discovered(bool*** t, struct board board) {
  for (int i = 0; i < board.lines; ++i) free((*t)[i]);
  free(*t);
}

// Wylicza i zapisuje w tablicy `t` wspolrzedne sasiadow komorki x.
void calc_neighbours(struct pos t[4], struct pos x) {
  for (int i = 0; i < 4; ++i) {
    t[i].line = x.line + Conversion[i][0];
    t[i].col = x.col + Conversion[i][1];
  }
}

/**
 * Zapisuje do magazynu wspolrzedne tych sasiadow,
 * gdzie mozna postawic skrzynie lub gracza.
 */
void add_neighbours(struct pos t[4], struct board board, Tqueue* container,
                    bool** Discovered) {
  for (int i = 0; i < 4; ++i) {
    if (validate_pos(board, t[i]) && !Discovered[t[i].line][t[i].col]) {
      Discovered[t[i].line][t[i].col] = true;
      pushq(container, t[i]);
    }
  }
}

/**
 * Sprawdza czy istnieje sciezka prowadzaca gracza do pozycji,
 * z ktorej musi wykonac pchniecie.
 */
bool find_path(struct board board, struct pos player, struct pos dest) {
  bool **Discovered, succ = false;
  init_discovered(&Discovered, board);
  Tqueue container;
  initq(&container);
  pushq(&container, player);
  Discovered[player.line][player.col] = true;
  while (!emptyq(container) && !succ) {
    struct pos current, neighbours[4];
    popq(&container, &current);
    succ = (current.line == dest.line && current.col == dest.col);
    calc_neighbours(neighbours, current);
    add_neighbours(neighbours, board, &container, Discovered);
  }
  clear_discovered(&Discovered, board);
  clearq(&container);
  return succ;
}

// Obsluga ruchu `0`.
void undo(Tstack** states, struct board* board) {
  Tstate state;
  pop(states, &state);
  if (!empty(*states)) {
    make_default(state, board);
    top(states, &state);
    apply_state_to_board(state, board);
  } else {
    push(states, state);
  }
}

// Zmienia stan na stan po ruchu `box``dir`.
void change_state(Tstate* state, int box, struct pos newBoxPos) {
  state->player.line = state->box[box].line;
  state->player.col = state->box[box].col;
  state->box[box] = newBoxPos;
}

// Wykonuje ruch `box``dir`.
void make_move(Tstack** states, struct board* board, int box,
               struct pos newBoxPos) {
  Tstate currentState;
  top(states, &currentState);
  Tstate newState = currentState;
  change_state(&newState, box, newBoxPos);
  push(states, newState);
  make_default(currentState, board);
  apply_state_to_board(newState, board);
}

// Wypisuje plansze.
void write_board(struct board board) {
  for (int i = 0; i < board.lines; ++i) {
    for (int j = 0; j < board.board[i].len; ++j)
      putchar(board.board[i].line[j]);
    putchar('\n');
  }
}

// Obsluga kolejnych ruchow gracza.
void sokoban(Tstack** states, struct board* board) {
  int c = getchar();
  while (c != '.') {
    if (c - '0') {
      Tstate state;
      top(states, &state);
      c = turn_name_to_index(c);
      int dir = getchar();
      dir -= '0';
      struct pos boxNewPos;
      translate_move_to_pos(&boxNewPos, state, c, dir / 2 - 1);
      /* Funkcja uzyta nizej dla obliczenia wspolrzednych gracza takich,
      zeby mogl popchnac skrzynie. */
      struct pos dest;
      translate_move_to_pos(&dest, state, c, (10 - dir) / 2 - 1);
      if (validate_pos(*board, boxNewPos) && validate_pos(*board, dest) &&
          find_path(*board, state.player, dest))
        make_move(states, board, c, boxNewPos);
    } else {
      undo(states, board);
    }
    write_board(*board);
    c = getchar();
    c = getchar();
  }
}

// Sprzata po programie.
void clean_up(struct board* board, Tstack** states) {
  for (int i = 0; i < board->lines; ++i) free(board->board[i].line);
  free(board->board);
  clear(states);
}

int main() {
  struct board board;
  Tstack* states;
  init_sequence(&states, &board);
  write_board(board);
  sokoban(&states, &board);
  clean_up(&board, &states);
  return 0;
}
