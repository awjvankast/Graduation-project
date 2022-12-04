import numpy as np
from sympy import *



x, y, z = symbols('x y z')
# w = np.array([0,0,50])
# f = np.array([0,50,60])
# m = np.array([60,90,55])
# exprtot = []
# for k in range(0,3):
#     exprtot.append( diff( ((x-w[k])**2+(y-f[k])**2-m[k]**2)**2,x ) )

# sol2 =  solve( exprtot, x )
# print(sol2)

# print(exprtot)

# sol2 = solve(Matrix(exprtot),x,y)
# print(sol2)

# for k in range(0,3):
#     exprtot.append( diff( ((x-w[k])**2+(y-f[k])**2-m[k]**2)**2,y ) )
# print(exprtot)


matrix = Matrix([
    [x**2 -4],
    [x*2+11],
      ])

sol = solve( matrix,x)
print(sol)


# sol2 = solve(Matrix(exprtot),x,y)
# print(sol2)
print('end')