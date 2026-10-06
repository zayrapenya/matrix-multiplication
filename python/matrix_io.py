import csv


def load_matrix(path):
    with open(path, "r", newline="") as f:
        return [
            [float(value) for value in row]
            for row in csv.reader(f)
        ]
