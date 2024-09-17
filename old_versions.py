import chess


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


def next_move_0_0_3(board, moves_ahead, initial_state = 0, player = True, stem = True):
    ## If no more moves, return the state of the board
    moves = [board.san(i) for i in board.legal_moves]
    #print(len(moves))
    if moves_ahead == 1:
        return score_board(board, moves, initial_state)
    if len(moves) == 0:
        if board.is_checkmate() and not player:
            return [10000.]
        if board.is_checkmate() and player:
            return [-10000.]
        if board.is_stalemate():
            return [0.]
    scores = score_board(board, moves, initial_state)
    assert len(scores) == len(moves), print(len(scores) - len(moves))
    ## Keep top five moves
    if len(scores) >= 20 and not stem:# moves_ahead % 2 == 1:
        top_moves = sorted(scores, reverse = player)[:20]
        top_indices = sorted(range(len(scores)), key=lambda j: scores[j], reverse = player)[:20]
    else:
        top_indices = range(len(scores))
    new_scores = []
    x = list(board.legal_moves)
    for i in top_indices:
        board.push(x[i])
        new_scores.append(next_move_0_0_3(board, moves_ahead - 1, scores[i], not player, False))
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


def next_move_0_0_5(board, depth, alpha, beta, player, stem = True, initial_state = 0):
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
            value = max(value, next_move_0_0_5(board, depth - 1, alpha, beta, False, False, new_state))
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
            value = min(value, next_move_0_0_5(board, depth - 1, alpha, beta, True, False, new_state))
            board.pop()
            if value < alpha:
                break
            beta = min(beta, value)
        return value


def next_move_0_0_6(board, depth, alpha, beta, player, stem = True, initial_state = 0):
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
            value = max(value, next_move_0_0_6(board, depth - 1, alpha, beta, False, False, new_state))
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
            value = min(value, next_move_0_0_6(board, depth - 1, alpha, beta, True, False, new_state))
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


## Old board scoring function
def score_board(board):
    score = 0
    ## Encourage putting the other king in check
    if board.is_check() and not board.turn:
        score += 1
    if board.is_stalemate():
        return 0.
    elif board.is_checkmate() and board.turn:
        return -10000.
    elif board.is_checkmate() and not board.turn:
        return 10000.
    ## Iterate over entire chessboard
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece is not None:
            val = piece_values[piece.piece_type] + 0.1 * (8 - chess.square_manhattan_distance(square, chess.E5))
            if piece.color == chess.WHITE:
                score += val
            else:
                score -= val
    return score


def next_move_0_0_7(board, depth, alpha = -10000, beta = 10000, player = True, stem = True, initial_state = 0):
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
            value = max(value, next_move_0_0_7(board, depth - 1, alpha, beta, False, False, new_state))
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
            value = min(value, next_move_0_0_7(board, depth - 1, alpha, beta, True, False, new_state))
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


def move_pair(board, depth = 2, keep = 6, initial_state = 0, player = True, stem = True):
    #iter_num = 0
    if board.is_checkmate() or board.is_stalemate() or depth == 0:
        return initial_state
    #moves = [board.san(i) for i in board.legal_moves]
    #moves = list(board.legal_moves)
    responses = []
    scores = []
    for i in board.legal_moves:
        new_state = current_version.score_move(board, board.san(i), initial_state)
        board.push(i)
        ## Opponent moves
        counters = board.legal_moves
        ## If a move leads to checkmate either way, break the loop
        over = board.is_checkmate()
        if over or board.is_stalemate():
            scores.append(over * 10000)
            responses.append('')
            board.pop()
            break
        ## Captures, checks, and promotions
        x = []
        for j in counters:
            if ('x' or '#' or '=' or '+') in board.san(j):
                x.append(j)
            if len(x) == 0:
                x.append(j)

        z = []
        for j in x:
            z.append(current_version.score_move(board, board.san(j), new_state, False))
            #iter_num += 1
        scores.append(min(z))
        responses.append(list(counters)[np.argmin(z)]) # responses should append a SAN
        board.pop()
    #print(iter_num)
    ## Get top 'keep' moves, return if at bottom of search
    if len(scores) >= keep and not stem:
        top_moves = sorted(scores, reverse = player)[:keep]
        top_indices = sorted(range(len(scores)), key=lambda j: scores[j], reverse = player)[:keep]
    else:
        top_indices = range(len(scores))
    moves = list(board.legal_moves)
    new_scores = []
    for i in top_indices:
        ## Push the move and opponent's best counter
        board.push(moves[i])
        if board.is_checkmate():
            board.pop()
            continue
        board.push(responses[i])
        new_scores.append(move_pair(board, depth - 2, keep, scores[i], player, False))
        board.pop()
        board.pop()
    if not stem:
        return new_scores
    else:
        if depth == 2:
            return moves[np.argmax(new_scores)]
        else:
            for d in range(int(depth / 2 - 1)):
                new_scores = [max(i) for i in new_scores]
            #print(iter_num)
            print([[board.san(a), round(b, 1)] for a, b in zip(moves, new_scores)])
            return moves[np.argmax(new_scores)]

## Alternative to first scoring function, with zero-sum square value
def score_board(board, initial_state = 0):
    score = 0
    ## Encourage putting the other king in check
    if board.is_check() and not board.turn:
        score += 1
    if board.is_stalemate():
        return 0.
    elif board.is_checkmate() and board.turn:
        return -10000.
    elif board.is_checkmate() and not board.turn:
        return 10000.
    ## Iterate over entire chessboard
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece is not None:
            val = piece_values[piece.piece_type] + 0.1 * (4 - chess.square_manhattan_distance(square, chess.E5))
            if piece.color == chess.WHITE:
                score += val
            else:
                score -= val
    return score


def score_board(board, moves, initial_state = 0.):
    '''
    Inputs:
    board - a chess.Board() object with the current state of the board
    moves - a list of board.Move objects in SAN notation
    Assumes that none of the moves in the object have already been made
    '''
    score = []
    to_move = board.turn
    for move in moves:
        val = initial_state
        ## Encourage putting the other king in check
        if move.find('+') != -1 and to_move:
            val += 1.0
        if board.is_stalemate():
            val = 0
        elif move.find('#') != -1 and not to_move:
            val -= 10000.
        elif move.find('#') != -1 and to_move:
            val += 10000.
        ## If potential move is a capture, evaluate
        if move.find('x') != -1:
            #print(board.parse_san(move))
            captured_piece = board.piece_at(board.parse_san(move).to_square)#; print(captured_piece)
            val += piece_values[captured_piece.piece_type] * (1 if to_move else -1)
        score.append(val)
    return score
################################################################################
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
