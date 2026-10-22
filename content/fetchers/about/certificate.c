/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf.
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * content generator for the about scheme certificate page
 */

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/ssl_certs.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "certificate.h"

/**
 * certificate name parameters
 */
ರಚನೆ ns_cert_name {
	ಅಕ್ಷರ *common_name;
	ಅಕ್ಷರ *organisation;
	ಅಕ್ಷರ *organisation_unit;
	ಅಕ್ಷರ *locality;
	ಅಕ್ಷರ *province;
	ಅಕ್ಷರ *country;
};

/**
 * Certificate public key parameters
 */
ರಚನೆ ns_cert_pkey {
	ಅಕ್ಷರ *algor;
	ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
	ಅಕ್ಷರ *modulus;
	ಅಕ್ಷರ *exponent;
	ಅಕ್ಷರ *ವಕ್ರ;
	ಅಕ್ಷರ *ಸಾರ್ವಜನಿಕ;
};

/**
 * Certificate subject alternative name
 */
ರಚನೆ ns_cert_san {
	ರಚನೆ ns_cert_san *ಮುಂದಿನ;
	ಅಕ್ಷರ *name;
};

/**
 * certificate information for certificate chain
 */
ರಚನೆ ns_cert_info {
	ರಚನೆ ns_cert_name subject_name; /**< Subject details */
	ರಚನೆ ns_cert_name issuer_name; /**< Issuer details */
	ರಚನೆ ns_cert_pkey ಬಹಿರಂಗ_ಕೀಲಿ; /**< public key details */
	ದೀರ್ಘ ಆವೃತ್ತಿ;		/**< Certificate version */
	ಅಕ್ಷರ *not_before;	/**< Valid from date */
	ಅಕ್ಷರ *not_after;	/**< Valid to date */
	ಪೂರ್ಣಾಂಕ sig_type;		/**< Signature type */
	ಅಕ್ಷರ *sig_algor;        /**< Signature Algorithm */
	ಅಕ್ಷರ *serialnum;	/**< Serial number */
	ಅಕ್ಷರ *sha1fingerprint; /**< fingerprint shar1 encoded */
	ಅಕ್ಷರ *sha256fingerprint; /**< fingerprint shar256 encoded */
	ರಚನೆ ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
ಸ್ಥಿರ nserror free_ns_cert_info(ರಚನೆ ns_cert_info *cinfo)
{
	ರಚನೆ ns_cert_san *san;

	ಬಿಡುಗಡೆ(cinfo->subject_name.common_name);
	ಬಿಡುಗಡೆ(cinfo->subject_name.organisation);
	ಬಿಡುಗಡೆ(cinfo->subject_name.organisation_unit);
	ಬಿಡುಗಡೆ(cinfo->subject_name.locality);
	ಬಿಡುಗಡೆ(cinfo->subject_name.province);
	ಬಿಡುಗಡೆ(cinfo->subject_name.country);
	ಬಿಡುಗಡೆ(cinfo->issuer_name.common_name);
	ಬಿಡುಗಡೆ(cinfo->issuer_name.organisation);
	ಬಿಡುಗಡೆ(cinfo->issuer_name.organisation_unit);
	ಬಿಡುಗಡೆ(cinfo->issuer_name.locality);
	ಬಿಡುಗಡೆ(cinfo->issuer_name.province);
	ಬಿಡುಗಡೆ(cinfo->issuer_name.country);
	ಬಿಡುಗಡೆ(cinfo->ಬಹಿರಂಗ_ಕೀಲಿ.algor);
	ಬಿಡುಗಡೆ(cinfo->ಬಹಿರಂಗ_ಕೀಲಿ.modulus);
	ಬಿಡುಗಡೆ(cinfo->ಬಹಿರಂಗ_ಕೀಲಿ.exponent);
	ಬಿಡುಗಡೆ(cinfo->ಬಹಿರಂಗ_ಕೀಲಿ.ವಕ್ರ);
	ಬಿಡುಗಡೆ(cinfo->ಬಹಿರಂಗ_ಕೀಲಿ.ಸಾರ್ವಜನಿಕ);
	ಬಿಡುಗಡೆ(cinfo->not_before);
	ಬಿಡುಗಡೆ(cinfo->not_after);
	ಬಿಡುಗಡೆ(cinfo->sig_algor);
	ಬಿಡುಗಡೆ(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	ಆಗ (san != ಖಾಲಿ) {
		ರಚನೆ ns_cert_san *ಮುಂದಿನ;
		ಮುಂದಿನ = san->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(san);
		san = ಮುಂದಿನ;
	}

	ಬಿಡುಗಡೆ(cinfo);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_OPENSSL

#ಅನ್ತರ್ಭಾವ <openssl/ssl.h>
#ಅನ್ತರ್ಭಾವ <openssl/x509v3.h>

#ಆದರೆ (OPENSSL_ಆವೃತ್ತಿ_ಸಂಖ್ಯೆ < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# ಆದರೆ defined(LIBRESSL_ಆವೃತ್ತಿ_ಸಂಖ್ಯೆ)
  /* LibreSSL */
#  ಆದರೆ (LIBRESSL_ಆವೃತ್ತಿ_ಸಂಖ್ಯೆ < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   ಆದರೆ (LIBRESSL_ಆವೃತ್ತಿ_ಸಂಖ್ಯೆ < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ns_X509_get_signature_nid(X509 *cert)
{
	ಹಿಂತಿರುಗಿ OBJ_obj2nid(cert->cert_info->ಕೀಲಿ->algor->algorithm);
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ns_ASN1_STRING_get0_ದತ್ತಾಂಶ(ASN1_STRING *asn1str)
{
	ಹಿಂತಿರುಗಿ (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ASN1_STRING_ದತ್ತಾಂಶ(asn1str);
}
#   ಇಲ್ಲದಿದ್ದರೆ
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_ದತ್ತಾಂಶ ASN1_STRING_get0_ದತ್ತಾಂಶ
#   endif

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ BIGNUM *ns_RSA_get0_n(ಸ್ಥಿರಾಂಕ RSA *d)
{
	ಹಿಂತಿರುಗಿ d->n;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ BIGNUM *ns_RSA_get0_e(ಸ್ಥಿರಾಂಕ RSA *d)
{
	ಹಿಂತಿರುಗಿ d->e;
}
#  ಇಲ್ಲದಿದ್ದರೆ
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_ದತ್ತಾಂಶ ASN1_STRING_get0_ದತ್ತಾಂಶ
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# ಇಲ್ಲದಿದ್ದರೆ
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_ದತ್ತಾಂಶ ASN1_STRING_get0_ದತ್ತಾಂಶ
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ns_EVP_PKEY_get_bn_param(ಸ್ಥಿರಾಂಕ EVP_PKEY *pkey,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *ಫಲಿತಾಂಶ = ಖಾಲಿ;

	/* Check parameters: only support allocation-form *bn */
	ಆದರೆ (pkey == ಖಾಲಿ || ಕೀಲಿ_name == ಖಾಲಿ || bn == ಖಾಲಿ || *bn != ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	/* Only support RSA keys */
	ಆದರೆ (EVP_PKEY_ತಳ_id(pkey) != EVP_PKEY_RSA)
		ಹಿಂತಿರುಗಿ 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	ಆದರೆ (rsa == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (strcmp(ಕೀಲಿ_name, "n") == 0) {
		ಸ್ಥಿರಾಂಕ BIGNUM *n = ns_RSA_get0_n(rsa);
		ಆದರೆ (n != ಖಾಲಿ)
			ಫಲಿತಾಂಶ = BN_dup(n);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ಕೀಲಿ_name, "e") == 0) {
		ಸ್ಥಿರಾಂಕ BIGNUM *e = ns_RSA_get0_e(rsa);
		ಆದರೆ (e != ಖಾಲಿ)
			ಫಲಿತಾಂಶ = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = ಫಲಿತಾಂಶ;

	ಹಿಂತಿರುಗಿ (ಫಲಿತಾಂಶ != ಖಾಲಿ) ? 1 : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ns_EVP_PKEY_get_utf8_string_param(ಸ್ಥಿರಾಂಕ EVP_PKEY *pkey,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ_name, ಅಕ್ಷರ *str, size_t max_len,
		size_t *ಹೊರಹೋಗು_len)
{
	ಸ್ಥಿರಾಂಕ EC_ಗುಂಪು *ecgroup;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುಂಪು = "";
	size_t ಗುಂಪು_len = 0;
	EC_ಕೀಲಿ *ec;
	ಪೂರ್ಣಾಂಕ ret = 0;

	ಆದರೆ (pkey == ಖಾಲಿ || ಕೀಲಿ_name == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	/* Only support EC keys */
	ಆದರೆ (EVP_PKEY_ತಳ_id(pkey) != EVP_PKEY_EC)
		ಹಿಂತಿರುಗಿ 0;

	/* Only support fetching the group */
	ಆದರೆ (strcmp(ಕೀಲಿ_name, "group") != 0)
		ಹಿಂತಿರುಗಿ 0;

	ec = EVP_PKEY_get1_EC_ಕೀಲಿ((EVP_PKEY *) pkey);

	ecgroup = EC_ಕೀಲಿ_get0_ಗುಂಪು(ec);
	ಆದರೆ (ecgroup != ಖಾಲಿ) {
		ಗುಂಪು = OBJ_nid2ln(EC_ಗುಂಪು_get_ವಕ್ರ_name(ecgroup));
		ಗುಂಪು_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಗುಂಪು);
	}

	ಆದರೆ (str != ಖಾಲಿ && max_len > ಗುಂಪು_len) {
		ಸ್ಮರಣೆ_ನಕಲು(str, ಗುಂಪು, ಗುಂಪು_len);
		str[ಗುಂಪು_len] = '\0';
		ret = 1;
	}

	ಆದರೆ (ಹೊರಹೋಗು_len != ಖಾಲಿ) {
		*ಹೊರಹೋಗು_len = ಗುಂಪು_len;
	}

	EC_ಕೀಲಿ_free(ec);

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ns_EVP_PKEY_get_octet_string_param(ಸ್ಥಿರಾಂಕ EVP_PKEY *pkey,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ_name, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, size_t max_len,
		size_t *ಹೊರಹೋಗು_len)
{
	ಸ್ಥಿರಾಂಕ EC_ಗುಂಪು *ecgroup;
	ಸ್ಥಿರಾಂಕ EC_ಬಿಂದು *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_ಕೀಲಿ *ec;
	ಪೂರ್ಣಾಂಕ ret = 0;

	ಆದರೆ (pkey == ಖಾಲಿ || ಕೀಲಿ_name == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	/* Only support EC keys */
	ಆದರೆ (EVP_PKEY_ತಳ_id(pkey) != EVP_PKEY_EC)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (strcmp(ಕೀಲಿ_name, "encoded-pub-key") != 0)
		ಹಿಂತಿರುಗಿ 0;

	ec = EVP_PKEY_get1_EC_ಕೀಲಿ((EVP_PKEY *) pkey);
	ಆದರೆ (ec == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	ecgroup = EC_ಕೀಲಿ_get0_ಗುಂಪು(ec);
	ಆದರೆ (ecgroup != ಖಾಲಿ) {
		ecpoint = EC_ಕೀಲಿ_get0_ಬಹಿರಂಗ_ಕೀಲಿ(ec);
		ಆದರೆ (ecpoint != ಖಾಲಿ) {
			bnctx = BN_CTX_new();
			len = EC_ಬಿಂದು_point2oct(ecgroup,
						 ecpoint,
						 ಬಿಂದು_CONVERSION_UNCOMPRESSED,
						 ಖಾಲಿ,
						 0,
						 bnctx);
			ಆದರೆ (len != 0 && len <= max_len) {
				ಆದರೆ (EC_ಬಿಂದು_point2oct(ecgroup,
						       ecpoint,
						       ಬಿಂದು_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			ಆದರೆ (ಹೊರಹೋಗು_len != ಖಾಲಿ)
				*ಹೊರಹೋಗು_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_ಕೀಲಿ_free(ec);

	ಹಿಂತಿರುಗಿ ret;
}
#ಇಲ್ಲದಿದ್ದರೆ
/* OpenSSL 3.x and later */
#ವ್ಯಾಖ್ಯಾನಿಸು ns_X509_get_signature_nid X509_get_signature_nid
#ವ್ಯಾಖ್ಯಾನಿಸು ns_ASN1_STRING_get0_ದತ್ತಾಂಶ ASN1_STRING_get0_ದತ್ತಾಂಶ
#ವ್ಯಾಖ್ಯಾನಿಸು ns_RSA_get0_n RSA_get0_n
#ವ್ಯಾಖ್ಯಾನಿಸು ns_RSA_get0_e RSA_get0_e
#ವ್ಯಾಖ್ಯಾನಿಸು ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#ವ್ಯಾಖ್ಯಾನಿಸು ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#ವ್ಯಾಖ್ಯಾನಿಸು ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#ಆದರೆ_ಅಂತ್ಯ

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
ಸ್ಥಿರ nserror
xname_ಗಮ್ಯ_info(X509_NAME *xname, ರಚನೆ ns_cert_name *iname)
{
	ಪೂರ್ಣಾಂಕ entryidx;
	ಪೂರ್ಣಾಂಕ entrycnt;
	X509_NAME_ನಮೂದು *ನಮೂದು; /* current name entry */
	ASN1_STRING *ಮೌಲ್ಯ;
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮೌಲ್ಯ_str;
	ASN1_OBJECT *name;
	ಪೂರ್ಣಾಂಕ name_nid;
	ಅಕ್ಷರ **ಕ್ಷೇತ್ರ;

	entrycnt = X509_NAME_ನಮೂದು_ಎಣಿಕೆ(xname);

	ಗಾಗಿ (entryidx = 0; entryidx < entrycnt; entryidx++) {
		ನಮೂದು = X509_NAME_get_ನಮೂದು(xname, entryidx);
		name = X509_NAME_ನಮೂದು_get_object(ನಮೂದು);
		name_nid = OBJ_obj2nid(name);
		ಮೌಲ್ಯ = X509_NAME_ನಮೂದು_get_ದತ್ತಾಂಶ(ನಮೂದು);
		ಮೌಲ್ಯ_str = ns_ASN1_STRING_get0_ದತ್ತಾಂಶ(ಮೌಲ್ಯ);
		ಆಯ್ಕೆ (name_nid) {
		ಸ್ಥಿತಿ NID_commonName:
			ಕ್ಷೇತ್ರ = &iname->common_name;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NID_countryName:
			ಕ್ಷೇತ್ರ = &iname->country;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NID_localityName:
			ಕ್ಷೇತ್ರ = &iname->locality;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NID_stateOrProvinceName:
			ಕ್ಷೇತ್ರ = &iname->province;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NID_organizationName:
			ಕ್ಷೇತ್ರ = &iname->organisation;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NID_organizationalUnitName:
			ಕ್ಷೇತ್ರ = &iname->organisation_unit;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ :
			ಕ್ಷೇತ್ರ = ಖಾಲಿ;
			ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (ಕ್ಷೇತ್ರ != ಖಾಲಿ) {
			*ಕ್ಷೇತ್ರ = strdup((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ಮೌಲ್ಯ_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *ಕ್ಷೇತ್ರ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	ಆದರೆ (iname->common_name == ಖಾಲಿ) {
		iname->common_name = strdup("Unknown");
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *hexdup(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hex)
{
	ಪೂರ್ಣಾಂಕ hexlen;
	ಅಕ್ಷರ *dst;
	ಅಕ್ಷರ *ಹೊರಹೋಗು;
	ಪೂರ್ಣಾಂಕ cn = 0;

	hexlen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = ಸ್ಮರಣೆ_ಕೊಡು(((hexlen * 7) + 6) / 2);

	ಆದರೆ (dst != ಖಾಲಿ) {
		ಗಾಗಿ (ಹೊರಹೋಗು = dst; *hex != 0; hex++) {
			ಆದರೆ (cn == 2) {
				cn = 0;
				*ಹೊರಹೋಗು++ = '&';
				*ಹೊರಹೋಗು++ = '#';
				*ಹೊರಹೋಗು++ = '5';
				*ಹೊರಹೋಗು++ = '8';
				*ಹೊರಹೋಗು++ = ';';
			}
			*ಹೊರಹೋಗು++ = *hex;
			cn++;
		}
		*ಹೊರಹೋಗು = 0;
	}
	ಹಿಂತಿರುಗಿ dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *bindup(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *bin, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ binlen)
{
	ಅಕ್ಷರ *dst;
	ಅಕ್ಷರ *ಹೊರಹೋಗು;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ idx;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = ಸ್ಮರಣೆ_ಕೊಡು(binlen * 7);

	ಆದರೆ (dst != ಖಾಲಿ) {
		ಹೊರಹೋಗು = dst;
		ಗಾಗಿ (idx = 0; idx < binlen; idx++) {
			*ಹೊರಹೋಗು++ = hex[(bin[idx] & 0xf0) >> 4];
			*ಹೊರಹೋಗು++ = hex[bin[idx] & 0xf];

			*ಹೊರಹೋಗು++ = '&';
			*ಹೊರಹೋಗು++ = '#';
			*ಹೊರಹೋಗು++ = '5';
			*ಹೊರಹೋಗು++ = '8';
			*ಹೊರಹೋಗು++ = ';';
		}
		ಹೊರಹೋಗು -= 5;
		*ಹೊರಹೋಗು = 0;
	}
	ಹಿಂತಿರುಗಿ dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
rsa_ಗಮ್ಯ_info(EVP_PKEY *pkey, ರಚನೆ ns_cert_pkey *ikey)
{
	BIGNUM *n = ಖಾಲಿ, *e = ಖಾಲಿ;
	ಅಕ್ಷರ *tmp;

	ಆದರೆ (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}

	ಆದರೆ (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->ಗಾತ್ರ_2 = EVP_PKEY_ಬಿಟ್ಟುಗಳು(pkey);

	tmp = BN_bn2hex(n);
	ಆದರೆ (tmp != ಖಾಲಿ) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	ಆದರೆ (tmp != ಖಾಲಿ) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
dsa_ಗಮ್ಯ_info(EVP_PKEY *pkey, ರಚನೆ ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->ಗಾತ್ರ_2 = EVP_PKEY_ಬಿಟ್ಟುಗಳು(pkey);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
dh_ಗಮ್ಯ_info(EVP_PKEY *pkey, ರಚನೆ ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->ಗಾತ್ರ_2 = EVP_PKEY_ಬಿಟ್ಟುಗಳು(pkey);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
ec_ಗಮ್ಯ_info(EVP_PKEY *pkey, ರಚನೆ ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->ಗಾತ್ರ_2 = EVP_PKEY_ಬಿಟ್ಟುಗಳು(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ಖಾಲಿ, 0, &len);
	ಆದರೆ (len != 0) {
		ikey->ವಕ್ರ = ಸ್ಮರಣೆ_ಕೊಡು(len + 1);
		ಆದರೆ (ikey->ವಕ್ರ != ಖಾಲಿ) {
			ಆದರೆ (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->ವಕ್ರ, len + 1, ಖಾಲಿ) == 0) {
				ಬಿಡುಗಡೆ(ikey->ವಕ್ರ);
				ikey->ವಕ್ರ = ಖಾಲಿ;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ಖಾಲಿ, 0, &len);
	ಆದರೆ (len != 0) {
		ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಬಿಂದು = ಸ್ಮರಣೆ_ಕೊಡು(len);
		ಆದರೆ (ಬಿಂದು != ಖಾಲಿ) {
			ಆದರೆ (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", ಬಿಂದು, len,
					ಖಾಲಿ) == 1) {
				ikey->ಸಾರ್ವಜನಿಕ = bindup(ಬಿಂದು, len);
			}
			ಬಿಡುಗಡೆ(ಬಿಂದು);
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
pkey_ಗಮ್ಯ_info(EVP_PKEY *pkey, ರಚನೆ ns_cert_pkey *ikey)
{
	nserror res;

	ಆದರೆ (pkey == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}

	ಆಯ್ಕೆ (EVP_PKEY_ತಳ_id(pkey)) {
	ಸ್ಥಿತಿ EVP_PKEY_RSA:
		res = rsa_ಗಮ್ಯ_info(pkey, ikey);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ EVP_PKEY_DSA:
		res = dsa_ಗಮ್ಯ_info(pkey, ikey);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ EVP_PKEY_DH:
		res = dh_ಗಮ್ಯ_info(pkey, ikey);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ EVP_PKEY_EC:
		res = ec_ಗಮ್ಯ_info(pkey, ikey);
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		res = NSERROR_NOT_IMPLEMENTED;
		ನಿಲ್ಲಿಸು;
	}

	EVP_PKEY_free(pkey);

	ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ nserror san_ಗಮ್ಯ_info(X509 *cert, ರಚನೆ ns_cert_san **prev_ಮುಂದಿನ)
{
	ಪೂರ್ಣಾಂಕ idx;
	ಪೂರ್ಣಾಂಕ san_ಹೆಸರುಗಳು_nb = -1;
	ಸ್ಥಿರಾಂಕ GENERAL_NAME *current_name;
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *dns_name;
	ರಚನೆ ns_cert_san *isan;

	ರಾಶಿ_ದ(GENERAL_NAME) *san_ಹೆಸರುಗಳು = ಖಾಲಿ;

	san_ಹೆಸರುಗಳು = X509_get_ext_d2i(cert, NID_subject_alt_name, ಖಾಲಿ, ಖಾಲಿ);
	ಆದರೆ (san_ಹೆಸರುಗಳು == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	san_ಹೆಸರುಗಳು_nb = sk_GENERAL_NAME_ಅಂಕೆ(san_ಹೆಸರುಗಳು);

	/* Check each name within the extension */
	ಗಾಗಿ (idx = 0; idx < san_ಹೆಸರುಗಳು_nb; idx++) {
		current_name = sk_GENERAL_NAME_ಮೌಲ್ಯ(san_ಹೆಸರುಗಳು, idx);

		ಆದರೆ (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_ದತ್ತಾಂಶ(current_name->d.dNSName);

			isan = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ ns_cert_san));
			ಆದರೆ (isan != ಖಾಲಿ) {
				isan->name = strdup((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dns_name);
				isan->ಮುಂದಿನ = ಖಾಲಿ;
				*prev_ಮುಂದಿನ = isan;
				prev_ಮುಂದಿನ = &isan->ಮುಂದಿನ;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	ಗಾಗಿ (idx = 0; idx < san_ಹೆಸರುಗಳು_nb; idx++) {
		GENERAL_NAME *ನಮೂದು = sk_GENERAL_NAME_ತೆಗೆ(san_ಹೆಸರುಗಳು);
		GENERAL_NAME_free(ನಮೂದು);
	}
	sk_GENERAL_NAME_free(san_ಹೆಸರುಗಳು);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror
der_ಗಮ್ಯ_certinfo(ಸ್ಥಿರಾಂಕ uint8_t *der,
		size_t der_ಉದ್ದಳತೆ,
		ರಚನೆ ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ಸ್ಥಿರಾಂಕ ASN1_INTEGER *asn1_ಅಂಕೆ;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	ಆದರೆ (der == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	cert = d2i_X509(ಖಾಲಿ, &der, der_ಉದ್ದಳತೆ);
	ಆದರೆ (cert == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->ಆವೃತ್ತಿ = X509_get_ಆವೃತ್ತಿ(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_ಸಮಯ_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ಶೂನ್ಯ) BIO_ಗಣ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, buf->ಉದ್ದಳತೆ + 1);
	ಆದರೆ (info->not_before != ಖಾಲಿ) {
		ಸ್ಮರಣೆ_ನಕಲು(info->not_before, buf->ದತ್ತಾಂಶ, (ಚಿಹ್ನೆರಹಿತ)buf->ಉದ್ದಳತೆ);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_ಸಮಯ_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ಶೂನ್ಯ) BIO_ಗಣ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, buf->ಉದ್ದಳತೆ + 1);
	ಆದರೆ (info->not_after != ಖಾಲಿ) {
		ಸ್ಮರಣೆ_ನಕಲು(info->not_after, buf->ದತ್ತಾಂಶ, (ಚಿಹ್ನೆರಹಿತ)buf->ಉದ್ದಳತೆ);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	ಪೂರ್ಣಾಂಕ pkey_nid = ns_X509_get_signature_nid(cert);
	ಆದರೆ (pkey_nid != NID_undef) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* sslbuf = OBJ_nid2ln(pkey_nid);
		ಆದರೆ (sslbuf != ಖಾಲಿ) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_ಅಂಕೆ = X509_get_serial_ಸಂಖ್ಯೆ(cert);
	ಆದರೆ (asn1_ಅಂಕೆ != ಖಾಲಿ) {
		bignum = ASN1_INTEGER_ಗಮ್ಯ_BN(asn1_ಅಂಕೆ, ಖಾಲಿ);
		ಆದರೆ (bignum != ಖಾಲಿ) {
			ಅಕ್ಷರ *tmp = BN_bn2hex(bignum);
			ಆದರೆ (tmp != ಖಾಲಿ) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ಖಾಲಿ;
		}
	}

	/* fingerprints */
	ಸ್ಥಿರಾಂಕ EVP_MD *digest;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ dig_len;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buff;
	ಪೂರ್ಣಾಂಕ rc;

	digest = EVP_sha1();
	buff = ಸ್ಮರಣೆ_ಕೊಡು(EVP_MD_ಗಾತ್ರ(digest));
	ಆದರೆ (buff != ಖಾಲಿ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ಆದರೆ ((rc == 1) && (dig_len == (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)EVP_MD_ಗಾತ್ರ(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		ಬಿಡುಗಡೆ(buff);
	}

	digest = EVP_sha256();
	buff = ಸ್ಮರಣೆ_ಕೊಡು(EVP_MD_ಗಾತ್ರ(digest));
	ಆದರೆ (buff != ಖಾಲಿ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ಆದರೆ ((rc == 1) && (dig_len == (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)EVP_MD_ಗಾತ್ರ(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		ಬಿಡುಗಡೆ(buff);
	}

	/* subject alternative names */
	san_ಗಮ್ಯ_info(cert, &info->san);

	/* issuer name */
	xname_ಗಮ್ಯ_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_ಗಮ್ಯ_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_ಗಮ್ಯ_info(X509_get_pubkey(cert), &info->ಬಹಿರಂಗ_ಕೀಲಿ);

	X509_free(cert);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* copy certificate data */
ಸ್ಥಿರ nserror
convert_chain_ಗಮ್ಯ_cert_info(ಸ್ಥಿರಾಂಕ ರಚನೆ cert_chain *chain,
			   ರಚನೆ ns_cert_info **cert_info_ಹೊರಹೋಗು)
{
	ರಚನೆ ns_cert_info *certs;
	size_t ಆಳ;
	nserror res;

	certs = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(chain->ಆಳ, ಗಾತ್ರ(ರಚನೆ ns_cert_info));
	ಆದರೆ (certs == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಗಾಗಿ (ಆಳ = 0; ಆಳ < chain->ಆಳ;ಆಳ++) {
		res = der_ಗಮ್ಯ_certinfo(chain->certs[ಆಳ].der,
				      chain->certs[ಆಳ].der_ಉದ್ದಳತೆ,
				      certs + ಆಳ);
		ಆದರೆ (res != NSERROR_OK) {
			ಬಿಡುಗಡೆ(certs);
			ಹಿಂತಿರುಗಿ res;
		}
		certs[ಆಳ].err = chain->certs[ಆಳ].err;
	}

	*cert_info_ಹೊರಹೋಗು = certs;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ nserror
convert_chain_ಗಮ್ಯ_cert_info(ಸ್ಥಿರಾಂಕ ರಚನೆ cert_chain *chain,
			   ರಚನೆ ns_cert_info **cert_info_ಹೊರಹೋಗು)
{
	ಹಿಂತಿರುಗಿ NSERROR_NOT_IMPLEMENTED;
}
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ nserror
format_certificate_name(ರಚನೆ fetch_about_context *ctx,
			ರಚನೆ ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆದರೆ (cert_name->organisation != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (cert_name->organisation_unit != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (cert_name->locality != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (cert_name->province != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (cert_name->country != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಹಿಂತಿರುಗಿ res;
}

/**
 * output formatted certificate subject alternate names
 */
ಸ್ಥಿರ nserror
format_certificate_san(ರಚನೆ fetch_about_context *ctx,
			      ರಚನೆ ns_cert_san *san)
{
	nserror res;

	ಆದರೆ (san == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆಗ (san != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}

		san = san->ಮುಂದಿನ;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ಹಿಂತಿರುಗಿ res;

}


ಸ್ಥಿರ nserror
format_certificate_ಬಹಿರಂಗ_ಕೀಲಿ(ರಚನೆ fetch_about_context *ctx,
			      ರಚನೆ ns_cert_pkey *ಬಹಿರಂಗ_ಕೀಲಿ)
{
	nserror res;

	ಆದರೆ (ಬಹಿರಂಗ_ಕೀಲಿ->algor == ಖಾಲಿ) {
		/* skip the table if no algorithm name */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 ಬಹಿರಂಗ_ಕೀಲಿ->algor,
			 ಬಹಿರಂಗ_ಕೀಲಿ->ಗಾತ್ರ_2);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}


	ಆದರೆ (ಬಹಿರಂಗ_ಕೀಲಿ->exponent != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 ಬಹಿರಂಗ_ಕೀಲಿ->exponent);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (ಬಹಿರಂಗ_ಕೀಲಿ->modulus != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 ಬಹಿರಂಗ_ಕೀಲಿ->modulus);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (ಬಹಿರಂಗ_ಕೀಲಿ->ವಕ್ರ != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 ಬಹಿರಂಗ_ಕೀಲಿ->ವಕ್ರ);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (ಬಹಿರಂಗ_ಕೀಲಿ->ಸಾರ್ವಜನಿಕ != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 ಬಹಿರಂಗ_ಕೀಲಿ->ಸಾರ್ವಜನಿಕ);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ nserror
format_certificate_fingerprint(ರಚನೆ fetch_about_context *ctx,
			      ರಚನೆ ns_cert_info *cert_info)
{
	nserror res;

	ಆದರೆ ((cert_info->sha1fingerprint == ಖಾಲಿ) &&
	    (cert_info->sha256fingerprint == ಖಾಲಿ))  {
		/* skip the table if no fingerprints */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆದರೆ (cert_info->sha256fingerprint != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (cert_info->sha1fingerprint != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ nserror
format_certificate(ರಚನೆ fetch_about_context *ctx,
		   ರಚನೆ ns_cert_info *cert_info,
		   size_t ಆಳ)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 ಆಳ, cert_info->subject_name.common_name);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆದರೆ (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = format_certificate_ಬಹಿರಂಗ_ಕೀಲಿ(ctx, &cert_info->ಬಹಿರಂಗ_ಕೀಲಿ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆದರೆ (cert_info->serialnum != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಆದರೆ (cert_info->sig_algor != ಖಾಲಿ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->ಆವೃತ್ತಿ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಹಿಂತಿರುಗಿ res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = 200;
	nserror res;
	ರಚನೆ cert_chain *chain = ಖಾಲಿ;

	/* content is going to return ok */
	fetch_about_ಗಣ_http_ಸಂಕೇತ(ctx, ಸಂಕೇತ_2);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html"))
		ಹೋಗು fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	ಆದರೆ (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		ಆದರೆ (res != NSERROR_OK) {
			ಹೋಗು fetch_about_certificate_handler_aborted;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ರಚನೆ ns_cert_info *cert_info;
		res = convert_chain_ಗಮ್ಯ_cert_info(chain, &cert_info);
		ಆದರೆ (res == NSERROR_OK) {
			size_t ಆಳ;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			ಆದರೆ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ಹೋಗು fetch_about_certificate_handler_aborted;
			}

			ಗಾಗಿ (ಆಳ = 0; ಆಳ < chain->ಆಳ; ಆಳ++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						ಆಳ, (cert_info + ಆಳ)
							->subject_name
								.common_name);
				ಆದರೆ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ಹೋಗು fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			ಆದರೆ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ಹೋಗು fetch_about_certificate_handler_aborted;
			}

			ಗಾಗಿ (ಆಳ = 0; ಆಳ < chain->ಆಳ; ಆಳ++) {
				res = format_certificate(ctx, cert_info + ಆಳ,
						ಆಳ);
				ಆದರೆ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ಹೋಗು fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} ಇಲ್ಲದಿದ್ದರೆ {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			ಆದರೆ (res != NSERROR_OK) {
				ಹೋಗು fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	cert_chain_free(chain);

	ಹಿಂತಿರುಗಿ true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ಹಿಂತಿರುಗಿ false;
}
