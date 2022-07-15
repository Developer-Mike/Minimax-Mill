print("Compiling...")
from dataclasses import field
import cppyy
cppyy.cppdef(open("main.cpp").read())
import cppyy.gbl as core
print("Compiled.")

from json.encoder import INFINITY
import tkinter
import tkinter.ttk as tk
import tqdm
import time

positions = {
    (0, 0): (0, 0),
    (0, 1): (0, 3),
    (0, 2): (0, 6),
    (0, 3): (3, 6),
    (0, 4): (6, 6),
    (0, 5): (6, 3),
    (0, 6): (6, 0),
    (0, 7): (3, 0),

    (1, 0): (1, 1),
    (1, 1): (1, 3),
    (1, 2): (1, 5),
    (1, 3): (3, 5),
    (1, 4): (5, 5),
    (1, 5): (5, 3),
    (1, 6): (5, 1),
    (1, 7): (3, 1),

    (2, 0): (2, 2),
    (2, 1): (2, 3),
    (2, 2): (2, 4),
    (2, 3): (3, 4),
    (2, 4): (4, 4),
    (2, 5): (4, 3),
    (2, 6): (4, 2),
    (2, 7): (3, 2),
}

root = tkinter.Tk()
pixel = tkinter.PhotoImage(width=40, height=40)

player_is_black = True
black_turn = True
depth = 6
board = core.Board({ 16, 16 }, { { ' ', ' ', ' ', 'w', ' ', 'b', 'w', 'w' }, { ' ', ' ', ' ', ' ', 'b', 'b', 'b', ' ' }, { ' ', ' ', ' ', 'b', ' ', 'b', ' ', ' ' } }, 2)
fields = [[], [], []]

def tile_position_equal(pos1, pos2):
    return pos1.ring == pos2.ring and pos1.i == pos2.i

def is_move_possible(move, possible_moves):
    for possible_move in possible_moves:
        if tile_position_equal(possible_move.from_, move.from_) and tile_position_equal(possible_move.to, move.to) and tile_position_equal(possible_move.removeTile, move.removeTile):
            return True
    return False
 
def make_move():
    global fields, player_is_black, black_turn, board, et_from_entry, et_to_entry, et_remove_entry

    if player_is_black and black_turn:
        black_turn = not black_turn

    # Player Move
    from_ = tuple([(int(coord.strip()) if len(coord) == 1 else None) for coord in et_from_entry.get().split(",")])
    if len(from_) != 2:
        from_ = (-1, -1)

    to = tuple([(int(coord.strip()) if len(coord) == 1 else None) for coord in et_to_entry.get().split(",")])
    if len(to) != 2:
        to = (-1, -1)

    removeTile = tuple([(int(coord.strip()) if len(coord) == 1 else None) for coord in et_remove_entry.get().split(",")])
    if len(removeTile) != 2:
        removeTile = (-1, -1)
    
    print(f"Making move from {from_} to {to} and remove {removeTile}")
    player_move = core.Move(core.TilePosition(from_[0], from_[1]), core.TilePosition(to[0], to[1]), core.TilePosition(removeTile[0], removeTile[1]))

    possible_moves = core.getPossibleMoves(board, player_is_black)
    if not is_move_possible(player_move, possible_moves):
        print("Invalid move.")
        for move in possible_moves:
            print(f"A possible move would be {move.toString()}")
        core.printBoard(board)

        return

    board = core.makeMove(board, player_move, player_is_black)
    refresh_fields()

    # Computer Move
    print("Processing")
    start = time.time()

    best_move = None
    best_move_value = None
    for move in core.getPossibleMoves(board, not player_is_black):
        start_minimax = time.time()
        move_value = core.minimax(core.makeMove(board, move, not player_is_black), depth, not player_is_black)
        print(f"Minimax took {time.time() - start_minimax} seconds")

        if best_move is None or (best_move_value < move_value and not player_is_black) or (best_move_value > move_value and player_is_black):
            best_move = move
            best_move_value = move_value

    print(f"\nBest move score: {best_move_value}")
    print(f"Best move: {best_move.toString()}")

    board = core.makeMove(board, best_move, not player_is_black)
    refresh_fields()

    print(f"Processing took {time.time() - start} seconds")
    print("Processed")

    et_from_entry.delete(0, tkinter.END)
    et_to_entry.delete(0, tkinter.END)
    et_remove_entry.delete(0, tkinter.END)

    et_from_entry.focus()
    print(f"core.Board({board.moveAmount}, {board.array}, {board.gameState})")

def field_pressed(ring_i, i):
    global et_from_entry, et_to_entry, et_remove_entry

    et_to_edit = root.focus_get()

    if type(et_to_edit) != tk.Entry:
        return

    et_to_edit.delete(0, tkinter.END)
    et_to_edit.insert(0, f"{ring_i},{i}")

    if et_to_edit == et_from_entry:
        et_to_entry.focus()
    elif et_to_edit == et_to_entry:
        et_remove_entry.focus()

def refresh_fields():
    global fields, board

    for ring_i in range(3):
        for i in range(8):
            field = fields[ring_i][i]
            field["text"] = board.array[ring_i][i]

#7x7 grid
game_frame = tk.Frame(root)
for ring_i in range(3):
    for i in range(8):
        field = tk.Label(game_frame, text="", image=pixel, compound='center', foreground='white', background='black')
        field.bind("<Button-1>", lambda event, ring_i=ring_i, i=i: field_pressed(ring_i, i))

        pos = positions[(ring_i, i)]
        field.grid(row=pos[0], column=pos[1], padx=5, pady=5)

        fields[ring_i].append(field)
game_frame.pack()

move_frame = tk.Frame(root)
tk.Label(move_frame, text="From:").grid(row=0, column=0)
et_from_entry = tk.Entry(move_frame)
et_from_entry.grid(row=0, column=1)
et_from_entry.focus()

tk.Label(move_frame, text="To:").grid(row=1, column=0)
et_to_entry = tk.Entry(move_frame)
et_to_entry.grid(row=1, column=1)

tk.Label(move_frame, text="Remove:").grid(row=2, column=0)
et_remove_entry = tk.Entry(move_frame)
et_remove_entry.grid(row=2, column=1)

bt_make_move = tk.Button(move_frame, text="Make move", command=make_move)
bt_make_move.grid(row=3, column=0, columnspan=2)
move_frame.pack()

refresh_fields()
root.mainloop()