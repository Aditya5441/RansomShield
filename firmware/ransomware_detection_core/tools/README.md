# AI model tools

Training pipeline from `AI_Model_Notebook` (Kaggle notebook in the workspace).

## Use your Keras layer weights (recommended)

After training in Kaggle, export the missing `dense_2` kernel:

```python
import numpy as np
w = model.get_weights()
np.savez('model_weights.npz', dense2_w=w[4])  # shape (64, 32)
```

Copy `model_weights.npz` to `tools/model_weights.npz`, then:

```bash
make keras-weights
make
```

`tools/generate_keras_weights_h.py` already embeds your **normalization**, **dense_1**, **dense_2 biases**, and **output** weights from the notebook.

## Retrain with your exact Kaggle dataset

1. Copy `ransomware_dataset_3k_55_45 (3).csv` into `tools/data/ransomware_dataset_3k_55_45.csv`
2. From the project root:

```bash
make train
```

This regenerates `Application/AI/ransomware_weights.h` used by the firmware.

## Model inputs (order matters)

| Index | Name        | Firmware field              |
|-------|-------------|-----------------------------|
| 0     | USB_V       | `SensorData_t.usb_voltage`  |
| 1     | USB_A       | `SensorData_t.usb_current`  |
| 2     | USB_W       | `SensorData_t.usb_power`    |
| 3     | Int_Accel   | `accel_internal[0]`       |
| 4     | Ext_Accel   | `accel_external[0]`         |
| 5     | Net_Traffic | `net_traffic`               |
| 6     | RF_Density  | `rf_nrf_dbm`                |
| 7     | Sys_Temp    | `temp`                      |

Output: probability in `[0, 1]`. Alert when `>= 0.45` (`RANSOMWARE_THRESHOLD`).
