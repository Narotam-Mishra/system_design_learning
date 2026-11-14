#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Move {
    int from;
    int diceValue;
    int landedAt;
    int destination;
};

struct Solution {
    bool reachable = false;
    vector<Move> moves;
};

class Board {
private:
    int finalCell;
    int diceFaces;
    unordered_map<int, int> jumps;

public:
    explicit Board(int finalCell, int diceFaces = 6)
        : finalCell(finalCell), diceFaces(diceFaces) {
        if (finalCell < 2) {
            throw invalid_argument("The board must contain at least two cells");
        }
        if (diceFaces < 1) {
            throw invalid_argument("The dice must have at least one face");
        }
    }

    void addSnake(int head, int tail) {
        if (head <= tail) {
            throw invalid_argument("A snake's head must be above its tail");
        }
        addJump(head, tail);
    }

    void addLadder(int bottom, int top) {
        if (bottom >= top) {
            throw invalid_argument("A ladder's top must be above its bottom");
        }
        addJump(bottom, top);
    }

    int getFinalCell() const { return finalCell; }
    int getDiceFaces() const { return diceFaces; }

    int destinationAfterLanding(int cell) const {
        auto jump = jumps.find(cell);
        return jump == jumps.end() ? cell : jump->second;
    }

    string jumpDescription(int cell) const {
        auto jump = jumps.find(cell);
        if (jump == jumps.end()) return "";
        return jump->second > cell ? "ladder" : "snake";
    }

private:
    void addJump(int start, int end) {
        if (start <= 1 || start >= finalCell || end <= 0 || end >= finalCell) {
            throw out_of_range("Snake/ladder cells must be inside the board");
        }
        if (jumps.count(start)) {
            throw invalid_argument("Only one snake or ladder may start at a cell");
        }
        jumps[start] = end;
    }
};

class SolverStrategy {
public:
    virtual ~SolverStrategy() = default;
    virtual Solution solve(const Board& board) const = 0;
    virtual string name() const = 0;
};

static vector<Move> reconstructPath(int finalCell,
                                    const vector<int>& parent,
                                    const vector<Move>& moveUsed) {
    vector<Move> path;
    for (int cell = finalCell; parent[cell] != -1; cell = parent[cell]) {
        path.push_back(moveUsed[cell]);
    }
    reverse(path.begin(), path.end());
    return path;
}

class BfsSolver : public SolverStrategy {
public:
    Solution solve(const Board& board) const override {
        const int finish = board.getFinalCell();
        vector<bool> visited(finish + 1, false);
        vector<int> parent(finish + 1, -1);
        vector<Move> moveUsed(finish + 1);
        queue<int> cells;

        visited[1] = true;
        cells.push(1);

        while (!cells.empty()) {
            int current = cells.front();
            cells.pop();

            if (current == finish) break;

            for (int dice = 1; dice <= board.getDiceFaces(); ++dice) {
                int landedAt = current + dice;
                if (landedAt > finish) continue;  // An exact roll is required.

                int next = board.destinationAfterLanding(landedAt);
                if (!visited[next]) {
                    visited[next] = true;
                    parent[next] = current;
                    moveUsed[next] = {current, dice, landedAt, next};
                    cells.push(next);
                }
            }
        }

        if (!visited[finish]) return {};
        return {true, reconstructPath(finish, parent, moveUsed)};
    }

    string name() const override { return "BFS (minimum throws)"; }
};

class DfsSolver : public SolverStrategy {
private:
    bool search(const Board& board, int current, vector<bool>& visited,
                vector<Move>& path) const {
        if (current == board.getFinalCell()) return true;

        visited[current] = true;
        for (int dice = 1; dice <= board.getDiceFaces(); ++dice) {
            int landedAt = current + dice;
            if (landedAt > board.getFinalCell()) continue;

            int next = board.destinationAfterLanding(landedAt);
            if (visited[next]) continue;

            path.push_back({current, dice, landedAt, next});
            if (search(board, next, visited, path)) return true;
            path.pop_back();
        }
        return false;
    }

public:
    Solution solve(const Board& board) const override {
        vector<bool> visited(board.getFinalCell() + 1, false);
        vector<Move> path;
        bool reachable = search(board, 1, visited, path);
        return {reachable, reachable ? path : vector<Move>{}};
    }

    string name() const override { return "DFS (a valid path)"; }
};

static void printSolution(const Board& board, const SolverStrategy& solver,
                          const Solution& solution) {
    cout << "\n" << solver.name() << "\n";
    cout << string(solver.name().size(), '-') << "\n";

    if (!solution.reachable) {
        cout << "The final cell is unreachable.\n";
        return;
    }

    cout << "Throws required: " << solution.moves.size() << "\n";
    cout << "Path: 1";
    for (const Move& move : solution.moves) {
        cout << " --[roll " << move.diceValue << "]--> " << move.landedAt;
        if (move.destination != move.landedAt) {
            cout << " --[" << board.jumpDescription(move.landedAt) << "]--> "
                 << move.destination;
        }
    }
    cout << "\n";
}

int main() {
    try {
        Board board(100);

        // Ladders
        board.addLadder(2, 38);
        board.addLadder(7, 14);
        board.addLadder(8, 31);
        board.addLadder(15, 26);
        board.addLadder(21, 42);
        board.addLadder(28, 84);
        board.addLadder(36, 44);
        board.addLadder(51, 67);
        board.addLadder(71, 91);
        board.addLadder(78, 98);
        board.addLadder(87, 94);

        // Snakes
        board.addSnake(16, 6);
        board.addSnake(46, 25);
        board.addSnake(49, 11);
        board.addSnake(62, 19);
        board.addSnake(64, 60);
        board.addSnake(74, 53);
        board.addSnake(89, 68);
        board.addSnake(92, 88);
        board.addSnake(95, 75);
        board.addSnake(99, 80);

        vector<unique_ptr<SolverStrategy>> solvers;
        solvers.push_back(make_unique<BfsSolver>());
        solvers.push_back(make_unique<DfsSolver>());

        cout << "Snake and Ladder solver (start = 1, finish = 100)\n";
        for (const auto& solver : solvers) {
            printSolution(board, *solver, solver->solve(board));
        }
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
