// Chess moves v0.1.1
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <map>
#include <variant>

#include "chess.hpp"
using namespace chess;


static const std::map<PieceType, int> piece_values = {
        {PieceType::PAWN, 10},
        {PieceType::KNIGHT, 30},
        {PieceType::BISHOP, 30},
        {PieceType::ROOK, 50},
        {PieceType::QUEEN, 90},
        {PieceType::KING, 127}
};


int8_t score_move_int(Board board, 
                    Move move, 
                    int8_t initial_state = 0, 
                    bool to_move = true){
    // Return negative values for opponent move
    int8_t direction = 1;
    if (!to_move){
        direction *= -1;
    }
    std::string pos = uci::moveToSan(board, move);
    // If this move is a checkmate, award a large number of points
    if (pos.find('#') != std::string::npos){
        return 127 * direction;
    }
    // Award 0.3 points for castling
    if (pos[0] == 'O'){
        return initial_state + 3 * direction;
    }
    double val = 0;
    // Score a capture
    //if (pos.find('x') != std::string::npos) {
    if (board.isCapture(move)){
        PieceType captured_piece = board.at(move.to()).type();
        if (move.typeOf() == Move::ENPASSANT){ // en passant
            return val + 12 * direction;
        }
        val += (piece_values.find(captured_piece) -> second);
    }
    // Penalize a king move until the late game
    if (pos[0] == 'K') {
        val -= 15;
    }
    // Small bonus if the move results in check
    if (pos.find('+') != std::string::npos){
        val += 3;
    }
    // Pawn positioning
    if (!std::isupper(pos[0])){
        if (pos.find('Q') != std::string::npos){
            val += 80;
        }
        val += 2;
    }
    return initial_state + direction * val;
}


Movelist order_moves(std::vector<int8_t>& scores, Movelist moves, bool player = true) {
    // Create a vector of pairs (index, score) for scores
    std::vector<std::pair<int, int8_t>> indexedVec;
    for (int i = 0; i < scores.size(); ++i) {
        indexedVec.push_back({scores[i], i}); // Pair value with its original index
    }
    // Sort the indexed vector based on the scores
    if (player){
        std::sort(indexedVec.begin(), indexedVec.end(), [](const std::pair<int, int8_t>& a, const std::pair<int, int8_t>& b) {
            return std::greater<int8_t>()(a.second, b.second);
        });
    } else {
        std::sort(indexedVec.begin(), indexedVec.end(), [](const std::pair<int, int8_t>& a, const std::pair<int, int8_t>& b) {
            return std::less<int8_t>()(a.second, b.second);
        });
    }
    // Create a new vector for sorted moves
    Movelist sorted_moves = Movelist();
    // Apply the same ordering to the move vector
    for (int i = 0; i < indexedVec.size(); ++i) {
        //sorted_moves[i] = moves[indexedVec[i].second];  // Use the original index to map strings
        sorted_moves.add(moves[indexedVec[i].second]);
    }
    return sorted_moves;
}


// Function to sort moves from best to worst
Movelist sort_moves(Board board, 
                    Movelist moves, 
                    bool player, 
                    const std::vector<int8_t>& scores = {-127}){
    int len = moves.size();
    // Generate vector of move scores if not given
    if (scores[0] == -127){
        std::vector<int8_t> scores(len);
        int iter = 0;
        for (const auto& move : moves) {
            scores[iter] = score_move_int(board, move, 0, true);
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


std::vector<int> order(std::vector<int8_t> scores, bool player = true){// From ChatGPT
        // Create a vector of pairs where each pair contains the value and its original index
        std::vector<std::pair<int8_t, int>> indexed_scores;
        for (int i = 0; i < scores.size(); ++i) {
            indexed_scores.push_back({scores[i], i});
        }
    
        // Sort the vector by the numbers (scores)
        if (player){
            sort(indexed_scores.begin(), indexed_scores.end(), [](const std::pair<int8_t, int>& a, const std::pair<int8_t, int>& b) {
                return a.first > b.first;  
            });
        } else {
            sort(indexed_scores.begin(), indexed_scores.end(), [](const std::pair<int8_t, int>& a, const std::pair<int8_t, int>& b) {
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




int8_t next_move(Board board, 
//                Movelist moves,
                int depth, 
                int8_t alpha = -127, 
                int8_t beta = 127, 
                bool player = true,
                int8_t initial_state = 0) {
    // End recursion if we have checkmate
    if (depth == 0 || std::abs(initial_state) > 100) {
        return initial_state;
    }
    Movelist moves;
    movegen::legalmoves(moves, board);
    Move best_move = moves[0];
    if (player) {
        int8_t maxEval = -127; 
        for (const auto& move : moves) {
            int8_t new_state = score_move_int(board, move, initial_state, true);
            board.makeMove(move);
            Movelist moves;
            movegen::legalmoves(moves, board);
            int8_t eval = next_move(
                board, depth - 1, alpha, beta, false, new_state);
            board.unmakeMove(move);
            if (maxEval < eval){
                maxEval = eval;
                best_move = move;
                alpha = eval;
            }
            if (beta <= alpha) {
                break; 
            }
        }
        return maxEval;
    } else {
        int8_t minEval = 127; 
        for (const auto& move : moves) {
            int8_t new_state = score_move_int(board, move, initial_state, false);
            board.makeMove(move);
            Movelist moves;
            movegen::legalmoves(moves, board);
            int8_t eval = next_move(
                board, depth - 1, alpha, beta, true, new_state);
            board.unmakeMove(move);
            if (minEval > eval){
                minEval = eval;
                best_move = move;
                beta = eval;
            }
            if (beta <= alpha) {
                break; 
            }
        }
        return minEval;
    }
}


// Function to calculate best move using alpha-beta pruning
Movelist stem(Board board, Movelist moves, int depth, bool player = true){
    // Stop condition
    if (depth == 0) {return moves;}
    std::vector<int8_t> scores(moves.size(), 0);
    for (int m = 0; m < moves.size(); m++){
        scores[m] = moves[m].score();
    }
    std::vector<int> inds = order(scores, player);

/*    int iter = 0;
    for (const auto &move : moves){
        int8_t new_state = score_move_int(board, move, 0, player);
        board.makeMove(move);
        scores[iter] = next_move(board, depth - 1, -127, 127, !player, new_state);
        //move.setScore(scores[iter]);
        board.unmakeMove(move);
        iter++;
    }*/
   int8_t alpha = -127;
   int8_t beta = 127;
    for (int i = 0; i < moves.size(); i++){
        Move move = moves[inds[i]];
        int8_t new_state = score_move_int(board, move, 0, player);
        board.makeMove(move);
        scores[i] = next_move(board, depth - 1, -127, 127, !player, new_state);
        moves[inds[i]].setScore(scores[i]);
        board.unmakeMove(move);
        if(player){
            if (scores[i] > beta){ alpha = scores[i]; }
        } else {
            if (scores[i] < alpha){ beta = scores[i]; }
        }
    }
    //Movelist sorted_moves = order_moves(scores, moves, player);
    for (int j = 0; j < moves.size(); j++){
        std::cout << moves[inds[j]] << " " << moves[inds[j]].score() << std::endl;
    }
    return moves;
}


Move IDDFS(Board board, int depth, bool player = true) {
    Movelist moves;
    movegen::legalmoves(moves, board);
    std::vector<int8_t> scores(moves.size(), 0);
//    Movelist sorted_moves = moves;
    int8_t alpha = -127;
    for (int i = 1; i < (depth + 1); ++i){
        Movelist sorted_moves = stem(board, moves, i, player);
        for(int j = 0; j < moves.size(); j++){
            moves[j] = sorted_moves[j];
        }
        std::cout << i << std::endl;
    }
    for (int m = 0; m < moves.size(); m++){
        scores[m] = moves[m].score();
    }
    std::vector<int8_t>::iterator result;
    if (player) {
        result = std::max_element(scores.begin(), scores.end());
    } else {
        result = std::min_element(scores.begin(), scores.end());
    }
    //return moves[0];
    return moves[std::distance(scores.begin(), result)]; 
}


void uciProtocol() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "uci") {
            std::cout << "id name Cavalier\n";
            std::cout << "id author Sydney Louit\n";
            std::cout << "uciok\n";  // Indicate that the bot is ready
        } 
        else if (line == "isready") {
            std::cout << "readyok\n";  // Bot is ready
        }
        else if (line == "quit") {
            break;  // End the engine
        } 
        else if (line.find("position") == 0) {
            // Handle position setup command
            Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            std::cout << "Position set\n";
        } 
        else if (line.find("go") == 0) {
            Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            Move bot_move = IDDFS(board, 2);
            std::cout << "bestmove " << uci::moveToUci(bot_move) << "\n";  // Return the move
        }
    }
}

/*
int main(){
    uciProtocol();
    return 0;
}
*/

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
    Move bot_move = IDDFS(board, 4);
    std::cout << uci::moveToUci(bot_move) << std::endl;
    //board.makeMove(bot_move);
    //std::cout << perft(board, 4) << std::endl;
    return 0;
}

