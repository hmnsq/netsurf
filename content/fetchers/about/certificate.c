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
	ຈຳນວນເຕັມ size;
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
	ຈຳນວນເຕັມ sig_type;		/**< Signature type */
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
	ໃນຂະນະທີ່ (san != NULL) {
		sa_nracanaa ns_cert_san *next;
		next = san->next;
		free(san);
		san = next;
	}

	free(cinfo);

	ສົ່ງຄືນ NSERROR_OK;
}

#ifdef WITH_OPENSSL

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#ຖ້າ (OPENSSL_VERSION_NUMBER < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# ຖ້າ defined(LIBRESSL_VERSION_NUMBER)
  /* LibreSSL */
#  ຖ້າ (LIBRESSL_VERSION_NUMBER < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   ຖ້າ (LIBRESSL_VERSION_NUMBER < 0x2070000fL)
    /* LibreSSL <2.7.0 */
static ຈຳນວນເຕັມ ns_X509_get_signature_nid(X509 *cert)
{
	ສົ່ງຄືນ OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
}

static const unsigned var_na *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	ສົ່ງຄືນ (const unsigned var_na *)ASN1_STRING_data(asn1str);
}
#   ບໍ່ດັ່ງນັ້ນ
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

static const BIGNUM *ns_RSA_get0_n(const RSA *d)
{
	ສົ່ງຄືນ d->n;
}

static const BIGNUM *ns_RSA_get0_e(const RSA *d)
{
	ສົ່ງຄືນ d->e;
}
#  ບໍ່ດັ່ງນັ້ນ
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# ບໍ່ດັ່ງນັ້ນ
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

static ຈຳນວນເຕັມ ns_EVP_PKEY_get_bn_param(const EVP_PKEY *pkey,
		const var_na *key_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = NULL;

	/* Check parameters: only support allocation-form *bn */
	ຖ້າ (pkey == NULL || key_name == NULL || bn == NULL || *bn != NULL)
		ສົ່ງຄືນ 0;

	/* Only support RSA keys */
	ຖ້າ (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		ສົ່ງຄືນ 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	ຖ້າ (rsa == NULL)
		ສົ່ງຄືນ 0;

	ຖ້າ (strcmp(key_name, "n") == 0) {
		const BIGNUM *n = ns_RSA_get0_n(rsa);
		ຖ້າ (n != NULL)
			result = BN_dup(n);
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(key_name, "e") == 0) {
		const BIGNUM *e = ns_RSA_get0_e(rsa);
		ຖ້າ (e != NULL)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	ສົ່ງຄືນ (result != NULL) ? 1 : 0;
}

static ຈຳນວນເຕັມ ns_EVP_PKEY_get_utf8_string_param(const EVP_PKEY *pkey,
		const var_na *key_name, var_na *str, size_t max_len,
		size_t *out_len)
{
	const EC_GROUP *ecgroup;
	const var_na *group = "";
	size_t group_len = 0;
	EC_KEY *ec;
	ຈຳນວນເຕັມ ret = 0;

	ຖ້າ (pkey == NULL || key_name == NULL)
		ສົ່ງຄືນ 0;

	/* Only support EC keys */
	ຖ້າ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ສົ່ງຄືນ 0;

	/* Only support fetching the group */
	ຖ້າ (strcmp(key_name, "group") != 0)
		ສົ່ງຄືນ 0;

	ec = EVP_PKEY_get1_EC_KEY((EVP_PKEY *) pkey);

	ecgroup = EC_KEY_get0_group(ec);
	ຖ້າ (ecgroup != NULL) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	ຖ້າ (str != NULL && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	ຖ້າ (out_len != NULL) {
		*out_len = group_len;
	}

	EC_KEY_free(ec);

	ສົ່ງຄືນ ret;
}

static ຈຳນວນເຕັມ ns_EVP_PKEY_get_octet_string_param(const EVP_PKEY *pkey,
		const var_na *key_name, unsigned var_na *buf, size_t max_len,
		size_t *out_len)
{
	const EC_GROUP *ecgroup;
	const EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_KEY *ec;
	ຈຳນວນເຕັມ ret = 0;

	ຖ້າ (pkey == NULL || key_name == NULL)
		ສົ່ງຄືນ 0;

	/* Only support EC keys */
	ຖ້າ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ສົ່ງຄືນ 0;

	ຖ້າ (strcmp(key_name, "encoded-pub-key") != 0)
		ສົ່ງຄືນ 0;

	ec = EVP_PKEY_get1_EC_KEY((EVP_PKEY *) pkey);
	ຖ້າ (ec == NULL)
		ສົ່ງຄືນ 0;

	ecgroup = EC_KEY_get0_group(ec);
	ຖ້າ (ecgroup != NULL) {
		ecpoint = EC_KEY_get0_public_key(ec);
		ຖ້າ (ecpoint != NULL) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 NULL,
						 0,
						 bnctx);
			ຖ້າ (len != 0 && len <= max_len) {
				ຖ້າ (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			ຖ້າ (out_len != NULL)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_KEY_free(ec);

	ສົ່ງຄືນ ret;
}
#ບໍ່ດັ່ງນັ້ນ
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
	ຈຳນວນເຕັມ entryidx;
	ຈຳນວນເຕັມ entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *value;
	const unsigned var_na *value_str;
	ASN1_OBJECT *name;
	ຈຳນວນເຕັມ name_nid;
	var_na **field;

	entrycnt = X509_NAME_entry_count(xname);

	ສຳລັບ (entryidx = 0; entryidx < entrycnt; entryidx++) {
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
		ຖ້າ (field != NULL) {
			*field = strdup((const var_na *)value_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} ບໍ່ດັ່ງນັ້ນ {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	ຖ້າ (iname->common_name == NULL) {
		iname->common_name = strdup("Unknown");
	}

	ສົ່ງຄືນ NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
static var_na *hexdup(const var_na *hex)
{
	ຈຳນວນເຕັມ hexlen;
	var_na *dst;
	var_na *out;
	ຈຳນວນເຕັມ cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	ຖ້າ (dst != NULL) {
		ສຳລັບ (out = dst; *hex != 0; hex++) {
			ຖ້າ (cn == 2) {
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
	ສົ່ງຄືນ dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
static var_na *bindup(unsigned var_na *bin, unsigned ຈຳນວນເຕັມ binlen)
{
	var_na *dst;
	var_na *out;
	unsigned ຈຳນວນເຕັມ idx;
	const var_na hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	ຖ້າ (dst != NULL) {
		out = dst;
		ສຳລັບ (idx = 0; idx < binlen; idx++) {
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
	ສົ່ງຄືນ dst;
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

	ຖ້າ (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ສົ່ງຄືນ NSERROR_BAD_PARAMETER;
	}

	ຖ້າ (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ສົ່ງຄືນ NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	ຖ້າ (tmp != NULL) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	ຖ້າ (tmp != NULL) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ສົ່ງຄືນ NSERROR_OK;
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

	ສົ່ງຄືນ NSERROR_OK;
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

	ສົ່ງຄືນ NSERROR_OK;
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
	ຖ້າ (len != 0) {
		ikey->curve = malloc(len + 1);
		ຖ້າ (ikey->curve != NULL) {
			ຖ້າ (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, NULL) == 0) {
				free(ikey->curve);
				ikey->curve = NULL;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			NULL, 0, &len);
	ຖ້າ (len != 0) {
		unsigned var_na *point = malloc(len);
		ຖ້າ (point != NULL) {
			ຖ້າ (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					NULL) == 1) {
				ikey->public = bindup(point, len);
			}
			free(point);
		}
	}

	ສົ່ງຄືນ NSERROR_OK;
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

	ຖ້າ (pkey == NULL) {
		ສົ່ງຄືນ NSERROR_BAD_PARAMETER;
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

	ສົ່ງຄືນ res;
}

static nserror san_to_info(X509 *cert, sa_nracanaa ns_cert_san **prev_next)
{
	ຈຳນວນເຕັມ idx;
	ຈຳນວນເຕັມ san_names_nb = -1;
	const GENERAL_NAME *current_name;
	const unsigned var_na *dns_name;
	sa_nracanaa ns_cert_san *isan;

	STACK_OF(GENERAL_NAME) *san_names = NULL;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
	ຖ້າ (san_names == NULL) {
		ສົ່ງຄືນ NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_num(san_names);

	/* Check each name within the extension */
	ສຳລັບ (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_value(san_names, idx);

		ຖ້າ (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(parimaa_na(sa_nracanaa ns_cert_san));
			ຖ້າ (isan != NULL) {
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
	ສຳລັບ (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_pop(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	ສົ່ງຄືນ NSERROR_OK;
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

	ຖ້າ (der == NULL) {
		ສົ່ງຄືນ NSERROR_OK;
	}

	cert = d2i_X509(NULL, &der, der_length);
	ຖ້າ (cert == NULL) {
		ສົ່ງຄືນ NSERROR_INVALID;
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
	ຖ້າ (info->not_before != NULL) {
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
	ຖ້າ (info->not_after != NULL) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	ຈຳນວນເຕັມ pkey_nid = ns_X509_get_signature_nid(cert);
	ຖ້າ (pkey_nid != NID_undef) {
		const var_na* sslbuf = OBJ_nid2ln(pkey_nid);
		ຖ້າ (sslbuf != NULL) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_num = X509_get_serialNumber(cert);
	ຖ້າ (asn1_num != NULL) {
		bignum = ASN1_INTEGER_to_BN(asn1_num, NULL);
		ຖ້າ (bignum != NULL) {
			var_na *tmp = BN_bn2hex(bignum);
			ຖ້າ (tmp != NULL) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = NULL;
		}
	}

	/* fingerprints */
	const EVP_MD *digest;
	unsigned ຈຳນວນເຕັມ dig_len;
	unsigned var_na *buff;
	ຈຳນວນເຕັມ rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	ຖ້າ (buff != NULL) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ຖ້າ ((rc == 1) && (dig_len == (unsigned ຈຳນວນເຕັມ)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		free(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	ຖ້າ (buff != NULL) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ຖ້າ ((rc == 1) && (dig_len == (unsigned ຈຳນວນເຕັມ)EVP_MD_size(digest))) {
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

	ສົ່ງຄືນ NSERROR_OK;
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
	ຖ້າ (certs == NULL) {
		ສົ່ງຄືນ NSERROR_NOMEM;
	}

	ສຳລັບ (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		ຖ້າ (res != NSERROR_OK) {
			free(certs);
			ສົ່ງຄືນ res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	ສົ່ງຄືນ NSERROR_OK;
}

#ບໍ່ດັ່ງນັ້ນ
static nserror
convert_chain_to_cert_info(const sa_nracanaa cert_chain *chain,
			   sa_nracanaa ns_cert_info **cert_info_out)
{
	ສົ່ງຄືນ NSERROR_NOT_IMPLEMENTED;
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
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	ຖ້າ (cert_name->organisation != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (cert_name->organisation_unit != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (cert_name->locality != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (cert_name->province != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (cert_name->country != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ສົ່ງຄືນ res;
}

/**
 * output formatted certificate subject alternate names
 */
static nserror
format_certificate_san(sa_nracanaa fetch_about_context *ctx,
			      sa_nracanaa ns_cert_san *san)
{
	nserror res;

	ຖ້າ (san == NULL) {
		ສົ່ງຄືນ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	ໃນຂະນະທີ່ (san != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ສົ່ງຄືນ res;

}


static nserror
format_certificate_public_key(sa_nracanaa fetch_about_context *ctx,
			      sa_nracanaa ns_cert_pkey *public_key)
{
	nserror res;

	ຖ້າ (public_key->algor == NULL) {
		/* skip the table if no algorithm name */
		ສົ່ງຄືນ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_key->algor,
			 public_key->size);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}


	ຖ້າ (public_key->exponent != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_key->exponent);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (public_key->modulus != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_key->modulus);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (public_key->curve != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_key->curve);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (public_key->public != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_key->public);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ສົ່ງຄືນ res;
}

static nserror
format_certificate_fingerprint(sa_nracanaa fetch_about_context *ctx,
			      sa_nracanaa ns_cert_info *cert_info)
{
	nserror res;

	ຖ້າ ((cert_info->sha1fingerprint == NULL) &&
	    (cert_info->sha256fingerprint == NULL))  {
		/* skip the table if no fingerprints */
		ສົ່ງຄືນ NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	ຖ້າ (cert_info->sha256fingerprint != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (cert_info->sha1fingerprint != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ສົ່ງຄືນ res;
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
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	ຖ້າ (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = format_certificate_public_key(ctx, &cert_info->public_key);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	ຖ້າ (cert_info->serialnum != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	ຖ້າ (cert_info->sig_algor != NULL) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		ຖ້າ (res != NSERROR_OK) {
			ສົ່ງຄືນ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	ຖ້າ (res != NSERROR_OK) {
		ສົ່ງຄືນ res;
	}

	ສົ່ງຄືນ res;
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
	ຈຳນວນເຕັມ code = 200;
	nserror res;
	sa_nracanaa cert_chain *chain = NULL;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ຖ້າ (fetch_about_send_header(ctx, "Content-Type: text/html"))
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
	ຖ້າ (res != NSERROR_OK) {
		goto fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	ຖ້າ (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		ຖ້າ (res != NSERROR_OK) {
			goto fetch_about_certificate_handler_aborted;
		}
	} ບໍ່ດັ່ງນັ້ນ {
		sa_nracanaa ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		ຖ້າ (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			ຖ້າ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				goto fetch_about_certificate_handler_aborted;
			}

			ສຳລັບ (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				ຖ້າ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					goto fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			ຖ້າ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				goto fetch_about_certificate_handler_aborted;
			}

			ສຳລັບ (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				ຖ້າ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					goto fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} ບໍ່ດັ່ງນັ້ນ {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			ຖ້າ (res != NSERROR_OK) {
				goto fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	ຖ້າ (res != NSERROR_OK) {
		goto fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	ສົ່ງຄືນ true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ສົ່ງຄືນ false;
}
