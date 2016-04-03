from copy import deepcopy
from random import randint


class Board(object):
    def __init__(self):
        self.board = [0 for _ in range(9)]

    def winner(self):
        def winning_line(a, b, c):
            if (
                self.board[a] != 0 and
                self.board[a] == self.board[b] == self.board[c]
            ):
                return self.board[a]

        lines = [
            [0, 1, 2], [3, 4, 5], [6, 7, 8],
            [0, 3, 6], [1, 4, 7], [2, 5, 8],
            [0, 4, 8], [2, 4, 6]]

        for a, b, c in lines:
            res = winning_line(a, b, c)
            if res:
                return res

    def valid_moves(self):
        return [i for i in range(9) if self.board[i] == 0]

    def is_full(self):
        return len(self.valid_moves()) == 0

    def apply(self, i, player):
        self.board[i] = player

    def __str__(self):
        def x(i):
            r = { -1 : 'O', 0 : '.', 1: 'X'}
            return r[self.board[i]]

        return '%s %s %s\n%s %s %s\n%s %s %s' % (
            x(0), x(1), x(2), x(3), x(4), x(5), x(6), x(7), x(8))


def minimax(board, max_player):
    winner = board.winner()
    if winner:
        return winner, None

    best_move, best_score = None, 0
    player = 1 if max_player else -1

    for move in board.valid_moves():
        new_board = deepcopy(board)
        new_board.apply(move, player)
        score, _ = minimax(new_board, not max_player)
        # if the move is a winning move, pick it, and return
        if score == player:
            return player, move
        if best_move is None:
            best_score = score
            best_move = move
        else:
            if (
                (max_player and score > best_score) or
                (not max_player and score < best_score)
            ):
                best_score = score
                best_move = move

    return best_score, best_move

board = Board()
board.apply(randint(0, 8), True)
print "%s\n" % board
player = False

while not board.is_full():
    score, move = minimax(board, player)
    board.apply(move, 1 if player else -1)
    player = not player
    print "%s\n" % board
