# Cavalier

Welcome to my chess bot project, known as Cavalier! This is a chess engine
that 

## What is Unique?
 - My bot scores _moves_, not board positions. As a result, the tree
 search functions need to account for the existing board state, and 
 add the score of the proposed move
 - The move evaluations are done using 8-bit integers in C++. This may 
 potentially allow for much more storage than what would be possible 
 using a 64-bit double.
 - A knight is treated as slightly more valuable than a bishop (3.2 pawns vs. 
 3.0 for a bishop). Almost all others consider a bishop to be at least as
 valuable as a knight.
 
## What is Mine and What is Borrowed
 - The following C++ library is used to generate legal moves, as well
 as make and unmake moves: 
 https://github.com/Disservin/chess-library/tree/master
 - In Python, the python chess library is used to generate legal moves, as well
 as make and unmake moves
 - The C++ and Python files are my original implementations.
 

## Goal
The ideal Cavalier bot will be compatible with existing chess GUIs like Arena,
allowing ease of both usage and evaluation. It will also be customizable, 
capable of modifying tree search depth, and playing 
openings with the Bongcloud.


Regarding skill level, the long-term objective is to be able to create a
bot using self-made heuristics and that can play at an Elo level of up to 3000.


Some neural network approaches have been attempted, but have not yet yielded
a good fit. However, the data used to train is only a few thousand games.
Many of the world's strongest chess engines use neural networks, so this 
approach may ultimately be necessary for this bot to be able to reach the 
stated goal of playing at a 3000 Elo.

## Current Progress
This bot is still in the extremely early stages of development, and the Elo
level of the best bot in this project is currently about *1400*. The 
current bot uses several techniques to improve performance.
 - Alpha-beta pruning
 - Move ordering and reduction
 - Opening book (Source: perfect.ctg)
 - Efficient heuristic that evaluates a move rather than the board

## Estimated Elo History
 - 7/12/2024:   30 
 - 7/13/2024:  644
 - 7/14/2024: 1100
 - 7/23/2024: 1250
 - 9/15/2024: 1370


## Potential Next steps
 - Early vs. Late-game heuristic changes
 - Monte Carlo search
 - NNUE (Efficiently Updatable Neural Network) evaluation


