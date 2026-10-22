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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "netsurf/ssl_certs.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "certificate.h"

/**
 * certificate name parameters
 */
কাঠামো ns_cert_name {
	অক্ষর *common_name;
	অক্ষর *organisation;
	অক্ষর *organisation_unit;
	অক্ষর *locality;
	অক্ষর *province;
	অক্ষর *country;
};

/**
 * Certificate public key parameters
 */
কাঠামো ns_cert_pkey {
	অক্ষর *algor;
	পূর্ণসংখ্যা আকার_2;
	অক্ষর *modulus;
	অক্ষর *exponent;
	অক্ষর *curve;
	অক্ষর *সর্বজনীন;
};

/**
 * Certificate subject alternative name
 */
কাঠামো ns_cert_san {
	কাঠামো ns_cert_san *পরবর্তী;
	অক্ষর *name;
};

/**
 * certificate information for certificate chain
 */
কাঠামো ns_cert_info {
	কাঠামো ns_cert_name subject_name; /**< Subject details */
	কাঠামো ns_cert_name issuer_name; /**< Issuer details */
	কাঠামো ns_cert_pkey public_চাবি; /**< public key details */
	long version;		/**< Certificate version */
	অক্ষর *not_before;	/**< Valid from date */
	অক্ষর *not_after;	/**< Valid to date */
	পূর্ণসংখ্যা sig_type;		/**< Signature type */
	অক্ষর *sig_algor;        /**< Signature Algorithm */
	অক্ষর *serialnum;	/**< Serial number */
	অক্ষর *sha1fingerprint; /**< fingerprint shar1 encoded */
	অক্ষর *sha256fingerprint; /**< fingerprint shar256 encoded */
	কাঠামো ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
স্থির nserror free_ns_cert_info(কাঠামো ns_cert_info *cinfo)
{
	কাঠামো ns_cert_san *san;

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
	মুক্ত(cinfo->public_চাবি.সর্বজনীন);
	মুক্ত(cinfo->not_before);
	মুক্ত(cinfo->not_after);
	মুক্ত(cinfo->sig_algor);
	মুক্ত(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	যখন (san != ফাঁকা) {
		কাঠামো ns_cert_san *পরবর্তী;
		পরবর্তী = san->পরবর্তী;
		মুক্ত(san);
		san = পরবর্তী;
	}

	মুক্ত(cinfo);

	ফেরত NSERROR_OK;
}

#ifdef WITH_OPENSSL

#অন্তর্ভাব <openssl/ssl.h>
#অন্তর্ভাব <openssl/x509v3.h>

#যদি (OPENSSL_VERSION_সংখ্যা < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# যদি defined(LIBRESSL_VERSION_সংখ্যা)
  /* LibreSSL */
#  যদি (LIBRESSL_VERSION_সংখ্যা < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   যদি (LIBRESSL_VERSION_সংখ্যা < 0x2070000fL)
    /* LibreSSL <2.7.0 */
স্থির পূর্ণসংখ্যা ns_X509_get_signature_nid(X509 *cert)
{
	ফেরত OBJ_obj2nid(cert->cert_info->চাবি->algor->algorithm);
}

স্থির ধ্রুবক unsigned অক্ষর *ns_ASN1_STRING_get0_তথ্য(ASN1_STRING *asn1str)
{
	ফেরত (ধ্রুবক unsigned অক্ষর *)ASN1_STRING_তথ্য(asn1str);
}
#   অন্যথা
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_তথ্য ASN1_STRING_get0_তথ্য
#   endif

স্থির ধ্রুবক BIGNUM *ns_RSA_get0_n(ধ্রুবক RSA *d)
{
	ফেরত d->n;
}

স্থির ধ্রুবক BIGNUM *ns_RSA_get0_e(ধ্রুবক RSA *d)
{
	ফেরত d->e;
}
#  অন্যথা
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_তথ্য ASN1_STRING_get0_তথ্য
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# অন্যথা
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_তথ্য ASN1_STRING_get0_তথ্য
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

স্থির পূর্ণসংখ্যা ns_EVP_PKEY_get_bn_param(ধ্রুবক EVP_PKEY *pkey,
		ধ্রুবক অক্ষর *চাবি_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *ফলাফল = ফাঁকা;

	/* Check parameters: only support allocation-form *bn */
	যদি (pkey == ফাঁকা || চাবি_name == ফাঁকা || bn == ফাঁকা || *bn != ফাঁকা)
		ফেরত 0;

	/* Only support RSA keys */
	যদি (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		ফেরত 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	যদি (rsa == ফাঁকা)
		ফেরত 0;

	যদি (strcmp(চাবি_name, "n") == 0) {
		ধ্রুবক BIGNUM *n = ns_RSA_get0_n(rsa);
		যদি (n != ফাঁকা)
			ফলাফল = BN_dup(n);
	} অন্যথা যদি (strcmp(চাবি_name, "e") == 0) {
		ধ্রুবক BIGNUM *e = ns_RSA_get0_e(rsa);
		যদি (e != ফাঁকা)
			ফলাফল = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = ফলাফল;

	ফেরত (ফলাফল != ফাঁকা) ? 1 : 0;
}

স্থির পূর্ণসংখ্যা ns_EVP_PKEY_get_utf8_string_param(ধ্রুবক EVP_PKEY *pkey,
		ধ্রুবক অক্ষর *চাবি_name, অক্ষর *str, size_t max_len,
		size_t *out_len)
{
	ধ্রুবক EC_GROUP *ecgroup;
	ধ্রুবক অক্ষর *group = "";
	size_t group_len = 0;
	EC_চাবি *ec;
	পূর্ণসংখ্যা ret = 0;

	যদি (pkey == ফাঁকা || চাবি_name == ফাঁকা)
		ফেরত 0;

	/* Only support EC keys */
	যদি (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ফেরত 0;

	/* Only support fetching the group */
	যদি (strcmp(চাবি_name, "group") != 0)
		ফেরত 0;

	ec = EVP_PKEY_get1_EC_চাবি((EVP_PKEY *) pkey);

	ecgroup = EC_চাবি_get0_group(ec);
	যদি (ecgroup != ফাঁকা) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	যদি (str != ফাঁকা && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	যদি (out_len != ফাঁকা) {
		*out_len = group_len;
	}

	EC_চাবি_free(ec);

	ফেরত ret;
}

স্থির পূর্ণসংখ্যা ns_EVP_PKEY_get_octet_string_param(ধ্রুবক EVP_PKEY *pkey,
		ধ্রুবক অক্ষর *চাবি_name, unsigned অক্ষর *buf, size_t max_len,
		size_t *out_len)
{
	ধ্রুবক EC_GROUP *ecgroup;
	ধ্রুবক EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_চাবি *ec;
	পূর্ণসংখ্যা ret = 0;

	যদি (pkey == ফাঁকা || চাবি_name == ফাঁকা)
		ফেরত 0;

	/* Only support EC keys */
	যদি (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		ফেরত 0;

	যদি (strcmp(চাবি_name, "encoded-pub-key") != 0)
		ফেরত 0;

	ec = EVP_PKEY_get1_EC_চাবি((EVP_PKEY *) pkey);
	যদি (ec == ফাঁকা)
		ফেরত 0;

	ecgroup = EC_চাবি_get0_group(ec);
	যদি (ecgroup != ফাঁকা) {
		ecpoint = EC_চাবি_get0_public_চাবি(ec);
		যদি (ecpoint != ফাঁকা) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 ফাঁকা,
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
			যদি (out_len != ফাঁকা)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_চাবি_free(ec);

	ফেরত ret;
}
#অন্যথা
/* OpenSSL 3.x and later */
#define ns_X509_get_signature_nid X509_get_signature_nid
#define ns_ASN1_STRING_get0_তথ্য ASN1_STRING_get0_তথ্য
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
xname_to_info(X509_NAME *xname, কাঠামো ns_cert_name *iname)
{
	পূর্ণসংখ্যা entryidx;
	পূর্ণসংখ্যা entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *মান;
	ধ্রুবক unsigned অক্ষর *মান_str;
	ASN1_OBJECT *name;
	পূর্ণসংখ্যা name_nid;
	অক্ষর **field;

	entrycnt = X509_NAME_entry_গুনতি(xname);

	জন্য (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		মান = X509_NAME_ENTRY_get_তথ্য(entry);
		মান_str = ns_ASN1_STRING_get0_তথ্য(মান);
		নির্বাচন (name_nid) {
		ক্ষেত্র NID_commonName:
			field = &iname->common_name;
			বিরতি;
		ক্ষেত্র NID_countryName:
			field = &iname->country;
			বিরতি;
		ক্ষেত্র NID_localityName:
			field = &iname->locality;
			বিরতি;
		ক্ষেত্র NID_stateOrProvinceName:
			field = &iname->province;
			বিরতি;
		ক্ষেত্র NID_organizationName:
			field = &iname->organisation;
			বিরতি;
		ক্ষেত্র NID_organizationalUnitName:
			field = &iname->organisation_unit;
			বিরতি;
		পূর্বনির্ধারিত :
			field = ফাঁকা;
			বিরতি;
		}
		যদি (field != ফাঁকা) {
			*field = strdup((ধ্রুবক অক্ষর *)মান_str);
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
	যদি (iname->common_name == ফাঁকা) {
		iname->common_name = strdup("Unknown");
	}

	ফেরত NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
স্থির অক্ষর *hexdup(ধ্রুবক অক্ষর *hex)
{
	পূর্ণসংখ্যা hexlen;
	অক্ষর *dst;
	অক্ষর *out;
	পূর্ণসংখ্যা cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	যদি (dst != ফাঁকা) {
		জন্য (out = dst; *hex != 0; hex++) {
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
	ফেরত dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
স্থির অক্ষর *bindup(unsigned অক্ষর *bin, unsigned পূর্ণসংখ্যা binlen)
{
	অক্ষর *dst;
	অক্ষর *out;
	unsigned পূর্ণসংখ্যা idx;
	ধ্রুবক অক্ষর hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	যদি (dst != ফাঁকা) {
		out = dst;
		জন্য (idx = 0; idx < binlen; idx++) {
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
	ফেরত dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
rsa_to_info(EVP_PKEY *pkey, কাঠামো ns_cert_pkey *ikey)
{
	BIGNUM *n = ফাঁকা, *e = ফাঁকা;
	অক্ষর *tmp;

	যদি (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		ফেরত NSERROR_BAD_PARAMETER;
	}

	যদি (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		ফেরত NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->আকার_2 = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	যদি (tmp != ফাঁকা) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	যদি (tmp != ফাঁকা) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	ফেরত NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
dsa_to_info(EVP_PKEY *pkey, কাঠামো ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->আকার_2 = EVP_PKEY_bits(pkey);

	ফেরত NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
dh_to_info(EVP_PKEY *pkey, কাঠামো ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->আকার_2 = EVP_PKEY_bits(pkey);

	ফেরত NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
ec_to_info(EVP_PKEY *pkey, কাঠামো ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->আকার_2 = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ফাঁকা, 0, &len);
	যদি (len != 0) {
		ikey->curve = malloc(len + 1);
		যদি (ikey->curve != ফাঁকা) {
			যদি (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, ফাঁকা) == 0) {
				মুক্ত(ikey->curve);
				ikey->curve = ফাঁকা;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ফাঁকা, 0, &len);
	যদি (len != 0) {
		unsigned অক্ষর *point = malloc(len);
		যদি (point != ফাঁকা) {
			যদি (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					ফাঁকা) == 1) {
				ikey->সর্বজনীন = bindup(point, len);
			}
			মুক্ত(point);
		}
	}

	ফেরত NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
স্থির nserror
pkey_to_info(EVP_PKEY *pkey, কাঠামো ns_cert_pkey *ikey)
{
	nserror res;

	যদি (pkey == ফাঁকা) {
		ফেরত NSERROR_BAD_PARAMETER;
	}

	নির্বাচন (EVP_PKEY_base_id(pkey)) {
	ক্ষেত্র EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		বিরতি;

	ক্ষেত্র EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		বিরতি;

	ক্ষেত্র EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		বিরতি;

	ক্ষেত্র EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		বিরতি;

	পূর্বনির্ধারিত:
		res = NSERROR_NOT_IMPLEMENTED;
		বিরতি;
	}

	EVP_PKEY_free(pkey);

	ফেরত res;
}

স্থির nserror san_to_info(X509 *cert, কাঠামো ns_cert_san **পূর্ব_পরবর্তী)
{
	পূর্ণসংখ্যা idx;
	পূর্ণসংখ্যা san_names_nb = -1;
	ধ্রুবক GENERAL_NAME *current_name;
	ধ্রুবক unsigned অক্ষর *dns_name;
	কাঠামো ns_cert_san *isan;

	স্তূপ_OF(GENERAL_NAME) *san_names = ফাঁকা;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, ফাঁকা, ফাঁকা);
	যদি (san_names == ফাঁকা) {
		ফেরত NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_অঙ্ক(san_names);

	/* Check each name within the extension */
	জন্য (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_মান(san_names, idx);

		যদি (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_তথ্য(current_name->d.dNSName);

			isan = malloc(আকার(কাঠামো ns_cert_san));
			যদি (isan != ফাঁকা) {
				isan->name = strdup((ধ্রুবক অক্ষর *)dns_name);
				isan->পরবর্তী = ফাঁকা;
				*পূর্ব_পরবর্তী = isan;
				পূর্ব_পরবর্তী = &isan->পরবর্তী;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	জন্য (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_বের_করো(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	ফেরত NSERROR_OK;
}

স্থির nserror
der_to_certinfo(ধ্রুবক uint8_t *der,
		size_t der_দৈর্ঘ্য,
		কাঠামো ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ধ্রুবক ASN1_INTEGER *asn1_অঙ্ক;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	যদি (der == ফাঁকা) {
		ফেরত NSERROR_OK;
	}

	cert = d2i_X509(ফাঁকা, &der, der_দৈর্ঘ্য);
	যদি (cert == ফাঁকা) {
		ফেরত NSERROR_INVALID;
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
	info->not_before = calloc(1, buf->দৈর্ঘ্য + 1);
	যদি (info->not_before != ফাঁকা) {
		memcpy(info->not_before, buf->তথ্য, (unsigned)buf->দৈর্ঘ্য);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(শূন্য) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->দৈর্ঘ্য + 1);
	যদি (info->not_after != ফাঁকা) {
		memcpy(info->not_after, buf->তথ্য, (unsigned)buf->দৈর্ঘ্য);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	পূর্ণসংখ্যা pkey_nid = ns_X509_get_signature_nid(cert);
	যদি (pkey_nid != NID_undef) {
		ধ্রুবক অক্ষর* sslbuf = OBJ_nid2ln(pkey_nid);
		যদি (sslbuf != ফাঁকা) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_অঙ্ক = X509_get_serial_সংখ্যা(cert);
	যদি (asn1_অঙ্ক != ফাঁকা) {
		bignum = ASN1_INTEGER_to_BN(asn1_অঙ্ক, ফাঁকা);
		যদি (bignum != ফাঁকা) {
			অক্ষর *tmp = BN_bn2hex(bignum);
			যদি (tmp != ফাঁকা) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ফাঁকা;
		}
	}

	/* fingerprints */
	ধ্রুবক EVP_MD *digest;
	unsigned পূর্ণসংখ্যা dig_len;
	unsigned অক্ষর *buff;
	পূর্ণসংখ্যা rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_আকার(digest));
	যদি (buff != ফাঁকা) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		যদি ((rc == 1) && (dig_len == (unsigned পূর্ণসংখ্যা)EVP_MD_আকার(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		মুক্ত(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_আকার(digest));
	যদি (buff != ফাঁকা) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		যদি ((rc == 1) && (dig_len == (unsigned পূর্ণসংখ্যা)EVP_MD_আকার(digest))) {
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

	ফেরত NSERROR_OK;
}

/* copy certificate data */
স্থির nserror
convert_chain_to_cert_info(ধ্রুবক কাঠামো cert_chain *chain,
			   কাঠামো ns_cert_info **cert_info_out)
{
	কাঠামো ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, আকার(কাঠামো ns_cert_info));
	যদি (certs == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	জন্য (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_দৈর্ঘ্য,
				      certs + depth);
		যদি (res != NSERROR_OK) {
			মুক্ত(certs);
			ফেরত res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	ফেরত NSERROR_OK;
}

#অন্যথা
স্থির nserror
convert_chain_to_cert_info(ধ্রুবক কাঠামো cert_chain *chain,
			   কাঠামো ns_cert_info **cert_info_out)
{
	ফেরত NSERROR_NOT_IMPLEMENTED;
}
#endif


স্থির nserror
format_certificate_name(কাঠামো fetch_about_context *ctx,
			কাঠামো ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	যদি (cert_name->organisation != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (cert_name->organisation_unit != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (cert_name->locality != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (cert_name->province != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (cert_name->country != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	ফেরত res;
}

/**
 * output formatted certificate subject alternate names
 */
স্থির nserror
format_certificate_san(কাঠামো fetch_about_context *ctx,
			      কাঠামো ns_cert_san *san)
{
	nserror res;

	যদি (san == ফাঁকা) {
		ফেরত NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	যখন (san != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}

		san = san->পরবর্তী;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ফেরত res;

}


স্থির nserror
format_certificate_public_চাবি(কাঠামো fetch_about_context *ctx,
			      কাঠামো ns_cert_pkey *public_চাবি)
{
	nserror res;

	যদি (public_চাবি->algor == ফাঁকা) {
		/* skip the table if no algorithm name */
		ফেরত NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_চাবি->algor,
			 public_চাবি->আকার_2);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}


	যদি (public_চাবি->exponent != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_চাবি->exponent);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (public_চাবি->modulus != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_চাবি->modulus);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (public_চাবি->curve != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_চাবি->curve);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (public_চাবি->সর্বজনীন != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_চাবি->সর্বজনীন);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ফেরত res;
}

স্থির nserror
format_certificate_fingerprint(কাঠামো fetch_about_context *ctx,
			      কাঠামো ns_cert_info *cert_info)
{
	nserror res;

	যদি ((cert_info->sha1fingerprint == ফাঁকা) &&
	    (cert_info->sha256fingerprint == ফাঁকা))  {
		/* skip the table if no fingerprints */
		ফেরত NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	যদি (cert_info->sha256fingerprint != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (cert_info->sha1fingerprint != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	ফেরত res;
}

স্থির nserror
format_certificate(কাঠামো fetch_about_context *ctx,
		   কাঠামো ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	যদি (res != NSERROR_OK) {
		ফেরত res;
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
			ফেরত res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	যদি (res != NSERROR_OK) {
		ফেরত res;
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
		ফেরত res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = format_certificate_public_চাবি(ctx, &cert_info->public_চাবি);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	যদি (cert_info->serialnum != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	যদি (cert_info->sig_algor != ফাঁকা) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		যদি (res != NSERROR_OK) {
			ফেরত res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	ফেরত res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(কাঠামো fetch_about_context *ctx)
{
	পূর্ণসংখ্যা code = 200;
	nserror res;
	কাঠামো cert_chain *chain = ফাঁকা;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/html"))
		যাও fetch_about_certificate_handler_aborted;

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
		যাও fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	যদি (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		যদি (res != NSERROR_OK) {
			যাও fetch_about_certificate_handler_aborted;
		}
	} অন্যথা {
		কাঠামো ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		যদি (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			যদি (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				যাও fetch_about_certificate_handler_aborted;
			}

			জন্য (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				যদি (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					যাও fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			যদি (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				যাও fetch_about_certificate_handler_aborted;
			}

			জন্য (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				যদি (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					যাও fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} অন্যথা {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			যদি (res != NSERROR_OK) {
				যাও fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	যদি (res != NSERROR_OK) {
		যাও fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	ফেরত true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	ফেরত false;
}
