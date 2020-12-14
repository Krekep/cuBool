from . import wrapper
from . import dll
from . import matrix

__all__ = [
    "kron"
]


def kron(result_matrix: matrix.Matrix, a_matrix: matrix.Matrix, b_matrix: matrix.Matrix):
    status = wrapper.loaded_dll.CuBool_Kron(wrapper.instance,
                                            result_matrix.hnd,
                                            a_matrix.hnd,
                                            b_matrix.hnd)

    dll.check(status)
