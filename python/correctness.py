from matrix_multiplication import multiply


def assert_matrix_equal(actual, expected, tolerance=1e-9):
    assert len(actual) == len(expected)

    for i in range(len(expected)):
        assert len(actual[i]) == len(expected[i])

        for j in range(len(expected[i])):
            assert abs(actual[i][j] - expected[i][j]) <= tolerance, (
                f"Mismatch at ({i}, {j}): "
                f"{actual[i][j]} != {expected[i][j]}"
            )


def test_known_matrix():
    A = [
        [1.0, 2.0],
        [3.0, 4.0]
    ]

    B = [
        [5.0, 6.0],
        [7.0, 8.0]
    ]

    expected = [
        [19.0, 22.0],
        [43.0, 50.0]
    ]

    result = multiply(A, B)

    assert_matrix_equal(result, expected)


def test_identity_matrix():
    A = [
        [1.0, 2.0],
        [3.0, 4.0]
    ]

    identity = [
        [1.0, 0.0],
        [0.0, 1.0]
    ]

    result = multiply(A, identity)

    assert_matrix_equal(result, A)


def test_zero_matrix():
    A = [
        [1.0, 2.0],
        [3.0, 4.0]
    ]

    zero = [
        [0.0, 0.0],
        [0.0, 0.0]
    ]

    expected = [
        [0.0, 0.0],
        [0.0, 0.0]
    ]

    result = multiply(A, zero)

    assert_matrix_equal(result, expected)


if __name__ == "__main__":
    test_known_matrix()
    test_identity_matrix()
    test_zero_matrix()

    print("All correctness tests passed.")
