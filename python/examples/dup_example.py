"""
Example of matrix duplication 
"""

import pycubool as cb

#
#  Matrix initialization
#

shape = (3, 3)                          # Matrix shape
a = cb.Matrix.empty(shape=shape)
a[1, 0] = True
a[1, 1] = True
a[1, 2] = True
a[0, 1] = True

#
#  Matrix duplicate operation
#

result = a.dup()

print("Result of matrix duplication operation:")
print(result, sep='\n')                 # Matrix output
