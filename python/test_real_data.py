from matrix_io import load_matrix
from matrix_multiplication import multiply


A = load_matrix("data/input/n50_A.csv")
B = load_matrix("data/input/n50_B.csv")

assert len(A) == 50
assert len(B) == 50
assert all(len(row) == 50 for row in A)
assert all(len(row) == 50 for row in B)

C = multiply(A, B)

assert len(C) == 50
assert all(len(row) == 50 for row in C)

checksum = sum(sum(row) for row in C)

print("Real data test passed.")
print(f"Matrix size: 50x50")
print(f"Result checksum: {checksum:.12f}")
