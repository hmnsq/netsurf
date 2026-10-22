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

#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "utils/errors.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "netsurf/ssl_certs.h"

#অন্তর্ভাৱ "private.h"
#অন্তর্ভাৱ "certificate.h"

/**
 * certificate name parameters
 */
সংরচনা ns_cert_name {
	ৱর্ণ *common_name;
	ৱর্ণ *organisation;
	ৱর্ণ *organisation_unit;
	ৱর্ণ *locality;
	ৱর্ণ *province;
	ৱর্ণ *country;
};

/**
 * Certificate public key parameters
 */
সংরচনা ns_cert_pkey {
	ৱর্ণ *algor;
	পূর্ণাঙ্ক size;
	ৱর্ণ *modulus;
	ৱর্ণ *exponent;
	ৱর্ণ *curve;
	ৱর্ণ *সার্ৱজনিক;
};

/**
 * Certificate subject alternative name
 */
সংরচনা ns_cert_san {
	সংরচনা ns_cert_san *next;
	ৱর্ণ *name;
};

/**
 * certificate information for certificate chain
 */
সংরচনা ns_cert_info {
	সংরচনা ns_cert_name subject_name; /**< Subject details */
	সংরচনা ns_cert_name issuer_name; /**< Issuer details */
	সংরচনা ns_cert_pkey public_চাবি; /**< public key details */
	long version;		/**< Certificate version */
	ৱর্ণ *not_before;	/**< Valid from date */
	ৱর্ণ *not_after;	/**< Valid to date */
	পূর্ণাঙ্ক sig_type;		/**< Signature type */
	ৱর্ণ *sig_algor;        /**< Signature Algorithm */
	ৱর্ণ *serialnum;	/**< Serial number */
	ৱর্ণ *sha1fingerprint; /**< fingerprint shar1 encoded */
	ৱর্ণ *sha256fingerprint; /**< fingerprint shar256 encoded */
	সংরচনা ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
স্থির nserror free_ns_cert_info(সংরচনা ns_cert_info *cinfo)
{
	সংরচনা ns_cert_san *san;

	মুক্ত(cinfo->subject_name.common_name);
	মুক্ত(cinfo->subject_name.organisation);
	মুক্ত(cinfo->subject_name.organisation_unit);
	মুক্ত(cinfo->subject_name.locality);
	মুক্ত(cinfo->subject_name.province);
	মুক্ত(cinfo->subject_name.country);
	মুক্ত(cinfo->issuer_name.common_name);
	মুক্ত(cinfo->issuer_name.organisation);
	মুক্ত(cinfo->issuer_name.organisation_unit);
	মুক্ত(cinfo->issuer_name.locality);
	মুক্ত(cinfo->issuer_name.province);
	মুক্ত(cinfo->issuer_name.country);
	মুক্ত(cinfo->public_চাবি.algor);
	মুক্ত(cinfo->public_চাবি.modulus);
	মুক্ত(cinfo->public_চাবি.exponent);
	মুক্ত(cinfo->public_চাবি.curve);
	মুক্ত(cinfo->public_চাবি.সার্ৱজনিক);
	মুক্ত(cinfo->not_before);
	মুক্ত(cinfo->not_after);
	মুক্ত(cinfo->sig_algor);
	মুক্ত(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	যেতিয়ালৈ (san != রিক্ত) {
		সংরচনা ns_cert_san *next;
		next = san->next;
		মুক্ত(san);
		san = next;
	}

	মুক্ত(cinfo);

	ঘূৰাই NSERROR_OK;
}

#ifdef WITH_OPENSSL

#অন্তর্ভাৱ <openssl/ssl.h>
#অন্তর্ভাৱ <openssl/x509v3.h>

#যদি (OPENSSL_VERSION_সংখ্যা < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# যদি defined(LIBRESSL_VERSION_সংখ্যা)
  /* LibreSSL */
#  যদি (LIBRESSL_VERSION_সংখ্যা < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   যদি (LIBRESSL_VERSION_সংখ্যা < 0x2070000fL)
    /* LibreSSL <2.7.0 */
স্থির পূর্ণাঙ্ক ns_X509_get_signature_nid(X509 *cert)
{
	ঘূৰাই OBJ_obj2nid(cert->cert_info->চাবি->algor->algorithm);
}

স্থির ধ্রুৱ unsigned ৱর্ণ *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	ঘূৰাই (ধ্রুৱ unsigned ৱর্ণ *)ASN1_STRING_data(asn1str);
}
#   অন্যথা
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

স্থির ধ্রুৱ BIGNUM *ns_RSA_get0_n(ধ্রুৱ RSA *d)
{
	ঘূৰাই d->n;
}

স্থির ধ্রুৱ BIGNUM *ns_RSA_get0_e(ধ্রুৱ RSA *d)
{
	ঘূৰাই d->e;
}
#  অন্যথা
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# অন্যথা
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

স্থির পূর্ণাঙ্ক ns_EVP_PKEY_get_bn_param(ধ্রুৱ EVP_PKEY *pkey,
		ধ্রুৱ ৱর্ণ *চাবি_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = রিক্ত;

	/* Check parameters: only support allocation-form *bn */
	যদি (pkey == রিক্ত || চাবি_name == রিক্ত || bn == রিক্ত || *bn != রিক্ত)
		ঘূৰাই 0;

	/* Only support RSA keys */
	যদি (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		ঘূৰাই 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	যদি (rsa == রিক্ত)
		ঘূৰাই 0;

	যদি (strcmp(চাবি_name, "n") == 0) {
		ধ্রুৱ BIGNUM *n = ns_RSA_get0_n(rsa);
		যদি (n != রিক্ত)
			result = BN_dup(n);
	} অন্যথা যদি (strcmp(চাবি_name, "e") == 0) {
		ধ্রুৱ BIGNUM *e = ns_RSA_get0_e(rsa);
		যদি (e != রিক্ত)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	ঘূৰাই (result != রিক্ত) ? 1 : 0;
}

স্থির পূর্ণাঙ্ক ns_EVP_PKEY_get_utf8_string_param(ধ্রুৱ EVP_PKEY *pkey,
		ধ্রুৱ ৱর্ণ *চাবি_name, ৱর্ণ *str, size_t max_len,
		size_t *out_len)
{
	ধ্রুৱ EC_GROUP *ecgroup;
	ধ্রুৱ ৱর্ণ *group = "";
	size_t group_len = 0;
	EC_চাবি *ec;
	পূর্ণাঙ্ক ret = 0;

	যদি (pkey == রিক্ত || চাবি_name == রিক্ত)
		ঘূৰাই 0;

	/* Only support EC keys */
	যদি (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ঘূৰাই 0;

	/* Only support fetching the group */
	যদি (strcmp(চাবি_name, "group") != 0)
		ঘূৰাই 0;

	ec = EVP_PKEY_get1_EC_চাবি((EVP_PKEY *) pkey);

	ecgroup = EC_চাবি_get0_group(ec);
	যদি (ecgroup != রিক্ত) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	যদি (str != রিক্ত && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	যদি (out_len != রিক্ত) {
		*out_len = group_len;
	}

	EC_চাবি_free(ec);

	ঘূৰাই ret;
}

স্থির পূর্ণাঙ্ক ns_EVP_PKEY_get_octet_string_param(ধ্রুৱ EVP_PKEY *pkey,
		ধ্রুৱ ৱর্ণ *চাবি_name, unsigned ৱর্ণ *buf, size_t max_len,
		size_t *out_len)
{
	ধ্রুৱ EC_GROUP *ecgroup;
	ধ্রুৱ EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_চাবি *ec;
	পূর্ণাঙ্ক ret = 0;

	যদি (pkey == রিক্ত || চাবি_name == রিক্ত)
		ঘূৰাই 0;

	/* Only support EC keys */
	যদি (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ঘূৰাই 0;

	যদি (strcmp(চাবি_name, "encoded-pub-key") != 0)
		ঘূৰাই 0;

	ec = EVP_PKEY_get1_EC_চাবি((EVP_PKEY *) pkey);
	যদি (ec == রিক্ত)
		ঘূৰাই 0;

	ecgroup = EC_চাবি_get0_group(ec);
	যদি (ecgroup != রিক্ত) {
		ecpoint = EC_চাবি_get0_public_চাবি(ec);
		যদি (ecpoint != রিক্ত) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 রিক্ত,
						 0,
						 bnctx);
			যদি (len != 0 && len <= max_len) {
				যদি (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			যদি (out_len != রিক্ত)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_চাবি_free(ec);

	ঘূৰাই ret;
}
#অন্যথা
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
স্থির nserror
xname_to_info(X509_NAME *xname, সংরচনা ns_cert_name *iname)
{
	পূর্ণাঙ্ক entryidx;
	পূর্ণাঙ্ক entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *মান;
	ধ্রুৱ unsigned ৱর্ণ *মান_str;
	ASN1_OBJECT *name;
	পূর্ণাঙ্ক name_nid;
	ৱর্ণ **field;

	entrycnt = X509_NAME_entry_গুনতি(xname);

	বাবে (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		মান = X509_NAME_ENTRY_get_data(entry);
		মান_str = ns_ASN1_STRING_get0_data(মান);
		ৱিকল্প (name_nid) {
		পক্ষ NID_commonName:
			field = &iname->common_name;
			ভঙ্গ;
		পক্ষ NID_countryName:
			field = &iname->country;
			ভঙ্গ;
		পক্ষ NID_localityName:
			field = &iname->locality;
			ভঙ্গ;
		পক্ষ NID_stateOrProvinceName:
			field = &iname->province;
			ভঙ্গ;
		পক্ষ NID_organizationName:
			field = &iname->organisation;
			ভঙ্গ;
		পক্ষ NID_organizationalUnitName:
			field = &iname->organisation_unit;
			ভঙ্গ;
		স্ৱভাৱ :
			field = রিক্ত;
			ভঙ্গ;
		}
		যদি (field != রিক্ত) {
			*field = strdup((ধ্রুৱ ৱর্ণ *)মান_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} অন্যথা {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	যদি (iname->common_name == রিক্ত) {
		iname->common_name = strdup("Unknown");
	}

	ঘূৰাই NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
স্থির ৱর্ণ *hexdup(ধ্রুৱ ৱর্ণ *hex)
{
	পূর্ণাঙ্ক hexlen;
	ৱর্ণ *dst;
	ৱর্ণ *out;
	পূর্ণাঙ্ক cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	যদি (dst != রিক্ত) {
		বাবে (out = dst; *hex != 0; hex++) {
			যদি (cn == 2) {
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
	ঘূৰাই dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
স্থির ৱর্ণ *bindup(unsigned ৱর্ণ *bin, unsigned পূর্ণাঙ্ক binlen)
{
	ৱর্ণ *dst;
	ৱর্ণ *out;
	unsigned পূর্ণাঙ্ক idx;
	ধ্রুৱ ৱর্ণ hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	যদি (dst != রিক্ত) {
		out = dst;
		বাবে (idx = 0; idx < binlen; idx++) {
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
	ঘূৰাই dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
rsa_to_info(EVP_PKEY *pkey, সংরচনা ns_cert_pkey *ikey)
{
	BIGNUM *n = রিক্ত, *e = রিক্ত;
	ৱর্ণ *tmp;

	যদি (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	যদি (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	যদি (tmp != রিক্ত) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	যদি (tmp != রিক্ত) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ঘূৰাই NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
dsa_to_info(EVP_PKEY *pkey, সংরচনা ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	ঘূৰাই NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
dh_to_info(EVP_PKEY *pkey, সংরচনা ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	ঘূৰাই NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
ec_to_info(EVP_PKEY *pkey, সংরচনা ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", রিক্ত, 0, &len);
	যদি (len != 0) {
		ikey->curve = malloc(len + 1);
		যদি (ikey->curve != রিক্ত) {
			যদি (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, রিক্ত) == 0) {
				মুক্ত(ikey->curve);
				ikey->curve = রিক্ত;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			রিক্ত, 0, &len);
	যদি (len != 0) {
		unsigned ৱর্ণ *point = malloc(len);
		যদি (point != রিক্ত) {
			যদি (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					রিক্ত) == 1) {
				ikey->সার্ৱজনিক = bindup(point, len);
			}
			মুক্ত(point);
		}
	}

	ঘূৰাই NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
pkey_to_info(EVP_PKEY *pkey, সংরচনা ns_cert_pkey *ikey)
{
	nserror res;

	যদি (pkey == রিক্ত) {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	ৱিকল্প (EVP_PKEY_base_id(pkey)) {
	পক্ষ EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		ভঙ্গ;

	পক্ষ EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		ভঙ্গ;

	পক্ষ EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		ভঙ্গ;

	পক্ষ EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		ভঙ্গ;

	স্ৱভাৱ:
		res = NSERROR_NOT_IMPLEMENTED;
		ভঙ্গ;
	}

	EVP_PKEY_free(pkey);

	ঘূৰাই res;
}

স্থির nserror san_to_info(X509 *cert, সংরচনা ns_cert_san **prev_next)
{
	পূর্ণাঙ্ক idx;
	পূর্ণাঙ্ক san_names_nb = -1;
	ধ্রুৱ GENERAL_NAME *current_name;
	ধ্রুৱ unsigned ৱর্ণ *dns_name;
	সংরচনা ns_cert_san *isan;

	স্তূপ_OF(GENERAL_NAME) *san_names = রিক্ত;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, রিক্ত, রিক্ত);
	যদি (san_names == রিক্ত) {
		ঘূৰাই NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_অঙ্ক(san_names);

	/* Check each name within the extension */
	বাবে (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_মান(san_names, idx);

		যদি (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(পরিমাণ(সংরচনা ns_cert_san));
			যদি (isan != রিক্ত) {
				isan->name = strdup((ধ্রুৱ ৱর্ণ *)dns_name);
				isan->next = রিক্ত;
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
	বাবে (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_বেৰ_কৰো(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	ঘূৰাই NSERROR_OK;
}

স্থির nserror
der_to_certinfo(ধ্রুৱ uint8_t *der,
		size_t der_length,
		সংরচনা ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ধ্রুৱ ASN1_INTEGER *asn1_অঙ্ক;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	যদি (der == রিক্ত) {
		ঘূৰাই NSERROR_OK;
	}

	cert = d2i_X509(রিক্ত, &der, der_length);
	যদি (cert == রিক্ত) {
		ঘূৰাই NSERROR_INVALID;
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
	(শূন্য) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	যদি (info->not_before != রিক্ত) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(শূন্য) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	যদি (info->not_after != রিক্ত) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	পূর্ণাঙ্ক pkey_nid = ns_X509_get_signature_nid(cert);
	যদি (pkey_nid != NID_undef) {
		ধ্রুৱ ৱর্ণ* sslbuf = OBJ_nid2ln(pkey_nid);
		যদি (sslbuf != রিক্ত) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_অঙ্ক = X509_get_serial_সংখ্যা(cert);
	যদি (asn1_অঙ্ক != রিক্ত) {
		bignum = ASN1_INTEGER_to_BN(asn1_অঙ্ক, রিক্ত);
		যদি (bignum != রিক্ত) {
			ৱর্ণ *tmp = BN_bn2hex(bignum);
			যদি (tmp != রিক্ত) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = রিক্ত;
		}
	}

	/* fingerprints */
	ধ্রুৱ EVP_MD *digest;
	unsigned পূর্ণাঙ্ক dig_len;
	unsigned ৱর্ণ *buff;
	পূর্ণাঙ্ক rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	যদি (buff != রিক্ত) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		যদি ((rc == 1) && (dig_len == (unsigned পূর্ণাঙ্ক)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		মুক্ত(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	যদি (buff != রিক্ত) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		যদি ((rc == 1) && (dig_len == (unsigned পূর্ণাঙ্ক)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		মুক্ত(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_চাবি);

	X509_free(cert);

	ঘূৰাই NSERROR_OK;
}

/* copy certificate data */
স্থির nserror
convert_chain_to_cert_info(ধ্রুৱ সংরচনা cert_chain *chain,
			   সংরচনা ns_cert_info **cert_info_out)
{
	সংরচনা ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, পরিমাণ(সংরচনা ns_cert_info));
	যদি (certs == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	বাবে (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		যদি (res != NSERROR_OK) {
			মুক্ত(certs);
			ঘূৰাই res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	ঘূৰাই NSERROR_OK;
}

#অন্যথা
স্থির nserror
convert_chain_to_cert_info(ধ্রুৱ সংরচনা cert_chain *chain,
			   সংরচনা ns_cert_info **cert_info_out)
{
	ঘূৰাই NSERROR_NOT_IMPLEMENTED;
}
#endif


স্থির nserror
format_certificate_name(সংরচনা fetch_about_context *ctx,
			সংরচনা ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	যদি (cert_name->organisation != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (cert_name->organisation_unit != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (cert_name->locality != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (cert_name->province != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (cert_name->country != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	ঘূৰাই res;
}

/**
 * output formatted certificate subject alternate names
 */
স্থির nserror
format_certificate_san(সংরচনা fetch_about_context *ctx,
			      সংরচনা ns_cert_san *san)
{
	nserror res;

	যদি (san == রিক্ত) {
		ঘূৰাই NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	যেতিয়ালৈ (san != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ঘূৰাই res;

}


স্থির nserror
format_certificate_public_চাবি(সংরচনা fetch_about_context *ctx,
			      সংরচনা ns_cert_pkey *public_চাবি)
{
	nserror res;

	যদি (public_চাবি->algor == রিক্ত) {
		/* skip the table if no algorithm name */
		ঘূৰাই NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_চাবি->algor,
			 public_চাবি->size);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}


	যদি (public_চাবি->exponent != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_চাবি->exponent);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (public_চাবি->modulus != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_চাবি->modulus);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (public_চাবি->curve != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_চাবি->curve);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (public_চাবি->সার্ৱজনিক != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_চাবি->সার্ৱজনিক);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ঘূৰাই res;
}

স্থির nserror
format_certificate_fingerprint(সংরচনা fetch_about_context *ctx,
			      সংরচনা ns_cert_info *cert_info)
{
	nserror res;

	যদি ((cert_info->sha1fingerprint == রিক্ত) &&
	    (cert_info->sha256fingerprint == রিক্ত))  {
		/* skip the table if no fingerprints */
		ঘূৰাই NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	যদি (cert_info->sha256fingerprint != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (cert_info->sha1fingerprint != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ঘূৰাই res;
}

স্থির nserror
format_certificate(সংরচনা fetch_about_context *ctx,
		   সংরচনা ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	যদি (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = format_certificate_public_চাবি(ctx, &cert_info->public_চাবি);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	যদি (cert_info->serialnum != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	যদি (cert_info->sig_algor != রিক্ত) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	ঘূৰাই res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(সংরচনা fetch_about_context *ctx)
{
	পূর্ণাঙ্ক code = 200;
	nserror res;
	সংরচনা cert_chain *chain = রিক্ত;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/html"))
		গচ্ছ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	যদি (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		যদি (res != NSERROR_OK) {
			গচ্ছ fetch_about_certificate_handler_aborted;
		}
	} অন্যথা {
		সংরচনা ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		যদি (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			যদি (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				গচ্ছ fetch_about_certificate_handler_aborted;
			}

			বাবে (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				যদি (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					গচ্ছ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			যদি (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				গচ্ছ fetch_about_certificate_handler_aborted;
			}

			বাবে (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				যদি (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					গচ্ছ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} অন্যথা {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			যদি (res != NSERROR_OK) {
				গচ্ছ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	ঘূৰাই true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ঘূৰাই false;
}
