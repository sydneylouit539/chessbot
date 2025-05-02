// Performance testing for Cavalier 0.1.1
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <map>
#include <variant>

#include "chess.hpp"
using namespace chess;


static const std::map<PieceType, int16_t> piece_values = {
        {PieceType::PAWN, 100},
        {PieceType::BISHOP, 300},
        {PieceType::KNIGHT, 320},
        {PieceType::ROOK, 500},
        {PieceType::QUEEN, 900},
        {PieceType::KING, 32767}
};

static const std::map<Square, int16_t> square_values = {
    {Square(0), -20}, {Square(1), -15}, {Square(2), -10}, {Square(3), -5}, {Square(4), -5}, {Square(5), -10}, {Square(6), -15}, {Square(7), -20},
    {Square(8), -15}, {Square(9), -10}, {Square(10), -5}, {Square(11), 0}, {Square(12), 0}, {Square(13), -5}, {Square(14), -10}, {Square(15), -15},
    {Square(16), -10}, {Square(17), -5}, {Square(18), 0}, {Square(19), 5}, {Square(20), 5}, {Square(21), 0}, {Square(22), -5}, {Square(23), -10},
    {Square(24), -5}, {Square(25), 0}, {Square(26), 5}, {Square(27), 10}, {Square(28), 10}, {Square(29), 5}, {Square(30), 0}, {Square(31), -5},
    {Square(32), -5}, {Square(33), 0}, {Square(34), 5}, {Square(35), 10}, {Square(36), 10}, {Square(37), 5}, {Square(38), 0}, {Square(39), -5},
    {Square(40), -10}, {Square(41), -5}, {Square(42), 0}, {Square(43), 5}, {Square(44), 5}, {Square(45), 0}, {Square(46), -5}, {Square(47), -10},
    {Square(48), -15}, {Square(49), -10}, {Square(50), -5}, {Square(51), 0}, {Square(52), 0}, {Square(53), -5}, {Square(54), -10}, {Square(55), -15},
    {Square(56), -20}, {Square(57), -15}, {Square(58), -10}, {Square(59), -5}, {Square(60), -5}, {Square(61), -10}, {Square(62), -15}, {Square(63), -20},
};



int16_t score_move_int(Board& board, 
                    Move move, 
                    int16_t initial_state = 0, 
                    bool to_move = true
                ){
    // Return negative values for opponent move
    int16_t direction = 1;
    // Initialize move value
    int16_t val = 0;
    if (!to_move){
        direction *= -1;
    }
    // Award 0.4 pawns for castling
    if (move.typeOf() == Move::CASTLING){
        return initial_state + 40 * direction;
    }
    // Score a capture
    if (board.isCapture(move)){
        PieceType captured_piece = board.at(move.to()).type();
        if (move.typeOf() == Move::ENPASSANT){ // en passant
            return val + 120 * direction;
        }
        val += (piece_values.find(captured_piece) -> second);
    }
    // Positional value (prioritize getting pieces closer to center)
    Square from = move.from();
    Square to = move.to();
    val += ((square_values.find(to) -> second) - (square_values.find(from) -> second));
    PieceType piece_to_move = board.at(from).type();
    if (piece_to_move == PieceType::KING){
        val -= 40;
        return initial_state + val * direction;
    }
    else if (piece_to_move == PieceType::PAWN){
        if (move.typeOf() == Move::PROMOTION){
            PieceType promoted_piece = move.promotionType();
            val += ((piece_values.find(promoted_piece) -> second) - 100);
        } else {
            val += 10 * (to.rank() - from.rank());
        }
    }
    /*
    // Small bonus if the move results in check
    if (board.inCheck()){
        val += 3;
    }*/
    return initial_state + val * direction;
}


std::vector<int> order(std::vector<int16_t> scores, bool player = true){// From ChatGPT
        // Create a vector of pairs where each pair contains the value and its original index
        std::vector<std::pair<int16_t, int>> indexed_scores;
        for (int i = 0; i < scores.size(); ++i) {
            indexed_scores.push_back({scores[i], i});
        }
        // Sort the vector by the numbers (scores)
        if (player){
            sort(indexed_scores.begin(), indexed_scores.end(), [](const std::pair<int16_t, int>& a, const std::pair<int16_t, int>& b) {
                return a.first > b.first;  
            });
        } else {
            sort(indexed_scores.begin(), indexed_scores.end(), [](const std::pair<int16_t, int>& a, const std::pair<int16_t, int>& b) {
                return a.first < b.first;  
            });
        }   
        // Create a vector to store the indices in ranked order
        std::vector<int> ranked_indices;
        for (const auto& pair : indexed_scores) {
            ranked_indices.push_back(pair.second);  // Push original indices based on rank
        }
        return ranked_indices;
}


int16_t next_move_new(Board& board, 
    int depth, 
    int& total_moves,
    int16_t alpha = -32767, 
    int16_t beta = 32767, 
    bool player = true,
    int16_t initial_state = 0
) {
    if (depth == 0 || std::abs(initial_state) > 10000) {
        total_moves += 1;
        return initial_state;
    }
    // Terminal recursion node or game state (currently slow)
    /*chess::GameResult game_result = board.isGameOver().second;
    if (depth == 0 || (game_result != chess::GameResult::NONE)){
        total_moves += 1;
        if (game_result == chess::GameResult::NONE){ return initial_state; }
        else if (game_result == chess::GameResult::WIN){ return 127; }
        else if (game_result == chess::GameResult::LOSE){ return -127; }
        return 0;
    }*/
    Movelist moves;
    movegen::legalmoves(moves, board);
    //Move best_move = moves[0];
    if (player) {
        int16_t maxEval = -32767; 
        // Rank captures ahead of non-captures
        int num_moves = moves.size();
        std::vector<int> captures(num_moves, 0);
        for (int i = 0; i < num_moves; ++i){
            if (board.isCapture(moves[i])){captures[i] = 1;}
        }
        int iter = 0;
        // Captures
        for (const auto& move : moves) {
            if (captures[iter] == 1){
                int16_t new_state = score_move_int(board, move, initial_state, true);
                board.makeMove(move);
                maxEval = std::max(maxEval, next_move_new(
                    board, depth - 1, total_moves, alpha, beta, false, new_state));
                board.unmakeMove(move);
                iter += 1;
                if (maxEval > beta){
                    break;
                }
                alpha = std::max(alpha, maxEval);
            } else {
                iter += 1;
                continue;
            }
        }
        //Non-captures
        iter = 0;
        for (const auto& move : moves) {
            if (captures[iter] == 0){
                int16_t new_state = score_move_int(board, move, initial_state, true);
                board.makeMove(move);
                maxEval = std::max(maxEval, next_move_new(
                    board, depth - 1, total_moves, alpha, beta, false, new_state));
                iter += 1;
                board.unmakeMove(move);
                if (maxEval > beta){
                    break;
                }
                alpha = std::max(alpha, maxEval);
            } else {
                iter += 1;
                continue;
            }    
        }
        return maxEval;
    } else {
        int16_t minEval = 32767; 
        // Rank captures ahead of non-captures
        int num_moves = moves.size();
        std::vector<int> captures(num_moves, 0);
        for (int i = 0; i < num_moves; ++i){
            if (board.isCapture(moves[i])){captures[i] = 1;}
        }
        // Captures
        int iter = 0;
        for (const auto& move : moves) {
            if (captures[iter] == 1){
                int16_t new_state = score_move_int(board, move, initial_state, false);
                board.makeMove(move);
                minEval = std::min(minEval, next_move_new(
                    board, depth - 1, total_moves, alpha, beta, true, new_state));
                board.unmakeMove(move);
                iter += 1;
                if (minEval < alpha){
                    break;
                }
                beta = std::min(beta, minEval);
            } else {
                iter += 1;
                continue;
            }
        }
        // Non-captures
        iter = 0;
        for (const auto& move : moves) {
            if (captures[iter] == 0){
                int16_t new_state = score_move_int(board, move, initial_state, false);
                board.makeMove(move);
                minEval = std::min(minEval, next_move_new(
                    board, depth - 1, total_moves, alpha, beta, true, new_state));
                board.unmakeMove(move);
                iter += 1;
                if (minEval < alpha){
                    break;
                }
                beta = std::min(beta, minEval);
            } else {
                iter += 1;
                continue;
            }
        }
        return minEval;
    }
}

// Function to calculate best move by scoring all moves at given depth
Movelist stem(
    Board& board, 
    Movelist moves, 
    int& total_moves, 
    int depth, 
    bool player = true,
    int16_t alpha = -32767,
    int16_t beta = 32767
){
    // Stop condition
    if (depth == 0) {return moves;}
    // Initialize move-specific scores
    int16_t value = -32767;
    if (!player) {value *= -1;}
    std::vector<int16_t> scores(moves.size(), value);
    for (int m = 0; m < moves.size(); m++){
        scores[m] = moves[m].score();
    }
    std::vector<int> inds = order(scores, player);
    for (int i = 0; i < moves.size(); i++){
        Move move = moves[inds[i]];
        int16_t new_state = score_move_int(board, move, 0, player);
        board.makeMove(move);
        scores[i] = next_move_new(board, depth - 1, total_moves, alpha, beta, !player, new_state);
        if (player){value = std::max(value, scores[i]);} else {value = std::min(value, scores[i]);}
        moves[inds[i]].setScore(scores[i]);
        board.unmakeMove(move);
        
        if (player){
            if (value > beta){ break; }
            alpha = std::max(alpha, scores[i]);
        } else {
            if (value < alpha){ break; }
            beta = std::min(beta, scores[i]);
        }
        
    }
    //Movelist sorted_moves = order_moves(scores, moves, player);
    for (int j = 0; j < moves.size(); j++){
        std::cout << moves[inds[j]] << " " << moves[inds[j]].score() << std::endl;
    }
    return moves;
}


Move IDDFS(Board board, int depth, bool player = true) {
    if (!player){board.sideToMove() == Color::BLACK;}
    //std::cout << board.sideToMove() << std::endl;
    Movelist moves;
    movegen::legalmoves(moves, board);
    std::vector<int16_t> scores(moves.size(), 0);
//    Movelist sorted_moves = moves;
    int16_t alpha = -32767;
    for (int i = 2; i < (depth + 1); i += 2){
        int total_moves = 0;
        Movelist sorted_moves = stem(board, moves, total_moves, i, player);
        for(int j = 0; j < moves.size(); j++){
            moves[j] = sorted_moves[j];
        }
        std::cout << i << std::endl;
        std::cout << "Total moves: " << total_moves << std::endl;
    }
    for (int m = 0; m < moves.size(); m++){
        scores[m] = moves[m].score();
    }
    std::vector<int16_t>::iterator result;
    if (player) {
        result = std::max_element(scores.begin(), scores.end());
    } else {
        result = std::min_element(scores.begin(), scores.end());
    }
    //return moves[0];
    return moves[std::distance(scores.begin(), result)]; 
}


uint64_t perft(Board& board, int depth) {
    Movelist moves;
    movegen::legalmoves(moves, board);

    if (depth == 1) {
        for (const auto& move : moves) {
            int16_t new_state = score_move_int(board, move, 0, false);
        }
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


int main(){
    /*
    Board board = Board("r2qkb1r/2p2ppp/p1n1b3/1pn1P1N1/8/1Bp5/PP1N1PPP/R1BQ1RK1 w kq - 0 12");
    for (int i = 1; i < 6; i++){
        std::cout << perft(board, i) << std::endl;
    }*/
    //uciProtocol();
    std::string fen;
    std::cout << "Enter board fen: ";
    std::cin >> fen;
    Board board;
    if (fen == "startpos"){
        board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else {
        board = Board(fen);
    }
    Move bot_move = IDDFS(board, 6);
    std::cout << uci::moveToSan(board, bot_move) << std::endl;
    //std::cout << uci::moveToUci(bot_move) << std::endl;
    //board.makeMove(bot_move);
    //std::cout << perft(board, 4) << std::endl;
    return 0;
}
