#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <array>
#include <iterator>
#include <map>
#include <limits>
#include <set>
using namespace std;

const int infinity = numeric_limits<int>::infinity();

enum GameState {
    BEGINNING,
    NORMAL,
    ENDING,
    FINISHED
};

struct Board {
    array<array<char, 8>, 3> array = {{
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
    }};

    GameState gameState = BEGINNING;
    map<char, int> moveAmount = {{'b', 0}, {'w', 0}};
};

struct TilePosition {
    int ring = -1;
    int i = -1;

    string toString() {
        return "(" + to_string(ring) + ":" + to_string(i) + ")";
    }

    char value(Board board) {
        wrapAround();
        return board.array[ring][i];
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

map<char, int> countTiles(Board board) {
    map<char, int> pieceCount = {
        {'b', 0},
        {'w', 0},
        {' ', 0}
    };

    for (int ringI = 0; ringI < 3; ringI++) {
        for (int i = 0; i < 8; i++) {
            pieceCount[TilePosition{ringI, i}.value(board)]++;
        }
    }

    return pieceCount;
}

bool isInMill(Board board, TilePosition tilePosition, bool isBlack) {
    // Ignore passed tile, because it is for willCreateMill

    if (tilePosition.i % 2 != 0) {
        // Check row
        TilePosition leftTile = tilePosition;
        leftTile.i--;

        TilePosition rightTile = tilePosition;
        rightTile.i++;

        if (leftTile.value(board) == (isBlack ? 'b' : 'w') && leftTile.value(board) == rightTile.value(board)) {
            return true;
        }

        // Check for ring
        set<char> otherTileValues;
        int arrayI = 0;
        for (int ringI = 0; ringI < 3; ringI++) {
            if (tilePosition.ring == ringI) continue;

            otherTileValues.insert(TilePosition{ringI, tilePosition.i}.value(board));
            arrayI++;
        }

        if (otherTileValues.size() == 1 && *(otherTileValues.begin()) == (isBlack ? 'b' : 'w')) {
            return true;
        }
    } else  {
        // Check edge TODO
        for (int direction = -1; direction <= 1; direction += 2) {
            set<char> otherTileSet;
            for (int i = 1; i < 3; i++) {
                otherTileSet.insert(TilePosition{tilePosition.ring, tilePosition.i + (direction * i)}.value(board));
            }

            if (otherTileSet.size() == 1 && *(otherTileSet.begin()) == (isBlack ? 'b' : 'w')) {
                return true;
            }
        }
    }
    return false;
}

int evaluateBoard(Board board) {
    map<char, int> pieceCount = countTiles(board);

    return (pieceCount['b'] - pieceCount['w']);
}

bool canMoveThere(Board board, TilePosition targetPosition) {
    return targetPosition.value(board) == ' ';
}

list<Move> createListWithRemovedTiles(Board board, TilePosition from, TilePosition to, bool isBlack) {
    list<Move> moves;

    if (!isInMill(board, to, isBlack)) {
        moves.push_back(Move{from, to, {}});
    } else {
        for (int ringI = 0; ringI < 3; ringI++) {
            for (int i = 0; i < 8; i++) {
                TilePosition tilePosition = {ringI, i};
                char tilePositionValue = tilePosition.value(board);

                if (tilePositionValue == ' ') continue;
                if (tilePositionValue == 'b' && isBlack) continue;
                if (tilePositionValue == 'w' && !isBlack) continue;
                
                if (!isInMill(board, tilePosition, !isBlack)) {
                    moves.push_back(Move{from, to, tilePosition});
                }
            }
        }
    }

    return moves;
}

list<Move> getPossibleMovesForTile(Board board, TilePosition tilePosition, bool isBlack) {
    list<Move> possibleMoves;
    list<TilePosition> targetPositions;

    if (tilePosition.ring % 2 != 0) {
        targetPositions.push_back(TilePosition{tilePosition.ring - 1, tilePosition.i});
        targetPositions.push_back(TilePosition{tilePosition.ring + 1, tilePosition.i});
    }

    targetPositions.push_back(TilePosition{tilePosition.ring, tilePosition.i - 1});
    targetPositions.push_back(TilePosition{tilePosition.ring, tilePosition.i + 1});

    for (TilePosition targetPosition : targetPositions) {
        targetPosition.wrapAround();

        if (canMoveThere(board, targetPosition)) {
            possibleMoves.splice(possibleMoves.end(), createListWithRemovedTiles(board, tilePosition, targetPosition, isBlack));
        }
    }

    return possibleMoves;
}

list<Move> getPossibleEndingMovesForTile(Board board, TilePosition tilePosition, bool isBlack) {
    list<Move> possibleMoves;

    for (int targetRingI = 0; targetRingI < 3; targetRingI++) {
        for (int targetI = 0; targetI < 8; targetI++) {
            TilePosition targetTilePosition = {targetRingI, targetI};
            possibleMoves.splice(possibleMoves.end(), createListWithRemovedTiles(board, tilePosition, targetTilePosition, isBlack));
        }
    }

    return possibleMoves;
}

list<Move> getPossibleMoves(Board board, bool isBlack) {
    list<Move> possibleMoves;
    int ownTileCount = countTiles(board)[(isBlack ? 'b' : 'w')];

    if (board.gameState == BEGINNING && board.moveAmount[(isBlack ? 'b' : 'w')] < 9) { // If game is at start
        for (int ringI = 0; ringI < 3; ringI++) {
            for (int i = 0; i < 8; i++) {
                TilePosition targetPosition = {ringI, i};
                if (canMoveThere(board, targetPosition)) { // If field is empty
                    possibleMoves.splice(possibleMoves.end(), createListWithRemovedTiles(board, {}, targetPosition, isBlack));
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
                
                possibleMoves.splice(possibleMoves.end(), (board.gameState == (ENDING && ownTileCount == 3) ? 
                    getPossibleEndingMovesForTile(board, tilePosition, isBlack) :
                    getPossibleMovesForTile(board, {ringI, i}, isBlack)));
            }
        }
    }

    return possibleMoves;
}

Board makeMove(Board board, Move move, bool isBlack) {
    Board newBoard = board;

    if (move.from_.ring != -1) newBoard.array.at(move.from_.ring).at(move.from_.i) = ' ';
    if (move.to.ring != -1) newBoard.array.at(move.to.ring).at(move.to.i) = (isBlack ? 'b' : 'w');
    if (move.removeTile.ring != -1) newBoard.array.at(move.removeTile.ring).at(move.removeTile.i) = ' ';

    newBoard.moveAmount[move.to.value(board)]++;

    map<char, int> tileCount = countTiles(newBoard);
    if (board.gameState == BEGINNING && (board.moveAmount['b'] >= 9 && board.moveAmount['w'] >= 9)) {
        newBoard.gameState = NORMAL;
    } else if (board.gameState == NORMAL && (tileCount['b'] == 3 || tileCount['w'] == 3)) {
        newBoard.gameState = ENDING;
    } else if (board.gameState == ENDING && (tileCount['b'] < 3 || tileCount['w'] < 3)) {
        newBoard.gameState = FINISHED;
    }

    return newBoard;
}

int minimax(Board board, int depth, bool isBlack, int alpha = -infinity, int beta = infinity) {
    if (depth == 0 || board.gameState == FINISHED) {
        return evaluateBoard(board);
    }

    if (isBlack) {
        int maxEval = -infinity;

        list<Move> moves = getPossibleMoves(board, isBlack);
        for (Move move : moves) {
            Board newBoard = makeMove(board, move, isBlack);

            int eval = minimax(newBoard, depth - 1, !isBlack, alpha, beta);
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);

            if (beta <= alpha) {
                break;
            }
        }

        return maxEval;
    } else {
        int minEval = infinity;

        list<Move> moves = getPossibleMoves(board, isBlack);
        for (Move move : moves) {
            Board newBoard = makeMove(board, move, isBlack);

            int eval = minimax(newBoard, depth - 1, !isBlack, alpha, beta);
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            
            if (beta <= alpha) {
                break;
            }
        }

        return minEval;
    }
}

int main() {
    Board debugBoard;

    printBoard(debugBoard);

    list<Move> possibleMoves = getPossibleMoves(debugBoard, false);
    for (Move move : possibleMoves) {
        cout << move.toString() << endl;
    }
    
    return 0;
}