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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/ssl_certs.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "certificate.h"

/**
 * certificate name parameters
 */
ସଂରଚନା ns_cert_name {
	ଅକ୍ଷର *common_name;
	ଅକ୍ଷର *organisation;
	ଅକ୍ଷର *organisation_unit;
	ଅକ୍ଷର *locality;
	ଅକ୍ଷର *province;
	ଅକ୍ଷର *country;
};

/**
 * Certificate public key parameters
 */
ସଂରଚନା ns_cert_pkey {
	ଅକ୍ଷର *algor;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;
	ଅକ୍ଷର *modulus;
	ଅକ୍ଷର *exponent;
	ଅକ୍ଷର *curve;
	ଅକ୍ଷର *ସର୍ବସାଧାରଣ;
};

/**
 * Certificate subject alternative name
 */
ସଂରଚନା ns_cert_san {
	ସଂରଚନା ns_cert_san *next;
	ଅକ୍ଷର *name;
};

/**
 * certificate information for certificate chain
 */
ସଂରଚନା ns_cert_info {
	ସଂରଚନା ns_cert_name subject_name; /**< Subject details */
	ସଂରଚନା ns_cert_name issuer_name; /**< Issuer details */
	ସଂରଚନା ns_cert_pkey public_ଚାବି; /**< public key details */
	long version;		/**< Certificate version */
	ଅକ୍ଷର *not_before;	/**< Valid from date */
	ଅକ୍ଷର *not_after;	/**< Valid to date */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig_type;		/**< Signature type */
	ଅକ୍ଷର *sig_algor;        /**< Signature Algorithm */
	ଅକ୍ଷର *serialnum;	/**< Serial number */
	ଅକ୍ଷର *sha1fingerprint; /**< fingerprint shar1 encoded */
	ଅକ୍ଷର *sha256fingerprint; /**< fingerprint shar256 encoded */
	ସଂରଚନା ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
ସ୍ଥିର nserror free_ns_cert_info(ସଂରଚନା ns_cert_info *cinfo)
{
	ସଂରଚନା ns_cert_san *san;

	ମୁକ୍ତ(cinfo->subject_name.common_name);
	ମୁକ୍ତ(cinfo->subject_name.organisation);
	ମୁକ୍ତ(cinfo->subject_name.organisation_unit);
	ମୁକ୍ତ(cinfo->subject_name.locality);
	ମୁକ୍ତ(cinfo->subject_name.province);
	ମୁକ୍ତ(cinfo->subject_name.country);
	ମୁକ୍ତ(cinfo->issuer_name.common_name);
	ମୁକ୍ତ(cinfo->issuer_name.organisation);
	ମୁକ୍ତ(cinfo->issuer_name.organisation_unit);
	ମୁକ୍ତ(cinfo->issuer_name.locality);
	ମୁକ୍ତ(cinfo->issuer_name.province);
	ମୁକ୍ତ(cinfo->issuer_name.country);
	ମୁକ୍ତ(cinfo->public_ଚାବି.algor);
	ମୁକ୍ତ(cinfo->public_ଚାବି.modulus);
	ମୁକ୍ତ(cinfo->public_ଚାବି.exponent);
	ମୁକ୍ତ(cinfo->public_ଚାବି.curve);
	ମୁକ୍ତ(cinfo->public_ଚାବି.ସର୍ବସାଧାରଣ);
	ମୁକ୍ତ(cinfo->not_before);
	ମୁକ୍ତ(cinfo->not_after);
	ମୁକ୍ତ(cinfo->sig_algor);
	ମୁକ୍ତ(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	ଯେତେବେଳେ (san != ଖାଲି) {
		ସଂରଚନା ns_cert_san *next;
		next = san->next;
		ମୁକ୍ତ(san);
		san = next;
	}

	ମୁକ୍ତ(cinfo);

	ଫେରନ୍ତୁ NSERROR_OK;
}

#ifdef WITH_OPENSSL

#ଅନ୍ତର୍ଭାଵ <openssl/ssl.h>
#ଅନ୍ତର୍ଭାଵ <openssl/x509v3.h>

#ଯଦି (OPENSSL_VERSION_ସଂଖ୍ୟା < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# ଯଦି defined(LIBRESSL_VERSION_ସଂଖ୍ୟା)
  /* LibreSSL */
#  ଯଦି (LIBRESSL_VERSION_ସଂଖ୍ୟା < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   ଯଦି (LIBRESSL_VERSION_ସଂଖ୍ୟା < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ns_X509_get_signature_nid(X509 *cert)
{
	ଫେରନ୍ତୁ OBJ_obj2nid(cert->cert_info->ଚାବି->algor->algorithm);
}

ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	ଫେରନ୍ତୁ (ଅଚଳ unsigned ଅକ୍ଷର *)ASN1_STRING_data(asn1str);
}
#   ଅନ୍ୟଥା
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

ସ୍ଥିର ଅଚଳ BIGNUM *ns_RSA_get0_n(ଅଚଳ RSA *d)
{
	ଫେରନ୍ତୁ d->n;
}

ସ୍ଥିର ଅଚଳ BIGNUM *ns_RSA_get0_e(ଅଚଳ RSA *d)
{
	ଫେରନ୍ତୁ d->e;
}
#  ଅନ୍ୟଥା
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# ଅନ୍ୟଥା
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ns_EVP_PKEY_get_bn_param(ଅଚଳ EVP_PKEY *pkey,
		ଅଚଳ ଅକ୍ଷର *ଚାବି_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = ଖାଲି;

	/* Check parameters: only support allocation-form *bn */
	ଯଦି (pkey == ଖାଲି || ଚାବି_name == ଖାଲି || bn == ଖାଲି || *bn != ଖାଲି)
		ଫେରନ୍ତୁ 0;

	/* Only support RSA keys */
	ଯଦି (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		ଫେରନ୍ତୁ 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	ଯଦି (rsa == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	ଯଦି (strcmp(ଚାବି_name, "n") == 0) {
		ଅଚଳ BIGNUM *n = ns_RSA_get0_n(rsa);
		ଯଦି (n != ଖାଲି)
			result = BN_dup(n);
	} ଅନ୍ୟଥା ଯଦି (strcmp(ଚାବି_name, "e") == 0) {
		ଅଚଳ BIGNUM *e = ns_RSA_get0_e(rsa);
		ଯଦି (e != ଖାଲି)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	ଫେରନ୍ତୁ (result != ଖାଲି) ? 1 : 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ns_EVP_PKEY_get_utf8_string_param(ଅଚଳ EVP_PKEY *pkey,
		ଅଚଳ ଅକ୍ଷର *ଚାବି_name, ଅକ୍ଷର *str, size_t max_len,
		size_t *out_len)
{
	ଅଚଳ EC_GROUP *ecgroup;
	ଅଚଳ ଅକ୍ଷର *group = "";
	size_t group_len = 0;
	EC_ଚାବି *ec;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	ଯଦି (pkey == ଖାଲି || ଚାବି_name == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	/* Only support EC keys */
	ଯଦି (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ଫେରନ୍ତୁ 0;

	/* Only support fetching the group */
	ଯଦି (strcmp(ଚାବି_name, "group") != 0)
		ଫେରନ୍ତୁ 0;

	ec = EVP_PKEY_get1_EC_ଚାବି((EVP_PKEY *) pkey);

	ecgroup = EC_ଚାବି_get0_group(ec);
	ଯଦି (ecgroup != ଖାଲି) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	ଯଦି (str != ଖାଲି && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	ଯଦି (out_len != ଖାଲି) {
		*out_len = group_len;
	}

	EC_ଚାବି_free(ec);

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ns_EVP_PKEY_get_octet_string_param(ଅଚଳ EVP_PKEY *pkey,
		ଅଚଳ ଅକ୍ଷର *ଚାବି_name, unsigned ଅକ୍ଷର *buf, size_t max_len,
		size_t *out_len)
{
	ଅଚଳ EC_GROUP *ecgroup;
	ଅଚଳ EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_ଚାବି *ec;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	ଯଦି (pkey == ଖାଲି || ଚାବି_name == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	/* Only support EC keys */
	ଯଦି (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ଫେରନ୍ତୁ 0;

	ଯଦି (strcmp(ଚାବି_name, "encoded-pub-key") != 0)
		ଫେରନ୍ତୁ 0;

	ec = EVP_PKEY_get1_EC_ଚାବି((EVP_PKEY *) pkey);
	ଯଦି (ec == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	ecgroup = EC_ଚାବି_get0_group(ec);
	ଯଦି (ecgroup != ଖାଲି) {
		ecpoint = EC_ଚାବି_get0_public_ଚାବି(ec);
		ଯଦି (ecpoint != ଖାଲି) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 ଖାଲି,
						 0,
						 bnctx);
			ଯଦି (len != 0 && len <= max_len) {
				ଯଦି (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			ଯଦି (out_len != ଖାଲି)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_ଚାବି_free(ec);

	ଫେରନ୍ତୁ ret;
}
#ଅନ୍ୟଥା
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
ସ୍ଥିର nserror
xname_to_info(X509_NAME *xname, ସଂରଚନା ns_cert_name *iname)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା entryidx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *ମୂଲ୍ୟ;
	ଅଚଳ unsigned ଅକ୍ଷର *ମୂଲ୍ୟ_str;
	ASN1_OBJECT *name;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା name_nid;
	ଅକ୍ଷର **field;

	entrycnt = X509_NAME_entry_ଗଣନା(xname);

	ପାଇଁ (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		ମୂଲ୍ୟ = X509_NAME_ENTRY_get_data(entry);
		ମୂଲ୍ୟ_str = ns_ASN1_STRING_get0_data(ମୂଲ୍ୟ);
		ବାଛ (name_nid) {
		ସ୍ଥିତି NID_commonName:
			field = &iname->common_name;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NID_countryName:
			field = &iname->country;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NID_localityName:
			field = &iname->locality;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NID_stateOrProvinceName:
			field = &iname->province;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NID_organizationName:
			field = &iname->organisation;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NID_organizationalUnitName:
			field = &iname->organisation_unit;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ :
			field = ଖାଲି;
			ଭାଙ୍ଗ;
		}
		ଯଦି (field != ଖାଲି) {
			*field = strdup((ଅଚଳ ଅକ୍ଷର *)ମୂଲ୍ୟ_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} ଅନ୍ୟଥା {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	ଯଦି (iname->common_name == ଖାଲି) {
		iname->common_name = strdup("Unknown");
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ସ୍ଥିର ଅକ୍ଷର *hexdup(ଅଚଳ ଅକ୍ଷର *hex)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hexlen;
	ଅକ୍ଷର *dst;
	ଅକ୍ଷର *out;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	ଯଦି (dst != ଖାଲି) {
		ପାଇଁ (out = dst; *hex != 0; hex++) {
			ଯଦି (cn == 2) {
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
	ଫେରନ୍ତୁ dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ସ୍ଥିର ଅକ୍ଷର *bindup(unsigned ଅକ୍ଷର *bin, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା binlen)
{
	ଅକ୍ଷର *dst;
	ଅକ୍ଷର *out;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;
	ଅଚଳ ଅକ୍ଷର hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	ଯଦି (dst != ଖାଲି) {
		out = dst;
		ପାଇଁ (idx = 0; idx < binlen; idx++) {
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
	ଫେରନ୍ତୁ dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ସ୍ଥିର nserror
rsa_to_info(EVP_PKEY *pkey, ସଂରଚନା ns_cert_pkey *ikey)
{
	BIGNUM *n = ଖାଲି, *e = ଖାଲି;
	ଅକ୍ଷର *tmp;

	ଯଦି (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	ଯଦି (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	ଯଦି (tmp != ଖାଲି) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	ଯଦି (tmp != ଖାଲି) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ସ୍ଥିର nserror
dsa_to_info(EVP_PKEY *pkey, ସଂରଚନା ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ସ୍ଥିର nserror
dh_to_info(EVP_PKEY *pkey, ସଂରଚନା ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ସ୍ଥିର nserror
ec_to_info(EVP_PKEY *pkey, ସଂରଚନା ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ଖାଲି, 0, &len);
	ଯଦି (len != 0) {
		ikey->curve = malloc(len + 1);
		ଯଦି (ikey->curve != ଖାଲି) {
			ଯଦି (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, ଖାଲି) == 0) {
				ମୁକ୍ତ(ikey->curve);
				ikey->curve = ଖାଲି;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ଖାଲି, 0, &len);
	ଯଦି (len != 0) {
		unsigned ଅକ୍ଷର *point = malloc(len);
		ଯଦି (point != ଖାଲି) {
			ଯଦି (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					ଖାଲି) == 1) {
				ikey->ସର୍ବସାଧାରଣ = bindup(point, len);
			}
			ମୁକ୍ତ(point);
		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ସ୍ଥିର nserror
pkey_to_info(EVP_PKEY *pkey, ସଂରଚନା ns_cert_pkey *ikey)
{
	nserror res;

	ଯଦି (pkey == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	ବାଛ (EVP_PKEY_base_id(pkey)) {
	ସ୍ଥିତି EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		res = NSERROR_NOT_IMPLEMENTED;
		ଭାଙ୍ଗ;
	}

	EVP_PKEY_free(pkey);

	ଫେରନ୍ତୁ res;
}

ସ୍ଥିର nserror san_to_info(X509 *cert, ସଂରଚନା ns_cert_san **prev_next)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା san_names_nb = -1;
	ଅଚଳ GENERAL_NAME *current_name;
	ଅଚଳ unsigned ଅକ୍ଷର *dns_name;
	ସଂରଚନା ns_cert_san *isan;

	ଗଦା_OF(GENERAL_NAME) *san_names = ଖାଲି;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, ଖାଲି, ଖାଲି);
	ଯଦି (san_names == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_ଅଙ୍କ(san_names);

	/* Check each name within the extension */
	ପାଇଁ (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_ମୂଲ୍ୟ(san_names, idx);

		ଯଦି (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(ଆକାର(ସଂରଚନା ns_cert_san));
			ଯଦି (isan != ଖାଲି) {
				isan->name = strdup((ଅଚଳ ଅକ୍ଷର *)dns_name);
				isan->next = ଖାଲି;
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
	ପାଇଁ (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_କାଢ଼(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror
der_to_certinfo(ଅଚଳ uint8_t *der,
		size_t der_length,
		ସଂରଚନା ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ଅଚଳ ASN1_INTEGER *asn1_ଅଙ୍କ;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	ଯଦି (der == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	cert = d2i_X509(ଖାଲି, &der, der_length);
	ଯଦି (cert == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_INVALID;
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
	(ଶୂନ୍ୟ) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	ଯଦି (info->not_before != ଖାଲି) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ଶୂନ୍ୟ) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	ଯଦି (info->not_after != ଖାଲି) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pkey_nid = ns_X509_get_signature_nid(cert);
	ଯଦି (pkey_nid != NID_undef) {
		ଅଚଳ ଅକ୍ଷର* sslbuf = OBJ_nid2ln(pkey_nid);
		ଯଦି (sslbuf != ଖାଲି) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_ଅଙ୍କ = X509_get_serial_ସଂଖ୍ୟା(cert);
	ଯଦି (asn1_ଅଙ୍କ != ଖାଲି) {
		bignum = ASN1_INTEGER_to_BN(asn1_ଅଙ୍କ, ଖାଲି);
		ଯଦି (bignum != ଖାଲି) {
			ଅକ୍ଷର *tmp = BN_bn2hex(bignum);
			ଯଦି (tmp != ଖାଲି) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ଖାଲି;
		}
	}

	/* fingerprints */
	ଅଚଳ EVP_MD *digest;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା dig_len;
	unsigned ଅକ୍ଷର *buff;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	ଯଦି (buff != ଖାଲି) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ଯଦି ((rc == 1) && (dig_len == (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		ମୁକ୍ତ(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	ଯଦି (buff != ଖାଲି) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ଯଦି ((rc == 1) && (dig_len == (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		ମୁକ୍ତ(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_ଚାବି);

	X509_free(cert);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* copy certificate data */
ସ୍ଥିର nserror
convert_chain_to_cert_info(ଅଚଳ ସଂରଚନା cert_chain *chain,
			   ସଂରଚନା ns_cert_info **cert_info_out)
{
	ସଂରଚନା ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, ଆକାର(ସଂରଚନା ns_cert_info));
	ଯଦି (certs == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ପାଇଁ (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		ଯଦି (res != NSERROR_OK) {
			ମୁକ୍ତ(certs);
			ଫେରନ୍ତୁ res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	ଫେରନ୍ତୁ NSERROR_OK;
}

#ଅନ୍ୟଥା
ସ୍ଥିର nserror
convert_chain_to_cert_info(ଅଚଳ ସଂରଚନା cert_chain *chain,
			   ସଂରଚନା ns_cert_info **cert_info_out)
{
	ଫେରନ୍ତୁ NSERROR_NOT_IMPLEMENTED;
}
#endif


ସ୍ଥିର nserror
format_certificate_name(ସଂରଚନା fetch_about_context *ctx,
			ସଂରଚନା ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯଦି (cert_name->organisation != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (cert_name->organisation_unit != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (cert_name->locality != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (cert_name->province != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (cert_name->country != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଫେରନ୍ତୁ res;
}

/**
 * output formatted certificate subject alternate names
 */
ସ୍ଥିର nserror
format_certificate_san(ସଂରଚନା fetch_about_context *ctx,
			      ସଂରଚନା ns_cert_san *san)
{
	nserror res;

	ଯଦି (san == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯେତେବେଳେ (san != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ଫେରନ୍ତୁ res;

}


ସ୍ଥିର nserror
format_certificate_public_ଚାବି(ସଂରଚନା fetch_about_context *ctx,
			      ସଂରଚନା ns_cert_pkey *public_ଚାବି)
{
	nserror res;

	ଯଦି (public_ଚାବି->algor == ଖାଲି) {
		/* skip the table if no algorithm name */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_ଚାବି->algor,
			 public_ଚାବି->size);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}


	ଯଦି (public_ଚାବି->exponent != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_ଚାବି->exponent);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (public_ଚାବି->modulus != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_ଚାବି->modulus);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (public_ଚାବି->curve != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_ଚାବି->curve);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (public_ଚାବି->ସର୍ବସାଧାରଣ != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_ଚାବି->ସର୍ବସାଧାରଣ);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ଫେରନ୍ତୁ res;
}

ସ୍ଥିର nserror
format_certificate_fingerprint(ସଂରଚନା fetch_about_context *ctx,
			      ସଂରଚନା ns_cert_info *cert_info)
{
	nserror res;

	ଯଦି ((cert_info->sha1fingerprint == ଖାଲି) &&
	    (cert_info->sha256fingerprint == ଖାଲି))  {
		/* skip the table if no fingerprints */
		ଫେରନ୍ତୁ NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯଦି (cert_info->sha256fingerprint != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (cert_info->sha1fingerprint != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ଫେରନ୍ତୁ res;
}

ସ୍ଥିର nserror
format_certificate(ସଂରଚନା fetch_about_context *ctx,
		   ସଂରଚନା ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯଦି (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = format_certificate_public_ଚାବି(ctx, &cert_info->public_ଚାବି);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯଦି (cert_info->serialnum != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଯଦି (cert_info->sig_algor != ଖାଲି) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଫେରନ୍ତୁ res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(ସଂରଚନା fetch_about_context *ctx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = 200;
	nserror res;
	ସଂରଚନା cert_chain *chain = ଖାଲି;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ଯଦି (fetch_about_send_header(ctx, "Content-Type: text/html"))
		ଯାଅ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	ଯଦି (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		ଯଦି (res != NSERROR_OK) {
			ଯାଅ fetch_about_certificate_handler_aborted;
		}
	} ଅନ୍ୟଥା {
		ସଂରଚନା ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		ଯଦି (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			ଯଦି (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ଯାଅ fetch_about_certificate_handler_aborted;
			}

			ପାଇଁ (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				ଯଦି (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ଯାଅ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			ଯଦି (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ଯାଅ fetch_about_certificate_handler_aborted;
			}

			ପାଇଁ (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				ଯଦି (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ଯାଅ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} ଅନ୍ୟଥା {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			ଯଦି (res != NSERROR_OK) {
				ଯାଅ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	ଫେରନ୍ତୁ true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ଫେରନ୍ତୁ false;
}
