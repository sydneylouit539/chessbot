import chess
import random
import numpy as np


def sim_game(nsim):
    result = np.zeros(nsim)
    for i in range(nsim):
        board = chess.Board()
        while not board.is_game_over():
            legal_moves = list(board.legal_moves)
            #print("Legal moves:", [move.uci() for move in legal_moves])
            #user_move = input("Enter your move (e.g., 'e2e4'): ")
            try:
                ## User-generated move
                #move = chess.Move.from_uci(user_move)
                if board.legal_moves.count() == 0:
                    #print(board.result)
                    break
                ## Random user move
                #user_move = random.choice(list(board.legal_moves))
                ## Engine move
                user_move = next_move(board)
                #print(user_move)
                board.push(user_move)
                #if move in legal_moves:
                #board.push(move)
                #print(board)
                ## Make a random move for the computer
                if board.legal_moves.count() == 0:
                    #print(board.result)
                    break
                computer_move = random.choice(list(board.legal_moves))
                board.push(computer_move)
                #print("Computer's move:", computer_move.uci())
                #print(board)
                #else:
                #    print("Invalid move. Try again.")
            except ValueError:
                print("Invalid move format. Try again.")
        if board.result().partition('-')[0] == '1/2':
            result[i] = 0.5
        else:
            result[i] = board.result().partition('-')[0]
        if (i+1) % 10 == 0:
            print(f"Game {i+1} Result: {board.result()}")
    return np.mean(result)

## sim_game(100)
