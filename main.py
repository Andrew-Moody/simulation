from ctypes import cdll, c_float, c_int
lib = cdll.LoadLibrary('./build/libsimulation.so')

lib.delaunay_sample.argtypes = [c_float, c_int]
lib.delaunay_sample(1.0, 5)