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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "netsurf/ssl_certs.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "certificate.h"

/**
 * certificate name parameters
 */
ഘടന ns_cert_name {
	അക്ഷരം *common_name;
	അക്ഷരം *organisation;
	അക്ഷരം *organisation_unit;
	അക്ഷരം *locality;
	അക്ഷരം *province;
	അക്ഷരം *country;
};

/**
 * Certificate public key parameters
 */
ഘടന ns_cert_pkey {
	അക്ഷരം *algor;
	പൂർണ്ണസംഖ്യ size;
	അക്ഷരം *modulus;
	അക്ഷരം *exponent;
	അക്ഷരം *curve;
	അക്ഷരം *പൊതു;
};

/**
 * Certificate subject alternative name
 */
ഘടന ns_cert_san {
	ഘടന ns_cert_san *next;
	അക്ഷരം *name;
};

/**
 * certificate information for certificate chain
 */
ഘടന ns_cert_info {
	ഘടന ns_cert_name subject_name; /**< Subject details */
	ഘടന ns_cert_name issuer_name; /**< Issuer details */
	ഘടന ns_cert_pkey public_താക്കോൽ; /**< public key details */
	long version;		/**< Certificate version */
	അക്ഷരം *not_before;	/**< Valid from date */
	അക്ഷരം *not_after;	/**< Valid to date */
	പൂർണ്ണസംഖ്യ sig_type;		/**< Signature type */
	അക്ഷരം *sig_algor;        /**< Signature Algorithm */
	അക്ഷരം *serialnum;	/**< Serial number */
	അക്ഷരം *sha1fingerprint; /**< fingerprint shar1 encoded */
	അക്ഷരം *sha256fingerprint; /**< fingerprint shar256 encoded */
	ഘടന ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
സ്ഥിതം nserror free_ns_cert_info(ഘടന ns_cert_info *cinfo)
{
	ഘടന ns_cert_san *san;

	സ്വതന്ത്രം(cinfo->subject_name.common_name);
	സ്വതന്ത്രം(cinfo->subject_name.organisation);
	സ്വതന്ത്രം(cinfo->subject_name.organisation_unit);
	സ്വതന്ത്രം(cinfo->subject_name.locality);
	സ്വതന്ത്രം(cinfo->subject_name.province);
	സ്വതന്ത്രം(cinfo->subject_name.country);
	സ്വതന്ത്രം(cinfo->issuer_name.common_name);
	സ്വതന്ത്രം(cinfo->issuer_name.organisation);
	സ്വതന്ത്രം(cinfo->issuer_name.organisation_unit);
	സ്വതന്ത്രം(cinfo->issuer_name.locality);
	സ്വതന്ത്രം(cinfo->issuer_name.province);
	സ്വതന്ത്രം(cinfo->issuer_name.country);
	സ്വതന്ത്രം(cinfo->public_താക്കോൽ.algor);
	സ്വതന്ത്രം(cinfo->public_താക്കോൽ.modulus);
	സ്വതന്ത്രം(cinfo->public_താക്കോൽ.exponent);
	സ്വതന്ത്രം(cinfo->public_താക്കോൽ.curve);
	സ്വതന്ത്രം(cinfo->public_താക്കോൽ.പൊതു);
	സ്വതന്ത്രം(cinfo->not_before);
	സ്വതന്ത്രം(cinfo->not_after);
	സ്വതന്ത്രം(cinfo->sig_algor);
	സ്വതന്ത്രം(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	ആകുമ്പോൾ (san != ശൂന്യ) {
		ഘടന ns_cert_san *next;
		next = san->next;
		സ്വതന്ത്രം(san);
		san = next;
	}

	സ്വതന്ത്രം(cinfo);

	തിരികെ NSERROR_OK;
}

#ifdef WITH_OPENSSL

#അന്തര്ഭാവ <openssl/ssl.h>
#അന്തര്ഭാവ <openssl/x509v3.h>

#എങ്കിൽ (OPENSSL_VERSION_സംഖ്യ < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# എങ്കിൽ defined(LIBRESSL_VERSION_സംഖ്യ)
  /* LibreSSL */
#  എങ്കിൽ (LIBRESSL_VERSION_സംഖ്യ < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   എങ്കിൽ (LIBRESSL_VERSION_സംഖ്യ < 0x2070000fL)
    /* LibreSSL <2.7.0 */
സ്ഥിതം പൂർണ്ണസംഖ്യ ns_X509_get_signature_nid(X509 *cert)
{
	തിരികെ OBJ_obj2nid(cert->cert_info->താക്കോൽ->algor->algorithm);
}

സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	തിരികെ (സ്ഥിരം unsigned അക്ഷരം *)ASN1_STRING_data(asn1str);
}
#   അല്ലെങ്കിൽ
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

സ്ഥിതം സ്ഥിരം BIGNUM *ns_RSA_get0_n(സ്ഥിരം RSA *d)
{
	തിരികെ d->n;
}

സ്ഥിതം സ്ഥിരം BIGNUM *ns_RSA_get0_e(സ്ഥിരം RSA *d)
{
	തിരികെ d->e;
}
#  അല്ലെങ്കിൽ
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# അല്ലെങ്കിൽ
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

സ്ഥിതം പൂർണ്ണസംഖ്യ ns_EVP_PKEY_get_bn_param(സ്ഥിരം EVP_PKEY *pkey,
		സ്ഥിരം അക്ഷരം *താക്കോൽ_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = ശൂന്യ;

	/* Check parameters: only support allocation-form *bn */
	എങ്കിൽ (pkey == ശൂന്യ || താക്കോൽ_name == ശൂന്യ || bn == ശൂന്യ || *bn != ശൂന്യ)
		തിരികെ 0;

	/* Only support RSA keys */
	എങ്കിൽ (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		തിരികെ 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	എങ്കിൽ (rsa == ശൂന്യ)
		തിരികെ 0;

	എങ്കിൽ (strcmp(താക്കോൽ_name, "n") == 0) {
		സ്ഥിരം BIGNUM *n = ns_RSA_get0_n(rsa);
		എങ്കിൽ (n != ശൂന്യ)
			result = BN_dup(n);
	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(താക്കോൽ_name, "e") == 0) {
		സ്ഥിരം BIGNUM *e = ns_RSA_get0_e(rsa);
		എങ്കിൽ (e != ശൂന്യ)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	തിരികെ (result != ശൂന്യ) ? 1 : 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ ns_EVP_PKEY_get_utf8_string_param(സ്ഥിരം EVP_PKEY *pkey,
		സ്ഥിരം അക്ഷരം *താക്കോൽ_name, അക്ഷരം *str, size_t max_len,
		size_t *out_len)
{
	സ്ഥിരം EC_GROUP *ecgroup;
	സ്ഥിരം അക്ഷരം *group = "";
	size_t group_len = 0;
	EC_താക്കോൽ *ec;
	പൂർണ്ണസംഖ്യ ret = 0;

	എങ്കിൽ (pkey == ശൂന്യ || താക്കോൽ_name == ശൂന്യ)
		തിരികെ 0;

	/* Only support EC keys */
	എങ്കിൽ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		തിരികെ 0;

	/* Only support fetching the group */
	എങ്കിൽ (strcmp(താക്കോൽ_name, "group") != 0)
		തിരികെ 0;

	ec = EVP_PKEY_get1_EC_താക്കോൽ((EVP_PKEY *) pkey);

	ecgroup = EC_താക്കോൽ_get0_group(ec);
	എങ്കിൽ (ecgroup != ശൂന്യ) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	എങ്കിൽ (str != ശൂന്യ && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	എങ്കിൽ (out_len != ശൂന്യ) {
		*out_len = group_len;
	}

	EC_താക്കോൽ_free(ec);

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ ns_EVP_PKEY_get_octet_string_param(സ്ഥിരം EVP_PKEY *pkey,
		സ്ഥിരം അക്ഷരം *താക്കോൽ_name, unsigned അക്ഷരം *buf, size_t max_len,
		size_t *out_len)
{
	സ്ഥിരം EC_GROUP *ecgroup;
	സ്ഥിരം EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_താക്കോൽ *ec;
	പൂർണ്ണസംഖ്യ ret = 0;

	എങ്കിൽ (pkey == ശൂന്യ || താക്കോൽ_name == ശൂന്യ)
		തിരികെ 0;

	/* Only support EC keys */
	എങ്കിൽ (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		തിരികെ 0;

	എങ്കിൽ (strcmp(താക്കോൽ_name, "encoded-pub-key") != 0)
		തിരികെ 0;

	ec = EVP_PKEY_get1_EC_താക്കോൽ((EVP_PKEY *) pkey);
	എങ്കിൽ (ec == ശൂന്യ)
		തിരികെ 0;

	ecgroup = EC_താക്കോൽ_get0_group(ec);
	എങ്കിൽ (ecgroup != ശൂന്യ) {
		ecpoint = EC_താക്കോൽ_get0_public_താക്കോൽ(ec);
		എങ്കിൽ (ecpoint != ശൂന്യ) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 ശൂന്യ,
						 0,
						 bnctx);
			എങ്കിൽ (len != 0 && len <= max_len) {
				എങ്കിൽ (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			എങ്കിൽ (out_len != ശൂന്യ)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_താക്കോൽ_free(ec);

	തിരികെ ret;
}
#അല്ലെങ്കിൽ
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
സ്ഥിതം nserror
xname_to_info(X509_NAME *xname, ഘടന ns_cert_name *iname)
{
	പൂർണ്ണസംഖ്യ entryidx;
	പൂർണ്ണസംഖ്യ entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *മൂല്യം;
	സ്ഥിരം unsigned അക്ഷരം *മൂല്യം_str;
	ASN1_OBJECT *name;
	പൂർണ്ണസംഖ്യ name_nid;
	അക്ഷരം **field;

	entrycnt = X509_NAME_entry_എണ്ണം(xname);

	വേണ്ടി (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		മൂല്യം = X509_NAME_ENTRY_get_data(entry);
		മൂല്യം_str = ns_ASN1_STRING_get0_data(മൂല്യം);
		തിരഞ്ഞെടുപ്പ് (name_nid) {
		അവസ്ഥ NID_commonName:
			field = &iname->common_name;
			നിർത്തുക;
		അവസ്ഥ NID_countryName:
			field = &iname->country;
			നിർത്തുക;
		അവസ്ഥ NID_localityName:
			field = &iname->locality;
			നിർത്തുക;
		അവസ്ഥ NID_stateOrProvinceName:
			field = &iname->province;
			നിർത്തുക;
		അവസ്ഥ NID_organizationName:
			field = &iname->organisation;
			നിർത്തുക;
		അവസ്ഥ NID_organizationalUnitName:
			field = &iname->organisation_unit;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി :
			field = ശൂന്യ;
			നിർത്തുക;
		}
		എങ്കിൽ (field != ശൂന്യ) {
			*field = strdup((സ്ഥിരം അക്ഷരം *)മൂല്യം_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} അല്ലെങ്കിൽ {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	എങ്കിൽ (iname->common_name == ശൂന്യ) {
		iname->common_name = strdup("Unknown");
	}

	തിരികെ NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
സ്ഥിതം അക്ഷരം *hexdup(സ്ഥിരം അക്ഷരം *hex)
{
	പൂർണ്ണസംഖ്യ hexlen;
	അക്ഷരം *dst;
	അക്ഷരം *out;
	പൂർണ്ണസംഖ്യ cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	എങ്കിൽ (dst != ശൂന്യ) {
		വേണ്ടി (out = dst; *hex != 0; hex++) {
			എങ്കിൽ (cn == 2) {
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
	തിരികെ dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
സ്ഥിതം അക്ഷരം *bindup(unsigned അക്ഷരം *bin, unsigned പൂർണ്ണസംഖ്യ binlen)
{
	അക്ഷരം *dst;
	അക്ഷരം *out;
	unsigned പൂർണ്ണസംഖ്യ idx;
	സ്ഥിരം അക്ഷരം hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	എങ്കിൽ (dst != ശൂന്യ) {
		out = dst;
		വേണ്ടി (idx = 0; idx < binlen; idx++) {
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
	തിരികെ dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
സ്ഥിതം nserror
rsa_to_info(EVP_PKEY *pkey, ഘടന ns_cert_pkey *ikey)
{
	BIGNUM *n = ശൂന്യ, *e = ശൂന്യ;
	അക്ഷരം *tmp;

	എങ്കിൽ (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	എങ്കിൽ (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	എങ്കിൽ (tmp != ശൂന്യ) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	എങ്കിൽ (tmp != ശൂന്യ) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	തിരികെ NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
സ്ഥിതം nserror
dsa_to_info(EVP_PKEY *pkey, ഘടന ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	തിരികെ NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
സ്ഥിതം nserror
dh_to_info(EVP_PKEY *pkey, ഘടന ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	തിരികെ NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
സ്ഥിതം nserror
ec_to_info(EVP_PKEY *pkey, ഘടന ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ശൂന്യ, 0, &len);
	എങ്കിൽ (len != 0) {
		ikey->curve = malloc(len + 1);
		എങ്കിൽ (ikey->curve != ശൂന്യ) {
			എങ്കിൽ (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, ശൂന്യ) == 0) {
				സ്വതന്ത്രം(ikey->curve);
				ikey->curve = ശൂന്യ;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ശൂന്യ, 0, &len);
	എങ്കിൽ (len != 0) {
		unsigned അക്ഷരം *point = malloc(len);
		എങ്കിൽ (point != ശൂന്യ) {
			എങ്കിൽ (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					ശൂന്യ) == 1) {
				ikey->പൊതു = bindup(point, len);
			}
			സ്വതന്ത്രം(point);
		}
	}

	തിരികെ NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
സ്ഥിതം nserror
pkey_to_info(EVP_PKEY *pkey, ഘടന ns_cert_pkey *ikey)
{
	nserror res;

	എങ്കിൽ (pkey == ശൂന്യ) {
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	തിരഞ്ഞെടുപ്പ് (EVP_PKEY_base_id(pkey)) {
	അവസ്ഥ EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		നിർത്തുക;

	അവസ്ഥ EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		നിർത്തുക;

	അവസ്ഥ EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		നിർത്തുക;

	അവസ്ഥ EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		res = NSERROR_NOT_IMPLEMENTED;
		നിർത്തുക;
	}

	EVP_PKEY_free(pkey);

	തിരികെ res;
}

സ്ഥിതം nserror san_to_info(X509 *cert, ഘടന ns_cert_san **prev_next)
{
	പൂർണ്ണസംഖ്യ idx;
	പൂർണ്ണസംഖ്യ san_names_nb = -1;
	സ്ഥിരം GENERAL_NAME *current_name;
	സ്ഥിരം unsigned അക്ഷരം *dns_name;
	ഘടന ns_cert_san *isan;

	അട്ടി_OF(GENERAL_NAME) *san_names = ശൂന്യ;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, ശൂന്യ, ശൂന്യ);
	എങ്കിൽ (san_names == ശൂന്യ) {
		തിരികെ NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_അക്കം(san_names);

	/* Check each name within the extension */
	വേണ്ടി (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_മൂല്യം(san_names, idx);

		എങ്കിൽ (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(വലുപ്പം(ഘടന ns_cert_san));
			എങ്കിൽ (isan != ശൂന്യ) {
				isan->name = strdup((സ്ഥിരം അക്ഷരം *)dns_name);
				isan->next = ശൂന്യ;
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
	വേണ്ടി (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_എടുക്കുക(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror
der_to_certinfo(സ്ഥിരം uint8_t *der,
		size_t der_length,
		ഘടന ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	സ്ഥിരം ASN1_INTEGER *asn1_അക്കം;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	എങ്കിൽ (der == ശൂന്യ) {
		തിരികെ NSERROR_OK;
	}

	cert = d2i_X509(ശൂന്യ, &der, der_length);
	എങ്കിൽ (cert == ശൂന്യ) {
		തിരികെ NSERROR_INVALID;
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
	(ശൂന്യം) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	എങ്കിൽ (info->not_before != ശൂന്യ) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(ശൂന്യം) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	എങ്കിൽ (info->not_after != ശൂന്യ) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	പൂർണ്ണസംഖ്യ pkey_nid = ns_X509_get_signature_nid(cert);
	എങ്കിൽ (pkey_nid != NID_undef) {
		സ്ഥിരം അക്ഷരം* sslbuf = OBJ_nid2ln(pkey_nid);
		എങ്കിൽ (sslbuf != ശൂന്യ) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_അക്കം = X509_get_serial_സംഖ്യ(cert);
	എങ്കിൽ (asn1_അക്കം != ശൂന്യ) {
		bignum = ASN1_INTEGER_to_BN(asn1_അക്കം, ശൂന്യ);
		എങ്കിൽ (bignum != ശൂന്യ) {
			അക്ഷരം *tmp = BN_bn2hex(bignum);
			എങ്കിൽ (tmp != ശൂന്യ) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ശൂന്യ;
		}
	}

	/* fingerprints */
	സ്ഥിരം EVP_MD *digest;
	unsigned പൂർണ്ണസംഖ്യ dig_len;
	unsigned അക്ഷരം *buff;
	പൂർണ്ണസംഖ്യ rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	എങ്കിൽ (buff != ശൂന്യ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		എങ്കിൽ ((rc == 1) && (dig_len == (unsigned പൂർണ്ണസംഖ്യ)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		സ്വതന്ത്രം(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	എങ്കിൽ (buff != ശൂന്യ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		എങ്കിൽ ((rc == 1) && (dig_len == (unsigned പൂർണ്ണസംഖ്യ)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		സ്വതന്ത്രം(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_താക്കോൽ);

	X509_free(cert);

	തിരികെ NSERROR_OK;
}

/* copy certificate data */
സ്ഥിതം nserror
convert_chain_to_cert_info(സ്ഥിരം ഘടന cert_chain *chain,
			   ഘടന ns_cert_info **cert_info_out)
{
	ഘടന ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, വലുപ്പം(ഘടന ns_cert_info));
	എങ്കിൽ (certs == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	വേണ്ടി (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		എങ്കിൽ (res != NSERROR_OK) {
			സ്വതന്ത്രം(certs);
			തിരികെ res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	തിരികെ NSERROR_OK;
}

#അല്ലെങ്കിൽ
സ്ഥിതം nserror
convert_chain_to_cert_info(സ്ഥിരം ഘടന cert_chain *chain,
			   ഘടന ns_cert_info **cert_info_out)
{
	തിരികെ NSERROR_NOT_IMPLEMENTED;
}
#endif


സ്ഥിതം nserror
format_certificate_name(ഘടന fetch_about_context *ctx,
			ഘടന ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	എങ്കിൽ (cert_name->organisation != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (cert_name->organisation_unit != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (cert_name->locality != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (cert_name->province != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (cert_name->country != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	തിരികെ res;
}

/**
 * output formatted certificate subject alternate names
 */
സ്ഥിതം nserror
format_certificate_san(ഘടന fetch_about_context *ctx,
			      ഘടന ns_cert_san *san)
{
	nserror res;

	എങ്കിൽ (san == ശൂന്യ) {
		തിരികെ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	ആകുമ്പോൾ (san != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	തിരികെ res;

}


സ്ഥിതം nserror
format_certificate_public_താക്കോൽ(ഘടന fetch_about_context *ctx,
			      ഘടന ns_cert_pkey *public_താക്കോൽ)
{
	nserror res;

	എങ്കിൽ (public_താക്കോൽ->algor == ശൂന്യ) {
		/* skip the table if no algorithm name */
		തിരികെ NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_താക്കോൽ->algor,
			 public_താക്കോൽ->size);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}


	എങ്കിൽ (public_താക്കോൽ->exponent != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_താക്കോൽ->exponent);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (public_താക്കോൽ->modulus != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_താക്കോൽ->modulus);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (public_താക്കോൽ->curve != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_താക്കോൽ->curve);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (public_താക്കോൽ->പൊതു != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_താക്കോൽ->പൊതു);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	തിരികെ res;
}

സ്ഥിതം nserror
format_certificate_fingerprint(ഘടന fetch_about_context *ctx,
			      ഘടന ns_cert_info *cert_info)
{
	nserror res;

	എങ്കിൽ ((cert_info->sha1fingerprint == ശൂന്യ) &&
	    (cert_info->sha256fingerprint == ശൂന്യ))  {
		/* skip the table if no fingerprints */
		തിരികെ NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	എങ്കിൽ (cert_info->sha256fingerprint != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (cert_info->sha1fingerprint != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	തിരികെ res;
}

സ്ഥിതം nserror
format_certificate(ഘടന fetch_about_context *ctx,
		   ഘടന ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	എങ്കിൽ (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = format_certificate_public_താക്കോൽ(ctx, &cert_info->public_താക്കോൽ);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	എങ്കിൽ (cert_info->serialnum != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	എങ്കിൽ (cert_info->sig_algor != ശൂന്യ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	തിരികെ res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(ഘടന fetch_about_context *ctx)
{
	പൂർണ്ണസംഖ്യ code = 200;
	nserror res;
	ഘടന cert_chain *chain = ശൂന്യ;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	എങ്കിൽ (fetch_about_send_header(ctx, "Content-Type: text/html"))
		പോകുക fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	എങ്കിൽ (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		എങ്കിൽ (res != NSERROR_OK) {
			പോകുക fetch_about_certificate_handler_aborted;
		}
	} അല്ലെങ്കിൽ {
		ഘടന ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		എങ്കിൽ (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			എങ്കിൽ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				പോകുക fetch_about_certificate_handler_aborted;
			}

			വേണ്ടി (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				എങ്കിൽ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					പോകുക fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			എങ്കിൽ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				പോകുക fetch_about_certificate_handler_aborted;
			}

			വേണ്ടി (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				എങ്കിൽ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					പോകുക fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} അല്ലെങ്കിൽ {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			എങ്കിൽ (res != NSERROR_OK) {
				പോകുക fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	തിരികെ true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	തിരികെ false;
}
