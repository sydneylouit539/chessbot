
## First bot (Elo ~30)
def next_move(board, moves = 1):
    moves = board.legal_moves
    scores = []
    for i in board.legal_moves:
        board.push(i)
        scores.append(score_board(board))
        ## Undo move
        board.pop()
    max_value = np.max(scores)
    max_indices = np.where(scores == max_value)[0]
    return list(moves)[random.choice(max_indices)]

## Second bot (Elo ~650)
def next_move(board, moves_ahead = 2, player = True):
    ## If no more moves, return the state of the board
    if moves_ahead == 0:
        return score_board(board)
    moves = board.legal_moves
    if board.legal_moves.count() == 0:
        if board.is_checkmate():
            return [10000]
        if board.is_stalemate():
            return [0]
    scores = []
    ## Evaluate all possible next moves
    for i in moves:
        board.push(i)
        #if moves_ahead == 1:
        #scores.append(score_board(board))
        #else:
        #print(board.is_stalemate())
        scores.append(next_move(board, moves_ahead - 1, not player))
        ## Undo move
        board.pop()
    return scores


def minimax(scenarios, depth, player = True):
    if depth == 2:
        losses = []
        for i in scenarios:
            losses.append(min(i))
        max_value = np.max(losses)
        max_indices = np.where(losses == max_value)[0]
        return [random.choice(max_indices), max_value]
    red_scenarios = []
    for i in range(len(scenarios)):
        slicer = []
        for j in range(len(scenarios[i])):
            slicer.append( minimax(scenarios[i][j], depth - 2, player)[1] )
            #print(scenarios[i][j])
        red_scenarios.append(slicer)
    return minimax(red_scenarios, depth - 2, player) 




















































































































































































