/*
 * OVERRIDES FOR USE WITH ATECC508A CHIP
 */

#include <cstring>

#include "esp_log.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/ecdh.h"
#include "mbedtls/ecdsa.h"

extern "C" {
  #include "cryptoauthlib.h"
  #include "tls/atcatls.h"
  #include "tls/atcatls_cfg.h"
}

namespace {
  const uint8_t HALF_ATCA_PUB_KEY_SIZE = ATCA_PUB_KEY_SIZE/2;
  const uint8_t HALF_ATCA_SIG_SIZE = ATCA_SIG_SIZE/2;

  const char * LOG_TAG = "ATECC";
}

#if defined(MBEDTLS_ECDSA_GENKEY_ALT)
#pragma message("Using alternate mbedtls_ecdsa_genkey")
// NOTE: Don't think we need this one
/*
 * Generate key pair
 */
int mbedtls_ecdsa_genkey( mbedtls_ecdsa_context *ctx, mbedtls_ecp_group_id gid,
                  int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
  ESP_LOGD(LOG_TAG, "ecdsa_genkey");
    return( mbedtls_ecp_group_load( &ctx->grp, gid ) ||
        mbedtls_ecdh_gen_public( &ctx->grp, &ctx->d, &ctx->Q, f_rng, p_rng ) );
}
#endif

#if defined(MBEDTLS_ECDH_GEN_PUBLIC_ALT)
#pragma message("Using alternate mbedtls_ecdh_gen_public")
/*
 * Generate public key: simple wrapper around mbedtls_ecp_gen_keypair
 */
int mbedtls_ecdh_gen_public( mbedtls_ecp_group *grp, mbedtls_mpi *d, mbedtls_ecp_point *Q,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng )
{
  int result = -1;
  uint8_t pubkey[ATCA_PUB_KEY_SIZE];

  ESP_LOGD(LOG_TAG, "ECC508A ECDH gen pub");

  // Check supported curve
  if ((grp->id == MBEDTLS_ECP_DP_SECP256R1) && (grp->pbits == 256)) {
    /* Create a unique public-private key pair in the specified slot. */
    // FIXME: This could potentially cause a problem if multiple ECDH calls in parallel.
    //        May have to cycle use of multiple key slots.
    result = atcatls_create_key(TLS_SLOT_ECDH_PRIV, pubkey);

    // TODO: Just testing this 2nd attempt. Maybe skip this or at least add a short delay before trying again
    if (result != static_cast<int>(ATCA_SUCCESS))
    {
      result = atcatls_create_key(TLS_SLOT_ECDH_PRIV, pubkey);
    }

    if (result == static_cast<int>(ATCA_SUCCESS)) {
      //Convert the x-coordinate of our public key to an octet string
      result = mbedtls_mpi_read_binary(&Q->X, static_cast<const uint8_t *>(pubkey), HALF_ATCA_PUB_KEY_SIZE);

      if (result == 0) {
        //Convert the y-coordinate of our public key to an octet string
        result = mbedtls_mpi_read_binary(&Q->Y, static_cast<const uint8_t *>(&pubkey[HALF_ATCA_PUB_KEY_SIZE]), HALF_ATCA_PUB_KEY_SIZE);

      }
      if (result == 0) {
        // for some reason this must be set to 1. otherwise q is not accepted as valid pubkey
        result = mbedtls_mpi_lset(&Q->Z, 1);
      }
      if (result == 0) {
        // Dummy private key (only to signal proper generation to compute_shared)
        result = mbedtls_mpi_lset(d, TLS_SLOT_ECDH_PRIV);
      }
    }
  }
  else {
    //The specified elliptic curve is not supported
    result = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    // Do we need a fallback here?
    // return mbedtls_ecp_gen_keypair( grp, d, Q, f_rng, p_rng );
  }

  ESP_LOGD(LOG_TAG, "ECC508A exit with code: %d", result);
  return result;
}
#endif

#if defined(MBEDTLS_ECDH_COMPUTE_SHARED_ALT)
#pragma message("Using alternate mbedtls_ecdh_compute_shared")
/*
 * Compute shared secret (SEC1 3.3.1)
 */
int mbedtls_ecdh_compute_shared( mbedtls_ecp_group *grp, mbedtls_mpi *z,
                         const mbedtls_ecp_point *Q, const mbedtls_mpi *d,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng )
{
  int result = -1;
  uint8_t pubkey[ATCA_PUB_KEY_SIZE];
  uint8_t secret[ATCA_KEY_SIZE];
  ATCA_STATUS status;

  /* Make sure Q is a valid pubkey before using it */
  result = mbedtls_ecp_check_pubkey(grp, Q);

  if (result == 0) {
    ESP_LOGD(LOG_TAG, "ECC508A ECDH shared");

    // Check supported curve
    if ((grp->id == MBEDTLS_ECP_DP_SECP256R1) && (grp->pbits == 256)) {

      //Convert the x-coordinate of the peer's public key to an octet string
      result = mbedtls_mpi_write_binary(&Q->X, static_cast<uint8_t *>(pubkey), HALF_ATCA_PUB_KEY_SIZE);

      if (result == 0) {
        //Convert the y-coordinate of the peer's public key to an octet string
        result = mbedtls_mpi_write_binary(&Q->Y, static_cast<uint8_t *>(&pubkey[HALF_ATCA_PUB_KEY_SIZE]), HALF_ATCA_PUB_KEY_SIZE);
      }

      if (result == 0) {
        // Expects mbedtls_ecdh_gen_public to have been called before!
        if ((mbedtls_mpi_size(d) == 1) && (mbedtls_mpi_cmp_int(d, TLS_SLOT_ECDH_PRIV) == 0)) {
          /* ECDH command with premaster secret returned in the response. */
          status = atcab_ecdh(TLS_SLOT_ECDH_PRIV, pubkey, secret);
          ESP_LOGD(LOG_TAG, "atcab_ecdh = %i", static_cast<int>(status));

          //Successful operation?
          if (status == static_cast<int>(ATCA_SUCCESS)) {
            result = mbedtls_mpi_read_binary(z, static_cast<const uint8_t *>(secret), ATCA_KEY_SIZE);
          }
          else {
            result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
          }
        }
        else {
          result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
          ESP_LOGE(LOG_TAG, "gen_public not valid!");
        }
      }
    }
    else {
      //The specified elliptic curve is not supported
      result = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }
  }

  ESP_LOGD(LOG_TAG, "ECC508A exit with code: %d", result);
  return result;
}
#endif


/* ecdsa.c overrides */

#if defined(MBEDTLS_ECDSA_SIGN_ALT)
#pragma message("Using alternate mbedtls_ecdsa_sign")
/*
 * Compute ECDSA signature of a hashed message (SEC1 4.1.3)
 * Obviously, compared to SEC1 4.1.3, we skip step 4 (hash message)
 */
int mbedtls_ecdsa_sign( mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s,
                const mbedtls_mpi *d, const unsigned char *buf, size_t blen,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
  int result = 0;
  ATCA_STATUS status;
  uint8_t signature[ATCA_SIG_SIZE];

  // Error checks from original function:
  /* Fail cleanly on curves such as Curve25519 that can't be used for ECDSA */
  if( grp->N.p == NULL ) {
    result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
  }

  /* Make sure d is in range 1..n-1 */
  if (result == 0) {
    if( mbedtls_mpi_cmp_int( d, 1 ) < 0 || mbedtls_mpi_cmp_mpi( d, &grp->N ) >= 0 ) {
      result = MBEDTLS_ERR_ECP_INVALID_KEY;
    }
  }

  if (result == 0) {
    ESP_LOGD(LOG_TAG, "ECC508A ECDSA signature generation");

    //Correct curve and SHA-256 hash algorithm (length)?
    if ((grp->id == MBEDTLS_ECP_DP_SECP256R1) && (blen == 32u)) {
      //Generate an ECDSA signature using the client's private key
      status = atcatls_sign(TLS_SLOT_AUTH_PRIV, static_cast<const uint8_t *>(buf), signature);
      ESP_LOGD(LOG_TAG, "atcatls_sign = %i", result);

      //Successful operation?
      if(status == static_cast<int>(ATCA_SUCCESS)) {
        //Convert R to a multiple precision integer
        result = mbedtls_mpi_read_binary(r, signature, HALF_ATCA_SIG_SIZE);

        if(result == 0) {
          //Convert S to a multiple precision integer
          result = mbedtls_mpi_read_binary(s, &signature[HALF_ATCA_SIG_SIZE], (HALF_ATCA_SIG_SIZE));
        }
      }
      else {
        //Failed to generate ECDSA signature
        result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
      }
    }
    else {
      //The specified hash algorithm is not supported
      result = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }
  }

  ESP_LOGD(LOG_TAG, "ECC508A exit with code: %i", result);
  return result;
}
#endif


#if defined(MBEDTLS_ECDSA_VERIFY_ALT)
#pragma message("Using alternate mbedtls_ecdsa_verify")
/*
 * Verify ECDSA signature of hashed message (SEC1 4.1.4)
 * Obviously, compared to SEC1 4.1.3, we skip step 2 (hash message)
 */
int mbedtls_ecdsa_verify( mbedtls_ecp_group *grp,
                  const unsigned char *buf, size_t blen,
                  const mbedtls_ecp_point *Q, const mbedtls_mpi *r, const mbedtls_mpi *s)
{
  int result = 0;
  ATCA_STATUS status;
  uint8_t signature[ATCA_SIG_SIZE];
  uint8_t pubKey[ATCA_PUB_KEY_SIZE];
  bool verified;

  // Error checks from original function:
  /* Fail cleanly on curves such as Curve25519 that can't be used for ECDSA */
  if( grp->N.p == NULL ) {
    result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
  }

  /* Step 1: make sure r and s are in range 1..n-1 */
  if (result == 0) {
    if( mbedtls_mpi_cmp_int( r, 1 ) < 0 || mbedtls_mpi_cmp_mpi( r, &grp->N ) >= 0 ||
        mbedtls_mpi_cmp_int( s, 1 ) < 0 || mbedtls_mpi_cmp_mpi( s, &grp->N ) >= 0 ) {
      result = MBEDTLS_ERR_ECP_VERIFY_FAILED;
    }
  }

  /* Additional precaution: make sure Q is valid */
  if (result == 0) {
    result = mbedtls_ecp_check_pubkey( grp, Q );
  }

  if (result == 0) {
    ESP_LOGD(LOG_TAG, "ECC508A ECDSA signature verification");

    //NIST-P256 elliptic curve?
    if ((grp->id == MBEDTLS_ECP_DP_SECP256R1) && (grp->pbits == 256)) {
      // Correct hash length? (sha256)
      if (blen == 32u) {
        //Convert R to an octet string
        result = mbedtls_mpi_write_binary(r, signature, HALF_ATCA_SIG_SIZE);

        if (result == 0) {
          //Convert S to an octet string
          result = mbedtls_mpi_write_binary(s, &signature[HALF_ATCA_SIG_SIZE], HALF_ATCA_SIG_SIZE);
        }

        if (result == 0) {
          //Convert the x-coordinate of the public key to an octet string
          result = mbedtls_mpi_write_binary(&Q->X, pubKey, HALF_ATCA_PUB_KEY_SIZE);
        }

        if (result == 0) {
          //Convert the y-coordinate of the public key to an octet string
          result = mbedtls_mpi_write_binary(&Q->Y, &pubKey[HALF_ATCA_PUB_KEY_SIZE], HALF_ATCA_PUB_KEY_SIZE);
        }

        //Check status code
        if (result == 0) {
          //Verify the ECDSA signature
          status = atcatls_verify(static_cast<const uint8_t *>(buf), signature, pubKey, &verified);
          ESP_LOGD(LOG_TAG, "atcatls_verify = %i,%i", status, static_cast<int>(verified));

          //Failed to verify ECDSA signature?
          if ((status != ATCA_SUCCESS) || (!verified)) {
            //Report an error
            result = MBEDTLS_ERR_ECP_VERIFY_FAILED;
          }
        }
      }
      else {
        //The specified hash algorithm is not supported
        result = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
      }
    }
    else {
      //The specified elliptic curve is not supported
      result = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }
  }

  ESP_LOGD(LOG_TAG, "ECC508A exit with code: %i", result);
  return result;
}
#endif
