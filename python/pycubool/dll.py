import ctypes


__all__ = [
    "CuBoolInstanceDesc",
    "load_and_configure",
    "check"
]


class CuBoolInstanceDesc(ctypes.Structure):
    _fields_ = [("memoryType", ctypes.c_uint),
                ("errorCallback.userData", ctypes.c_void_p),
                ("errorCallback.msgFun", ctypes.c_void_p),
                ("allocationCallback.userData", ctypes.c_void_p),
                ("allocationCallback.allocateFun", ctypes.c_void_p),
                ("allocationCallback.deallocateFun", ctypes.c_void_p)]


def load_and_configure(cubool_lib_path: str):
    lib = ctypes.cdll.LoadLibrary(cubool_lib_path)

    instance_p = ctypes.c_void_p
    p_to_instance_p = ctypes.POINTER(instance_p)

    matrix_p = ctypes.c_void_p
    p_to_matrix_p = ctypes.POINTER(matrix_p)

    lib.CuBool_Instance_New.restype = ctypes.c_uint
    lib.CuBool_Instance_New.argtypes = [ctypes.POINTER(CuBoolInstanceDesc),
                                        p_to_instance_p]

    lib.CuBool_Instance_Free.restype = ctypes.c_uint
    lib.CuBool_Instance_Free.argtypes = [instance_p]

    lib.CuBool_Matrix_New.restype = ctypes.c_uint
    lib.CuBool_Matrix_New.argtypes = [instance_p,
                                      p_to_matrix_p,
                                      ctypes.c_uint,
                                      ctypes.c_uint]

    lib.CuBool_Matrix_Free.restype = ctypes.c_uint
    lib.CuBool_Matrix_Free.argtypes = [instance_p,
                                       matrix_p]

    lib.CuBool_Matrix_Resize.restype = ctypes.c_uint
    lib.CuBool_Matrix_Resize.argtypes = [instance_p,
                                         matrix_p,
                                         ctypes.c_uint,
                                         ctypes.c_uint]

    lib.CuBool_Matrix_Build.restype = ctypes.c_uint
    lib.CuBool_Matrix_Build.argtypes = [instance_p,
                                        matrix_p,
                                        ctypes.POINTER(ctypes.c_uint),
                                        ctypes.POINTER(ctypes.c_uint),
                                        ctypes.c_size_t]

    lib.CuBool_Matrix_ExtractPairs.restype = ctypes.c_uint
    lib.CuBool_Matrix_ExtractPairs.argtypes = [instance_p,
                                               matrix_p,
                                               ctypes.POINTER(ctypes.c_uint),
                                               ctypes.POINTER(ctypes.c_uint),
                                               ctypes.POINTER(ctypes.c_size_t)]

    lib.CuBool_Matrix_Nrows.restype = ctypes.c_uint
    lib.CuBool_Matrix_Nrows.argtype = [instance_p,
                                       matrix_p,
                                       ctypes.POINTER(ctypes.c_uint)]

    lib.CuBool_Matrix_Ncols.restype = ctypes.c_uint
    lib.CuBool_Matrix_Ncols.argtype = [instance_p,
                                       matrix_p,
                                       ctypes.POINTER(ctypes.c_uint)]

    lib.CuBool_Matrix_Nvals.restype = ctypes.c_uint
    lib.CuBool_Matrix_Nvals.argtype = [instance_p,
                                       matrix_p,
                                       ctypes.POINTER(ctypes.c_size_t)]

    lib.CuBool_Matrix_Add.restype = ctypes.c_uint
    lib.CuBool_Matrix_Add.argtypes = [instance_p,
                                      matrix_p,
                                      matrix_p]

    return lib


"""
/** Possible status codes that can be returned from cubool api */

/** Successful execution of the function */
CUBOOL_STATUS_SUCCESS,

/** Generic error code */
CUBOOL_STATUS_ERROR,

/** No cuda compatible device in the system */
CUBOOL_STATUS_DEVICE_NOT_PRESENT,

/** Device side error */
CUBOOL_STATUS_DEVICE_ERROR,

/** Failed to allocate memory on cpy or gpu side */
CUBOOL_STATUS_MEM_OP_FAILED,

/** Passed invalid argument to some function */
CUBOOL_STATUS_INVALID_ARGUMENT,

/** Call of the function is not possible for some context */
CUBOOL_STATUS_INVALID_STATE

/** Some library feature is not implemented */
CUBOOL_STATUS_NOT_IMPLEMENTED
"""
_status_codes_mappings = {
    0: "CUBOOL_STATUS_SUCCESS",
    1: "CUBOOL_STATUS_ERROR",
    2: "CUBOOL_STATUS_DEVICE_NOT_PRESENT",
    3: "CUBOOL_STATUS_DEVICE_ERROR",
    4: "CUBOOL_STATUS_MEM_OP_FAILED",
    5: "CUBOOL_STATUS_INVALID_ARGUMENT",
    6: "CUBOOL_STATUS_INVALID_STATE",
    7: "CUBOOL_STATUS_NOT_IMPLEMENTED"
}

_success = 0


def check(status_code):
    if status_code != _success:
        raise Exception(_status_codes_mappings[status_code])