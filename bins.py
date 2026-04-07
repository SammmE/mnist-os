from __future__ import annotations

import argparse
import struct
from pathlib import Path

import numpy as np
from sklearn import datasets
from sklearn.model_selection import train_test_split

NN_CONFIG_MAGIC = 0x534F4E4D
NN_CONFIG_VERSION = 1


def write_bytes(path: Path, payload: bytes) -> None:
    path.write_bytes(payload)
    print(f"Wrote {path.name:<12} {len(payload):>8} bytes")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Export digits dataset binaries for MNIST-OS training."
    )
    parser.add_argument("--output-dir", default="../disk", help="Output directory")
    parser.add_argument("--hidden-size", type=int, default=32, help="Hidden neurons")
    parser.add_argument("--epochs", type=int, default=8, help="Training epochs")
    parser.add_argument(
        "--learning-rate", type=float, default=0.015, help="SGD learning rate"
    )
    parser.add_argument(
        "--sample-index",
        type=int,
        default=0,
        help="Test-set sample index displayed after training",
    )
    parser.add_argument("--test-size", type=float, default=0.2, help="Test split")
    parser.add_argument("--random-state", type=int, default=32, help="Split seed")
    args = parser.parse_args()

    digits = datasets.load_digits()
    inputs = digits["images"].reshape((len(digits["images"]), -1)).astype(np.float32)
    inputs /= 16.0
    labels = digits["target"].astype(np.uint8)

    x_train, x_test, y_train, y_test = train_test_split(
        inputs,
        labels,
        test_size=args.test_size,
        random_state=args.random_state,
        stratify=labels,
    )

    if not 0 <= args.sample_index < len(x_test):
        raise ValueError(
            f"sample-index {args.sample_index} is out of range for {len(x_test)} test samples"
        )

    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    config = struct.pack(
        "<9If",
        NN_CONFIG_MAGIC,
        NN_CONFIG_VERSION,
        x_train.shape[1],
        args.hidden_size,
        len(digits.target_names),
        len(x_train),
        len(x_test),
        args.epochs,
        args.sample_index,
        args.learning_rate,
    )

    print("Dataset export")
    print(f"Train samples : {len(x_train)}")
    print(f"Test samples  : {len(x_test)}")
    print(f"Input size    : {x_train.shape[1]}")
    print(f"Hidden size   : {args.hidden_size}")
    print(f"Output classes: {len(digits.target_names)}")
    print(f"Epochs        : {args.epochs}")
    print(f"Learning rate : {args.learning_rate:.4f}")
    print(f"Sample index  : {args.sample_index} (label {int(y_test[args.sample_index])})")
    print("")

    write_bytes(output_dir / "CONFIG.BIN", config)
    write_bytes(output_dir / "TRAIN.BIN", np.ascontiguousarray(x_train).tobytes())
    write_bytes(output_dir / "TRNLABEL.BIN", np.ascontiguousarray(y_train).tobytes())
    write_bytes(output_dir / "TEST.BIN", np.ascontiguousarray(x_test).tobytes())
    write_bytes(output_dir / "TSTLABEL.BIN", np.ascontiguousarray(y_test).tobytes())


if __name__ == "__main__":
    main()
