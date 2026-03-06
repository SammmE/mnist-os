import numpy as np
from sklearn import datasets
from sklearn.model_selection import ShuffleSplit, train_test_split, validation_curve
from sklearn.neural_network import MLPClassifier

IMAGE = 0

digits = datasets.load_digits()
X = digits["images"]
y = digits["target"]
target_names = digits["target_names"]

n_samples = len(X)
X = X.reshape((n_samples, -1))

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=32
)
image = X_test[IMAGE]
lbl = y_test[IMAGE]

cv = ShuffleSplit(n_splits=5, test_size=0.2, random_state=0)  # 64 -> 100 -> 10
clf = MLPClassifier()
clf.fit(X_train, y_train)
print(f"Train Accuracy = {clf.score(X_train, y_train)}")
print(f"Test Accuracy = {clf.score(X_test, y_test)}")

W1, W2 = clf.coefs_
b1, b2 = clf.intercepts_

W1 = W1.astype(np.float32)
b1 = b1.astype(np.float32)
W2 = W2.astype(np.float32)
b2 = b2.astype(np.float32)

test_image_f32 = image.astype(np.float32)

with open("WEIGHTS.BIN", "wb") as f:
    f.write(W1.tobytes())
    f.write(b1.tobytes())
    f.write(W2.tobytes())
    f.write(b2.tobytes())

with open("IMAGE.BIN", "wb") as f:
    f.write(test_image_f32.tobytes())

print(lbl)
