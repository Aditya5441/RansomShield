#!/usr/bin/env python3
"""Run this cell on Kaggle after training to export dense_2 for firmware."""

EXPORT = """
import numpy as np
# model = your trained keras model
w = model.get_weights()
# Sequential: [0]=norm_mean [1]=norm_var [2]=d1_kernel [3]=d1_bias [4]=d2_kernel [5]=d2_bias [6]=out_kernel [7]=out_bias
np.savez(
    'model_weights.npz',
    dense2_w=w[4],
)
print('Saved model_weights.npz — copy to tools/model_weights.npz in the firmware repo')
"""

if __name__ == "__main__":
    print(EXPORT)
