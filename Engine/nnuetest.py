from __future__ import print_function
from ctypes import *
nnue = cdll.LoadLibrary("libnnueprobe.so")
nnue.nnue_init(b"nn-6b4236f2ec01.nnue")
score = nnue.nnue_evaluate_fen(b"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
print("Score = ", score)