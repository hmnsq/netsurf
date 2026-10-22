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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "netsurf/ssl_certs.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "certificate.h"

/**
 * certificate name parameters
 */
संरचना ns_cert_name {
	वर्ण *common_name;
	वर्ण *organisation;
	वर्ण *organisation_unit;
	वर्ण *locality;
	वर्ण *province;
	वर्ण *country;
};

/**
 * Certificate public key parameters
 */
संरचना ns_cert_pkey {
	वर्ण *algor;
	पूर्णाङ्क size;
	वर्ण *modulus;
	वर्ण *exponent;
	वर्ण *curve;
	वर्ण *सार्वजनिक;
};

/**
 * Certificate subject alternative name
 */
संरचना ns_cert_san {
	संरचना ns_cert_san *next;
	वर्ण *name;
};

/**
 * certificate information for certificate chain
 */
संरचना ns_cert_info {
	संरचना ns_cert_name subject_name; /**< Subject details */
	संरचना ns_cert_name issuer_name; /**< Issuer details */
	संरचना ns_cert_pkey public_कुञ्चिका; /**< public key details */
	long version;		/**< Certificate version */
	वर्ण *not_before;	/**< Valid from date */
	वर्ण *not_after;	/**< Valid to date */
	पूर्णाङ्क sig_type;		/**< Signature type */
	वर्ण *sig_algor;        /**< Signature Algorithm */
	वर्ण *serialnum;	/**< Serial number */
	वर्ण *sha1fingerprint; /**< fingerprint shar1 encoded */
	वर्ण *sha256fingerprint; /**< fingerprint shar256 encoded */
	संरचना ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
स्थिर nserror free_ns_cert_info(संरचना ns_cert_info *cinfo)
{
	संरचना ns_cert_san *san;

	मुक्त(cinfo->subject_name.common_name);
	मुक्त(cinfo->subject_name.organisation);
	मुक्त(cinfo->subject_name.organisation_unit);
	मुक्त(cinfo->subject_name.locality);
	मुक्त(cinfo->subject_name.province);
	मुक्त(cinfo->subject_name.country);
	मुक्त(cinfo->issuer_name.common_name);
	मुक्त(cinfo->issuer_name.organisation);
	मुक्त(cinfo->issuer_name.organisation_unit);
	मुक्त(cinfo->issuer_name.locality);
	मुक्त(cinfo->issuer_name.province);
	मुक्त(cinfo->issuer_name.country);
	मुक्त(cinfo->public_कुञ्चिका.algor);
	मुक्त(cinfo->public_कुञ्चिका.modulus);
	मुक्त(cinfo->public_कुञ्चिका.exponent);
	मुक्त(cinfo->public_कुञ्चिका.curve);
	मुक्त(cinfo->public_कुञ्चिका.सार्वजनिक);
	मुक्त(cinfo->not_before);
	मुक्त(cinfo->not_after);
	मुक्त(cinfo->sig_algor);
	मुक्त(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	याव (san != रिक्त) {
		संरचना ns_cert_san *next;
		next = san->next;
		मुक्त(san);
		san = next;
	}

	मुक्त(cinfo);

	पटिनिवत्त NSERROR_OK;
}

#ifdef WITH_OPENSSL

#अन्तर्भाव <openssl/ssl.h>
#अन्तर्भाव <openssl/x509v3.h>

#सचे (OPENSSL_VERSION_सङ्ख्या < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# सचे defined(LIBRESSL_VERSION_सङ्ख्या)
  /* LibreSSL */
#  सचे (LIBRESSL_VERSION_सङ्ख्या < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   सचे (LIBRESSL_VERSION_सङ्ख्या < 0x2070000fL)
    /* LibreSSL <2.7.0 */
स्थिर पूर्णाङ्क ns_X509_get_signature_nid(X509 *cert)
{
	पटिनिवत्त OBJ_obj2nid(cert->cert_info->कुञ्चिका->algor->algorithm);
}

स्थिर ध्रुव unsigned वर्ण *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	पटिनिवत्त (ध्रुव unsigned वर्ण *)ASN1_STRING_data(asn1str);
}
#   अञ्ञथा
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

स्थिर ध्रुव BIGNUM *ns_RSA_get0_n(ध्रुव RSA *d)
{
	पटिनिवत्त d->n;
}

स्थिर ध्रुव BIGNUM *ns_RSA_get0_e(ध्रुव RSA *d)
{
	पटिनिवत्त d->e;
}
#  अञ्ञथा
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# अञ्ञथा
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_bn_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुञ्चिका_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = रिक्त;

	/* Check parameters: only support allocation-form *bn */
	सचे (pkey == रिक्त || कुञ्चिका_name == रिक्त || bn == रिक्त || *bn != रिक्त)
		पटिनिवत्त 0;

	/* Only support RSA keys */
	सचे (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		पटिनिवत्त 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	सचे (rsa == रिक्त)
		पटिनिवत्त 0;

	सचे (strcmp(कुञ्चिका_name, "n") == 0) {
		ध्रुव BIGNUM *n = ns_RSA_get0_n(rsa);
		सचे (n != रिक्त)
			result = BN_dup(n);
	} अञ्ञथा सचे (strcmp(कुञ्चिका_name, "e") == 0) {
		ध्रुव BIGNUM *e = ns_RSA_get0_e(rsa);
		सचे (e != रिक्त)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	पटिनिवत्त (result != रिक्त) ? 1 : 0;
}

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_utf8_string_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुञ्चिका_name, वर्ण *str, size_t max_len,
		size_t *out_len)
{
	ध्रुव EC_GROUP *ecgroup;
	ध्रुव वर्ण *group = "";
	size_t group_len = 0;
	EC_कुञ्चिका *ec;
	पूर्णाङ्क ret = 0;

	सचे (pkey == रिक्त || कुञ्चिका_name == रिक्त)
		पटिनिवत्त 0;

	/* Only support EC keys */
	सचे (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		पटिनिवत्त 0;

	/* Only support fetching the group */
	सचे (strcmp(कुञ्चिका_name, "group") != 0)
		पटिनिवत्त 0;

	ec = EVP_PKEY_get1_EC_कुञ्चिका((EVP_PKEY *) pkey);

	ecgroup = EC_कुञ्चिका_get0_group(ec);
	सचे (ecgroup != रिक्त) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	सचे (str != रिक्त && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	सचे (out_len != रिक्त) {
		*out_len = group_len;
	}

	EC_कुञ्चिका_free(ec);

	पटिनिवत्त ret;
}

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_octet_string_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुञ्चिका_name, unsigned वर्ण *buf, size_t max_len,
		size_t *out_len)
{
	ध्रुव EC_GROUP *ecgroup;
	ध्रुव EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_कुञ्चिका *ec;
	पूर्णाङ्क ret = 0;

	सचे (pkey == रिक्त || कुञ्चिका_name == रिक्त)
		पटिनिवत्त 0;

	/* Only support EC keys */
	सचे (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		पटिनिवत्त 0;

	सचे (strcmp(कुञ्चिका_name, "encoded-pub-key") != 0)
		पटिनिवत्त 0;

	ec = EVP_PKEY_get1_EC_कुञ्चिका((EVP_PKEY *) pkey);
	सचे (ec == रिक्त)
		पटिनिवत्त 0;

	ecgroup = EC_कुञ्चिका_get0_group(ec);
	सचे (ecgroup != रिक्त) {
		ecpoint = EC_कुञ्चिका_get0_public_कुञ्चिका(ec);
		सचे (ecpoint != रिक्त) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 रिक्त,
						 0,
						 bnctx);
			सचे (len != 0 && len <= max_len) {
				सचे (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			सचे (out_len != रिक्त)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_कुञ्चिका_free(ec);

	पटिनिवत्त ret;
}
#अञ्ञथा
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
स्थिर nserror
xname_to_info(X509_NAME *xname, संरचना ns_cert_name *iname)
{
	पूर्णाङ्क entryidx;
	पूर्णाङ्क entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *मानम्;
	ध्रुव unsigned वर्ण *मानम्_str;
	ASN1_OBJECT *name;
	पूर्णाङ्क name_nid;
	वर्ण **field;

	entrycnt = X509_NAME_entry_गणनम्(xname);

	अत्थाय (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		मानम् = X509_NAME_ENTRY_get_data(entry);
		मानम्_str = ns_ASN1_STRING_get0_data(मानम्);
		विकल्प (name_nid) {
		पक्ष NID_commonName:
			field = &iname->common_name;
			भङ्ग;
		पक्ष NID_countryName:
			field = &iname->country;
			भङ्ग;
		पक्ष NID_localityName:
			field = &iname->locality;
			भङ्ग;
		पक्ष NID_stateOrProvinceName:
			field = &iname->province;
			भङ्ग;
		पक्ष NID_organizationName:
			field = &iname->organisation;
			भङ्ग;
		पक्ष NID_organizationalUnitName:
			field = &iname->organisation_unit;
			भङ्ग;
		स्वभाव :
			field = रिक्त;
			भङ्ग;
		}
		सचे (field != रिक्त) {
			*field = strdup((ध्रुव वर्ण *)मानम्_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} अञ्ञथा {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	सचे (iname->common_name == रिक्त) {
		iname->common_name = strdup("Unknown");
	}

	पटिनिवत्त NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
स्थिर वर्ण *hexdup(ध्रुव वर्ण *hex)
{
	पूर्णाङ्क hexlen;
	वर्ण *dst;
	वर्ण *out;
	पूर्णाङ्क cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	सचे (dst != रिक्त) {
		अत्थाय (out = dst; *hex != 0; hex++) {
			सचे (cn == 2) {
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
	पटिनिवत्त dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
स्थिर वर्ण *bindup(unsigned वर्ण *bin, unsigned पूर्णाङ्क binlen)
{
	वर्ण *dst;
	वर्ण *out;
	unsigned पूर्णाङ्क idx;
	ध्रुव वर्ण hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	सचे (dst != रिक्त) {
		out = dst;
		अत्थाय (idx = 0; idx < binlen; idx++) {
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
	पटिनिवत्त dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
स्थिर nserror
rsa_to_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	BIGNUM *n = रिक्त, *e = रिक्त;
	वर्ण *tmp;

	सचे (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		पटिनिवत्त NSERROR_BAD_PARAMETER;
	}

	सचे (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		पटिनिवत्त NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	सचे (tmp != रिक्त) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	सचे (tmp != रिक्त) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	पटिनिवत्त NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
स्थिर nserror
dsa_to_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	पटिनिवत्त NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
स्थिर nserror
dh_to_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	पटिनिवत्त NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
स्थिर nserror
ec_to_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", रिक्त, 0, &len);
	सचे (len != 0) {
		ikey->curve = malloc(len + 1);
		सचे (ikey->curve != रिक्त) {
			सचे (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, रिक्त) == 0) {
				मुक्त(ikey->curve);
				ikey->curve = रिक्त;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			रिक्त, 0, &len);
	सचे (len != 0) {
		unsigned वर्ण *point = malloc(len);
		सचे (point != रिक्त) {
			सचे (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					रिक्त) == 1) {
				ikey->सार्वजनिक = bindup(point, len);
			}
			मुक्त(point);
		}
	}

	पटिनिवत्त NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
स्थिर nserror
pkey_to_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	nserror res;

	सचे (pkey == रिक्त) {
		पटिनिवत्त NSERROR_BAD_PARAMETER;
	}

	विकल्प (EVP_PKEY_base_id(pkey)) {
	पक्ष EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		भङ्ग;

	पक्ष EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		भङ्ग;

	पक्ष EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		भङ्ग;

	पक्ष EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		भङ्ग;

	स्वभाव:
		res = NSERROR_NOT_IMPLEMENTED;
		भङ्ग;
	}

	EVP_PKEY_free(pkey);

	पटिनिवत्त res;
}

स्थिर nserror san_to_info(X509 *cert, संरचना ns_cert_san **prev_next)
{
	पूर्णाङ्क idx;
	पूर्णाङ्क san_names_nb = -1;
	ध्रुव GENERAL_NAME *current_name;
	ध्रुव unsigned वर्ण *dns_name;
	संरचना ns_cert_san *isan;

	राशिः_OF(GENERAL_NAME) *san_names = रिक्त;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, रिक्त, रिक्त);
	सचे (san_names == रिक्त) {
		पटिनिवत्त NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_अङ्कः(san_names);

	/* Check each name within the extension */
	अत्थाय (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_मानम्(san_names, idx);

		सचे (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(परिमाण(संरचना ns_cert_san));
			सचे (isan != रिक्त) {
				isan->name = strdup((ध्रुव वर्ण *)dns_name);
				isan->next = रिक्त;
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
	अत्थाय (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_निष्कासय(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	पटिनिवत्त NSERROR_OK;
}

स्थिर nserror
der_to_certinfo(ध्रुव uint8_t *der,
		size_t der_length,
		संरचना ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ध्रुव ASN1_INTEGER *asn1_अङ्कः;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	सचे (der == रिक्त) {
		पटिनिवत्त NSERROR_OK;
	}

	cert = d2i_X509(रिक्त, &der, der_length);
	सचे (cert == रिक्त) {
		पटिनिवत्त NSERROR_INVALID;
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
	(शून्य) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	सचे (info->not_before != रिक्त) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(शून्य) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	सचे (info->not_after != रिक्त) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	पूर्णाङ्क pkey_nid = ns_X509_get_signature_nid(cert);
	सचे (pkey_nid != NID_undef) {
		ध्रुव वर्ण* sslbuf = OBJ_nid2ln(pkey_nid);
		सचे (sslbuf != रिक्त) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_अङ्कः = X509_get_serial_सङ्ख्या(cert);
	सचे (asn1_अङ्कः != रिक्त) {
		bignum = ASN1_INTEGER_to_BN(asn1_अङ्कः, रिक्त);
		सचे (bignum != रिक्त) {
			वर्ण *tmp = BN_bn2hex(bignum);
			सचे (tmp != रिक्त) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = रिक्त;
		}
	}

	/* fingerprints */
	ध्रुव EVP_MD *digest;
	unsigned पूर्णाङ्क dig_len;
	unsigned वर्ण *buff;
	पूर्णाङ्क rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	सचे (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		सचे ((rc == 1) && (dig_len == (unsigned पूर्णाङ्क)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		मुक्त(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	सचे (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		सचे ((rc == 1) && (dig_len == (unsigned पूर्णाङ्क)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		मुक्त(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_कुञ्चिका);

	X509_free(cert);

	पटिनिवत्त NSERROR_OK;
}

/* copy certificate data */
स्थिर nserror
convert_chain_to_cert_info(ध्रुव संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_out)
{
	संरचना ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, परिमाण(संरचना ns_cert_info));
	सचे (certs == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	अत्थाय (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		सचे (res != NSERROR_OK) {
			मुक्त(certs);
			पटिनिवत्त res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	पटिनिवत्त NSERROR_OK;
}

#अञ्ञथा
स्थिर nserror
convert_chain_to_cert_info(ध्रुव संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_out)
{
	पटिनिवत्त NSERROR_NOT_IMPLEMENTED;
}
#endif


स्थिर nserror
format_certificate_name(संरचना fetch_about_context *ctx,
			संरचना ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	सचे (cert_name->organisation != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (cert_name->organisation_unit != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (cert_name->locality != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (cert_name->province != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (cert_name->country != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	पटिनिवत्त res;
}

/**
 * output formatted certificate subject alternate names
 */
स्थिर nserror
format_certificate_san(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_san *san)
{
	nserror res;

	सचे (san == रिक्त) {
		पटिनिवत्त NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	याव (san != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	पटिनिवत्त res;

}


स्थिर nserror
format_certificate_public_कुञ्चिका(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_pkey *public_कुञ्चिका)
{
	nserror res;

	सचे (public_कुञ्चिका->algor == रिक्त) {
		/* skip the table if no algorithm name */
		पटिनिवत्त NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_कुञ्चिका->algor,
			 public_कुञ्चिका->size);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}


	सचे (public_कुञ्चिका->exponent != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_कुञ्चिका->exponent);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (public_कुञ्चिका->modulus != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_कुञ्चिका->modulus);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (public_कुञ्चिका->curve != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_कुञ्चिका->curve);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (public_कुञ्चिका->सार्वजनिक != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_कुञ्चिका->सार्वजनिक);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	पटिनिवत्त res;
}

स्थिर nserror
format_certificate_fingerprint(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_info *cert_info)
{
	nserror res;

	सचे ((cert_info->sha1fingerprint == रिक्त) &&
	    (cert_info->sha256fingerprint == रिक्त))  {
		/* skip the table if no fingerprints */
		पटिनिवत्त NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	सचे (cert_info->sha256fingerprint != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (cert_info->sha1fingerprint != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	पटिनिवत्त res;
}

स्थिर nserror
format_certificate(संरचना fetch_about_context *ctx,
		   संरचना ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	सचे (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = format_certificate_public_कुञ्चिका(ctx, &cert_info->public_कुञ्चिका);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	सचे (cert_info->serialnum != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	सचे (cert_info->sig_algor != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	पटिनिवत्त res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(संरचना fetch_about_context *ctx)
{
	पूर्णाङ्क code = 200;
	nserror res;
	संरचना cert_chain *chain = रिक्त;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	सचे (fetch_about_send_header(ctx, "Content-Type: text/html"))
		गच्छ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	सचे (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		सचे (res != NSERROR_OK) {
			गच्छ fetch_about_certificate_handler_aborted;
		}
	} अञ्ञथा {
		संरचना ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		सचे (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			सचे (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				गच्छ fetch_about_certificate_handler_aborted;
			}

			अत्थाय (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				सचे (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					गच्छ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			सचे (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				गच्छ fetch_about_certificate_handler_aborted;
			}

			अत्थाय (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				सचे (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					गच्छ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} अञ्ञथा {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			सचे (res != NSERROR_OK) {
				गच्छ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	सचे (res != NSERROR_OK) {
		गच्छ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	पटिनिवत्त true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	पटिनिवत्त false;
}
