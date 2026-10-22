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
ಸಂರಚನಾ ns_cert_name {
	ವರ್ಣ *common_name;
	ವರ್ಣ *organisation;
	ವರ್ಣ *organisation_unit;
	ವರ್ಣ *locality;
	ವರ್ಣ *province;
	ವರ್ಣ *country;
};

/**
 * Certificate public key parameters
 */
ಸಂರಚನಾ ns_cert_pkey {
	ವರ್ಣ *algor;
	ಪೂರ್ಣಾಙ್ಕ size;
	ವರ್ಣ *modulus;
	ವರ್ಣ *exponent;
	ವರ್ಣ *curve;
	ವರ್ಣ *ಸಾರ್ವಜನಿಕ;
};

/**
 * Certificate subject alternative name
 */
ಸಂರಚನಾ ns_cert_san {
	ಸಂರಚನಾ ns_cert_san *next;
	ವರ್ಣ *name;
};

/**
 * certificate information for certificate chain
 */
ಸಂರಚನಾ ns_cert_info {
	ಸಂರಚನಾ ns_cert_name subject_name; /**< Subject details */
	ಸಂರಚನಾ ns_cert_name issuer_name; /**< Issuer details */
	ಸಂರಚನಾ ns_cert_pkey public_ಕೀಲಿ; /**< public key details */
	long version;		/**< Certificate version */
	ವರ್ಣ *not_before;	/**< Valid from date */
	ವರ್ಣ *not_after;	/**< Valid to date */
	ಪೂರ್ಣಾಙ್ಕ sig_type;		/**< Signature type */
	ವರ್ಣ *sig_algor;        /**< Signature Algorithm */
	ವರ್ಣ *serialnum;	/**< Serial number */
	ವರ್ಣ *sha1fingerprint; /**< fingerprint shar1 encoded */
	ವರ್ಣ *sha256fingerprint; /**< fingerprint shar256 encoded */
	ಸಂರಚನಾ ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
ಸ್ಥಿರ nserror free_ns_cert_info(ಸಂರಚನಾ ns_cert_info *cinfo)
{
	ಸಂರಚನಾ ns_cert_san *san;

	ಮುಕ್ತ(cinfo->subject_name.common_name);
	ಮುಕ್ತ(cinfo->subject_name.organisation);
	ಮುಕ್ತ(cinfo->subject_name.organisation_unit);
	ಮುಕ್ತ(cinfo->subject_name.locality);
	ಮುಕ್ತ(cinfo->subject_name.province);
	ಮುಕ್ತ(cinfo->subject_name.country);
	ಮುಕ್ತ(cinfo->issuer_name.common_name);
	ಮುಕ್ತ(cinfo->issuer_name.organisation);
	ಮುಕ್ತ(cinfo->issuer_name.organisation_unit);
	ಮುಕ್ತ(cinfo->issuer_name.locality);
	ಮುಕ್ತ(cinfo->issuer_name.province);
	ಮುಕ್ತ(cinfo->issuer_name.country);
	ಮುಕ್ತ(cinfo->public_ಕೀಲಿ.algor);
	ಮುಕ್ತ(cinfo->public_ಕೀಲಿ.modulus);
	ಮುಕ್ತ(cinfo->public_ಕೀಲಿ.exponent);
	ಮುಕ್ತ(cinfo->public_ಕೀಲಿ.curve);
	ಮುಕ್ತ(cinfo->public_ಕೀಲಿ.ಸಾರ್ವಜನಿಕ);
	ಮುಕ್ತ(cinfo->not_before);
	ಮುಕ್ತ(cinfo->not_after);
	ಮುಕ್ತ(cinfo->sig_algor);
	ಮುಕ್ತ(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	ಯಾವತ್ (san != ರಿಕ್ತ) {
		ಸಂರಚನಾ ns_cert_san *next;
		next = san->next;
		ಮುಕ್ತ(san);
		san = next;
	}

	ಮುಕ್ತ(cinfo);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

#ifdef WITH_OPENSSL

#ಅನ್ತರ್ಭಾವ <openssl/ssl.h>
#ಅನ್ತರ್ಭಾವ <openssl/x509v3.h>

#ಯದಿ (OPENSSL_VERSION_ಸಂಖ್ಯೆ < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# ಯದಿ defined(LIBRESSL_VERSION_ಸಂಖ್ಯೆ)
  /* LibreSSL */
#  ಯದಿ (LIBRESSL_VERSION_ಸಂಖ್ಯೆ < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   ಯದಿ (LIBRESSL_VERSION_ಸಂಖ್ಯೆ < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ns_X509_get_signature_nid(X509 *cert)
{
	ಪ್ರತ್ಯಾಗಮ OBJ_obj2nid(cert->cert_info->ಕೀಲಿ->algor->algorithm);
}

ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	ಪ್ರತ್ಯಾಗಮ (ಧ್ರುವ unsigned ವರ್ಣ *)ASN1_STRING_data(asn1str);
}
#   ಅನ್ಯಥಾ
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

ಸ್ಥಿರ ಧ್ರುವ BIGNUM *ns_RSA_get0_n(ಧ್ರುವ RSA *d)
{
	ಪ್ರತ್ಯಾಗಮ d->n;
}

ಸ್ಥಿರ ಧ್ರುವ BIGNUM *ns_RSA_get0_e(ಧ್ರುವ RSA *d)
{
	ಪ್ರತ್ಯಾಗಮ d->e;
}
#  ಅನ್ಯಥಾ
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# ಅನ್ಯಥಾ
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ns_EVP_PKEY_get_bn_param(ಧ್ರುವ EVP_PKEY *pkey,
		ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = ರಿಕ್ತ;

	/* Check parameters: only support allocation-form *bn */
	ಯದಿ (pkey == ರಿಕ್ತ || ಕೀಲಿ_name == ರಿಕ್ತ || bn == ರಿಕ್ತ || *bn != ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* Only support RSA keys */
	ಯದಿ (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		ಪ್ರತ್ಯಾಗಮ 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	ಯದಿ (rsa == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (strcmp(ಕೀಲಿ_name, "n") == 0) {
		ಧ್ರುವ BIGNUM *n = ns_RSA_get0_n(rsa);
		ಯದಿ (n != ರಿಕ್ತ)
			result = BN_dup(n);
	} ಅನ್ಯಥಾ ಯದಿ (strcmp(ಕೀಲಿ_name, "e") == 0) {
		ಧ್ರುವ BIGNUM *e = ns_RSA_get0_e(rsa);
		ಯದಿ (e != ರಿಕ್ತ)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	ಪ್ರತ್ಯಾಗಮ (result != ರಿಕ್ತ) ? 1 : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ns_EVP_PKEY_get_utf8_string_param(ಧ್ರುವ EVP_PKEY *pkey,
		ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ_name, ವರ್ಣ *str, size_t max_len,
		size_t *out_len)
{
	ಧ್ರುವ EC_GROUP *ecgroup;
	ಧ್ರುವ ವರ್ಣ *group = "";
	size_t group_len = 0;
	EC_ಕೀಲಿ *ec;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	ಯದಿ (pkey == ರಿಕ್ತ || ಕೀಲಿ_name == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* Only support EC keys */
	ಯದಿ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* Only support fetching the group */
	ಯದಿ (strcmp(ಕೀಲಿ_name, "group") != 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ec = EVP_PKEY_get1_EC_ಕೀಲಿ((EVP_PKEY *) pkey);

	ecgroup = EC_ಕೀಲಿ_get0_group(ec);
	ಯದಿ (ecgroup != ರಿಕ್ತ) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	ಯದಿ (str != ರಿಕ್ತ && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	ಯದಿ (out_len != ರಿಕ್ತ) {
		*out_len = group_len;
	}

	EC_ಕೀಲಿ_free(ec);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ns_EVP_PKEY_get_octet_string_param(ಧ್ರುವ EVP_PKEY *pkey,
		ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ_name, unsigned ವರ್ಣ *buf, size_t max_len,
		size_t *out_len)
{
	ಧ್ರುವ EC_GROUP *ecgroup;
	ಧ್ರುವ EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_ಕೀಲಿ *ec;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	ಯದಿ (pkey == ರಿಕ್ತ || ಕೀಲಿ_name == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* Only support EC keys */
	ಯದಿ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (strcmp(ಕೀಲಿ_name, "encoded-pub-key") != 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ec = EVP_PKEY_get1_EC_ಕೀಲಿ((EVP_PKEY *) pkey);
	ಯದಿ (ec == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ecgroup = EC_ಕೀಲಿ_get0_group(ec);
	ಯದಿ (ecgroup != ರಿಕ್ತ) {
		ecpoint = EC_ಕೀಲಿ_get0_public_ಕೀಲಿ(ec);
		ಯದಿ (ecpoint != ರಿಕ್ತ) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 ರಿಕ್ತ,
						 0,
						 bnctx);
			ಯದಿ (len != 0 && len <= max_len) {
				ಯದಿ (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			ಯದಿ (out_len != ರಿಕ್ತ)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_ಕೀಲಿ_free(ec);

	ಪ್ರತ್ಯಾಗಮ ret;
}
#ಅನ್ಯಥಾ
/* OpenSSL 3.x and later */
#define ns_X509_get_signature_nid X509_get_signature_nid
#define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#define ns_RSA_get0_n RSA_get0_n
#define ns_RSA_get0_e RSA_get0_e
#define ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#define ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#define ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#endif

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
ಸ್ಥಿರ nserror
xname_to_info(X509_NAME *xname, ಸಂರಚನಾ ns_cert_name *iname)
{
	ಪೂರ್ಣಾಙ್ಕ entryidx;
	ಪೂರ್ಣಾಙ್ಕ entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *ಮೌಲ್ಯ;
	ಧ್ರುವ unsigned ವರ್ಣ *ಮೌಲ್ಯ_str;
	ASN1_OBJECT *name;
	ಪೂರ್ಣಾಙ್ಕ name_nid;
	ವರ್ಣ **field;

	entrycnt = X509_NAME_entry_ಎಣಿಕೆ(xname);

	ಪ್ರತಿ (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		ಮೌಲ್ಯ = X509_NAME_ENTRY_get_data(entry);
		ಮೌಲ್ಯ_str = ns_ASN1_STRING_get0_data(ಮೌಲ್ಯ);
		ವಿಕಲ್ಪ (name_nid) {
		ಪಕ್ಷ NID_commonName:
			field = &iname->common_name;
			ಭಙ್ಗ;
		ಪಕ್ಷ NID_countryName:
			field = &iname->country;
			ಭಙ್ಗ;
		ಪಕ್ಷ NID_localityName:
			field = &iname->locality;
			ಭಙ್ಗ;
		ಪಕ್ಷ NID_stateOrProvinceName:
			field = &iname->province;
			ಭಙ್ಗ;
		ಪಕ್ಷ NID_organizationName:
			field = &iname->organisation;
			ಭಙ್ಗ;
		ಪಕ್ಷ NID_organizationalUnitName:
			field = &iname->organisation_unit;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ :
			field = ರಿಕ್ತ;
			ಭಙ್ಗ;
		}
		ಯದಿ (field != ರಿಕ್ತ) {
			*field = strdup((ಧ್ರುವ ವರ್ಣ *)ಮೌಲ್ಯ_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} ಅನ್ಯಥಾ {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	ಯದಿ (iname->common_name == ರಿಕ್ತ) {
		iname->common_name = strdup("Unknown");
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ಸ್ಥಿರ ವರ್ಣ *hexdup(ಧ್ರುವ ವರ್ಣ *hex)
{
	ಪೂರ್ಣಾಙ್ಕ hexlen;
	ವರ್ಣ *dst;
	ವರ್ಣ *out;
	ಪೂರ್ಣಾಙ್ಕ cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	ಯದಿ (dst != ರಿಕ್ತ) {
		ಪ್ರತಿ (out = dst; *hex != 0; hex++) {
			ಯದಿ (cn == 2) {
				cn = 0;
				*out++ = '&';
				*out++ = '#';
				*out++ = '5';
				*out++ = '8';
				*out++ = ';';
			}
			*out++ = *hex;
			cn++;
		}
		*out = 0;
	}
	ಪ್ರತ್ಯಾಗಮ dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ಸ್ಥಿರ ವರ್ಣ *bindup(unsigned ವರ್ಣ *bin, unsigned ಪೂರ್ಣಾಙ್ಕ binlen)
{
	ವರ್ಣ *dst;
	ವರ್ಣ *out;
	unsigned ಪೂರ್ಣಾಙ್ಕ idx;
	ಧ್ರುವ ವರ್ಣ hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	ಯದಿ (dst != ರಿಕ್ತ) {
		out = dst;
		ಪ್ರತಿ (idx = 0; idx < binlen; idx++) {
			*out++ = hex[(bin[idx] & 0xf0) >> 4];
			*out++ = hex[bin[idx] & 0xf];

			*out++ = '&';
			*out++ = '#';
			*out++ = '5';
			*out++ = '8';
			*out++ = ';';
		}
		out -= 5;
		*out = 0;
	}
	ಪ್ರತ್ಯಾಗಮ dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
rsa_to_info(EVP_PKEY *pkey, ಸಂರಚನಾ ns_cert_pkey *ikey)
{
	BIGNUM *n = ರಿಕ್ತ, *e = ರಿಕ್ತ;
	ವರ್ಣ *tmp;

	ಯದಿ (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	ಯದಿ (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	ಯದಿ (tmp != ರಿಕ್ತ) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	ಯದಿ (tmp != ರಿಕ್ತ) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
dsa_to_info(EVP_PKEY *pkey, ಸಂರಚನಾ ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
dh_to_info(EVP_PKEY *pkey, ಸಂರಚನಾ ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
ec_to_info(EVP_PKEY *pkey, ಸಂರಚನಾ ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ರಿಕ್ತ, 0, &len);
	ಯದಿ (len != 0) {
		ikey->curve = malloc(len + 1);
		ಯದಿ (ikey->curve != ರಿಕ್ತ) {
			ಯದಿ (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, ರಿಕ್ತ) == 0) {
				ಮುಕ್ತ(ikey->curve);
				ikey->curve = ರಿಕ್ತ;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ರಿಕ್ತ, 0, &len);
	ಯದಿ (len != 0) {
		unsigned ವರ್ಣ *point = malloc(len);
		ಯದಿ (point != ರಿಕ್ತ) {
			ಯದಿ (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					ರಿಕ್ತ) == 1) {
				ikey->ಸಾರ್ವಜನಿಕ = bindup(point, len);
			}
			ಮುಕ್ತ(point);
		}
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ಸ್ಥಿರ nserror
pkey_to_info(EVP_PKEY *pkey, ಸಂರಚನಾ ns_cert_pkey *ikey)
{
	nserror res;

	ಯದಿ (pkey == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	ವಿಕಲ್ಪ (EVP_PKEY_base_id(pkey)) {
	ಪಕ್ಷ EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		ಭಙ್ಗ;

	ಪಕ್ಷ EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		ಭಙ್ಗ;

	ಪಕ್ಷ EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		ಭಙ್ಗ;

	ಪಕ್ಷ EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		res = NSERROR_NOT_IMPLEMENTED;
		ಭಙ್ಗ;
	}

	EVP_PKEY_free(pkey);

	ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ nserror san_to_info(X509 *cert, ಸಂರಚನಾ ns_cert_san **prev_next)
{
	ಪೂರ್ಣಾಙ್ಕ idx;
	ಪೂರ್ಣಾಙ್ಕ san_names_nb = -1;
	ಧ್ರುವ GENERAL_NAME *current_name;
	ಧ್ರುವ unsigned ವರ್ಣ *dns_name;
	ಸಂರಚನಾ ns_cert_san *isan;

	ರಾಶಿ_OF(GENERAL_NAME) *san_names = ರಿಕ್ತ;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, ರಿಕ್ತ, ರಿಕ್ತ);
	ಯದಿ (san_names == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_ಅಂಕೆ(san_names);

	/* Check each name within the extension */
	ಪ್ರತಿ (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_ಮೌಲ್ಯ(san_names, idx);

		ಯದಿ (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ ns_cert_san));
			ಯದಿ (isan != ರಿಕ್ತ) {
				isan->name = strdup((ಧ್ರುವ ವರ್ಣ *)dns_name);
				isan->next = ರಿಕ್ತ;
				*prev_next = isan;
				prev_next = &isan->next;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	ಪ್ರತಿ (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_ತೆಗೆ(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ nserror
der_to_certinfo(ಧ್ರುವ uint8_t *der,
		size_t der_length,
		ಸಂರಚನಾ ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ಧ್ರುವ ASN1_INTEGER *asn1_ಅಂಕೆ;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	ಯದಿ (der == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	cert = d2i_X509(ರಿಕ್ತ, &der, der_length);
	ಯದಿ (cert == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->version = X509_get_version(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ಶೂನ್ಯ) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	ಯದಿ (info->not_before != ರಿಕ್ತ) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ಶೂನ್ಯ) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	ಯದಿ (info->not_after != ರಿಕ್ತ) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	ಪೂರ್ಣಾಙ್ಕ pkey_nid = ns_X509_get_signature_nid(cert);
	ಯದಿ (pkey_nid != NID_undef) {
		ಧ್ರುವ ವರ್ಣ* sslbuf = OBJ_nid2ln(pkey_nid);
		ಯದಿ (sslbuf != ರಿಕ್ತ) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_ಅಂಕೆ = X509_get_serial_ಸಂಖ್ಯೆ(cert);
	ಯದಿ (asn1_ಅಂಕೆ != ರಿಕ್ತ) {
		bignum = ASN1_INTEGER_to_BN(asn1_ಅಂಕೆ, ರಿಕ್ತ);
		ಯದಿ (bignum != ರಿಕ್ತ) {
			ವರ್ಣ *tmp = BN_bn2hex(bignum);
			ಯದಿ (tmp != ರಿಕ್ತ) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ರಿಕ್ತ;
		}
	}

	/* fingerprints */
	ಧ್ರುವ EVP_MD *digest;
	unsigned ಪೂರ್ಣಾಙ್ಕ dig_len;
	unsigned ವರ್ಣ *buff;
	ಪೂರ್ಣಾಙ್ಕ rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	ಯದಿ (buff != ರಿಕ್ತ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ಯದಿ ((rc == 1) && (dig_len == (unsigned ಪೂರ್ಣಾಙ್ಕ)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		ಮುಕ್ತ(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	ಯದಿ (buff != ರಿಕ್ತ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ಯದಿ ((rc == 1) && (dig_len == (unsigned ಪೂರ್ಣಾಙ್ಕ)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		ಮುಕ್ತ(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_ಕೀಲಿ);

	X509_free(cert);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* copy certificate data */
ಸ್ಥಿರ nserror
convert_chain_to_cert_info(ಧ್ರುವ ಸಂರಚನಾ cert_chain *chain,
			   ಸಂರಚನಾ ns_cert_info **cert_info_out)
{
	ಸಂರಚನಾ ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, ಪರಿಮಾಣ(ಸಂರಚನಾ ns_cert_info));
	ಯದಿ (certs == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಪ್ರತಿ (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		ಯದಿ (res != NSERROR_OK) {
			ಮುಕ್ತ(certs);
			ಪ್ರತ್ಯಾಗಮ res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

#ಅನ್ಯಥಾ
ಸ್ಥಿರ nserror
convert_chain_to_cert_info(ಧ್ರುವ ಸಂರಚನಾ cert_chain *chain,
			   ಸಂರಚನಾ ns_cert_info **cert_info_out)
{
	ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_IMPLEMENTED;
}
#endif


ಸ್ಥಿರ nserror
format_certificate_name(ಸಂರಚನಾ fetch_about_context *ctx,
			ಸಂರಚನಾ ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಯದಿ (cert_name->organisation != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (cert_name->organisation_unit != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (cert_name->locality != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (cert_name->province != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (cert_name->country != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಪ್ರತ್ಯಾಗಮ res;
}

/**
 * output formatted certificate subject alternate names
 */
ಸ್ಥಿರ nserror
format_certificate_san(ಸಂರಚನಾ fetch_about_context *ctx,
			      ಸಂರಚನಾ ns_cert_san *san)
{
	nserror res;

	ಯದಿ (san == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಯಾವತ್ (san != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ಪ್ರತ್ಯಾಗಮ res;

}


ಸ್ಥಿರ nserror
format_certificate_public_ಕೀಲಿ(ಸಂರಚನಾ fetch_about_context *ctx,
			      ಸಂರಚನಾ ns_cert_pkey *public_ಕೀಲಿ)
{
	nserror res;

	ಯದಿ (public_ಕೀಲಿ->algor == ರಿಕ್ತ) {
		/* skip the table if no algorithm name */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_ಕೀಲಿ->algor,
			 public_ಕೀಲಿ->size);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}


	ಯದಿ (public_ಕೀಲಿ->exponent != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_ಕೀಲಿ->exponent);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (public_ಕೀಲಿ->modulus != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_ಕೀಲಿ->modulus);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (public_ಕೀಲಿ->curve != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_ಕೀಲಿ->curve);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (public_ಕೀಲಿ->ಸಾರ್ವಜನಿಕ != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_ಕೀಲಿ->ಸಾರ್ವಜನಿಕ);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ nserror
format_certificate_fingerprint(ಸಂರಚನಾ fetch_about_context *ctx,
			      ಸಂರಚನಾ ns_cert_info *cert_info)
{
	nserror res;

	ಯದಿ ((cert_info->sha1fingerprint == ರಿಕ್ತ) &&
	    (cert_info->sha256fingerprint == ರಿಕ್ತ))  {
		/* skip the table if no fingerprints */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಯದಿ (cert_info->sha256fingerprint != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (cert_info->sha1fingerprint != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ nserror
format_certificate(ಸಂರಚನಾ fetch_about_context *ctx,
		   ಸಂರಚನಾ ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಯದಿ (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = format_certificate_public_ಕೀಲಿ(ctx, &cert_info->public_ಕೀಲಿ);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಯದಿ (cert_info->serialnum != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಯದಿ (cert_info->sig_algor != ರಿಕ್ತ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಪ್ರತ್ಯಾಗಮ res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಪೂರ್ಣಾಙ್ಕ code = 200;
	nserror res;
	ಸಂರಚನಾ cert_chain *chain = ರಿಕ್ತ;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx, "Content-Type: text/html"))
		ಗಚ್ಛ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	ಯದಿ (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		ಯದಿ (res != NSERROR_OK) {
			ಗಚ್ಛ fetch_about_certificate_handler_aborted;
		}
	} ಅನ್ಯಥಾ {
		ಸಂರಚನಾ ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		ಯದಿ (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			ಯದಿ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ಗಚ್ಛ fetch_about_certificate_handler_aborted;
			}

			ಪ್ರತಿ (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				ಯದಿ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ಗಚ್ಛ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			ಯದಿ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ಗಚ್ಛ fetch_about_certificate_handler_aborted;
			}

			ಪ್ರತಿ (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				ಯದಿ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ಗಚ್ಛ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} ಅನ್ಯಥಾ {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			ಯದಿ (res != NSERROR_OK) {
				ಗಚ್ಛ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	ಪ್ರತ್ಯಾಗಮ true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ಪ್ರತ್ಯಾಗಮ false;
}
