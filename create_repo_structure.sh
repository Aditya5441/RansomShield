#!/usr/bin/env bash
# Run this INSIDE your repo folder (where LICENSE/README will live).
set -e

mkdir -p docs
mkdir -p firmware/secure_boot
mkdir -p firmware/bootloader_dsp
mkdir -p firmware/ransomware_detection_core
mkdir -p gateway/esp32_gateway_ecdh
mkdir -p ml/data_set
mkdir -p ml/ai_google_collab_notebook
mkdir -p matlab/dsp_robustness_analysis_script
mkdir -p matlab/security_robustness_analysis_script
mkdir -p tools/firmware_image_security_sign_tool
mkdir -p tools/ransomware_behaviour_generation_tool
mkdir -p validation_video
mkdir -p media_pics/oscilloscope
mkdir -p media_pics/matlab_plots
mkdir -p media_pics/thingspeak_telegram

echo "Folder structure created:"
find . -maxdepth 3 -type d -not -path "./.git*" | sort
