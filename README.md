# cokoban

Sokoban game which can be played in the terminal.

Project works for a Linux system with a working gcc compiler, to run tests you
should have [valgrind](https://valgrind.org/). A [Makefile](Makefile) with a
default compilation command is supplied. It also has phony targets clean and
test. Running `make` produces a `build/sokoban` binary which operates on
standard input and standard output.

Here's how you can run the program from the project's root directory while using
one of the example boards from the [tests](tests) directory.

```bash
cat tests/board7.txt - | build/sokoban
```

## Introduction

[Sokoban](https://en.wikipedia.org/wiki/Sokoban) is a single player puzzle game
played on a two-dimensional board with square fields. Some board fields are
empty, on others there are walls or boxes. A certain number of fields is marked
as target fields. The target field can be an empty field or a field where a box
is. On one of the fields of the board there is a character controlled by the
player. It can move to fields adjacent to the current one, either vertically or
horizontally. In particular, adjacent fields to a field in line L, column C are:

- line L - 1, column C,
- line L + 1, column C,
- line L, column C - 1,
- line L, column C + 1.

The character can move to a field if it is empty or there is a box on it that
the character can push.

Pushing the box is possible if directly behind it, in the direction of the
movement of the character, there is an empty field. It is not possible to move
the character or push the box outside the board. It is not possible to push more
than one box at once.

The goal of the game is to place the crates on the target fields.

Unlike typical Sokoban implementations, the user does not have to give movements
moving the character through empty fields. The program itself determines how to
bring the character to the field from which it will be possible to push the box
in the indicated direction.

## Input format

The program's valid input is a description of the initial state of the board, an
empty line, and a sequence of instructions on separate lines, ending with a line
that starts with a dot `.`.

The program ignores the input content after the data-ending dot.

The description of the board consists of non-empty lines in which there are one
character representations of the fields:

- `-` - an empty field that is not the target field,
- `+` - the empty field that is the target field,
- `#` - the wall,
- `@` - field, which is not the target field on which the character is,
- `*` - the target field where the character is,
- `[a .. z](small letter)` - a field that is not a target field on which is a box with the given name,
- `[A .. Z](large letter)` - the target field on which is a box called a lowercase corresponding to the given capital letter.

In the correct description of the board there is exactly one character. Each box, marked with the letter of the Latin alphabet, can only occur once.

The program recognizes commands:

    [a .. z][2 | 4 | 6 | 8](a small letter after which there is a number 2, 4, 6or 8)

    Pushing a box with the name, which is the first command sign, in the direction specified by the second character. The number 2means pushing down, 8up, 4left a 6to the right.

    On the field from which the push can be made, the character is fed by a path consisting of empty fields. There can't be a wall or a chest on it.

    If the execution of the push is not possible, because the character cannot approach the chest or the chest cannot be pushed, the state of the board does not change.

    0

    Withdrawal of the last performed and not yet withdrawn push.

    The character returns to the field where she was before the backward push.

    If there was no push that could be undone, the command does not change the state of the board.

Character of the result

The result of the program is a string of board descriptions. The first is a description of the initial state and the next are descriptions of the state of the board after each command is executed.

The description of the board as a result of the program is in the form of such as in the data, but without the end of the blank line.
