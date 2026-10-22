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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/ssl_certs.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "certificate.h"

/**
 * certificate name parameters
 */
ਬਣਤਰ ns_cert_name {
	ਅੱਖਰ *common_name;
	ਅੱਖਰ *organisation;
	ਅੱਖਰ *organisation_unit;
	ਅੱਖਰ *locality;
	ਅੱਖਰ *province;
	ਅੱਖਰ *country;
};

/**
 * Certificate public key parameters
 */
ਬਣਤਰ ns_cert_pkey {
	ਅੱਖਰ *algor;
	ਪੂਰਨ ਮਾਪ;
	ਅੱਖਰ *modulus;
	ਅੱਖਰ *exponent;
	ਅੱਖਰ *ਵਕਰ;
	ਅੱਖਰ *ਜਨਤਕ;
};

/**
 * Certificate subject alternative name
 */
ਬਣਤਰ ns_cert_san {
	ਬਣਤਰ ns_cert_san *ਅਗਲਾ;
	ਅੱਖਰ *name;
};

/**
 * certificate information for certificate chain
 */
ਬਣਤਰ ns_cert_info {
	ਬਣਤਰ ns_cert_name subject_name; /**< Subject details */
	ਬਣਤਰ ns_cert_name issuer_name; /**< Issuer details */
	ਬਣਤਰ ns_cert_pkey ਜਨਤਕ_ਕੁੰਜੀ; /**< public key details */
	ਲੰਮਾ ਸੰਸਕਰਣ;		/**< Certificate version */
	ਅੱਖਰ *not_before;	/**< Valid from date */
	ਅੱਖਰ *not_after;	/**< Valid to date */
	ਪੂਰਨ sig_type;		/**< Signature type */
	ਅੱਖਰ *sig_algor;        /**< Signature Algorithm */
	ਅੱਖਰ *serialnum;	/**< Serial number */
	ਅੱਖਰ *sha1fingerprint; /**< fingerprint shar1 encoded */
	ਅੱਖਰ *sha256fingerprint; /**< fingerprint shar256 encoded */
	ਬਣਤਰ ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
ਸਥਿਰ nserror free_ns_cert_info(ਬਣਤਰ ns_cert_info *cinfo)
{
	ਬਣਤਰ ns_cert_san *san;

	ਮੁਕਤ(cinfo->subject_name.common_name);
	ਮੁਕਤ(cinfo->subject_name.organisation);
	ਮੁਕਤ(cinfo->subject_name.organisation_unit);
	ਮੁਕਤ(cinfo->subject_name.locality);
	ਮੁਕਤ(cinfo->subject_name.province);
	ਮੁਕਤ(cinfo->subject_name.country);
	ਮੁਕਤ(cinfo->issuer_name.common_name);
	ਮੁਕਤ(cinfo->issuer_name.organisation);
	ਮੁਕਤ(cinfo->issuer_name.organisation_unit);
	ਮੁਕਤ(cinfo->issuer_name.locality);
	ਮੁਕਤ(cinfo->issuer_name.province);
	ਮੁਕਤ(cinfo->issuer_name.country);
	ਮੁਕਤ(cinfo->ਜਨਤਕ_ਕੁੰਜੀ.algor);
	ਮੁਕਤ(cinfo->ਜਨਤਕ_ਕੁੰਜੀ.modulus);
	ਮੁਕਤ(cinfo->ਜਨਤਕ_ਕੁੰਜੀ.exponent);
	ਮੁਕਤ(cinfo->ਜਨਤਕ_ਕੁੰਜੀ.ਵਕਰ);
	ਮੁਕਤ(cinfo->ਜਨਤਕ_ਕੁੰਜੀ.ਜਨਤਕ);
	ਮੁਕਤ(cinfo->not_before);
	ਮੁਕਤ(cinfo->not_after);
	ਮੁਕਤ(cinfo->sig_algor);
	ਮੁਕਤ(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	ਜਦੋਂ (san != ਨਿਰਾ) {
		ਬਣਤਰ ns_cert_san *ਅਗਲਾ;
		ਅਗਲਾ = san->ਅਗਲਾ;
		ਮੁਕਤ(san);
		san = ਅਗਲਾ;
	}

	ਮੁਕਤ(cinfo);

	ਵਾਪਸ NSERROR_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_OPENSSL

#ਅਨ੍ਤਰ੍ਭਾਵ <openssl/ssl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <openssl/x509v3.h>

#ਜੇ (OPENSSL_ਸੰਸਕਰਣ_ਸੰਖਿਆ < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# ਜੇ defined(LIBRESSL_ਸੰਸਕਰਣ_ਸੰਖਿਆ)
  /* LibreSSL */
#  ਜੇ (LIBRESSL_ਸੰਸਕਰਣ_ਸੰਖਿਆ < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   ਜੇ (LIBRESSL_ਸੰਸਕਰਣ_ਸੰਖਿਆ < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ਸਥਿਰ ਪੂਰਨ ns_X509_get_signature_nid(X509 *cert)
{
	ਵਾਪਸ OBJ_obj2nid(cert->cert_info->ਕੁੰਜੀ->algor->algorithm);
}

ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ns_ASN1_STRING_get0_ਅੰਕੜੇ(ASN1_STRING *asn1str)
{
	ਵਾਪਸ (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ASN1_STRING_ਅੰਕੜੇ(asn1str);
}
#   ਨਹੀਂਤਾਂ
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_ਅੰਕੜੇ ASN1_STRING_get0_ਅੰਕੜੇ
#   endif

ਸਥਿਰ ਅਚਲ BIGNUM *ns_RSA_get0_n(ਅਚਲ RSA *d)
{
	ਵਾਪਸ d->n;
}

ਸਥਿਰ ਅਚਲ BIGNUM *ns_RSA_get0_e(ਅਚਲ RSA *d)
{
	ਵਾਪਸ d->e;
}
#  ਨਹੀਂਤਾਂ
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_ਅੰਕੜੇ ASN1_STRING_get0_ਅੰਕੜੇ
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# ਨਹੀਂਤਾਂ
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_ਅੰਕੜੇ ASN1_STRING_get0_ਅੰਕੜੇ
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ਸਥਿਰ ਪੂਰਨ ns_EVP_PKEY_get_bn_param(ਅਚਲ EVP_PKEY *pkey,
		ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *ਨਤੀਜਾ = ਨਿਰਾ;

	/* Check parameters: only support allocation-form *bn */
	ਜੇ (pkey == ਨਿਰਾ || ਕੁੰਜੀ_name == ਨਿਰਾ || bn == ਨਿਰਾ || *bn != ਨਿਰਾ)
		ਵਾਪਸ 0;

	/* Only support RSA keys */
	ਜੇ (EVP_PKEY_ਅਧਾਰ_id(pkey) != EVP_PKEY_RSA)
		ਵਾਪਸ 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	ਜੇ (rsa == ਨਿਰਾ)
		ਵਾਪਸ 0;

	ਜੇ (strcmp(ਕੁੰਜੀ_name, "n") == 0) {
		ਅਚਲ BIGNUM *n = ns_RSA_get0_n(rsa);
		ਜੇ (n != ਨਿਰਾ)
			ਨਤੀਜਾ = BN_dup(n);
	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਕੁੰਜੀ_name, "e") == 0) {
		ਅਚਲ BIGNUM *e = ns_RSA_get0_e(rsa);
		ਜੇ (e != ਨਿਰਾ)
			ਨਤੀਜਾ = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = ਨਤੀਜਾ;

	ਵਾਪਸ (ਨਤੀਜਾ != ਨਿਰਾ) ? 1 : 0;
}

ਸਥਿਰ ਪੂਰਨ ns_EVP_PKEY_get_utf8_string_param(ਅਚਲ EVP_PKEY *pkey,
		ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ_name, ਅੱਖਰ *str, size_t max_len,
		size_t *ਬਾਹਰਲਾ_len)
{
	ਅਚਲ EC_ਸਮੂਹ *ecgroup;
	ਅਚਲ ਅੱਖਰ *ਸਮੂਹ = "";
	size_t ਸਮੂਹ_len = 0;
	EC_ਕੁੰਜੀ *ec;
	ਪੂਰਨ ret = 0;

	ਜੇ (pkey == ਨਿਰਾ || ਕੁੰਜੀ_name == ਨਿਰਾ)
		ਵਾਪਸ 0;

	/* Only support EC keys */
	ਜੇ (EVP_PKEY_ਅਧਾਰ_id(pkey) != EVP_PKEY_EC)
		ਵਾਪਸ 0;

	/* Only support fetching the group */
	ਜੇ (strcmp(ਕੁੰਜੀ_name, "group") != 0)
		ਵਾਪਸ 0;

	ec = EVP_PKEY_get1_EC_ਕੁੰਜੀ((EVP_PKEY *) pkey);

	ecgroup = EC_ਕੁੰਜੀ_get0_ਸਮੂਹ(ec);
	ਜੇ (ecgroup != ਨਿਰਾ) {
		ਸਮੂਹ = OBJ_nid2ln(EC_ਸਮੂਹ_get_ਵਕਰ_name(ecgroup));
		ਸਮੂਹ_len = ਲੜੀ_ਲੰਬਾਈ(ਸਮੂਹ);
	}

	ਜੇ (str != ਨਿਰਾ && max_len > ਸਮੂਹ_len) {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(str, ਸਮੂਹ, ਸਮੂਹ_len);
		str[ਸਮੂਹ_len] = '\0';
		ret = 1;
	}

	ਜੇ (ਬਾਹਰਲਾ_len != ਨਿਰਾ) {
		*ਬਾਹਰਲਾ_len = ਸਮੂਹ_len;
	}

	EC_ਕੁੰਜੀ_free(ec);

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ ns_EVP_PKEY_get_octet_string_param(ਅਚਲ EVP_PKEY *pkey,
		ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ_name, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, size_t max_len,
		size_t *ਬਾਹਰਲਾ_len)
{
	ਅਚਲ EC_ਸਮੂਹ *ecgroup;
	ਅਚਲ EC_ਬਿੰਦੂ *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_ਕੁੰਜੀ *ec;
	ਪੂਰਨ ret = 0;

	ਜੇ (pkey == ਨਿਰਾ || ਕੁੰਜੀ_name == ਨਿਰਾ)
		ਵਾਪਸ 0;

	/* Only support EC keys */
	ਜੇ (EVP_PKEY_ਅਧਾਰ_id(pkey) != EVP_PKEY_EC)
		ਵਾਪਸ 0;

	ਜੇ (strcmp(ਕੁੰਜੀ_name, "encoded-pub-key") != 0)
		ਵਾਪਸ 0;

	ec = EVP_PKEY_get1_EC_ਕੁੰਜੀ((EVP_PKEY *) pkey);
	ਜੇ (ec == ਨਿਰਾ)
		ਵਾਪਸ 0;

	ecgroup = EC_ਕੁੰਜੀ_get0_ਸਮੂਹ(ec);
	ਜੇ (ecgroup != ਨਿਰਾ) {
		ecpoint = EC_ਕੁੰਜੀ_get0_ਜਨਤਕ_ਕੁੰਜੀ(ec);
		ਜੇ (ecpoint != ਨਿਰਾ) {
			bnctx = BN_CTX_new();
			len = EC_ਬਿੰਦੂ_point2oct(ecgroup,
						 ecpoint,
						 ਬਿੰਦੂ_CONVERSION_UNCOMPRESSED,
						 ਨਿਰਾ,
						 0,
						 bnctx);
			ਜੇ (len != 0 && len <= max_len) {
				ਜੇ (EC_ਬਿੰਦੂ_point2oct(ecgroup,
						       ecpoint,
						       ਬਿੰਦੂ_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			ਜੇ (ਬਾਹਰਲਾ_len != ਨਿਰਾ)
				*ਬਾਹਰਲਾ_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_ਕੁੰਜੀ_free(ec);

	ਵਾਪਸ ret;
}
#ਨਹੀਂਤਾਂ
/* OpenSSL 3.x and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ns_X509_get_signature_nid X509_get_signature_nid
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ns_ASN1_STRING_get0_ਅੰਕੜੇ ASN1_STRING_get0_ਅੰਕੜੇ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ns_RSA_get0_n RSA_get0_n
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ns_RSA_get0_e RSA_get0_e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#ਜੇ_ਅੰਤ

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
ਸਥਿਰ nserror
xname_ਮੰਜ਼ਿਲ_info(X509_NAME *xname, ਬਣਤਰ ns_cert_name *iname)
{
	ਪੂਰਨ entryidx;
	ਪੂਰਨ entrycnt;
	X509_NAME_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ; /* current name entry */
	ASN1_STRING *ਮੁੱਲ;
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਮੁੱਲ_str;
	ASN1_OBJECT *name;
	ਪੂਰਨ name_nid;
	ਅੱਖਰ **ਖੇਤਰ;

	entrycnt = X509_NAME_ਇੰਦਰਾਜ_ਗਿਣਤੀ(xname);

	ਲਈ (entryidx = 0; entryidx < entrycnt; entryidx++) {
		ਇੰਦਰਾਜ = X509_NAME_get_ਇੰਦਰਾਜ(xname, entryidx);
		name = X509_NAME_ਇੰਦਰਾਜ_get_object(ਇੰਦਰਾਜ);
		name_nid = OBJ_obj2nid(name);
		ਮੁੱਲ = X509_NAME_ਇੰਦਰਾਜ_get_ਅੰਕੜੇ(ਇੰਦਰਾਜ);
		ਮੁੱਲ_str = ns_ASN1_STRING_get0_ਅੰਕੜੇ(ਮੁੱਲ);
		ਚੋਣ (name_nid) {
		ਹਾਲਤ NID_commonName:
			ਖੇਤਰ = &iname->common_name;
			ਰੋਕੋ;
		ਹਾਲਤ NID_countryName:
			ਖੇਤਰ = &iname->country;
			ਰੋਕੋ;
		ਹਾਲਤ NID_localityName:
			ਖੇਤਰ = &iname->locality;
			ਰੋਕੋ;
		ਹਾਲਤ NID_stateOrProvinceName:
			ਖੇਤਰ = &iname->province;
			ਰੋਕੋ;
		ਹਾਲਤ NID_organizationName:
			ਖੇਤਰ = &iname->organisation;
			ਰੋਕੋ;
		ਹਾਲਤ NID_organizationalUnitName:
			ਖੇਤਰ = &iname->organisation_unit;
			ਰੋਕੋ;
		ਮੂਲ :
			ਖੇਤਰ = ਨਿਰਾ;
			ਰੋਕੋ;
		}
		ਜੇ (ਖੇਤਰ != ਨਿਰਾ) {
			*ਖੇਤਰ = strdup((ਅਚਲ ਅੱਖਰ *)ਮੁੱਲ_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *ਖੇਤਰ);
		} ਨਹੀਂਤਾਂ {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	ਜੇ (iname->common_name == ਨਿਰਾ) {
		iname->common_name = strdup("Unknown");
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ਸਥਿਰ ਅੱਖਰ *hexdup(ਅਚਲ ਅੱਖਰ *hex)
{
	ਪੂਰਨ hexlen;
	ਅੱਖਰ *dst;
	ਅੱਖਰ *ਬਾਹਰਲਾ;
	ਪੂਰਨ cn = 0;

	hexlen = ਲੜੀ_ਲੰਬਾਈ(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = ਸਮ੍ਰਿਤੀ_ਦਿਓ(((hexlen * 7) + 6) / 2);

	ਜੇ (dst != ਨਿਰਾ) {
		ਲਈ (ਬਾਹਰਲਾ = dst; *hex != 0; hex++) {
			ਜੇ (cn == 2) {
				cn = 0;
				*ਬਾਹਰਲਾ++ = '&';
				*ਬਾਹਰਲਾ++ = '#';
				*ਬਾਹਰਲਾ++ = '5';
				*ਬਾਹਰਲਾ++ = '8';
				*ਬਾਹਰਲਾ++ = ';';
			}
			*ਬਾਹਰਲਾ++ = *hex;
			cn++;
		}
		*ਬਾਹਰਲਾ = 0;
	}
	ਵਾਪਸ dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ਸਥਿਰ ਅੱਖਰ *bindup(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *bin, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ binlen)
{
	ਅੱਖਰ *dst;
	ਅੱਖਰ *ਬਾਹਰਲਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ idx;
	ਅਚਲ ਅੱਖਰ hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = ਸਮ੍ਰਿਤੀ_ਦਿਓ(binlen * 7);

	ਜੇ (dst != ਨਿਰਾ) {
		ਬਾਹਰਲਾ = dst;
		ਲਈ (idx = 0; idx < binlen; idx++) {
			*ਬਾਹਰਲਾ++ = hex[(bin[idx] & 0xf0) >> 4];
			*ਬਾਹਰਲਾ++ = hex[bin[idx] & 0xf];

			*ਬਾਹਰਲਾ++ = '&';
			*ਬਾਹਰਲਾ++ = '#';
			*ਬਾਹਰਲਾ++ = '5';
			*ਬਾਹਰਲਾ++ = '8';
			*ਬਾਹਰਲਾ++ = ';';
		}
		ਬਾਹਰਲਾ -= 5;
		*ਬਾਹਰਲਾ = 0;
	}
	ਵਾਪਸ dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ਸਥਿਰ nserror
rsa_ਮੰਜ਼ਿਲ_info(EVP_PKEY *pkey, ਬਣਤਰ ns_cert_pkey *ikey)
{
	BIGNUM *n = ਨਿਰਾ, *e = ਨਿਰਾ;
	ਅੱਖਰ *tmp;

	ਜੇ (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	ਜੇ (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->ਮਾਪ = EVP_PKEY_ਬਿੱਟ(pkey);

	tmp = BN_bn2hex(n);
	ਜੇ (tmp != ਨਿਰਾ) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	ਜੇ (tmp != ਨਿਰਾ) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ਸਥਿਰ nserror
dsa_ਮੰਜ਼ਿਲ_info(EVP_PKEY *pkey, ਬਣਤਰ ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->ਮਾਪ = EVP_PKEY_ਬਿੱਟ(pkey);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ਸਥਿਰ nserror
dh_ਮੰਜ਼ਿਲ_info(EVP_PKEY *pkey, ਬਣਤਰ ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->ਮਾਪ = EVP_PKEY_ਬਿੱਟ(pkey);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ਸਥਿਰ nserror
ec_ਮੰਜ਼ਿਲ_info(EVP_PKEY *pkey, ਬਣਤਰ ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->ਮਾਪ = EVP_PKEY_ਬਿੱਟ(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ਨਿਰਾ, 0, &len);
	ਜੇ (len != 0) {
		ikey->ਵਕਰ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len + 1);
		ਜੇ (ikey->ਵਕਰ != ਨਿਰਾ) {
			ਜੇ (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->ਵਕਰ, len + 1, ਨਿਰਾ) == 0) {
				ਮੁਕਤ(ikey->ਵਕਰ);
				ikey->ਵਕਰ = ਨਿਰਾ;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ਨਿਰਾ, 0, &len);
	ਜੇ (len != 0) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਿੰਦੂ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len);
		ਜੇ (ਬਿੰਦੂ != ਨਿਰਾ) {
			ਜੇ (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", ਬਿੰਦੂ, len,
					ਨਿਰਾ) == 1) {
				ikey->ਜਨਤਕ = bindup(ਬਿੰਦੂ, len);
			}
			ਮੁਕਤ(ਬਿੰਦੂ);
		}
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ਸਥਿਰ nserror
pkey_ਮੰਜ਼ਿਲ_info(EVP_PKEY *pkey, ਬਣਤਰ ns_cert_pkey *ikey)
{
	nserror res;

	ਜੇ (pkey == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	ਚੋਣ (EVP_PKEY_ਅਧਾਰ_id(pkey)) {
	ਹਾਲਤ EVP_PKEY_RSA:
		res = rsa_ਮੰਜ਼ਿਲ_info(pkey, ikey);
		ਰੋਕੋ;

	ਹਾਲਤ EVP_PKEY_DSA:
		res = dsa_ਮੰਜ਼ਿਲ_info(pkey, ikey);
		ਰੋਕੋ;

	ਹਾਲਤ EVP_PKEY_DH:
		res = dh_ਮੰਜ਼ਿਲ_info(pkey, ikey);
		ਰੋਕੋ;

	ਹਾਲਤ EVP_PKEY_EC:
		res = ec_ਮੰਜ਼ਿਲ_info(pkey, ikey);
		ਰੋਕੋ;

	ਮੂਲ:
		res = NSERROR_NOT_IMPLEMENTED;
		ਰੋਕੋ;
	}

	EVP_PKEY_free(pkey);

	ਵਾਪਸ res;
}

ਸਥਿਰ nserror san_ਮੰਜ਼ਿਲ_info(X509 *cert, ਬਣਤਰ ns_cert_san **prev_ਅਗਲਾ)
{
	ਪੂਰਨ idx;
	ਪੂਰਨ san_ਨਾਂ_nb = -1;
	ਅਚਲ GENERAL_NAME *current_name;
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *dns_name;
	ਬਣਤਰ ns_cert_san *isan;

	ਢੇਰ_ਦਾ(GENERAL_NAME) *san_ਨਾਂ = ਨਿਰਾ;

	san_ਨਾਂ = X509_get_ext_d2i(cert, NID_subject_alt_name, ਨਿਰਾ, ਨਿਰਾ);
	ਜੇ (san_ਨਾਂ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_OK;
	}

	san_ਨਾਂ_nb = sk_GENERAL_NAME_ਅੰਕ(san_ਨਾਂ);

	/* Check each name within the extension */
	ਲਈ (idx = 0; idx < san_ਨਾਂ_nb; idx++) {
		current_name = sk_GENERAL_NAME_ਮੁੱਲ(san_ਨਾਂ, idx);

		ਜੇ (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_ਅੰਕੜੇ(current_name->d.dNSName);

			isan = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ ns_cert_san));
			ਜੇ (isan != ਨਿਰਾ) {
				isan->name = strdup((ਅਚਲ ਅੱਖਰ *)dns_name);
				isan->ਅਗਲਾ = ਨਿਰਾ;
				*prev_ਅਗਲਾ = isan;
				prev_ਅਗਲਾ = &isan->ਅਗਲਾ;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	ਲਈ (idx = 0; idx < san_ਨਾਂ_nb; idx++) {
		GENERAL_NAME *ਇੰਦਰਾਜ = sk_GENERAL_NAME_ਕੱਢੋ(san_ਨਾਂ);
		GENERAL_NAME_free(ਇੰਦਰਾਜ);
	}
	sk_GENERAL_NAME_free(san_ਨਾਂ);

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror
der_ਮੰਜ਼ਿਲ_certinfo(ਅਚਲ uint8_t *der,
		size_t der_ਲੰਮਾਈ,
		ਬਣਤਰ ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ਅਚਲ ASN1_INTEGER *asn1_ਅੰਕ;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	ਜੇ (der == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_OK;
	}

	cert = d2i_X509(ਨਿਰਾ, &der, der_ਲੰਮਾਈ);
	ਜੇ (cert == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->ਸੰਸਕਰਣ = X509_get_ਸੰਸਕਰਣ(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_ਸਮਾਂ_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ਖਾਲੀ) BIO_ਸੈੱਟ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, buf->ਲੰਮਾਈ + 1);
	ਜੇ (info->not_before != ਨਿਰਾ) {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(info->not_before, buf->ਅੰਕੜੇ, (ਬਿਨਾਨਿਸ਼ਾਨੀ)buf->ਲੰਮਾਈ);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_ਸਮਾਂ_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ਖਾਲੀ) BIO_ਸੈੱਟ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, buf->ਲੰਮਾਈ + 1);
	ਜੇ (info->not_after != ਨਿਰਾ) {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(info->not_after, buf->ਅੰਕੜੇ, (ਬਿਨਾਨਿਸ਼ਾਨੀ)buf->ਲੰਮਾਈ);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	ਪੂਰਨ pkey_nid = ns_X509_get_signature_nid(cert);
	ਜੇ (pkey_nid != NID_undef) {
		ਅਚਲ ਅੱਖਰ* sslbuf = OBJ_nid2ln(pkey_nid);
		ਜੇ (sslbuf != ਨਿਰਾ) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_ਅੰਕ = X509_get_serial_ਸੰਖਿਆ(cert);
	ਜੇ (asn1_ਅੰਕ != ਨਿਰਾ) {
		bignum = ASN1_INTEGER_ਮੰਜ਼ਿਲ_BN(asn1_ਅੰਕ, ਨਿਰਾ);
		ਜੇ (bignum != ਨਿਰਾ) {
			ਅੱਖਰ *tmp = BN_bn2hex(bignum);
			ਜੇ (tmp != ਨਿਰਾ) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ਨਿਰਾ;
		}
	}

	/* fingerprints */
	ਅਚਲ EVP_MD *digest;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ dig_len;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buff;
	ਪੂਰਨ rc;

	digest = EVP_sha1();
	buff = ਸਮ੍ਰਿਤੀ_ਦਿਓ(EVP_MD_ਮਾਪ(digest));
	ਜੇ (buff != ਨਿਰਾ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ਜੇ ((rc == 1) && (dig_len == (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)EVP_MD_ਮਾਪ(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		ਮੁਕਤ(buff);
	}

	digest = EVP_sha256();
	buff = ਸਮ੍ਰਿਤੀ_ਦਿਓ(EVP_MD_ਮਾਪ(digest));
	ਜੇ (buff != ਨਿਰਾ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ਜੇ ((rc == 1) && (dig_len == (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)EVP_MD_ਮਾਪ(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		ਮੁਕਤ(buff);
	}

	/* subject alternative names */
	san_ਮੰਜ਼ਿਲ_info(cert, &info->san);

	/* issuer name */
	xname_ਮੰਜ਼ਿਲ_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_ਮੰਜ਼ਿਲ_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_ਮੰਜ਼ਿਲ_info(X509_get_pubkey(cert), &info->ਜਨਤਕ_ਕੁੰਜੀ);

	X509_free(cert);

	ਵਾਪਸ NSERROR_OK;
}

/* copy certificate data */
ਸਥਿਰ nserror
convert_chain_ਮੰਜ਼ਿਲ_cert_info(ਅਚਲ ਬਣਤਰ cert_chain *chain,
			   ਬਣਤਰ ns_cert_info **cert_info_ਬਾਹਰਲਾ)
{
	ਬਣਤਰ ns_cert_info *certs;
	size_t ਡੂੰਘਾਈ;
	nserror res;

	certs = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(chain->ਡੂੰਘਾਈ, ਆਕਾਰ(ਬਣਤਰ ns_cert_info));
	ਜੇ (certs == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਲਈ (ਡੂੰਘਾਈ = 0; ਡੂੰਘਾਈ < chain->ਡੂੰਘਾਈ;ਡੂੰਘਾਈ++) {
		res = der_ਮੰਜ਼ਿਲ_certinfo(chain->certs[ਡੂੰਘਾਈ].der,
				      chain->certs[ਡੂੰਘਾਈ].der_ਲੰਮਾਈ,
				      certs + ਡੂੰਘਾਈ);
		ਜੇ (res != NSERROR_OK) {
			ਮੁਕਤ(certs);
			ਵਾਪਸ res;
		}
		certs[ਡੂੰਘਾਈ].err = chain->certs[ਡੂੰਘਾਈ].err;
	}

	*cert_info_ਬਾਹਰਲਾ = certs;
	ਵਾਪਸ NSERROR_OK;
}

#ਨਹੀਂਤਾਂ
ਸਥਿਰ nserror
convert_chain_ਮੰਜ਼ਿਲ_cert_info(ਅਚਲ ਬਣਤਰ cert_chain *chain,
			   ਬਣਤਰ ns_cert_info **cert_info_ਬਾਹਰਲਾ)
{
	ਵਾਪਸ NSERROR_NOT_IMPLEMENTED;
}
#ਜੇ_ਅੰਤ


ਸਥਿਰ nserror
format_certificate_name(ਬਣਤਰ fetch_about_context *ctx,
			ਬਣਤਰ ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਜੇ (cert_name->organisation != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (cert_name->organisation_unit != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (cert_name->locality != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (cert_name->province != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (cert_name->country != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਵਾਪਸ res;
}

/**
 * output formatted certificate subject alternate names
 */
ਸਥਿਰ nserror
format_certificate_san(ਬਣਤਰ fetch_about_context *ctx,
			      ਬਣਤਰ ns_cert_san *san)
{
	nserror res;

	ਜੇ (san == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਜਦੋਂ (san != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}

		san = san->ਅਗਲਾ;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ਵਾਪਸ res;

}


ਸਥਿਰ nserror
format_certificate_ਜਨਤਕ_ਕੁੰਜੀ(ਬਣਤਰ fetch_about_context *ctx,
			      ਬਣਤਰ ns_cert_pkey *ਜਨਤਕ_ਕੁੰਜੀ)
{
	nserror res;

	ਜੇ (ਜਨਤਕ_ਕੁੰਜੀ->algor == ਨਿਰਾ) {
		/* skip the table if no algorithm name */
		ਵਾਪਸ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 ਜਨਤਕ_ਕੁੰਜੀ->algor,
			 ਜਨਤਕ_ਕੁੰਜੀ->ਮਾਪ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}


	ਜੇ (ਜਨਤਕ_ਕੁੰਜੀ->exponent != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 ਜਨਤਕ_ਕੁੰਜੀ->exponent);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (ਜਨਤਕ_ਕੁੰਜੀ->modulus != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 ਜਨਤਕ_ਕੁੰਜੀ->modulus);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (ਜਨਤਕ_ਕੁੰਜੀ->ਵਕਰ != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 ਜਨਤਕ_ਕੁੰਜੀ->ਵਕਰ);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (ਜਨਤਕ_ਕੁੰਜੀ->ਜਨਤਕ != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 ਜਨਤਕ_ਕੁੰਜੀ->ਜਨਤਕ);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ਵਾਪਸ res;
}

ਸਥਿਰ nserror
format_certificate_fingerprint(ਬਣਤਰ fetch_about_context *ctx,
			      ਬਣਤਰ ns_cert_info *cert_info)
{
	nserror res;

	ਜੇ ((cert_info->sha1fingerprint == ਨਿਰਾ) &&
	    (cert_info->sha256fingerprint == ਨਿਰਾ))  {
		/* skip the table if no fingerprints */
		ਵਾਪਸ NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਜੇ (cert_info->sha256fingerprint != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (cert_info->sha1fingerprint != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ਵਾਪਸ res;
}

ਸਥਿਰ nserror
format_certificate(ਬਣਤਰ fetch_about_context *ctx,
		   ਬਣਤਰ ns_cert_info *cert_info,
		   size_t ਡੂੰਘਾਈ)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 ਡੂੰਘਾਈ, cert_info->subject_name.common_name);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਜੇ (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = format_certificate_ਜਨਤਕ_ਕੁੰਜੀ(ctx, &cert_info->ਜਨਤਕ_ਕੁੰਜੀ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਜੇ (cert_info->serialnum != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਜੇ (cert_info->sig_algor != ਨਿਰਾ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->ਸੰਸਕਰਣ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਵਾਪਸ res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਪੂਰਨ ਕੋਡ = 200;
	nserror res;
	ਬਣਤਰ cert_chain *chain = ਨਿਰਾ;

	/* content is going to return ok */
	fetch_about_ਸੈੱਟ_http_ਕੋਡ(ctx, ਕੋਡ);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html"))
		ਜਾਓ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	ਜੇ (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		ਜੇ (res != NSERROR_OK) {
			ਜਾਓ fetch_about_certificate_handler_aborted;
		}
	} ਨਹੀਂਤਾਂ {
		ਬਣਤਰ ns_cert_info *cert_info;
		res = convert_chain_ਮੰਜ਼ਿਲ_cert_info(chain, &cert_info);
		ਜੇ (res == NSERROR_OK) {
			size_t ਡੂੰਘਾਈ;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			ਜੇ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ਜਾਓ fetch_about_certificate_handler_aborted;
			}

			ਲਈ (ਡੂੰਘਾਈ = 0; ਡੂੰਘਾਈ < chain->ਡੂੰਘਾਈ; ਡੂੰਘਾਈ++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						ਡੂੰਘਾਈ, (cert_info + ਡੂੰਘਾਈ)
							->subject_name
								.common_name);
				ਜੇ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ਜਾਓ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			ਜੇ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				ਜਾਓ fetch_about_certificate_handler_aborted;
			}

			ਲਈ (ਡੂੰਘਾਈ = 0; ਡੂੰਘਾਈ < chain->ਡੂੰਘਾਈ; ਡੂੰਘਾਈ++) {
				res = format_certificate(ctx, cert_info + ਡੂੰਘਾਈ,
						ਡੂੰਘਾਈ);
				ਜੇ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					ਜਾਓ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} ਨਹੀਂਤਾਂ {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			ਜੇ (res != NSERROR_OK) {
				ਜਾਓ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	cert_chain_free(chain);

	ਵਾਪਸ true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ਵਾਪਸ false;
}
