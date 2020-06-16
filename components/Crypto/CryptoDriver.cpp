#include "esp_log.h"
#include "sdkconfig.h"

extern "C" {
  #include "cryptoauthlib.h"
  #include "atcacert/atcacert_client.h"
  #include "tls/atcatls.h"
}
#include "cert_def_1_signer.h"
#include "cert_def_2_device.h"

#include "Constants.h"
#include "CryptoDriver.h"


namespace {
  // Cryptoauthlib device definition
  // statically allocated here because the HAL driver uses it by pointer
  ATCAIfaceCfg atcaDevice = cfg_ateccx08a_i2c_default;

  const char LOG_TAG[] = "Crypto";
}

CryptoDriver::CryptoDriver()
: m_clientCertificateSize(0u), m_signerCertificateSize(0u)
{
  memset(m_clientCertificate, 0, sizeof(m_clientCertificate));
  memset(m_signerCertificate, 0, sizeof(m_signerCertificate));
}


bool CryptoDriver::resetAndInit()
{
  // override defaults
  atcaDevice.atcai2c.slave_address = CRYPTO_SLAVE_ADDRESS;
  atcaDevice.atcai2c.baud = Constants::TWI_BUS_MAX_FREQUENCY;
  atcaDevice.wake_delay = 1500u;  // TODO: According to default and datasheet this should be 1500. GW uses 800.

  //Crypto device initialization
  ATCA_STATUS result = atcab_init(&atcaDevice);
  if (result != static_cast<int>(ATCA_SUCCESS)) {
    ESP_LOGE(LOG_TAG, "Failed to initialize chip! code:%i", static_cast<int>(result));
  }

  return (result == static_cast<int>(ATCA_SUCCESS));
}

void CryptoDriver::DEBUG_testingCode()
{
// Testing shit
  bool provisioned = isProvisioned();
  ESP_LOGI(LOG_TAG, "Read lock_status: %i", static_cast<int>(provisioned));

  uint32_t revision = readChipRevision();
  ESP_LOGI(LOG_TAG, "Read chip revision: 0x%04x", revision);

  // Get chip serial number
  uint8_t buffer[9] = {};
  if (readSerialNumber(buffer)) {
    ESP_LOGI(LOG_TAG, "Read serial number: 0x%02x %02x %02x %02x %02x %02x %02x %02x %02x",
        buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
  }

  bool certOK = readCertificates();
  ESP_LOGI(LOG_TAG, "Read client certificate: %i", static_cast<int>(certOK));

}

bool CryptoDriver::isProvisioned() const
{
  bool cfgLocked = false;
  ATCA_STATUS status = ATCA_GEN_FAIL;

  //Verifiy is config zone is locked
  status = atcab_is_locked(LOCK_ZONE_CONFIG, &cfgLocked);

  if (status != static_cast<int>(ATCA_SUCCESS)) {
    ESP_LOGE(LOG_TAG, "Failed to read lock_status: %i", static_cast<int>(status));
  }
  return cfgLocked;
}

uint32_t CryptoDriver::readChipRevision() const
{
  uint32_t result = UINT32_MAX;
  uint8_t buffer[4];
  ATCA_STATUS status = atcab_info(buffer);
  if (status == static_cast<int>(ATCA_SUCCESS)) {
    result = ((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
  }
  else {
    ESP_LOGE(LOG_TAG, "Failed to read chip revision: %i", static_cast<int>(status));
  }

  return result;
}

bool CryptoDriver::readSerialNumber(uint8_t (&buffer)[9]) const
{
  bool result = false;
  ATCA_STATUS status = ATCA_GEN_FAIL;

  //Verifiy is config zone is locked
  status = atcab_read_serial_number(buffer);
  result = (status == static_cast<int>(ATCA_SUCCESS));

  if (!result) {
    ESP_LOGE(LOG_TAG, "Failed to read serial number: %i", static_cast<int>(status));
  }
  return result;
}


bool CryptoDriver::readCertificates()
{
  atcert_t x509_cert_struct = {};
  bool result = false;

  if (isProvisioned()) {
    if (rebuildCerts(&x509_cert_struct)) {
      if (buildClientTrustedChain(&x509_cert_struct)) {
        result = true;
      }
    }

    freeCerts(&x509_cert_struct);
    ESP_LOGD(LOG_TAG, "Device Certificate: %.*s", m_clientCertificateSize, m_clientCertificate);
    ESP_LOGD(LOG_TAG, "Signer Certificate: %.*s", m_signerCertificateSize, m_signerCertificate);
  } else {
    ESP_LOGE(LOG_TAG, "ECC508A chip is not provisioned");
  }
  return result;
}

const char * CryptoDriver::getClientCertificate() const
{
  return m_clientCertificate;
}

size_t CryptoDriver::getClientCertificateSize() const
{
  return m_clientCertificateSize;
}

const char * CryptoDriver::getSignerCertificate() const
{
  return m_signerCertificate;
}

size_t CryptoDriver::getSignerCertificateSize() const
{
  return m_signerCertificateSize;
}

bool CryptoDriver::rebuildSignerCert(atcert_t* cert)
{
  bool result = false;
  ATCA_STATUS status;
  int retval;

  if ((cert->signer_der != nullptr) && (cert->signer_pem != nullptr)) {

    status = atcatls_get_cert(&g_cert_def_1_signer, nullptr, cert->signer_der, static_cast<size_t *>(&cert->signer_der_size));
    if (status == static_cast<int>(ATCA_SUCCESS)) {

      retval = atcacert_encode_pem_cert(cert->signer_der, cert->signer_der_size, reinterpret_cast<char*>(cert->signer_pem), static_cast<size_t *>(&cert->signer_pem_size));
      if ((retval == static_cast<int>(ATCA_SUCCESS)) && (cert->signer_pem_size > 0)) {

        retval = atcacert_get_subj_public_key(&g_cert_def_1_signer, cert->signer_der, cert->signer_der_size, cert->signer_pubkey);
        if (retval == ATCACERT_E_SUCCESS) {
          result = true;
        }
        else {
          ESP_LOGE(LOG_TAG, "Failed: read signer public key - %i", retval);
        }
      }
      else {
        ESP_LOGE(LOG_TAG, "Failed: convert signer certificate - %i", retval);
      }
    }
    else {
      ESP_LOGE(LOG_TAG, "Failed: read signer certificate - %i", static_cast<int>(status));
    }
  }
  else {
    ESP_LOGE(LOG_TAG, "Failed: invalid signer param");
  }

  return result;
}

bool CryptoDriver::rebuildDeviceCert(atcert_t* cert)
{
  bool result = false;
  ATCA_STATUS status;
  int retval;

  if ((cert->device_der != nullptr) && (cert->device_pem != nullptr)) {

    status = atcatls_get_cert(&g_cert_def_2_device, cert->signer_pubkey, cert->device_der, static_cast<size_t *>(&cert->device_der_size));
    if (status == static_cast<int>(ATCA_SUCCESS)) {

      retval = atcacert_encode_pem_cert(cert->device_der, cert->device_der_size, reinterpret_cast<char*>(cert->device_pem), static_cast<size_t *>(&cert->device_pem_size));
      if ((retval == static_cast<int>(ATCA_SUCCESS)) && (cert->device_pem_size > 0)) {

        retval = atcacert_get_subj_public_key(&g_cert_def_2_device, cert->device_der, cert->device_der_size, cert->device_pubkey);
        if (retval == ATCACERT_E_SUCCESS) {
          result = true;
        }
        else {
          ESP_LOGE(LOG_TAG, "Failed: read device public key - %i", retval);
        }
      }
      else {
        ESP_LOGE(LOG_TAG, "Failed: convert device certificate - %i", retval);
      }
    }
    else {
      ESP_LOGE(LOG_TAG, "Failed: read device certificate - %i", static_cast<int>(status));
    }
  }
  else {
    ESP_LOGE(LOG_TAG, "Failed: invalid device param");
  }
  return result;
}

void CryptoDriver::freeCerts(atcert_t *cert)
{
  if (cert != nullptr)  {
    delete cert->signer_der;
    delete cert->signer_pem;
    delete cert->signer_pubkey;
    delete cert->device_der;
    delete cert->device_pem;
    delete cert->device_pubkey;
    cert->signer_der = nullptr;
    cert->signer_pem = nullptr;
    cert->signer_pubkey = nullptr;
    cert->device_der = nullptr;
    cert->device_pem = nullptr;
    cert->device_pubkey = nullptr;
  }
}

bool CryptoDriver::rebuildCerts(atcert_t *cert)
{
  bool result = false;
  if (cert != nullptr) {

    // Build signer certificate
    cert->signer_der = new uint8_t[Constants::CRYPTO_DER_CERT_INIT_SIZE];
    cert->signer_pem = new uint8_t[Constants::CRYPTO_PEM_CERT_INIT_SIZE];
    cert->signer_pubkey = new uint8_t[ATCA_PUB_KEY_SIZE];

    if ((cert->signer_der != nullptr) && (cert->signer_pem != nullptr) && (cert->signer_pubkey != nullptr)) {
      cert->signer_der_size = Constants::CRYPTO_DER_CERT_INIT_SIZE;
      cert->signer_pem_size = Constants::CRYPTO_PEM_CERT_INIT_SIZE;

      if (rebuildSignerCert(cert)) {

        // Build device certificate
        cert->device_der = new uint8_t[Constants::CRYPTO_DER_CERT_INIT_SIZE];
        cert->device_pem = new uint8_t[Constants::CRYPTO_PEM_CERT_INIT_SIZE];
        cert->device_pubkey = new uint8_t[ATCA_PUB_KEY_SIZE];

        if ((cert->device_der != nullptr) && (cert->device_pem != nullptr) && (cert->device_pubkey != nullptr)) {
          cert->device_der_size = Constants::CRYPTO_DER_CERT_INIT_SIZE;
          cert->device_pem_size = Constants::CRYPTO_PEM_CERT_INIT_SIZE;

          if (rebuildDeviceCert(cert)) {
            result = true;
          }
          else {
            ESP_LOGE(LOG_TAG, "Failed to rebuild device cert");
          }
        }
      }
      else {
        ESP_LOGE(LOG_TAG, "Failed to rebuild signer cert");
      }
    }
  }

  return result;
}

bool CryptoDriver::buildClientTrustedChain(atcert_t *cert)
{
  bool result = false;
  if (cert != nullptr) {
    if ((cert->device_pem_size > 0) && (cert->device_pem_size < (sizeof(m_clientCertificate) + 1))) {
      memcpy(m_clientCertificate, cert->device_pem, cert->device_pem_size);
      m_clientCertificateSize = cert->device_pem_size;
      // Adding zero-termination
      m_clientCertificate[m_clientCertificateSize] = '\0';
      m_clientCertificateSize += 1;
      result = true;
    }
    if ((cert->signer_pem_size > 0) && (cert->signer_pem_size < (sizeof(m_signerCertificate) + 1))) {
      memcpy(m_signerCertificate, cert->signer_pem, cert->signer_pem_size);
      m_signerCertificateSize = cert->signer_pem_size;
      // Adding zero-termination
      m_signerCertificate[m_signerCertificateSize] = '\0';
      m_signerCertificateSize += 1;
    }
  }
  return result;
}
