#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <array>
#include <iterator>
#include <map>
#include <set>
#include <limits>
#include <tuple>
using namespace std;

const int infinity = numeric_limits<int>::max();

enum GameState {
    BEGINNING,
    NORMAL,
    ENDING,
    FINISHED
};

struct Board {
    array<int, 2> moveAmount = {0, 0};
    array<array<char, 8>, 3> array = {{
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
    }};

    GameState gameState = BEGINNING;
};

struct TilePosition {
    int ring = -1;
    int i = -1;

    string toString() {
        return "(" + to_string(ring) + ":" + to_string(i) + ")";
    }

    char value(Board* board) {
        wrapAround();
        return board->array[ring][i];
    }

    void wrapAround() {
        if (i < 0) i = 7;
        if (i > 7) i = 0;
    }
};

struct Move {
    TilePosition from_;
    TilePosition to;
    TilePosition removeTile;

    string toString() {
        return "Move from " + from_.toString() + " to " + to.toString() + " remove " + removeTile.toString();
    }
};

void printBoard(Board board) {
    cout << board.array[0][0] << "--" << board.array[0][1] << "--" << board.array[0][2] << endl;
    cout << "|" << board.array[1][0] << "-" << board.array[1][1] << "-" << board.array[1][2] << "|" << endl;
    cout << "||" << board.array[2][0] << board.array[2][1] << board.array[2][2] << "||" << endl;

    cout << board.array[0][7] << board.array[1][7] << board.array[2][7] << " " << board.array[2][3] << board.array[1][3] << board.array[0][3] << endl;

    cout << "||" << board.array[2][6] << board.array[2][5] << board.array[2][4] << "||" << endl;
    cout << "|" << board.array[1][6] << "-" << board.array[1][5] << "-" << board.array[1][4] << "|" << endl;
    cout << board.array[0][6] << "--" << board.array[0][5] << "--" << board.array[0][4] << endl;
}

array<int, 2> countTiles(Board* board) {
    int bCount = 0;
    int wCount = 0;

    for (int ringI = 0; ringI < 3; ringI++) {
        for (int i = 0; i < 8; i++) {
            switch ((*board).array[ringI][i]) {
                case 'b':
                    bCount++;
                    break;
                case 'w':
                    wCount++;
                    break;
                default:
                    break;
            }
        }
    }

    return {bCount, wCount};
}

bool isInMill(Board* board, TilePosition* tilePosition, bool isBlack) {
    if ((*tilePosition).i % 2 != 0) {
        // Check row
        TilePosition leftTile = (*tilePosition);
        leftTile.i--;

        TilePosition rightTile = (*tilePosition);
        rightTile.i++;

        if (leftTile.value(board) == (isBlack ? 'b' : 'w') && leftTile.value(board) == rightTile.value(board)) {
            return true;
        }

        // Check for ring
        set<char> otherTileValues;
        int arrayI = 0;
        for (int ringI = 0; ringI < 3; ringI++) {
            if ((*tilePosition).ring == ringI) continue;

            otherTileValues.insert(TilePosition{ringI, (*tilePosition).i}.value(board));
            arrayI++;
        }

        if (otherTileValues.size() == 1 && *(otherTileValues.begin()) == (isBlack ? 'b' : 'w')) {
            return true;
        }
    } else  {
        // Check edge
        for (int relI = 0; relI < 3; relI++) {
            /*set<char> otherTileSet;
            for (int i = 1; i < 3; i++) {
                otherTileSet.insert(TilePosition{(*tilePosition).ring, (*tilePosition).i + (direction * i)}.value(board));
            }

            if (otherTileSet.size() == 1 && *(otherTileSet.begin()) == (isBlack ? 'b' : 'w')) {
                return true;
            }*/
        }
    }
    return false;
}

int evaluateBoard(Board* board) {
    array<int, 2> pieceCount = countTiles(board);
    return (pieceCount[0] - pieceCount[1] - (board->moveAmount[0] - board->moveAmount[1]));
}

bool canMoveThere(Board* board, TilePosition* targetPosition) {
    return targetPosition->value(board) == ' ';
}

Board makeMove(Board board, Move move, bool isBlack) {
    if (move.from_.ring != -1) board.array.at(move.from_.ring).at(move.from_.i) = ' ';
    if (move.to.ring != -1) board.array.at(move.to.ring).at(move.to.i) = (isBlack ? 'b' : 'w');
    if (move.removeTile.ring != -1) board.array.at(move.removeTile.ring).at(move.removeTile.i) = ' ';

    ++board.moveAmount[!isBlack];

    array<int, 2> tileCount = countTiles(&board);
    if (board.gameState == BEGINNING && (board.moveAmount[0] >= 9 && board.moveAmount[1] >= 9)) {
        board.gameState = NORMAL;
    } else if (board.gameState == NORMAL && (tileCount[0] == 3 || tileCount[1] == 3)) {
        board.gameState = ENDING;
    } else if (board.gameState == ENDING && (tileCount[0] < 3 || tileCount[1] < 3)) {
        board.gameState = FINISHED;
    }

    return board;
}

list<Move> createListWithRemovedTiles(Board* board, TilePosition from, TilePosition* to, bool isBlack) {
    list<Move> moves;

    Board newBoard = makeMove((*board), Move{from, (*to), {}}, isBlack);
    if (!isInMill(&newBoard, to, isBlack)) {
        moves.push_back(Move{from, (*to), {}});
    } else {
        for (int ringI = 0; ringI < 3; ringI++) {
            for (int i = 0; i < 8; i++) {
                TilePosition tilePosition = {ringI, i};
                char tilePositionValue = tilePosition.value(board);

                if (tilePositionValue == ' ') continue;
                if (tilePositionValue == 'b' && isBlack) continue;
                if (tilePositionValue == 'w' && !isBlack) continue;
                
                if (!isInMill(board, &tilePosition, !isBlack)) {
                    moves.push_back(Move{(from), (*to), tilePosition});
                }
            }
        }
    }

    return moves;
}

list<Move> getPossibleMovesForTile(Board* board, TilePosition* tilePosition, bool isBlack) {
    list<Move> possibleMoves;
    list<TilePosition> targetPositions;

    if (tilePosition->ring % 2 != 0) {
        targetPositions.push_back(TilePosition{tilePosition->ring - 1, tilePosition->i});
        targetPositions.push_back(TilePosition{tilePosition->ring + 1, tilePosition->i});
    }

    targetPositions.push_back(TilePosition{tilePosition->ring, tilePosition->i - 1});
    targetPositions.push_back(TilePosition{tilePosition->ring, tilePosition->i + 1});

    for (TilePosition targetPosition : targetPositions) {
        targetPosition.wrapAround();

        if (canMoveThere(board, &targetPosition)) {
            possibleMoves.splice(possibleMoves.end(), createListWithRemovedTiles(board, (*tilePosition), &targetPosition, isBlack));
        }
    }

    return possibleMoves;
}

list<Move> getPossibleEndingMovesForTile(Board* board, TilePosition* tilePosition, bool isBlack) {
    list<Move> possibleMoves;

    for (int targetRingI = 0; targetRingI < 3; targetRingI++) {
        for (int targetI = 0; targetI < 8; targetI++) {
            TilePosition targetTilePosition = {targetRingI, targetI};
            possibleMoves.splice(possibleMoves.end(), createListWithRemovedTiles(board, (*tilePosition), &targetTilePosition, isBlack));
        }
    }

    return possibleMoves;
}

list<Move> getPossibleMoves(Board* board, bool isBlack) {
    list<Move> possibleMoves;
    int ownTileCount = countTiles(board)[!isBlack];

    if (board->gameState == BEGINNING && board->moveAmount[!isBlack] < 9) { // If game is at start
        for (int ringI = 0; ringI < 3; ringI++) {
            for (int i = 0; i < 8; i++) {
                TilePosition targetPosition = {ringI, i};
                if (canMoveThere(board, &targetPosition)) { // If field is empty
                    possibleMoves.splice(possibleMoves.end(), createListWithRemovedTiles(board, {}, &targetPosition, isBlack));
                }
            }
        }
    } else { // Normal Game
        for (int ringI = 0; ringI < 3; ringI++) {
            for (int i = 0; i < 8; i++) {
                TilePosition tilePosition = {ringI, i};
                char tileValue = tilePosition.value(board);

                if (tileValue == ' ') continue;
                if (tileValue == 'b' && !isBlack) continue;
                if (tileValue == 'w' && isBlack) continue;
                
                possibleMoves.splice(possibleMoves.end(), (board->gameState == (ENDING && ownTileCount == 3) ? 
                    getPossibleEndingMovesForTile(board, &tilePosition, isBlack) :
                    getPossibleMovesForTile(board, &tilePosition, isBlack)));
            }
        }
    }

    possibleMoves.sort([](Move a, Move b) {
        return a.removeTile.ring != -1 && b.removeTile.ring == -1;
    });

    return possibleMoves;
}

int minimax(Board* board, int depth, bool isBlack, int alpha = -infinity, int beta = infinity) {
    if (depth == 0 || board->gameState == FINISHED) {
        return evaluateBoard(board);
    }

    if (isBlack) {
        int maxEval = -infinity;

        list<Move> moves = getPossibleMoves(board, isBlack);
        for (Move move : moves) {
            Board newBoard = makeMove(*board, move, isBlack);

            int eval = minimax(&newBoard, depth - 1, !isBlack, alpha, beta);
            maxEval = max(maxEval, eval);

            alpha = max(alpha, maxEval);
            if (beta <= alpha) {
                break;
            }
        }

        return maxEval;
    } else {
        int minEval = infinity;

        list<Move> moves = getPossibleMoves(board, isBlack);
        for (Move move : moves) {
            Board newBoard = makeMove(*board, move, isBlack);

            int eval = minimax(&newBoard, depth - 1, !isBlack, alpha, beta);
            minEval = min(minEval, eval);

            beta = min(beta, minEval);
            if (beta <= alpha) {
                break;
            }
        }

        return minEval;
    }
}

int main() {
    Board debugBoard;

    debugBoard = makeMove(debugBoard, (*getPossibleMoves(&debugBoard, false).begin()), false);
    printBoard(debugBoard);

    list<Move> possibleMoves = getPossibleMoves(&debugBoard, true);

    Board newBoard = makeMove(debugBoard, (*possibleMoves.begin()), true);
    cout << minimax(&newBoard, 6, true) << endl;
    
    return 0;
}