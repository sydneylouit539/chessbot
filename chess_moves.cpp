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
    if (player) {
        double maxEval = -1000.0; 
        Movelist moves;
        movegen::legalmoves(moves, board);
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
        Movelist moves;
        movegen::legalmoves(moves, board);
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


Move stem(Board board, int depth, bool player = true){
    Movelist moves;
    movegen::legalmoves(moves, board);
    std::vector<double> scores(moves.size(), 0.0);
    int iter = 0;
    for (const auto &move : moves){
        double new_state = score_move(board, move);//, to_move = true);
        board.makeMove(move);
        scores[iter] = next_move(board, depth - 1, -1000.0, 1000.0, false, new_state);
        board.unmakeMove(move);
        //std::cout << scores[iter] << std::endl;
        //std::cout << move << std::endl;
        iter++;
    }
    if (player) {
        std::vector<double>::iterator result;
        result = std::max_element(scores.begin(), scores.end());
        //std::cout << std::distance(scores.begin(), result) << std::endl;
        return moves[std::distance(scores.begin(), result)];   
    } else {
        return moves[0];
//        return moves[std::min_element(scores)];
    }
}

int main () {
    std::string fen;
    std::cout << "Enter board fen: ";
    std::cin >> fen;
    Board board = Board(fen);
    Move bot_move = stem(board, 4);
    std::cout << uci::moveToSan(board, bot_move) << std::endl;
    //std::cout << perft(board, 4) << std::endl;
    return 0;
}





