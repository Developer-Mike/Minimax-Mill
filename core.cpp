#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <list>
#include <array>
#include <limits>
#include <cmath>
#include <thread>
#include <chrono>

#define INF numeric_limits<int>::max()
#define EMPTY ' '
#define BLACK 'B'
#define WHITE 'W'

using namespace std;

// Struct for a position on the board
struct TilePosition {
    int ring = -1;
    int _i = -1;

    void i(int newI) {
        _i = newI;

        if (_i < 0) _i += 8;
        if (_i > 7) _i -= 8;
    }
    const int i() const { return _i; }

    bool operator==(const TilePosition& other) const {
        return ring == other.ring && i() == other.i();
    }

    string toString() {
        return "(" + to_string(ring) + ":" + to_string(i()) + ")";
    }
};

// Struct for a move
struct Move {
    TilePosition fromTile;
    TilePosition toTile;
    TilePosition removedTile;

    bool operator==(const Move& other) const {
        return fromTile == other.fromTile &&
            toTile == other.toTile &&
            removedTile == other.removedTile;
    }

    string toString() {
        return "From: " + fromTile.toString() + " To: " + toTile.toString() + " Removed: " + removedTile.toString();
    }
};

// Struct for the game board
struct Board {
    // 2D array for the board
    array<array<char, 8>, 3> array = {{
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}
    }};
    bool isBlackTurn = false;
    int startingPhase = 18;

    bool gameOver() {
        std::array<int, 2> tileCount = countTiles();
        return (tileCount[0] < 3 || tileCount[1] < 3) && startingPhase <= 0;
    }

    bool _isInMill(TilePosition* tilePosition) {
        std::array<int, 2> startIs = {int(floor(tilePosition->i() / 2) * 2), int((tilePosition->i() % 2 == 0) ? (tilePosition->i() - 2) : -1)};
        
        for (int startI : startIs) {
            if (startI == -1) continue;
            std::array<char, 3> sideTiles;

            for (int relI = 0; relI < 3; relI++) {
                TilePosition sideTilePosition = {tilePosition->ring, startI + relI};
                sideTiles[relI] = array[sideTilePosition.ring][sideTilePosition.i()];
            }

            if (sideTiles[0] == sideTiles[1] && sideTiles[1] == sideTiles[2] && sideTiles[0] != EMPTY)
                return true;
        }

        // Check Crosshair
        if (tilePosition->i() % 2 != 0) {
            std::array<char, 3> crosshairTiles;

            for (int ringI = 0; ringI < 3; ringI++) {
                crosshairTiles[ringI] = array[ringI][tilePosition->i()];
            }

            if (crosshairTiles[0] == crosshairTiles[1] && crosshairTiles[1] == crosshairTiles[2] && crosshairTiles[0] != EMPTY)
                return true;
        }
        return false;
    }

    list<Move> _getPossibleRemovedTiles(Move* move) {
        list<Move> moves;

        Board newBoard = *this;
        newBoard.makeMove(move);

        if (!newBoard._isInMill(&move->toTile)) {
            moves.push_back(Move{move->fromTile, move->toTile, {}});
        } else {
            for (int ringI = 0; ringI < 3; ringI++) {
                for (int i = 0; i < 8; i++) {
                    char tilePositionValue = array[ringI][i];

                    if (tilePositionValue == EMPTY) continue;
                    if (tilePositionValue == BLACK && isBlackTurn) continue;
                    if (tilePositionValue == WHITE && !isBlackTurn) continue;
                    
                    TilePosition removeTilePosition = {ringI, i};
                    if (!newBoard._isInMill(&removeTilePosition))
                        moves.push_back(Move{move->fromTile, move->toTile, removeTilePosition});
                }
            }
        }

        return moves;
    }

    list<Move> _getStarterMoves() {
        list<Move> moves;

        for (int ringI = 0; ringI < 3; ringI++) {
            for (int i = 0; i < 8; i++) {
                if (array[ringI][i] != EMPTY) continue;

                Move move = {{}, {ringI, i}, {}};
                list<Move> moveVariants = _getPossibleRemovedTiles(&move);
                moves.splice(moves.end(), moveVariants);
            }
        }

        return moves;
    }

    list<Move> _getJumpMoves() {
        list<Move> moves;

        for (int fromRingI = 0; fromRingI < 3; fromRingI++) {
            for (int fromI = 0; fromI < 8; fromI++) {
                if (array[fromRingI][fromI] != (isBlackTurn ? BLACK : WHITE)) continue;

                for (int toRingI = 0; toRingI < 3; toRingI++) {
                    for (int toI = 0; toI < 8; toI++) {
                        if (array[toRingI][toI] != EMPTY) continue;

                        Move move = {{fromRingI, fromI}, {toRingI, toI}, {}};
                        list<Move> moveVariants = _getPossibleRemovedTiles(&move);
                        moves.splice(moves.end(), moveVariants);
                    }
                }
            }
        }

        return moves;
    }

    list<Move> _getNormalMoves() {
        list<Move> moves;
        list<TilePosition> targetPositions;

        for (int fromRingI = 0; fromRingI < 3; fromRingI++) {
            for (int fromI = 0; fromI < 8; fromI++) {
                if (array[fromRingI][fromI] != (isBlackTurn ? BLACK : WHITE)) continue;

                if (fromI % 2 != 0) {
                    targetPositions.push_back({fromRingI - 1, fromI});
                    targetPositions.push_back({fromRingI + 1, fromI});
                }

                targetPositions.push_back({fromRingI, fromI - 1});
                targetPositions.push_back({fromRingI, fromI + 1});

                for (TilePosition targetPosition : targetPositions) {
                    if (array[targetPosition.ring][targetPosition.i()] != EMPTY) continue;

                    Move move = {{fromRingI, fromI}, targetPosition, {}};
                    list<Move> moveVariants = _getPossibleRemovedTiles(&move);
                    moves.splice(moves.end(), moveVariants);
                }
            }
        }

        return moves;
    }

    list<Move> getPossibleMoves() {
        list<Move> moves;

        std::array<int, 2> tileCount = countTiles();
        int blackCount = tileCount[0];
        int whiteCount = tileCount[1];

        if (startingPhase > 0)
            moves = _getStarterMoves();
        else if ((isBlackTurn && blackCount == 3) || (!isBlackTurn && whiteCount == 3))
            moves = _getJumpMoves();
        else
            moves = _getNormalMoves();

        return moves;
    }

    // Make a move
    void makeMove(Move* move) {
        // Remove the tile
        if (move->removedTile.ring != -1)
            array[move->removedTile.ring][move->removedTile.i()] = EMPTY;

        // Move the tile
        if (move->fromTile.ring != -1) {
            array[move->toTile.ring][move->toTile.i()] = array[move->fromTile.ring][move->fromTile.i()];
            array[move->fromTile.ring][move->fromTile.i()] = EMPTY;
        } else {
            array[move->toTile.ring][move->toTile.i()] = isBlackTurn ? BLACK : WHITE;
        }

        // Switch turn
        isBlackTurn = !isBlackTurn;
        startingPhase--;
    }

    std::array<int, 2> countTiles() {
        int blackCount = 0;
        int whiteCount = 0;

        // For each field
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 3; j++) {
                // If the tile is black, add to black count
                if (array[j][i] == BLACK) {
                    blackCount++;
                } else if (array[j][i] == WHITE) {
                    whiteCount++;
                }
            }
        }

        return {blackCount, whiteCount};
    }

    // Evaluate the score for the current board
    int evaluateScore(bool isBlack) {
        std::array<int, 2> tileCount = countTiles();
        return isBlack ? tileCount[0] - tileCount[1] : tileCount[1] - tileCount[0];
    }

    string toString() {
        ostringstream ss;

        ss << array[0][0] << "-----" << array[0][1] << "-----" << array[0][2] << endl;
        ss << "|     |     |" << endl;
        ss << "| " << array[1][0] << "---" << array[1][1] << "---" << array[1][2] << " |" << endl;
        ss << "| |       | |" << endl;
        ss << "| | " << array[2][0] << "-" << array[2][1] << "-" << array[2][2] << " | |" << endl;
        ss << "| | |   | | |" << endl;
        ss << array[2][7] << "-" << array[1][7] << "-" << array[0][7] << "   " << array[0][3] << "-" << array[1][3] << "-" << array[2][3] << endl;
        ss << "| | |   | | |" << endl;
        ss << "| | " << array[2][6] << "-" << array[2][5] << "-" << array[2][4] << " | |" << endl;
        ss << "| |       | |" << endl;
        ss << "| " << array[1][6] << "---" << array[1][5] << "---" << array[1][4] << " |" << endl;
        ss << "|     |     |" << endl;
        ss << array[2][7] << "-----" << array[2][1] << "-----" << array[2][2] << endl;
                
        return ss.str();
    }
};

// Minimax algorithm based on https://www.geeksforgeeks.org/minimax-algorithm-in-game-theory-set-4-alpha-beta-pruning/
int _minimax(Board* board, int depth, bool isBlack, bool forBlack, int alpha = -INF, int beta = INF) {
    // If depth is reached or game over return the score
    if (depth == 0 || board->gameOver()) return board->evaluateScore(forBlack);

    list<Move> moves = board->getPossibleMoves();

    int maxEval = -INF;
    for (Move move : moves) {
        Board newBoard(*board);
        newBoard.makeMove(&move);

        // Recursively call minimax and get the best possible score
        int eval = _minimax(&newBoard, depth - 1, !isBlack, forBlack, alpha, beta);
        maxEval = max(maxEval, eval);

        // Alpha-beta pruning
        if (isBlack) alpha = max(alpha, eval);
        else beta = min(beta, -eval);

        if (beta <= alpha) break;
    }

    return maxEval;
}

void _launchMinimax(Board board, Move move, int depth, int &bestEval, Move &bestMove, int &progress, int total) {
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    // Make move
    Board newBoard(board);
    newBoard.makeMove(&move);

    // Minimax and if best move yet, update best move
    int eval = _minimax(&newBoard, depth, board.isBlackTurn, board.isBlackTurn);
    if (eval > bestEval) {
        bestEval = eval;
        bestMove = move;
    }

    // Update debug info
    progress++;
    long duration = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count();
    cout << "Processed " << progress << "/" << total << " solutions (" << duration << "ms" << "). \r";
}

// Function to get the best move using minimax
Move getBestMove(Board* board, int depth) {
    list<Move> possibleMoves = board->getPossibleMoves();
    list<thread> threads;

    int progress = 0;
    int bestEval = -INF;
    Move bestMove;

    // Launch thread for each possible move
    for (Move move : possibleMoves) {
        threads.push_back(thread(_launchMinimax, *board, move, depth, ref(bestEval), ref(bestMove), ref(progress), possibleMoves.size()));
    }
    // Wait for all threads to finish
    for (thread &thread : threads) thread.join();

    cout << endl << "Best eval: " << bestEval << endl;
    return bestMove;
}

// For testing
int main() {
    Board board;
    Move move;

    long total = 0;
    long max = 0;
    int count = 0;
    do {
        chrono::steady_clock::time_point start = chrono::steady_clock::now();

        move = getBestMove(&board, 5);
        cout << move.toString() << endl;

        long duration = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count();
        if (duration > max) max = duration;
        total += duration;
        count++;

        board.makeMove(&move);
        cout << board.toString() << endl;
    } while (!board.gameOver());

    cout << "Average: " << to_string(total / count) << "ms" << endl;
    cout << "Max: " << to_string(max) << "ms" << endl;

    int score = board.evaluateScore(true);
    if (score > 0) cout << "Black wins!" << endl;
    else if (score < 0) cout << "White wins!" << endl;
    else cout << "Draw!" << endl;

    return 0;
}