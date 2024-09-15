## Functions for current version of the game
import chess
piece_values = {
    chess.PAWN: 1,
    chess.KNIGHT: 3,
    chess.BISHOP: 3,
    chess.ROOK: 5,
    chess.QUEEN: 9,
    chess.KING: 100
}


def minimax(node, depth, maximizing_player=True, final=True): # Assisted by ChatGPT
    if final:
        result = []
        for i in node:
            result.append(minimax(i, depth - 1, False, False))
        #print(result)
        return np.argmax(result)
    if depth == 0 or isinstance(node, float):
        return node

    if maximizing_player:
        max_eval = -float('inf')
        for child in node:
            eval = minimax(child, depth - 1, False, False)
            max_eval = max(max_eval, eval)
        return max_eval
    else:
        min_eval = float('inf')
        for child in node:
            eval = minimax(child, depth - 1, True, False)
            min_eval = min(min_eval, eval)
        return min_eval


def score_move(board, move, initial_state=0., to_move=True):
    #to_move = board.turn
    direction = 1 if to_move else -1
    ## If this move results in checkmate, award a large number of points
    if move.find('#') != -1:
        if to_move:
            return 10000.
        else:
            return -10000.
    ## Give a castle a +0.3
    if move[0] == 'O':
        return initial_state + 0.3 * direction
    val = 0
    ## Score a capture
    if move.find('x') != -1:
        captured_piece = board.piece_at(board.parse_san(move).to_square)
        if captured_piece is None: # en passant
            return val + 1.2 * direction
        val += piece_values[captured_piece.piece_type]
    ## Small bonus to put the other king in check
    if move.find('+') != -1:
        val += 0.5
    ## Pawn positioning
    if not move[0].isupper():
        val += 0.3
    ## Non-pawn positioning
    else:
        sq = board.parse_san(move)
        val += 0.1 * (chess.square_manhattan_distance(sq.from_square, 36) - chess.square_manhattan_distance(sq.to_square, 36))
    return initial_state + val * direction


## Rank the captures ahead of the non-captures
def sort_moves(board, moves, player = True, full = False):
    if not full:
        moves = [board.san(i) for i in moves]
        with_x = [i for i in moves if ('x' or '#' or '=') in i]
        without_x = [i for i in moves if ('x' or '#' or '=') not in i]
        return with_x + without_x
    else:
        scores = np.array([current_version.score_move(board, board.san(i), player) for i in moves])
        #return np.array([board.san(i) for i in moves])[np.argsort(scores * -1 if player else 1)]
        if player:
            return np.array([board.san(i) for i in moves])[np.argsort(scores)[::-1]]
        else:
            return np.array([board.san(i) for i in moves])[np.argsort(scores)[::-1]]

## Function to decide the next_move (Currently v0.0.6)
def next_move(board, depth, alpha = -10000, beta = 10000, player = True, stem = True, initial_state = 0):
    if depth == 0 or board.is_checkmate() or board.is_stalemate():
        return initial_state
    x = sort_moves(board.legal_moves)
    #x = [board.san(i) for i in board.legal_moves]
    if player:
        value = -10000
        #x = board.legal_moves
        if stem:
            moves = []
        for i in x:
            ## Score the move
            new_state = score_move(board, i, initial_state)
            board.push(board.parse_san(i))
            value = max(value, next_move(board, depth - 1, alpha, beta, False, False, new_state))
            board.pop()
            if value > beta:
                break
            alpha = max(alpha, value)
            if stem:
                moves.append(value)
        if not stem:
            return value
        else:
            print(alpha, beta)
            return x[np.argmax(moves)]
    else:
        value = 10000
        #x = board.legal_moves
        for i in x:
            ## Score the move
            new_state = score_move(board, i, initial_state)
            board.push(board.parse_san(i))
            value = min(value, next_move(board, depth - 1, alpha, beta, True, False, new_state))
            board.pop()
            if value < alpha:
                break
            beta = min(beta, value)
            if stem:
                moves.append(value)
        if not stem:
            return value
        else:
            print(alpha, beta)
            return x[np.argmin(moves)]
        return value
