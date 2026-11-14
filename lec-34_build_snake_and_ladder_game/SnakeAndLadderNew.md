# Solving Snake and Ladder with BFS and DFS

Yes, Snake and Ladder can be represented as a graph and searched with either
Breadth-First Search (BFS) or Depth-First Search (DFS).

## Graph representation

- Every board cell is a graph vertex.
- From a cell, a dice roll creates up to six outgoing edges.
- If the roll lands at the beginning of a ladder or the head of a snake, the
  edge ends at the ladder's top or the snake's tail.
- A move beyond cell 100 is ignored because this implementation requires an
  exact roll to win.

For example, from cell `1`, rolling `1` normally lands on cell `2`. Because a
ladder connects `2` to `38`, that graph edge is `1 -> 38`.

## Why BFS is the preferred strategy

BFS visits cells level by level. Level 1 contains cells reachable in one dice
throw, level 2 contains cells first reachable in two throws, and so on.
Consequently, the first time BFS reaches cell 100, it has found the minimum
number of dice throws.

The BFS implementation stores each cell's parent and the dice roll used to
reach it. After reaching cell 100, it follows those parent links backwards and
reverses them to print the complete shortest path.

## What DFS does

DFS follows one possible sequence of rolls as deeply as it can, then
backtracks when necessary. It can determine whether cell 100 is reachable and
produce a valid path. However, the first path found by DFS is not guaranteed to
use the fewest dice throws. A `visited` array also prevents infinite searching
when snakes and ladders introduce cycles in the graph.

For an actual shortest-path solver, use BFS. The DFS implementation is included
to demonstrate the difference between the two search strategies.

## Code structure

- `Board` validates and stores snakes, ladders, the final cell, and dice size.
- `SolverStrategy` defines a common solver interface.
- `BfsSolver` finds the minimum-throw path.
- `DfsSolver` finds any valid path.
- `Solution` and `Move` retain enough information to print every roll and every
  snake or ladder encountered.
- `main()` builds a complete 100-cell example board and runs both strategies.

## Compile and run

From the repository root:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic \
  lec-34_build_snake_and_ladder_game/SnakeAndLadderNew.cpp \
  -o /tmp/snake_and_ladder_solver

/tmp/snake_and_ladder_solver
```

## Complexity

Let `V` be the number of board cells and `F` the number of dice faces. Each
cell considers at most `F` rolls.

- BFS time: `O(V * F)`; space: `O(V)`.
- DFS time: `O(V * F)`; space: `O(V)` including recursion and visited state.

With a standard six-faced dice, `F` is constant, so both simplify to `O(V)`
time and `O(V)` space. Only BFS guarantees the minimum number of throws.
