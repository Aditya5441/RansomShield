// ================================================
// ESP32 Secure Gateway - ECDH session key + AES-256-CBC + CBC-MAC
// STM32 (UART) <-> ESP32 <-> TLS HTTPS (Thinger.io + Telegram)
//
// Transport note: earlier drafts of this file read data via an NRF24L01
// radio, but the STM32 side sends via LPUART (bm_lpuart_transmit /
// bm_lpuart_receive) - NRF24 on the STM32 side is a signal-strength
// SENSOR (NRF24_GetRSSI), not the STM32<->ESP32 data link. This version
// uses a hardware UART (Serial1) to match what the STM32 firmware
// actually does. Wire RX/TX (and GND) between the two boards' UART
// pins - adjust RX1/TX1 pin numbers below for your actual wiring.
//
// Handshake wire format (must match session.c on the STM32 side):
//   "HELO" (4 bytes) + 64-byte uncompressed P-256 public key (X||Y)
// Both sides send their HELO frame and wait for the peer's; ECDH is
// symmetric so there's no initiator/responder role to negotiate.
// ================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ThingerESP32.h>
#include <UniversalTelegramBot.h>
#include "mbedtls/aes.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/sha256.h"

// ====================== UART link to STM32 ======================
// Adjust pins to match your actual wiring; ESP32 Serial1 defaults may
// conflict with other peripherals depending on board variant.
#define STM32_RX_PIN  16
#define STM32_TX_PIN  17
HardwareSerial Stm32Link(1);  // UART1

// ====================== WiFi ======================
// Fill in from your own secrets/config. If the values previously here
// were real credentials, rotate them - a leaked bot token or WiFi
// password works for whoever has it, project or not.
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ====================== Thinger.io ======================
#define THINGER_USERNAME    "YOUR_THINGER_USERNAME"
#define THINGER_DEVICE_ID   "YOUR_THINGER_DEVICE_ID"
#define THINGER_CREDENTIAL  "YOUR_THINGER_DEVICE_CREDENTIAL"
ThingerESP32 thing(THINGER_USERNAME, THINGER_DEVICE_ID, THINGER_CREDENTIAL);

// ====================== Telegram ======================
#define BOT_TOKEN  "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID    "YOUR_TELEGRAM_CHAT_ID"
WiFiClientSecure secureClient;
UniversalTelegramBot bot(BOT_TOKEN, secureClient);

// DigiCert Global Root G2 (api.telegram.org). Certs rotate - re-pull
// from a live source before deploying rather than trusting this
// indefinitely.
const char* telegram_root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOgR6V7A\n" \
"-----END CERTIFICATE-----";

// ====================== Session key state ======================
static uint8_t g_session_key[32];
static bool    g_have_session = false;

// ====================== Packet structures ======================
// Mirrors STM32's SecurePacket_t (comm_encryption.h) - 64-byte payload,
// NOT 128 (the original header had this mismatched with what
// ENC_Encrypt actually wrote).
struct EncryptedPacket {
  uint8_t encrypted_payload[64];
  uint8_t iv[16];
  uint8_t tag[16];
};

// Mirrors STM32's SystemData_t field order EXACTLY (comm_encryption.h):
// voltage, current, power, accel_internal[3], accel_external[3],
// network, rf, temperature, ai_output.
struct PlainPacket {
  float voltage;
  float current;
  float power;
  float accel_internal[3];
  float accel_external[3];
  float network;
  float rf;
  float temperature;
  float ai_output;
};

PlainPacket plainData;

// ====================== Dashboard variables ======================
float voltage = 0, current = 0, power = 0, temperature = 0;
float rf_rssi = 0, network = 0, ai_output = 0;
float accel_internal[3] = {0, 0, 0};
float accel_external[3] = {0, 0, 0};

// ====================== ECDH handshake ======================

static bool ecdh_handshake(void)
{
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context drbg;
  mbedtls_ecdh_context ecdh;
  const char *pers = "aegis-ecdh";
  int ret;

  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&drbg);
  mbedtls_ecdh_init(&ecdh);

  ret = mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *)pers, strlen(pers));
  if (ret != 0) {
    Serial.println("DRBG seed failed");
    return false;
  }

  ret = mbedtls_ecdh_setup(&ecdh, MBEDTLS_ECP_DP_SECP256R1);
  if (ret != 0) {
    Serial.println("ECDH setup failed");
    return false;
  }

  // Generate our ephemeral keypair
  ret = mbedtls_ecdh_gen_public(&ecdh.ctx.mbed_ecdh.grp,
                                 &ecdh.ctx.mbed_ecdh.d,
                                 &ecdh.ctx.mbed_ecdh.Q,
                                 mbedtls_ctr_drbg_random, &drbg);
  if (ret != 0) {
    Serial.println("ECDH keygen failed");
    return false;
  }

  // Export our public key as raw X||Y (32+32 bytes)
  uint8_t our_pub[64];
  size_t olen;
  mbedtls_mpi_write_binary(&ecdh.ctx.mbed_ecdh.Q.X, our_pub, 32);
  mbedtls_mpi_write_binary(&ecdh.ctx.mbed_ecdh.Q.Y, our_pub + 32, 32);

  // Send HELO frame
  uint8_t tx_frame[4 + 64];
  memcpy(tx_frame, "HELO", 4);
  memcpy(tx_frame + 4, our_pub, 64);
  Stm32Link.write(tx_frame, sizeof(tx_frame));

  // Wait for peer's HELO frame (2s timeout, matching session.c)
  uint8_t rx_frame[4 + 64];
  unsigned long start = millis();
  size_t got = 0;
  while ((millis() - start) < 2000UL && got < sizeof(rx_frame)) {
    if (Stm32Link.available()) {
      rx_frame[got++] = Stm32Link.read();
    }
  }

  if (got < sizeof(rx_frame)) {
    Serial.println("ECDH handshake timeout - no HELO from STM32");
    mbedtls_ecdh_free(&ecdh);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&entropy);
    return false;
  }

  if (memcmp(rx_frame, "HELO", 4) != 0) {
    Serial.println("ECDH handshake: malformed frame");
    mbedtls_ecdh_free(&ecdh);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&entropy);
    return false;
  }

  // Import peer's public key
  ret = mbedtls_mpi_lset(&ecdh.ctx.mbed_ecdh.Qp.Z, 1);
  ret |= mbedtls_mpi_read_binary(&ecdh.ctx.mbed_ecdh.Qp.X, rx_frame + 4, 32);
  ret |= mbedtls_mpi_read_binary(&ecdh.ctx.mbed_ecdh.Qp.Y, rx_frame + 4 + 32, 32);
  if (ret != 0) {
    Serial.println("ECDH: failed to import peer public key");
    mbedtls_ecdh_free(&ecdh);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&entropy);
    return false;
  }

  // Compute shared secret
  ret = mbedtls_ecdh_compute_shared(&ecdh.ctx.mbed_ecdh.grp,
                                     &ecdh.ctx.mbed_ecdh.z,
                                     &ecdh.ctx.mbed_ecdh.Qp,
                                     &ecdh.ctx.mbed_ecdh.d,
                                     mbedtls_ctr_drbg_random, &drbg);
  if (ret != 0) {
    Serial.println("ECDH shared-secret computation failed");
    mbedtls_ecdh_free(&ecdh);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&entropy);
    return false;
  }

  uint8_t shared_x[32];
  mbedtls_mpi_write_binary(&ecdh.ctx.mbed_ecdh.z, shared_x, 32);

  // Derive session key: SHA-256(shared_x || "AEGIS-SESSION"), matching
  // session.c's KDF on the STM32 side exactly.
  uint8_t kdf_input[32 + 13];
  memcpy(kdf_input, shared_x, 32);
  memcpy(kdf_input + 32, "AEGIS-SESSION", 13);
  mbedtls_sha256(kdf_input, sizeof(kdf_input), g_session_key, 0);

  memset(shared_x, 0, sizeof(shared_x));
  mbedtls_ecdh_free(&ecdh);
  mbedtls_ctr_drbg_free(&drbg);
  mbedtls_entropy_free(&entropy);

  g_have_session = true;
  Serial.println("ECDH session key established");
  return true;
}

// ====================== AES-256-CBC / CBC-MAC ======================

static void aes256_cbc_decrypt(const uint8_t *key, const uint8_t *iv_in,
                                const uint8_t *in, uint8_t *out, size_t len)
{
  mbedtls_aes_context ctx;
  uint8_t iv[16];
  memcpy(iv, iv_in, 16);

  mbedtls_aes_init(&ctx);
  mbedtls_aes_setkey_dec(&ctx, key, 256);
  mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, len, iv, in, out);
  mbedtls_aes_free(&ctx);
}

static bool verify_cbc_mac(const uint8_t *key, const uint8_t *plain,
                            size_t len, const uint8_t *received_tag)
{
  mbedtls_aes_context ctx;
  uint8_t iv[16] = {0};
  uint8_t mac_buf[64];

  mbedtls_aes_init(&ctx);
  mbedtls_aes_setkey_enc(&ctx, key, 256);
  mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, len, iv, plain, mac_buf);
  mbedtls_aes_free(&ctx);

  return memcmp(mac_buf + (len - 16), received_tag, 16) == 0;
}

// ====================== Setup / Loop ======================

void setup() {
  Serial.begin(115200);
  Stm32Link.begin(115200, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  secureClient.setCACert(telegram_root_ca);

  thing["voltage"]     >> [](pson& out){ out = voltage; };
  thing["current"]     >> [](pson& out){ out = current; };
  thing["temperature"] >> [](pson& out){ out = temperature; };
  thing["ai_output"]   >> [](pson& out){ out = ai_output; };

  Serial.println("Waiting for ECDH handshake from STM32...");
  if (!ecdh_handshake()) {
    Serial.println("WARNING: no session key - packets will be dropped until reset");
  }
}

void loop() {
  thing.handle();

  if (!g_have_session) {
    // No retry/re-handshake logic yet - a failed boot-time handshake
    // currently means this device needs a manual reset once the STM32
    // side is confirmed up and sending its HELO frame.
    delay(500);
    return;
  }

  if (Stm32Link.available() >= (int)sizeof(EncryptedPacket)) {
    EncryptedPacket rxPacket;
    Stm32Link.readBytes((uint8_t *)&rxPacket, sizeof(rxPacket));

    uint8_t decrypted[64];
    aes256_cbc_decrypt(g_session_key, rxPacket.iv, rxPacket.encrypted_payload,
                        decrypted, sizeof(decrypted));

    if (!verify_cbc_mac(g_session_key, decrypted, sizeof(decrypted), rxPacket.tag)) {
      Serial.println("Packet failed CBC-MAC check - discarding");
      return;
    }

    if (sizeof(PlainPacket) > sizeof(decrypted)) {
      Serial.println("PlainPacket layout mismatch vs STM32 SystemData_t");
      return;
    }
    memcpy(&plainData, decrypted, sizeof(PlainPacket));

    voltage     = plainData.voltage;
    current     = plainData.current;
    power       = plainData.power;
    temperature = plainData.temperature;
    network     = plainData.network;
    rf_rssi     = plainData.rf;
    ai_output   = plainData.ai_output;
    memcpy(accel_internal, plainData.accel_internal, sizeof(accel_internal));
    memcpy(accel_external, plainData.accel_external, sizeof(accel_external));

    Serial.printf("Decrypted | V: %.2fV | I: %.3fA | T: %.1fC | AI: %.4f\n",
                  voltage, current, temperature, ai_output);

    thing.writeBucket("AI_Secure_Data", [](pson& data){
      data["voltage"]     = voltage;
      data["current"]     = current;
      data["power"]       = power;
      data["temperature"] = temperature;
      data["network"]     = network;
      data["rf_rssi"]     = rf_rssi;
      data["ai_output"]   = ai_output;
    });

    String msg = "AI anomaly level: " + String(ai_output, 4);
    if (bot.sendMessage(CHAT_ID, msg, "")) {
      Serial.println("AI result sent to Telegram (HTTPS)");
    }
  }

  delay(50);
}
