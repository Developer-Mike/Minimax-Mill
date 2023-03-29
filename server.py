print("Compiling...")
import cppyy
cppyy.cppdef(open("core.cpp").read())
import cppyy.gbl as core
print("Compiled.")

board = core.Board()

import json

def move_to_dict(move: core.Move) -> dict:
    return {
        "from": {
            "x": move.fromTile.x,
            "y": move.fromTile.y
        },
        "to": {
            "x": move.toTile.x,
            "y": move.toTile.y
        },
        "removed": {
            "x": move.removedTile.x,
            "y": move.removedTile.y
        }
    }

def start(json: dict):
    global board

    board = core.Board()
    return {
        "success": True,
        "board": str(board.toString()),
        "isBlackTurn": board.isBlackTurn
    }

def get_board(json: dict):
    return {
        "success": True,
        "board": str(board.toString()),
        "isBlackTurn": board.isBlackTurn,
        "scoreBlack": board.evaluateScore(True)
    }

# {
#   "from": {
#     "x": 0,
#     "y": 0
#   },
#   "to": {
#     "x": 0,
#     "y": 0
#   },
#   "removed": {
#     "x": 0,
#     "y": 0
#   }
# }
def register_move(json: dict):
    move = core.Move(
        core.TilePosition(json["from"]["x"], json["from"]["y"]), 
        core.TilePosition(json["to"]["x"], json["to"]["y"]), 
        core.TilePosition(json["removed"]["x"], json["removed"]["y"])
    )

    if move not in board.getPossibleMoves():
        return {
            "success": False,
            "message": "Invalid move."
        }

    board.makeMove(move)
    return {
        "success": True,
        "board": str(board.toString())
    }

def get_moves(json: dict):
    return {
        "success": True,
        "moves": [move_to_dict(move) for move in board.getPossibleMoves()]
    }

def ai_move(json: dict):
    move = core.getBestMove(board, 6)
    board.makeMove(move)
    
    return {
        "success": True,
        "move": move_to_dict(move),
        "board": str(board.toString())
    }

from shared import Functions
FUNCTIONS_MAP = {
    Functions.START: start,
    Functions.GET_BOARD: get_board,
    Functions.REGISTER_MOVE: register_move,
    Functions.GET_MOVES: get_moves,
    Functions.AI_MOVE: ai_move
}

import socket
s = None
client = None

def listen():
    global s, client

    s = socket.socket()
    ip = socket.gethostbyname(socket.gethostname())

    s.bind((ip, 3000))

    print("Listening on", ip)
    s.listen(1)

    client, address = s.accept()
    print("Connected to", address)

while True:
    if client is None:
        listen()

    try:
        data = client.recv(1024).decode("utf-8")
        data = json.loads(data)

        print("Received:", data)

        function = FUNCTIONS_MAP.get(data["function"])
        if function is not None:
            response = json.dumps(function(data["parameters"]))
            client.send(bytes(response, "utf-8"))
        else:
            print("Unknown function:", data)
    except:
        client = None
        print("Connection lost.")