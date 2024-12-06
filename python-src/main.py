import uuid
import argparse

from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("problem_name")
parser.add_argument("n")
parser.add_argument("i")
parser.add_argument("data_directory")
parser.add_argument("experiment_results_directory")
parser.add_argument("--gt", default=False, action="store_true")

LOCAL_DATA_PATH = Path("data.out")


def generate_file_name():
    return "tsk_" + str(uuid.uuid4()).replace("-", "")


def generate_unique_file_path(data_directory):
    file_name = generate_file_name()

    while (Path(data_directory) / (file_name + ".out")).exists():
        file_name = generate_file_name()

    return Path(data_directory) / (file_name + ".out"), file_name


if __name__ == "__main__":
    args = parser.parse_args()

    file_path, file_name = generate_unique_file_path(args.data_directory)

    with open(LOCAL_DATA_PATH, "r") as f:
        lines = f.readlines()
        time = lines[0]

        with open(file_path, "w") as f:
            f.writelines(lines[1:])

    if args.gt:
        name = "ground-truth"
    else:
        name = "local"

    with open(args.experiment_results_directory, "a") as f:
        f.write(
            f"{name}-{args.problem_name}-{args.n}-{args.i} {file_name} {int(time)/1000000:.6f}\n"
        )
    print(
        f"{name}-{args.problem_name}-{args.n}-{args.i} {file_name} {int(time)/1000000:.6f}"
    )
