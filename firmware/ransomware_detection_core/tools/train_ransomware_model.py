#!/usr/bin/env python3
"""Train the ransomware detector from AI_Model_Notebook and export firmware weights."""

from __future__ import annotations

import os
import sys
import time

import numpy as np

RANDOM_SEED = 42
FEATURES = [
    "USB_V",
    "USB_A",
    "USB_W",
    "Int_Accel",
    "Ext_Accel",
    "Net_Traffic",
    "RF_Density",
    "Sys_Temp",
]
LABEL = "Label"
LABEL_MAP = {"No": 0, "Yes": 1}

BATCH_SIZE = 32
MAX_EPOCHS = 150
LEARNING_RATE = 0.001
DROPOUT_RATE = 0.30
L2_LAMBDA = 1e-4
PATIENCE = 15
VAL_SPLIT = 0.15
TEST_SPLIT = 0.15
DECISION_THRESHOLD = 0.45

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
CSV_CANDIDATES = [
    os.path.join(SCRIPT_DIR, "data", "ransomware_dataset_3k_55_45.csv"),
    os.path.join(ROOT, "..", "AI_Model_Notebook_data", "ransomware_dataset_3k_55_45 (3).csv"),
]
OUT_HEADER = os.path.join(ROOT, "Application", "AI", "ransomware_weights.h")


def relu(x: np.ndarray) -> np.ndarray:
    return np.maximum(x, 0.0)


def relu_grad(x: np.ndarray) -> np.ndarray:
    return (x > 0.0).astype(np.float32)


def sigmoid(x: np.ndarray) -> np.ndarray:
    x = np.clip(x, -40.0, 40.0)
    return 1.0 / (1.0 + np.exp(-x))


def split_data(X: np.ndarray, y: np.ndarray):
    rng = np.random.default_rng(RANDOM_SEED)
    idx = np.arange(len(y))
    rng.shuffle(idx)
    X = X[idx]
    y = y[idx]

    n_test = int(len(y) * TEST_SPLIT)
    n_val = int(len(y) * VAL_SPLIT)

    X_test = X[:n_test]
    y_test = y[:n_test]
    X_val = X[n_test : n_test + n_val]
    y_val = y[n_test : n_test + n_val]
    X_train = X[n_test + n_val :]
    y_train = y[n_test + n_val :]
    return X_train, X_val, X_test, y_train, y_val, y_test


def load_or_generate_dataset() -> tuple[np.ndarray, np.ndarray]:
    for path in CSV_CANDIDATES:
        path = os.path.abspath(path)
        if not os.path.exists(path):
            continue
        import csv

        rows = []
        labels = []
        with open(path, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                rows.append([float(row[name]) for name in FEATURES])
                label = row[LABEL]
                if label in LABEL_MAP:
                    labels.append(LABEL_MAP[label])
                else:
                    labels.append(int(float(label)))
        print(f"Loaded dataset: {path}")
        return np.asarray(rows, dtype=np.float32), np.asarray(labels, dtype=np.float32)

    print("Dataset CSV not found — generating synthetic 3k sample set (seed=42).")
    rng = np.random.default_rng(RANDOM_SEED)

    def block(count: int, ranges: list[tuple[float, float]]) -> np.ndarray:
        cols = []
        for lo, hi in ranges:
            cols.append(rng.uniform(lo, hi, count).astype(np.float32))
        return np.column_stack(cols)

    normal_ranges = [
        (5.00, 5.10),
        (0.01, 0.60),
        (0.05, 3.00),
        (0.00, 0.50),
        (0.00, 0.50),
        (0.50, 10.0),
        (-110.0, -40.0),
        (30.0, 55.0),
    ]
    ransom_ranges = [
        (4.80, 5.00),
        (1.20, 3.95),
        (6.00, 20.0),
        (2.00, 5.00),
        (3.00, 6.00),
        (40.0, 150.0),
        (-60.0, 10.0),
        (70.0, 95.0),
    ]

    X0 = block(1650, normal_ranges)
    X1 = block(1350, ransom_ranges)
    X = np.vstack([X0, X1])
    y = np.concatenate(
        [np.zeros(1650, dtype=np.float32), np.ones(1350, dtype=np.float32)]
    )
    idx = rng.permutation(len(y))
    return X[idx], y[idx]


class Adam:
    def __init__(self, lr: float = 0.001):
        self.lr = lr
        self.m = {}
        self.v = {}
        self.t = 0

    def step(self, params: dict[str, np.ndarray], grads: dict[str, np.ndarray]):
        self.t += 1
        for key, param in params.items():
            g = grads[key]
            self.m[key] = 0.9 * self.m.get(key, 0.0) + (1.0 - 0.9) * g
            self.v[key] = 0.999 * self.v.get(key, 0.0) + (1.0 - 0.999) * (g * g)
            m_hat = self.m[key] / (1.0 - 0.9**self.t)
            v_hat = self.v[key] / (1.0 - 0.999**self.t)
            params[key] -= self.lr * m_hat / (np.sqrt(v_hat) + 1e-8)


class RansomwareModel:
    def __init__(self, input_dim: int):
        rng = np.random.default_rng(RANDOM_SEED)
        scale1 = np.sqrt(2.0 / input_dim)
        scale2 = np.sqrt(2.0 / 64)
        self.params = {
            "W1": rng.normal(0.0, scale1, (64, input_dim)).astype(np.float32),
            "b1": np.zeros(64, dtype=np.float32),
            "W2": rng.normal(0.0, scale2, (32, 64)).astype(np.float32),
            "b2": np.zeros(32, dtype=np.float32),
            "W3": rng.normal(0.0, np.sqrt(2.0 / 32), (1, 32)).astype(np.float32),
            "b3": np.zeros(1, dtype=np.float32),
        }
        self.norm_mean = np.zeros(input_dim, dtype=np.float32)
        self.norm_inv_std = np.ones(input_dim, dtype=np.float32)

    def set_normalization(self, X_train: np.ndarray):
        mean = X_train.mean(axis=0)
        var = X_train.var(axis=0)
        self.norm_mean = mean.astype(np.float32)
        self.norm_inv_std = (1.0 / np.sqrt(var + 1e-3)).astype(np.float32)

    def forward(self, X: np.ndarray, training: bool):
        cache = {}
        x = (X - self.norm_mean) * self.norm_inv_std
        cache["x"] = x

        z1 = x @ self.params["W1"].T + self.params["b1"]
        a1 = relu(z1)
        if training:
            mask1 = (np.random.rand(*a1.shape) > DROPOUT_RATE).astype(np.float32)
            a1 = a1 * mask1 / (1.0 - DROPOUT_RATE)
        else:
            mask1 = None
        cache["z1"], cache["a1"], cache["mask1"] = z1, a1, mask1

        z2 = a1 @ self.params["W2"].T + self.params["b2"]
        a2 = relu(z2)
        if training:
            mask2 = (np.random.rand(*a2.shape) > DROPOUT_RATE).astype(np.float32)
            a2 = a2 * mask2 / (1.0 - DROPOUT_RATE)
        else:
            mask2 = None
        cache["z2"], cache["a2"], cache["mask2"] = z2, a2, mask2

        z3 = a2 @ self.params["W3"].T + self.params["b3"]
        y_hat = sigmoid(z3)
        cache["z3"], cache["y_hat"] = z3, y_hat
        return y_hat, cache

    def backward(self, X: np.ndarray, y: np.ndarray, cache: dict):
        m = X.shape[0]
        grads = {k: np.zeros_like(v) for k, v in self.params.items()}

        y_hat = cache["y_hat"]
        dz3 = (y_hat - y.reshape(-1, 1)) / m
        grads["W3"] = dz3.T @ cache["a2"] + L2_LAMBDA * self.params["W3"]
        grads["b3"] = dz3.sum(axis=0)

        da2 = dz3 @ self.params["W3"]
        if cache["mask2"] is not None:
            da2 = da2 * cache["mask2"] / (1.0 - DROPOUT_RATE)
        dz2 = da2 * relu_grad(cache["z2"])
        grads["W2"] = dz2.T @ cache["a1"] + L2_LAMBDA * self.params["W2"]
        grads["b2"] = dz2.sum(axis=0)

        da1 = dz2 @ self.params["W2"]
        if cache["mask1"] is not None:
            da1 = da1 * cache["mask1"] / (1.0 - DROPOUT_RATE)
        dz1 = da1 * relu_grad(cache["z1"])
        grads["W1"] = dz1.T @ cache["x"] + L2_LAMBDA * self.params["W1"]
        grads["b1"] = dz1.sum(axis=0)

        return grads

    def predict(self, X: np.ndarray) -> np.ndarray:
        y_hat, _ = self.forward(X, training=False)
        return y_hat.reshape(-1)


def auc_score(y_true: np.ndarray, y_prob: np.ndarray) -> float:
    order = np.argsort(-y_prob)
    y_sorted = y_true[order]
    n_pos = y_sorted.sum()
    n_neg = len(y_sorted) - n_pos
    if n_pos == 0 or n_neg == 0:
        return 0.5
    tpr = np.cumsum(y_sorted) / n_pos
    fpr = np.cumsum(1.0 - y_sorted) / n_neg
    return float(np.trapz(tpr, fpr))


def iterate_minibatches(X, y, batch_size):
    rng = np.random.default_rng()
    idx = np.arange(len(y))
    rng.shuffle(idx)
    for start in range(0, len(y), batch_size):
        batch_idx = idx[start : start + batch_size]
        yield X[batch_idx], y[batch_idx]


def export_header(model: RansomwareModel, metrics: dict[str, float]) -> None:
    def fmt_array(name: str, arr: np.ndarray, cols: int) -> str:
        flat = arr.reshape(-1)
        lines = []
        for i in range(0, len(flat), cols):
            chunk = flat[i : i + cols]
            lines.append("    " + ", ".join(f"{v:.8f}f" for v in chunk) + ",")
        shape = " x ".join(str(s) for s in arr.shape)
        return f"/* {name} shape=[{shape}] */\nstatic const float {name}[] = {{\n" + "\n".join(lines) + "\n};\n"

    body = [
        "#ifndef RANSOMWARE_WEIGHTS_H",
        "#define RANSOMWARE_WEIGHTS_H",
        "",
        "/* Auto-generated from AI_Model_Notebook architecture (Dense NN + normalization). */",
        f"#define RANSOMWARE_INPUT_COUNT 8",
        f"#define RANSOMWARE_THRESHOLD {DECISION_THRESHOLD}f",
        f"/* val_auc={metrics['val_auc']:.4f} test_auc={metrics['test_auc']:.4f} */",
        "",
        fmt_array("ransomware_norm_mean", model.norm_mean, 4),
        fmt_array("ransomware_norm_inv_std", model.norm_inv_std, 4),
        fmt_array("ransomware_w1", model.params["W1"], 8),
        fmt_array("ransomware_b1", model.params["b1"], 8),
        fmt_array("ransomware_w2", model.params["W2"], 8),
        fmt_array("ransomware_b2", model.params["b2"], 8),
        fmt_array("ransomware_w3", model.params["W3"], 8),
        fmt_array("ransomware_b3", model.params["b3"], 1),
        "",
        "#endif",
        "",
    ]

    os.makedirs(os.path.dirname(OUT_HEADER), exist_ok=True)
    with open(OUT_HEADER, "w", encoding="utf-8") as f:
        f.write("\n".join(body))
    print(f"Wrote {OUT_HEADER}")


def main() -> int:
    np.random.seed(RANDOM_SEED)
    X, y = load_or_generate_dataset()
    X_train, X_val, X_test, y_train, y_val, y_test = split_data(X, y)

    model = RansomwareModel(X.shape[1])
    model.set_normalization(X_train)
    opt = Adam(LEARNING_RATE)

    best_val = -1.0
    best_state = None
    stale = 0

    print(f"Train={len(y_train)} Val={len(y_val)} Test={len(y_test)}")
    t0 = time.time()
    for epoch in range(1, MAX_EPOCHS + 1):
        for xb, yb in iterate_minibatches(X_train, y_train, BATCH_SIZE):
            _, cache = model.forward(xb, training=True)
            grads = model.backward(xb, yb, cache)
            opt.step(model.params, grads)

        val_prob = model.predict(X_val)
        val_auc = auc_score(y_val, val_prob)
        if val_auc > best_val:
            best_val = val_auc
            best_state = (
                {k: v.copy() for k, v in model.params.items()},
                model.norm_mean.copy(),
                model.norm_inv_std.copy(),
            )
            stale = 0
        else:
            stale += 1

        if epoch % 10 == 0 or epoch == 1:
            print(f"epoch {epoch:3d}  val_auc={val_auc:.4f}  best={best_val:.4f}")
        if stale >= PATIENCE:
            print(f"Early stop at epoch {epoch}")
            break

    if best_state is not None:
        model.params, model.norm_mean, model.norm_inv_std = best_state

    test_prob = model.predict(X_test)
    test_auc = auc_score(y_test, test_prob)
    val_prob = model.predict(X_val)
    val_auc = auc_score(y_val, val_prob)
    print(f"Training done in {time.time() - t0:.1f}s  val_auc={val_auc:.4f}  test_auc={test_auc:.4f}")

    export_header(model, {"val_auc": val_auc, "test_auc": test_auc})
    return 0


if __name__ == "__main__":
    sys.exit(main())
