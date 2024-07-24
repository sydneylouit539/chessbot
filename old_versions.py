


def next_move_0_0_1(board, moves_ahead, alpha, beta, player = True):
    ## If no more moves, return the state of the board
    if moves_ahead == 0:
        return score_board(board)
    moves = board.legal_moves
    if moves.count() == 0:
        if board.is_checkmate() and not player:
            return [10000. - moves_ahead]
        if board.is_checkmate() and player:
            return [-10000. + moves_ahead]
        if board.is_stalemate():
            return [0]
    scores = []
    max_eval = -10000; min_eval = 10000
    ## Evaluate all possible next moves
    for i in moves:
        board.push(i)
        x = next_move(board, moves_ahead - 1, alpha, beta, not player)
        if type(x) == float:
            if player:
                max_eval = max(max_eval, x)
                alpha = max(alpha, x)
                if beta <= alpha:
                    break
            else:
                min_eval = min(min_eval, x)
                alpha = min(alpha, x)
                if beta <= alpha:
                    break
        scores.append(x)
        ## Undo move
        board.pop()
    return scores


def next_move_0_0_2(board, moves_ahead, player = True, stem = True):
    ## If no more moves, return the state of the board
    if moves_ahead == 0:
        return score_board(board)
    moves = board.legal_moves
    if moves.count() == 0:
        if board.is_checkmate() and not player:
            return [10000.]
        if board.is_checkmate() and player:
            return [-10000.]
        if board.is_stalemate():
            return [0.]
    scores = []
    #print(moves.count())
    ## Evaluate all possible next moves
    for i in moves:
        board.push(i)
        scores.append(score_board(board))
        ## Undo move
        board.pop()
    ## Keep top five moves
    if len(scores) >= 6 and not stem:# moves_ahead % 2 == 1:
        top_moves = sorted(scores, reverse = player)[:6]
        top_indices = sorted(range(len(scores)), key=lambda j: scores[j], reverse = player)[:6]
    else:
        top_indices = range(len(scores))
    new_scores = []
    #print(top_indices)
    for i in top_indices:
        board.push(list(moves)[i])
        new_scores.append(next_move(board, moves_ahead - 1, not player, False))
        board.pop()
    if not stem:
        return new_scores
    else:
        return top_indices[minimax(new_scores, moves_ahead)]


def next_move_0_0_4(board, depth, alpha, beta, player, stem = True):
    if depth == 0 or board.is_checkmate() or board.is_stalemate():
        return score_board(board)
    x = board.legal_moves
    #x = [board.san(i) for i in board.legal_moves]
    if player:
        value = -10000
        #x = board.legal_moves
        if stem:
            moves = []
        for i in x:
            board.push(i)
            value = max(value, next_move_0_0_4(board, depth - 1, alpha, beta, False, False))
            board.pop()
            if value > beta:
                break
            alpha = max(alpha, value)
            if stem:
                moves.append(value)
        if not stem:
            return value
        else:
            return np.argmax(moves)
    else:
        value = 10000
        #x = board.legal_moves
        for i in x:
            board.push(i)
            value = min(value, next_move_0_0_4(board, depth - 1, alpha, beta, True, False))
            board.pop()
            if value < alpha:
                break
            beta = min(beta, value)
        return value




## Neural network functions

def parse_move(board, to_win):
    board_string = board.board_fen()
    next_board = []
    for i in board_string:
        try:
            blanks = int(i)
            for j in range(blanks):
                next_board.append(0)
            next
        except:
            next
        if i == '/':
            next
        elif i == 'p':
            next_board.append(-1)
        elif i == 'P':
            next_board.append(1)
        elif i == 'n':
            next_board.append(-3)
        elif i == 'N':
            next_board.append(3)
        elif i == 'b':
            next_board.append(-3)
        elif i == 'B':
            next_board.append(3)
        elif i == 'r':
            next_board.append(-5)
        elif i == 'R':
            next_board.append(5)
        elif i == 'q':
            next_board.append(-9)
        elif i == 'Q':
            next_board.append(9)
        elif i == 'k':
            next_board.append(-100)
        elif i == 'K':
            next_board.append(100)
    if board.turn:
        next_board.append(1)
    else:
        next_board.append(0)
    next_board.append(to_win)
    return next_board
