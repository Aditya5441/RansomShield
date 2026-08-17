#!/usr/bin/env python3
"""
keygen.py - Generate a P-256 keypair for AEGIS-ECU firmware signing.

Outputs:
  keys/signing_key.pem   Private key (PEM). KEEP THIS SECRET - anyone
                          with this file can sign firmware your boards
                          will trust. Do not commit it to git.
  keys/pubkey.bin         Public key as raw 64 bytes (32-byte X ||
                          32-byte Y), the exact format
                          SecureBoot_VerifyFirmware() / ECDSA_VerifyP256()
                          read from PUBKEY_ADDR (0x08800000) on the STM32.

Usage:
  python3 keygen.py
"""
import os
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization

OUT_DIR = "keys"


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    private_key = ec.generate_private_key(ec.SECP256R1())
    public_key = private_key.public_key()

    # Private key -> PEM (keep secret)
    priv_path = os.path.join(OUT_DIR, "signing_key.pem")
    with open(priv_path, "wb") as f:
        f.write(private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption(),
        ))
    os.chmod(priv_path, 0o600)

    # Public key -> raw 64 bytes (X || Y), matching what the STM32 side
    # reads directly out of flash with no parsing (no DER/ASN.1 - the
    # firmware code just memcpy()s 64 bytes).
    numbers = public_key.public_numbers()
    x_bytes = numbers.x.to_bytes(32, "big")
    y_bytes = numbers.y.to_bytes(32, "big")
    pub_path = os.path.join(OUT_DIR, "pubkey.bin")
    with open(pub_path, "wb") as f:
        f.write(x_bytes + y_bytes)

    print(f"Private key (keep secret): {priv_path}")
    print(f"Public key (flash to 0x08800000): {pub_path}")
    print(f"Public key X: {x_bytes.hex()}")
    print(f"Public key Y: {y_bytes.hex()}")
    print()
    print("Next step - flash the public key to the board with STM32CubeProgrammer:")
    print(f"  STM32_Programmer_CLI -c port=SWD -w {pub_path} 0x08800000")
    print("Consider enabling write protection on that flash sector afterward")
    print("so running firmware can't overwrite its own trust anchor.")


if __name__ == "__main__":
    main()
