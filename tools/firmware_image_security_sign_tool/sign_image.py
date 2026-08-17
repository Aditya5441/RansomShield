#!/usr/bin/env python3
"""
sign_image.py - Sign a compiled firmware image for AEGIS-ECU's Secure
Boot chain (matches SecureBoot_VerifyFirmware() in secure_boot.c on
both the Secure Boot and Bootloader stages).

On-flash image layout this produces:
  [0:8]    magic "STSECAPP"
  [8:12]   fw_size (uint32, little-endian) - total size: header + code + signature
  [12:16]  version (uint32, little-endian) - anti-rollback counter
  [16:64]  reserved / zero-padded
  [64:64+N]        raw compiled code (.bin)
  [64+N:64+N+64]   ECDSA P-256 signature, raw R (32 bytes) || S (32 bytes)

Usage:
  python3 sign_image.py --input app.bin --output app_signed.bin \\
      --version 1 --key keys/signing_key.pem

The --version value becomes the anti-rollback counter compared against
TAMP_BKP0R/BKP1R on-device - a lower version than what's already stored
will be rejected as a rollback.
"""
import argparse
import struct
import sys

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature
from cryptography.exceptions import InvalidSignature

MAGIC = b"STSECAPP"
HEADER_LEN = 64
SIGNATURE_LEN = 64  # raw R (32) || S (32) - NOT DER encoding


def build_and_sign(code: bytes, version: int, private_key) -> bytes:
    fw_size = HEADER_LEN + len(code) + SIGNATURE_LEN

    header = bytearray(HEADER_LEN)
    header[0:8] = MAGIC
    header[8:12] = struct.pack("<I", fw_size)
    header[12:16] = struct.pack("<I", version)
    # [16:64] left zero - reserved

    payload = bytes(header) + code  # this is exactly what the device hashes

    der_signature = private_key.sign(payload, ec.ECDSA(hashes.SHA256()))
    r, s = decode_dss_signature(der_signature)
    raw_signature = r.to_bytes(32, "big") + s.to_bytes(32, "big")

    return payload + raw_signature


def self_check(signed_image: bytes, public_key):
    """Re-verify what we just built, the same way the device will:
    hash header+code, check the trailing raw R||S against that hash."""
    payload = signed_image[:-SIGNATURE_LEN]
    signature = signed_image[-SIGNATURE_LEN:]
    r = int.from_bytes(signature[:32], "big")
    s = int.from_bytes(signature[32:], "big")

    from cryptography.hazmat.primitives.asymmetric.utils import encode_dss_signature
    der_signature = encode_dss_signature(r, s)

    try:
        public_key.verify(der_signature, payload, ec.ECDSA(hashes.SHA256()))
        return True
    except InvalidSignature:
        return False


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--input", required=True, help="Compiled raw .bin (code only, no header)")
    ap.add_argument("--output", required=True, help="Output path for the signed image")
    ap.add_argument("--version", type=int, required=True, help="Anti-rollback version counter")
    ap.add_argument("--key", default="keys/signing_key.pem", help="Path to private signing key (PEM)")
    args = ap.parse_args()

    with open(args.key, "rb") as f:
        private_key = serialization.load_pem_private_key(f.read(), password=None)

    if not isinstance(private_key, ec.EllipticCurvePrivateKey) or \
       not isinstance(private_key.curve, ec.SECP256R1):
        print("ERROR: key is not a P-256 (secp256r1) EC private key", file=sys.stderr)
        sys.exit(1)

    with open(args.input, "rb") as f:
        code = f.read()

    signed_image = build_and_sign(code, args.version, private_key)

    if not self_check(signed_image, private_key.public_key()):
        print("ERROR: self-check failed - signature does not verify against its own image. "
              "This should never happen; do not flash this output.", file=sys.stderr)
        sys.exit(1)

    with open(args.output, "wb") as f:
        f.write(signed_image)

    print(f"Signed image written: {args.output}")
    print(f"  code size:      {len(code)} bytes")
    print(f"  total fw_size:  {len(signed_image)} bytes")
    print(f"  version:        {args.version}")
    print(f"  self-check:     PASSED")


if __name__ == "__main__":
    main()
