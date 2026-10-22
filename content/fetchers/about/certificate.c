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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "netsurf/ssl_certs.h"

#અન્તર્ભાવ "private.h"
#અન્તર્ભાવ "certificate.h"

/**
 * certificate name parameters
 */
સંરચના ns_cert_name {
	અક્ષર *common_name;
	અક્ષર *organisation;
	અક્ષર *organisation_unit;
	અક્ષર *locality;
	અક્ષર *province;
	અક્ષર *country;
};

/**
 * Certificate public key parameters
 */
સંરચના ns_cert_pkey {
	અક્ષર *algor;
	પૂર્ણાંક size;
	અક્ષર *modulus;
	અક્ષર *exponent;
	અક્ષર *curve;
	અક્ષર *જાહેર;
};

/**
 * Certificate subject alternative name
 */
સંરચના ns_cert_san {
	સંરચના ns_cert_san *next;
	અક્ષર *name;
};

/**
 * certificate information for certificate chain
 */
સંરચના ns_cert_info {
	સંરચના ns_cert_name subject_name; /**< Subject details */
	સંરચના ns_cert_name issuer_name; /**< Issuer details */
	સંરચના ns_cert_pkey public_ચાવી; /**< public key details */
	long version;		/**< Certificate version */
	અક્ષર *not_before;	/**< Valid from date */
	અક્ષર *not_after;	/**< Valid to date */
	પૂર્ણાંક sig_type;		/**< Signature type */
	અક્ષર *sig_algor;        /**< Signature Algorithm */
	અક્ષર *serialnum;	/**< Serial number */
	અક્ષર *sha1fingerprint; /**< fingerprint shar1 encoded */
	અક્ષર *sha256fingerprint; /**< fingerprint shar256 encoded */
	સંરચના ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
સ્થિર nserror free_ns_cert_info(સંરચના ns_cert_info *cinfo)
{
	સંરચના ns_cert_san *san;

	મુક્ત(cinfo->subject_name.common_name);
	મુક્ત(cinfo->subject_name.organisation);
	મુક્ત(cinfo->subject_name.organisation_unit);
	મુક્ત(cinfo->subject_name.locality);
	મુક્ત(cinfo->subject_name.province);
	મુક્ત(cinfo->subject_name.country);
	મુક્ત(cinfo->issuer_name.common_name);
	મુક્ત(cinfo->issuer_name.organisation);
	મુક્ત(cinfo->issuer_name.organisation_unit);
	મુક્ત(cinfo->issuer_name.locality);
	મુક્ત(cinfo->issuer_name.province);
	મુક્ત(cinfo->issuer_name.country);
	મુક્ત(cinfo->public_ચાવી.algor);
	મુક્ત(cinfo->public_ચાવી.modulus);
	મુક્ત(cinfo->public_ચાવી.exponent);
	મુક્ત(cinfo->public_ચાવી.curve);
	મુક્ત(cinfo->public_ચાવી.જાહેર);
	મુક્ત(cinfo->not_before);
	મુક્ત(cinfo->not_after);
	મુક્ત(cinfo->sig_algor);
	મુક્ત(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	જ્યારે (san != ખાલી) {
		સંરચના ns_cert_san *next;
		next = san->next;
		મુક્ત(san);
		san = next;
	}

	મુક્ત(cinfo);

	પાછા NSERROR_OK;
}

#ifdef WITH_OPENSSL

#અન્તર્ભાવ <openssl/ssl.h>
#અન્તર્ભાવ <openssl/x509v3.h>

#જો (OPENSSL_VERSION_સંખ્યા < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# જો defined(LIBRESSL_VERSION_સંખ્યા)
  /* LibreSSL */
#  જો (LIBRESSL_VERSION_સંખ્યા < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   જો (LIBRESSL_VERSION_સંખ્યા < 0x2070000fL)
    /* LibreSSL <2.7.0 */
સ્થિર પૂર્ણાંક ns_X509_get_signature_nid(X509 *cert)
{
	પાછા OBJ_obj2nid(cert->cert_info->ચાવી->algor->algorithm);
}

સ્થિર અચલ unsigned અક્ષર *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	પાછા (અચલ unsigned અક્ષર *)ASN1_STRING_data(asn1str);
}
#   નહીંતર
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

સ્થિર અચલ BIGNUM *ns_RSA_get0_n(અચલ RSA *d)
{
	પાછા d->n;
}

સ્થિર અચલ BIGNUM *ns_RSA_get0_e(અચલ RSA *d)
{
	પાછા d->e;
}
#  નહીંતર
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# નહીંતર
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

સ્થિર પૂર્ણાંક ns_EVP_PKEY_get_bn_param(અચલ EVP_PKEY *pkey,
		અચલ અક્ષર *ચાવી_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = ખાલી;

	/* Check parameters: only support allocation-form *bn */
	જો (pkey == ખાલી || ચાવી_name == ખાલી || bn == ખાલી || *bn != ખાલી)
		પાછા 0;

	/* Only support RSA keys */
	જો (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		પાછા 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	જો (rsa == ખાલી)
		પાછા 0;

	જો (strcmp(ચાવી_name, "n") == 0) {
		અચલ BIGNUM *n = ns_RSA_get0_n(rsa);
		જો (n != ખાલી)
			result = BN_dup(n);
	} નહીંતર જો (strcmp(ચાવી_name, "e") == 0) {
		અચલ BIGNUM *e = ns_RSA_get0_e(rsa);
		જો (e != ખાલી)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	પાછા (result != ખાલી) ? 1 : 0;
}

સ્થિર પૂર્ણાંક ns_EVP_PKEY_get_utf8_string_param(અચલ EVP_PKEY *pkey,
		અચલ અક્ષર *ચાવી_name, અક્ષર *str, size_t max_len,
		size_t *out_len)
{
	અચલ EC_GROUP *ecgroup;
	અચલ અક્ષર *group = "";
	size_t group_len = 0;
	EC_ચાવી *ec;
	પૂર્ણાંક ret = 0;

	જો (pkey == ખાલી || ચાવી_name == ખાલી)
		પાછા 0;

	/* Only support EC keys */
	જો (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		પાછા 0;

	/* Only support fetching the group */
	જો (strcmp(ચાવી_name, "group") != 0)
		પાછા 0;

	ec = EVP_PKEY_get1_EC_ચાવી((EVP_PKEY *) pkey);

	ecgroup = EC_ચાવી_get0_group(ec);
	જો (ecgroup != ખાલી) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	જો (str != ખાલી && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	જો (out_len != ખાલી) {
		*out_len = group_len;
	}

	EC_ચાવી_free(ec);

	પાછા ret;
}

સ્થિર પૂર્ણાંક ns_EVP_PKEY_get_octet_string_param(અચલ EVP_PKEY *pkey,
		અચલ અક્ષર *ચાવી_name, unsigned અક્ષર *buf, size_t max_len,
		size_t *out_len)
{
	અચલ EC_GROUP *ecgroup;
	અચલ EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_ચાવી *ec;
	પૂર્ણાંક ret = 0;

	જો (pkey == ખાલી || ચાવી_name == ખાલી)
		પાછા 0;

	/* Only support EC keys */
	જો (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		પાછા 0;

	જો (strcmp(ચાવી_name, "encoded-pub-key") != 0)
		પાછા 0;

	ec = EVP_PKEY_get1_EC_ચાવી((EVP_PKEY *) pkey);
	જો (ec == ખાલી)
		પાછા 0;

	ecgroup = EC_ચાવી_get0_group(ec);
	જો (ecgroup != ખાલી) {
		ecpoint = EC_ચાવી_get0_public_ચાવી(ec);
		જો (ecpoint != ખાલી) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 ખાલી,
						 0,
						 bnctx);
			જો (len != 0 && len <= max_len) {
				જો (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			જો (out_len != ખાલી)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_ચાવી_free(ec);

	પાછા ret;
}
#નહીંતર
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
સ્થિર nserror
xname_to_info(X509_NAME *xname, સંરચના ns_cert_name *iname)
{
	પૂર્ણાંક entryidx;
	પૂર્ણાંક entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *કિંમત;
	અચલ unsigned અક્ષર *કિંમત_str;
	ASN1_OBJECT *name;
	પૂર્ણાંક name_nid;
	અક્ષર **field;

	entrycnt = X509_NAME_entry_ગણતરી(xname);

	માટે (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		કિંમત = X509_NAME_ENTRY_get_data(entry);
		કિંમત_str = ns_ASN1_STRING_get0_data(કિંમત);
		પસંદગી (name_nid) {
		સ્થિતિ NID_commonName:
			field = &iname->common_name;
			તોડો;
		સ્થિતિ NID_countryName:
			field = &iname->country;
			તોડો;
		સ્થિતિ NID_localityName:
			field = &iname->locality;
			તોડો;
		સ્થિતિ NID_stateOrProvinceName:
			field = &iname->province;
			તોડો;
		સ્થિતિ NID_organizationName:
			field = &iname->organisation;
			તોડો;
		સ્થિતિ NID_organizationalUnitName:
			field = &iname->organisation_unit;
			તોડો;
		મૂળભૂત :
			field = ખાલી;
			તોડો;
		}
		જો (field != ખાલી) {
			*field = strdup((અચલ અક્ષર *)કિંમત_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} નહીંતર {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	જો (iname->common_name == ખાલી) {
		iname->common_name = strdup("Unknown");
	}

	પાછા NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
સ્થિર અક્ષર *hexdup(અચલ અક્ષર *hex)
{
	પૂર્ણાંક hexlen;
	અક્ષર *dst;
	અક્ષર *out;
	પૂર્ણાંક cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	જો (dst != ખાલી) {
		માટે (out = dst; *hex != 0; hex++) {
			જો (cn == 2) {
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
	પાછા dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
સ્થિર અક્ષર *bindup(unsigned અક્ષર *bin, unsigned પૂર્ણાંક binlen)
{
	અક્ષર *dst;
	અક્ષર *out;
	unsigned પૂર્ણાંક idx;
	અચલ અક્ષર hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	જો (dst != ખાલી) {
		out = dst;
		માટે (idx = 0; idx < binlen; idx++) {
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
	પાછા dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
સ્થિર nserror
rsa_to_info(EVP_PKEY *pkey, સંરચના ns_cert_pkey *ikey)
{
	BIGNUM *n = ખાલી, *e = ખાલી;
	અક્ષર *tmp;

	જો (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		પાછા NSERROR_BAD_PARAMETER;
	}

	જો (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		પાછા NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	જો (tmp != ખાલી) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	જો (tmp != ખાલી) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	પાછા NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
સ્થિર nserror
dsa_to_info(EVP_PKEY *pkey, સંરચના ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	પાછા NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
સ્થિર nserror
dh_to_info(EVP_PKEY *pkey, સંરચના ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	પાછા NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
સ્થિર nserror
ec_to_info(EVP_PKEY *pkey, સંરચના ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ખાલી, 0, &len);
	જો (len != 0) {
		ikey->curve = malloc(len + 1);
		જો (ikey->curve != ખાલી) {
			જો (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, ખાલી) == 0) {
				મુક્ત(ikey->curve);
				ikey->curve = ખાલી;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ખાલી, 0, &len);
	જો (len != 0) {
		unsigned અક્ષર *point = malloc(len);
		જો (point != ખાલી) {
			જો (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					ખાલી) == 1) {
				ikey->જાહેર = bindup(point, len);
			}
			મુક્ત(point);
		}
	}

	પાછા NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
સ્થિર nserror
pkey_to_info(EVP_PKEY *pkey, સંરચના ns_cert_pkey *ikey)
{
	nserror res;

	જો (pkey == ખાલી) {
		પાછા NSERROR_BAD_PARAMETER;
	}

	પસંદગી (EVP_PKEY_base_id(pkey)) {
	સ્થિતિ EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		તોડો;

	સ્થિતિ EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		તોડો;

	સ્થિતિ EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		તોડો;

	સ્થિતિ EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		તોડો;

	મૂળભૂત:
		res = NSERROR_NOT_IMPLEMENTED;
		તોડો;
	}

	EVP_PKEY_free(pkey);

	પાછા res;
}

સ્થિર nserror san_to_info(X509 *cert, સંરચના ns_cert_san **prev_next)
{
	પૂર્ણાંક idx;
	પૂર્ણાંક san_names_nb = -1;
	અચલ GENERAL_NAME *current_name;
	અચલ unsigned અક્ષર *dns_name;
	સંરચના ns_cert_san *isan;

	ઢગલો_OF(GENERAL_NAME) *san_names = ખાલી;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, ખાલી, ખાલી);
	જો (san_names == ખાલી) {
		પાછા NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_અંક(san_names);

	/* Check each name within the extension */
	માટે (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_કિંમત(san_names, idx);

		જો (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(કદ(સંરચના ns_cert_san));
			જો (isan != ખાલી) {
				isan->name = strdup((અચલ અક્ષર *)dns_name);
				isan->next = ખાલી;
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
	માટે (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_કાઢો(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	પાછા NSERROR_OK;
}

સ્થિર nserror
der_to_certinfo(અચલ uint8_t *der,
		size_t der_length,
		સંરચના ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	અચલ ASN1_INTEGER *asn1_અંક;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	જો (der == ખાલી) {
		પાછા NSERROR_OK;
	}

	cert = d2i_X509(ખાલી, &der, der_length);
	જો (cert == ખાલી) {
		પાછા NSERROR_INVALID;
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
	(શૂન્ય) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	જો (info->not_before != ખાલી) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(શૂન્ય) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	જો (info->not_after != ખાલી) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	પૂર્ણાંક pkey_nid = ns_X509_get_signature_nid(cert);
	જો (pkey_nid != NID_undef) {
		અચલ અક્ષર* sslbuf = OBJ_nid2ln(pkey_nid);
		જો (sslbuf != ખાલી) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_અંક = X509_get_serial_સંખ્યા(cert);
	જો (asn1_અંક != ખાલી) {
		bignum = ASN1_INTEGER_to_BN(asn1_અંક, ખાલી);
		જો (bignum != ખાલી) {
			અક્ષર *tmp = BN_bn2hex(bignum);
			જો (tmp != ખાલી) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ખાલી;
		}
	}

	/* fingerprints */
	અચલ EVP_MD *digest;
	unsigned પૂર્ણાંક dig_len;
	unsigned અક્ષર *buff;
	પૂર્ણાંક rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	જો (buff != ખાલી) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		જો ((rc == 1) && (dig_len == (unsigned પૂર્ણાંક)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		મુક્ત(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	જો (buff != ખાલી) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		જો ((rc == 1) && (dig_len == (unsigned પૂર્ણાંક)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		મુક્ત(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_ચાવી);

	X509_free(cert);

	પાછા NSERROR_OK;
}

/* copy certificate data */
સ્થિર nserror
convert_chain_to_cert_info(અચલ સંરચના cert_chain *chain,
			   સંરચના ns_cert_info **cert_info_out)
{
	સંરચના ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, કદ(સંરચના ns_cert_info));
	જો (certs == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	માટે (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		જો (res != NSERROR_OK) {
			મુક્ત(certs);
			પાછા res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	પાછા NSERROR_OK;
}

#નહીંતર
સ્થિર nserror
convert_chain_to_cert_info(અચલ સંરચના cert_chain *chain,
			   સંરચના ns_cert_info **cert_info_out)
{
	પાછા NSERROR_NOT_IMPLEMENTED;
}
#endif


સ્થિર nserror
format_certificate_name(સંરચના fetch_about_context *ctx,
			સંરચના ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	જો (cert_name->organisation != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (cert_name->organisation_unit != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (cert_name->locality != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (cert_name->province != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (cert_name->country != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	પાછા res;
}

/**
 * output formatted certificate subject alternate names
 */
સ્થિર nserror
format_certificate_san(સંરચના fetch_about_context *ctx,
			      સંરચના ns_cert_san *san)
{
	nserror res;

	જો (san == ખાલી) {
		પાછા NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	જ્યારે (san != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		જો (res != NSERROR_OK) {
			પાછા res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	પાછા res;

}


સ્થિર nserror
format_certificate_public_ચાવી(સંરચના fetch_about_context *ctx,
			      સંરચના ns_cert_pkey *public_ચાવી)
{
	nserror res;

	જો (public_ચાવી->algor == ખાલી) {
		/* skip the table if no algorithm name */
		પાછા NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_ચાવી->algor,
			 public_ચાવી->size);
	જો (res != NSERROR_OK) {
		પાછા res;
	}


	જો (public_ચાવી->exponent != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_ચાવી->exponent);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (public_ચાવી->modulus != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_ચાવી->modulus);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (public_ચાવી->curve != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_ચાવી->curve);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (public_ચાવી->જાહેર != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_ચાવી->જાહેર);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	પાછા res;
}

સ્થિર nserror
format_certificate_fingerprint(સંરચના fetch_about_context *ctx,
			      સંરચના ns_cert_info *cert_info)
{
	nserror res;

	જો ((cert_info->sha1fingerprint == ખાલી) &&
	    (cert_info->sha256fingerprint == ખાલી))  {
		/* skip the table if no fingerprints */
		પાછા NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	જો (cert_info->sha256fingerprint != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (cert_info->sha1fingerprint != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	પાછા res;
}

સ્થિર nserror
format_certificate(સંરચના fetch_about_context *ctx,
		   સંરચના ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	જો (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = format_certificate_public_ચાવી(ctx, &cert_info->public_ચાવી);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	જો (cert_info->serialnum != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	જો (cert_info->sig_algor != ખાલી) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	પાછા res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(સંરચના fetch_about_context *ctx)
{
	પૂર્ણાંક code = 200;
	nserror res;
	સંરચના cert_chain *chain = ખાલી;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	જો (fetch_about_send_header(ctx, "Content-Type: text/html"))
		જાઓ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	જો (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		જો (res != NSERROR_OK) {
			જાઓ fetch_about_certificate_handler_aborted;
		}
	} નહીંતર {
		સંરચના ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		જો (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			જો (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				જાઓ fetch_about_certificate_handler_aborted;
			}

			માટે (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				જો (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					જાઓ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			જો (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				જાઓ fetch_about_certificate_handler_aborted;
			}

			માટે (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				જો (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					જાઓ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} નહીંતર {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			જો (res != NSERROR_OK) {
				જાઓ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	પાછા true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	પાછા false;
}
