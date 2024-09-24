#include <iostream>
#include <string>
#include <map>

#include "chess.hpp"

using namespace chess;

uint64_t perft(Board& board, int depth) {
    Movelist moves;
    movegen::legalmoves(moves, board);

    if (depth == 1) {
        return moves.size();
    }

    uint64_t nodes = 0;

    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.unmakeMove(move);
    }
    return nodes;
}


static const std::map<PieceType, int> piece_values = {
        {PieceType::PAWN, 1},
        {PieceType::KNIGHT, 3},
        {PieceType::BISHOP, 3},
        {PieceType::ROOK, 5},
        {PieceType::QUEEN, 9},
        {PieceType::KING, 100} // King value is typically not assigned in evaluation
};



double score_move(Board board, 
                    Move move, 
                    double initial_state = 0.0, 
                    bool to_move = true){
    // Return negative values for opponent move
    int direction = 1;
    if (!to_move){
        direction *= -1;
    }
    std::string pos = uci::moveToSan(board, move);
    // If this move is a checkmate, award a large number of points
    if (pos.find('#') != std::string::npos){
        return 100.0 * direction;
    }
    // Award 0.3 points for castling
    if (pos[0] == 'O'){
        return initial_state + 0.3 * direction;
    }
    double val = 0;
    // Score a capture
    //if (pos.find('x') != std::string::npos) {
    if (board.isCapture(move)){
        PieceType captured_piece = board.at(move.to()).type();
        if (move.typeOf() == Move::ENPASSANT){ // en passant
            return val + 1.2 * direction;
        }
        val += piece_values.find(captured_piece) -> second;
    }
    // Penalize a king move until the late game
    if (pos[0] == 'K') {
        val -= 1.5;
    }
    // Small bonus if the move results in check
    if (pos.find('+') != std::string::npos){
        val += 0.3;
    }
    // Pawn positioning
    if (!std::isupper(pos[0])){
        if (pos.find('Q') != std::string::npos){
            val += 8;
        }
        val += 0.2;
    }
    return initial_state + direction * val;
}

// Function to sort moves from best to worst
Movelist sort_moves(Board board, 
                    Movelist moves, 
                    bool player, 
                    const std::vector<double>& scores = {-10.0}){
    int len = moves.size();
    // Generate vector of move scores if not given
    if (scores[0] == -10.0){
        std::vector<double> scores(len);
        int iter = 0;
        for (const auto& move : moves) {
            scores[iter] = score_move(board, move, 0.0, true);
            iter++;
        }
    }
    // Vector of indices
    std::vector<size_t> indices(len);
    for (size_t i = 0; i < len; ++i){
        indices[i] = i;
    }
    // Sort the indices
    if (player){
        std::sort(indices.begin(), indices.end(),
        [&scores](size_t i1, size_t i2){
            return scores[i1] > scores[i2];
        });
    } else {
        std::sort(indices.begin(), indices.end(),
        [&scores](size_t i1, size_t i2){
            return scores[i1] < scores[i2];
        });
    }
    Movelist sorted_moves = moves;
    for (size_t i = 0; i < len; ++i){
        sorted_moves[i] = moves[indices[i]];
    }
    return sorted_moves;
}


double next_move(Board board, 
                int depth, 
                double alpha = -1000.0, 
                double beta = 1000.0, 
                bool player = true,
                double initial_state = 0.0) {
    // End recursion if we have checkmate
    if (depth == 0 || std::abs(initial_state) > 50.0) {
        return initial_state;
    }
    Movelist moves;
    movegen::legalmoves(moves, board);
    if (player) {
        double maxEval = -1000.0; 
        for (const auto& move : moves) {
            double new_state = score_move(board, move, initial_state, true);
            board.makeMove(move);
            double eval = next_move(board, depth - 1, alpha, beta, false, new_state);
            board.unmakeMove(move);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break; 
            }
        }
        return maxEval;
    } else {
        double minEval = 1000.0; 
        for (const auto& move : moves) {
            double new_state = score_move(board, move, initial_state, false);
            board.makeMove(move);
            double eval = next_move(board, depth - 1, alpha, beta, true, new_state);
            board.unmakeMove(move);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break; 
            }
        }
        return minEval;
    }
}

// Function to calculate best move using alpha-beta pruning
Move stem(Board board, int depth, bool player = true){
    Movelist moves;
    movegen::legalmoves(moves, board);
    std::vector<double> scores(moves.size(), 0.0);
    int iter = 0;
    Movelist sorted_moves = sort_moves(board, moves, player);
    for (const auto &move : sorted_moves){
        double new_state = score_move(board, move);//, to_move = true);
        board.makeMove(move);
        scores[iter] = next_move(board, depth - 1, -1000.0, 1000.0, false, new_state);
        board.unmakeMove(move);
        std::cout << scores[iter] << std::endl;
        std::cout << move << std::endl;
        iter++;
    }
    std::vector<double>::iterator result;
    if (player) {
//        std::vector<double>::iterator result;
        result = std::max_element(scores.begin(), scores.end());
        //std::cout << std::distance(scores.begin(), result) << std::endl;
        return sorted_moves[std::distance(scores.begin(), result)];   
    } else {
        result = std::min_element(scores.begin(), scores.end());
        return sorted_moves[std::distance(scores.begin(), scores.end())];
//        return moves[std::min_element(scores)];
    }
}

// Iterative deepening depth-first search
Move IDDFS(Board board, int depth, bool player = true) {
    Movelist moves;
    movegen::legalmoves(moves, board);
    std::vector<double> scores(moves.size(), 0.0);
    Movelist sorted_moves = moves;
    for (int i = 1; i < (depth + 1); ++i){
        int iter = 0;
        for (const auto &move : sorted_moves){
            double new_state = score_move(board, move);//, to_move = true);
            board.makeMove(move);
            scores[iter] = next_move(board, i - 1, -1000.0, 1000.0, false, new_state);
            board.unmakeMove(move);
            std::cout << scores[iter] << std::endl;
            std::cout << move << std::endl;
            iter++;
        }
        // After iterating through all moves, re-sort
        sorted_moves = sort_moves(board, moves, player, scores);
        
    }
    std::vector<double>::iterator result;
    if (player) {
        result = std::max_element(scores.begin(), scores.end());
    } else {
        result = std::min_element(scores.begin(), scores.end());
    }
    return sorted_moves[std::distance(scores.begin(), result)]; 
}




int main () {
    std::string fen;
    std::cout << "Enter board fen: ";
    std::cin >> fen;
    Board board = Board(fen);
    /*
    while (board.isGameOver().second == chess::GameResult::NONE) {
        Move bot_move = stem(board, 4);
        std::cout << uci::moveToSan(board, bot_move) << std::endl;
        board.makeMove(bot_move);
        std::string opponent_move;
        std::cout << "Enter opponent move: ";
        std::cin >> opponent_move;
    }
    */
    //Move bot_move = stem(board, 4);
    Move bot_move = IDDFS(board, 5);
    std::cout << uci::moveToSan(board, bot_move) << std::endl;
    //board.makeMove(bot_move);
    //std::cout << perft(board, 4) << std::endl;
    return 0;
}





