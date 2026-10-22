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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/errors.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/inttypes.h"
#include "netsurf/ssl_certs.h"

#include "private.h"
#include "certificate.h"

/**
 * certificate name parameters
 */
sa_nracanaa ns_cert_name {
	var_na *common_name;
	var_na *organisation;
	var_na *organisation_unit;
	var_na *locality;
	var_na *province;
	var_na *country;
};

/**
 * Certificate public key parameters
 */
sa_nracanaa ns_cert_pkey {
	var_na *algor;
	puur_naa_nka size;
	var_na *modulus;
	var_na *exponent;
	var_na *curve;
	var_na *public;
};

/**
 * Certificate subject alternative name
 */
sa_nracanaa ns_cert_san {
	sa_nracanaa ns_cert_san *next;
	var_na *name;
};

/**
 * certificate information for certificate chain
 */
sa_nracanaa ns_cert_info {
	sa_nracanaa ns_cert_name subject_name; /**< Subject details */
	sa_nracanaa ns_cert_name issuer_name; /**< Issuer details */
	sa_nracanaa ns_cert_pkey public_key; /**< public key details */
	long version;		/**< Certificate version */
	var_na *not_before;	/**< Valid from date */
	var_na *not_after;	/**< Valid to date */
	puur_naa_nka sig_type;		/**< Signature type */
	var_na *sig_algor;        /**< Signature Algorithm */
	var_na *serialnum;	/**< Serial number */
	var_na *sha1fingerprint; /**< fingerprint shar1 encoded */
	var_na *sha256fingerprint; /**< fingerprint shar256 encoded */
	sa_nracanaa ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
static nserror free_ns_cert_info(sa_nracanaa ns_cert_info *cinfo)
{
	sa_nracanaa ns_cert_san *san;

	free(cinfo->subject_name.common_name);
	free(cinfo->subject_name.organisation);
	free(cinfo->subject_name.organisation_unit);
	free(cinfo->subject_name.locality);
	free(cinfo->subject_name.province);
	free(cinfo->subject_name.country);
	free(cinfo->issuer_name.common_name);
	free(cinfo->issuer_name.organisation);
	free(cinfo->issuer_name.organisation_unit);
	free(cinfo->issuer_name.locality);
	free(cinfo->issuer_name.province);
	free(cinfo->issuer_name.country);
	free(cinfo->public_key.algor);
	free(cinfo->public_key.modulus);
	free(cinfo->public_key.exponent);
	free(cinfo->public_key.curve);
	free(cinfo->public_key.public);
	free(cinfo->not_before);
	free(cinfo->not_after);
	free(cinfo->sig_algor);
	free(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	ᬲᭂᬤᭂᬮ (san != NULL) {
		sa_nracanaa ns_cert_san *next;
		next = san->next;
		free(san);
		san = next;
	}

	free(cinfo);

	ᬯᬮᬶ NSERROR_OK;
}

#ifdef WITH_OPENSSL

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#ᬬᬾᬦ᭄ (OPENSSL_VERSION_NUMBER < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# ᬬᬾᬦ᭄ defined(LIBRESSL_VERSION_NUMBER)
  /* LibreSSL */
#  ᬬᬾᬦ᭄ (LIBRESSL_VERSION_NUMBER < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   ᬬᬾᬦ᭄ (LIBRESSL_VERSION_NUMBER < 0x2070000fL)
    /* LibreSSL <2.7.0 */
static puur_naa_nka ns_X509_get_signature_nid(X509 *cert)
{
	ᬯᬮᬶ OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
}

static const unsigned var_na *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	ᬯᬮᬶ (const unsigned var_na *)ASN1_STRING_data(asn1str);
}
#   ᬮᬶᬬ
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

static const BIGNUM *ns_RSA_get0_n(const RSA *d)
{
	ᬯᬮᬶ d->n;
}

static const BIGNUM *ns_RSA_get0_e(const RSA *d)
{
	ᬯᬮᬶ d->e;
}
#  ᬮᬶᬬ
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# ᬮᬶᬬ
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

static puur_naa_nka ns_EVP_PKEY_get_bn_param(const EVP_PKEY *pkey,
		const var_na *key_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = NULL;

	/* Check parameters: only support allocation-form *bn */
	ᬬᬾᬦ᭄ (pkey == NULL || key_name == NULL || bn == NULL || *bn != NULL)
		ᬯᬮᬶ 0;

	/* Only support RSA keys */
	ᬬᬾᬦ᭄ (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		ᬯᬮᬶ 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	ᬬᬾᬦ᭄ (rsa == NULL)
		ᬯᬮᬶ 0;

	ᬬᬾᬦ᭄ (strcmp(key_name, "n") == 0) {
		const BIGNUM *n = ns_RSA_get0_n(rsa);
		ᬬᬾᬦ᭄ (n != NULL)
			result = BN_dup(n);
	} ᬮᬶᬬ ᬬᬾᬦ᭄ (strcmp(key_name, "e") == 0) {
		const BIGNUM *e = ns_RSA_get0_e(rsa);
		ᬬᬾᬦ᭄ (e != NULL)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	ᬯᬮᬶ (result != NULL) ? 1 : 0;
}

static puur_naa_nka ns_EVP_PKEY_get_utf8_string_param(const EVP_PKEY *pkey,
		const var_na *key_name, var_na *str, size_t max_len,
		size_t *out_len)
{
	const EC_GROUP *ecgroup;
	const var_na *group = "";
	size_t group_len = 0;
	EC_KEY *ec;
	puur_naa_nka ret = 0;

	ᬬᬾᬦ᭄ (pkey == NULL || key_name == NULL)
		ᬯᬮᬶ 0;

	/* Only support EC keys */
	ᬬᬾᬦ᭄ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ᬯᬮᬶ 0;

	/* Only support fetching the group */
	ᬬᬾᬦ᭄ (strcmp(key_name, "group") != 0)
		ᬯᬮᬶ 0;

	ec = EVP_PKEY_get1_EC_KEY((EVP_PKEY *) pkey);

	ecgroup = EC_KEY_get0_group(ec);
	ᬬᬾᬦ᭄ (ecgroup != NULL) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	ᬬᬾᬦ᭄ (str != NULL && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	ᬬᬾᬦ᭄ (out_len != NULL) {
		*out_len = group_len;
	}

	EC_KEY_free(ec);

	ᬯᬮᬶ ret;
}

static puur_naa_nka ns_EVP_PKEY_get_octet_string_param(const EVP_PKEY *pkey,
		const var_na *key_name, unsigned var_na *buf, size_t max_len,
		size_t *out_len)
{
	const EC_GROUP *ecgroup;
	const EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_KEY *ec;
	puur_naa_nka ret = 0;

	ᬬᬾᬦ᭄ (pkey == NULL || key_name == NULL)
		ᬯᬮᬶ 0;

	/* Only support EC keys */
	ᬬᬾᬦ᭄ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ᬯᬮᬶ 0;

	ᬬᬾᬦ᭄ (strcmp(key_name, "encoded-pub-key") != 0)
		ᬯᬮᬶ 0;

	ec = EVP_PKEY_get1_EC_KEY((EVP_PKEY *) pkey);
	ᬬᬾᬦ᭄ (ec == NULL)
		ᬯᬮᬶ 0;

	ecgroup = EC_KEY_get0_group(ec);
	ᬬᬾᬦ᭄ (ecgroup != NULL) {
		ecpoint = EC_KEY_get0_public_key(ec);
		ᬬᬾᬦ᭄ (ecpoint != NULL) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 NULL,
						 0,
						 bnctx);
			ᬬᬾᬦ᭄ (len != 0 && len <= max_len) {
				ᬬᬾᬦ᭄ (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			ᬬᬾᬦ᭄ (out_len != NULL)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_KEY_free(ec);

	ᬯᬮᬶ ret;
}
#ᬮᬶᬬ
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
static nserror
xname_to_info(X509_NAME *xname, sa_nracanaa ns_cert_name *iname)
{
	puur_naa_nka entryidx;
	puur_naa_nka entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *value;
	const unsigned var_na *value_str;
	ASN1_OBJECT *name;
	puur_naa_nka name_nid;
	var_na **field;

	entrycnt = X509_NAME_entry_count(xname);

	for (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		value = X509_NAME_ENTRY_get_data(entry);
		value_str = ns_ASN1_STRING_get0_data(value);
		switch (name_nid) {
		pak_sa NID_commonName:
			field = &iname->common_name;
			bha_nga;
		pak_sa NID_countryName:
			field = &iname->country;
			bha_nga;
		pak_sa NID_localityName:
			field = &iname->locality;
			bha_nga;
		pak_sa NID_stateOrProvinceName:
			field = &iname->province;
			bha_nga;
		pak_sa NID_organizationName:
			field = &iname->organisation;
			bha_nga;
		pak_sa NID_organizationalUnitName:
			field = &iname->organisation_unit;
			bha_nga;
		default :
			field = NULL;
			bha_nga;
		}
		ᬬᬾᬦ᭄ (field != NULL) {
			*field = strdup((const var_na *)value_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} ᬮᬶᬬ {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	ᬬᬾᬦ᭄ (iname->common_name == NULL) {
		iname->common_name = strdup("Unknown");
	}

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
static var_na *hexdup(const var_na *hex)
{
	puur_naa_nka hexlen;
	var_na *dst;
	var_na *out;
	puur_naa_nka cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	ᬬᬾᬦ᭄ (dst != NULL) {
		for (out = dst; *hex != 0; hex++) {
			ᬬᬾᬦ᭄ (cn == 2) {
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
	ᬯᬮᬶ dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
static var_na *bindup(unsigned var_na *bin, unsigned puur_naa_nka binlen)
{
	var_na *dst;
	var_na *out;
	unsigned puur_naa_nka idx;
	const var_na hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	ᬬᬾᬦ᭄ (dst != NULL) {
		out = dst;
		for (idx = 0; idx < binlen; idx++) {
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
	ᬯᬮᬶ dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
static nserror
rsa_to_info(EVP_PKEY *pkey, sa_nracanaa ns_cert_pkey *ikey)
{
	BIGNUM *n = NULL, *e = NULL;
	var_na *tmp;

	ᬬᬾᬦ᭄ (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ᬯᬮᬶ NSERROR_BAD_PARAMETER;
	}

	ᬬᬾᬦ᭄ (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ᬯᬮᬶ NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	ᬬᬾᬦ᭄ (tmp != NULL) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	ᬬᬾᬦ᭄ (tmp != NULL) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
static nserror
dsa_to_info(EVP_PKEY *pkey, sa_nracanaa ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
static nserror
dh_to_info(EVP_PKEY *pkey, sa_nracanaa ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
static nserror
ec_to_info(EVP_PKEY *pkey, sa_nracanaa ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", NULL, 0, &len);
	ᬬᬾᬦ᭄ (len != 0) {
		ikey->curve = malloc(len + 1);
		ᬬᬾᬦ᭄ (ikey->curve != NULL) {
			ᬬᬾᬦ᭄ (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, NULL) == 0) {
				free(ikey->curve);
				ikey->curve = NULL;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			NULL, 0, &len);
	ᬬᬾᬦ᭄ (len != 0) {
		unsigned var_na *point = malloc(len);
		ᬬᬾᬦ᭄ (point != NULL) {
			ᬬᬾᬦ᭄ (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					NULL) == 1) {
				ikey->public = bindup(point, len);
			}
			free(point);
		}
	}

	ᬯᬮᬶ NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
static nserror
pkey_to_info(EVP_PKEY *pkey, sa_nracanaa ns_cert_pkey *ikey)
{
	nserror res;

	ᬬᬾᬦ᭄ (pkey == NULL) {
		ᬯᬮᬶ NSERROR_BAD_PARAMETER;
	}

	switch (EVP_PKEY_base_id(pkey)) {
	pak_sa EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		bha_nga;

	pak_sa EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		bha_nga;

	pak_sa EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		bha_nga;

	pak_sa EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		bha_nga;

	default:
		res = NSERROR_NOT_IMPLEMENTED;
		bha_nga;
	}

	EVP_PKEY_free(pkey);

	ᬯᬮᬶ res;
}

static nserror san_to_info(X509 *cert, sa_nracanaa ns_cert_san **prev_next)
{
	puur_naa_nka idx;
	puur_naa_nka san_names_nb = -1;
	const GENERAL_NAME *current_name;
	const unsigned var_na *dns_name;
	sa_nracanaa ns_cert_san *isan;

	STACK_OF(GENERAL_NAME) *san_names = NULL;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
	ᬬᬾᬦ᭄ (san_names == NULL) {
		ᬯᬮᬶ NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_num(san_names);

	/* Check each name within the extension */
	for (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_value(san_names, idx);

		ᬬᬾᬦ᭄ (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(parimaa_na(sa_nracanaa ns_cert_san));
			ᬬᬾᬦ᭄ (isan != NULL) {
				isan->name = strdup((const var_na *)dns_name);
				isan->next = NULL;
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
	for (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_pop(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	ᬯᬮᬶ NSERROR_OK;
}

static nserror
der_to_certinfo(const uint8_t *der,
		size_t der_length,
		sa_nracanaa ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	const ASN1_INTEGER *asn1_num;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	ᬬᬾᬦ᭄ (der == NULL) {
		ᬯᬮᬶ NSERROR_OK;
	}

	cert = d2i_X509(NULL, &der, der_length);
	ᬬᬾᬦ᭄ (cert == NULL) {
		ᬯᬮᬶ NSERROR_INVALID;
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
	(void) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	ᬬᬾᬦ᭄ (info->not_before != NULL) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(void) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	ᬬᬾᬦ᭄ (info->not_after != NULL) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	puur_naa_nka pkey_nid = ns_X509_get_signature_nid(cert);
	ᬬᬾᬦ᭄ (pkey_nid != NID_undef) {
		const var_na* sslbuf = OBJ_nid2ln(pkey_nid);
		ᬬᬾᬦ᭄ (sslbuf != NULL) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_num = X509_get_serialNumber(cert);
	ᬬᬾᬦ᭄ (asn1_num != NULL) {
		bignum = ASN1_INTEGER_to_BN(asn1_num, NULL);
		ᬬᬾᬦ᭄ (bignum != NULL) {
			var_na *tmp = BN_bn2hex(bignum);
			ᬬᬾᬦ᭄ (tmp != NULL) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = NULL;
		}
	}

	/* fingerprints */
	const EVP_MD *digest;
	unsigned puur_naa_nka dig_len;
	unsigned var_na *buff;
	puur_naa_nka rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	ᬬᬾᬦ᭄ (buff != NULL) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ᬬᬾᬦ᭄ ((rc == 1) && (dig_len == (unsigned puur_naa_nka)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		free(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	ᬬᬾᬦ᭄ (buff != NULL) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ᬬᬾᬦ᭄ ((rc == 1) && (dig_len == (unsigned puur_naa_nka)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		free(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_key);

	X509_free(cert);

	ᬯᬮᬶ NSERROR_OK;
}

/* copy certificate data */
static nserror
convert_chain_to_cert_info(const sa_nracanaa cert_chain *chain,
			   sa_nracanaa ns_cert_info **cert_info_out)
{
	sa_nracanaa ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, parimaa_na(sa_nracanaa ns_cert_info));
	ᬬᬾᬦ᭄ (certs == NULL) {
		ᬯᬮᬶ NSERROR_NOMEM;
	}

	for (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			free(certs);
			ᬯᬮᬶ res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	ᬯᬮᬶ NSERROR_OK;
}

#ᬮᬶᬬ
static nserror
convert_chain_to_cert_info(const sa_nracanaa cert_chain *chain,
			   sa_nracanaa ns_cert_info **cert_info_out)
{
	ᬯᬮᬶ NSERROR_NOT_IMPLEMENTED;
}
#endif


static nserror
format_certificate_name(sa_nracanaa fetch_about_context *ctx,
			sa_nracanaa ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	ᬬᬾᬦ᭄ (cert_name->organisation != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (cert_name->organisation_unit != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (cert_name->locality != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (cert_name->province != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (cert_name->country != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬯᬮᬶ res;
}

/**
 * output formatted certificate subject alternate names
 */
static nserror
format_certificate_san(sa_nracanaa fetch_about_context *ctx,
			      sa_nracanaa ns_cert_san *san)
{
	nserror res;

	ᬬᬾᬦ᭄ (san == NULL) {
		ᬯᬮᬶ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	ᬲᭂᬤᭂᬮ (san != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ᬯᬮᬶ res;

}


static nserror
format_certificate_public_key(sa_nracanaa fetch_about_context *ctx,
			      sa_nracanaa ns_cert_pkey *public_key)
{
	nserror res;

	ᬬᬾᬦ᭄ (public_key->algor == NULL) {
		/* skip the table if no algorithm name */
		ᬯᬮᬶ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_key->algor,
			 public_key->size);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}


	ᬬᬾᬦ᭄ (public_key->exponent != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_key->exponent);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (public_key->modulus != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_key->modulus);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (public_key->curve != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_key->curve);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (public_key->public != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_key->public);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ᬯᬮᬶ res;
}

static nserror
format_certificate_fingerprint(sa_nracanaa fetch_about_context *ctx,
			      sa_nracanaa ns_cert_info *cert_info)
{
	nserror res;

	ᬬᬾᬦ᭄ ((cert_info->sha1fingerprint == NULL) &&
	    (cert_info->sha256fingerprint == NULL))  {
		/* skip the table if no fingerprints */
		ᬯᬮᬶ NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	ᬬᬾᬦ᭄ (cert_info->sha256fingerprint != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (cert_info->sha1fingerprint != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ᬯᬮᬶ res;
}

static nserror
format_certificate(sa_nracanaa fetch_about_context *ctx,
		   sa_nracanaa ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	ᬬᬾᬦ᭄ (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = format_certificate_public_key(ctx, &cert_info->public_key);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	ᬬᬾᬦ᭄ (cert_info->serialnum != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	ᬬᬾᬦ᭄ (cert_info->sig_algor != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			ᬯᬮᬶ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		ᬯᬮᬶ res;
	}

	ᬯᬮᬶ res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(sa_nracanaa fetch_about_context *ctx)
{
	puur_naa_nka code = 200;
	nserror res;
	sa_nracanaa cert_chain *chain = NULL;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ᬬᬾᬦ᭄ (fetch_about_send_header(ctx, "Content-Type: text/html"))
		goto fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		goto fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		ᬬᬾᬦ᭄ (res != NSERROR_OK) {
			goto fetch_about_certificate_handler_aborted;
		}
	} ᬮᬶᬬ {
		sa_nracanaa ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		ᬬᬾᬦ᭄ (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			ᬬᬾᬦ᭄ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				goto fetch_about_certificate_handler_aborted;
			}

			for (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				ᬬᬾᬦ᭄ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					goto fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			ᬬᬾᬦ᭄ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				goto fetch_about_certificate_handler_aborted;
			}

			for (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				ᬬᬾᬦ᭄ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					goto fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} ᬮᬶᬬ {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			ᬬᬾᬦ᭄ (res != NSERROR_OK) {
				goto fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	ᬬᬾᬦ᭄ (res != NSERROR_OK) {
		goto fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	ᬯᬮᬶ true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ᬯᬮᬶ false;
}
