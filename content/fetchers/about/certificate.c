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
	अक्षर *common_name;
	अक्षर *organisation;
	अक्षर *organisation_unit;
	अक्षर *locality;
	अक्षर *province;
	अक्षर *country;
};

/**
 * Certificate public key parameters
 */
संरचना ns_cert_pkey {
	अक्षर *algor;
	पूर्णांक size;
	अक्षर *modulus;
	अक्षर *exponent;
	अक्षर *curve;
	अक्षर *सार्वजनिक;
};

/**
 * Certificate subject alternative name
 */
संरचना ns_cert_san {
	संरचना ns_cert_san *next;
	अक्षर *name;
};

/**
 * certificate information for certificate chain
 */
संरचना ns_cert_info {
	संरचना ns_cert_name subject_name; /**< Subject details */
	संरचना ns_cert_name issuer_name; /**< Issuer details */
	संरचना ns_cert_pkey public_किल्ली; /**< public key details */
	दीर्घ version;		/**< Certificate version */
	अक्षर *not_before;	/**< Valid from date */
	अक्षर *not_after;	/**< Valid to date */
	पूर्णांक sig_type;		/**< Signature type */
	अक्षर *sig_algor;        /**< Signature Algorithm */
	अक्षर *serialnum;	/**< Serial number */
	अक्षर *sha1fingerprint; /**< fingerprint shar1 encoded */
	अक्षर *sha256fingerprint; /**< fingerprint shar256 encoded */
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
	मुक्त(cinfo->public_किल्ली.algor);
	मुक्त(cinfo->public_किल्ली.modulus);
	मुक्त(cinfo->public_किल्ली.exponent);
	मुक्त(cinfo->public_किल्ली.curve);
	मुक्त(cinfo->public_किल्ली.सार्वजनिक);
	मुक्त(cinfo->not_before);
	मुक्त(cinfo->not_after);
	मुक्त(cinfo->sig_algor);
	मुक्त(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	पर्यंत (san != रिक्त) {
		संरचना ns_cert_san *next;
		next = san->next;
		मुक्त(san);
		san = next;
	}

	मुक्त(cinfo);

	परत NSERROR_OK;
}

#ifdef WITH_OPENSSL

#अन्तर्भाव <openssl/ssl.h>
#अन्तर्भाव <openssl/x509v3.h>

#जर (OPENSSL_VERSION_संख्या < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# जर defined(LIBRESSL_VERSION_संख्या)
  /* LibreSSL */
#  जर (LIBRESSL_VERSION_संख्या < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   जर (LIBRESSL_VERSION_संख्या < 0x2070000fL)
    /* LibreSSL <2.7.0 */
स्थिर पूर्णांक ns_X509_get_signature_nid(X509 *cert)
{
	परत OBJ_obj2nid(cert->cert_info->किल्ली->algor->algorithm);
}

स्थिर स्थिरांक अचिन्हित अक्षर *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	परत (स्थिरांक अचिन्हित अक्षर *)ASN1_STRING_data(asn1str);
}
#   अन्यथा
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

स्थिर स्थिरांक BIGNUM *ns_RSA_get0_n(स्थिरांक RSA *d)
{
	परत d->n;
}

स्थिर स्थिरांक BIGNUM *ns_RSA_get0_e(स्थिरांक RSA *d)
{
	परत d->e;
}
#  अन्यथा
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# अन्यथा
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

स्थिर पूर्णांक ns_EVP_PKEY_get_bn_param(स्थिरांक EVP_PKEY *pkey,
		स्थिरांक अक्षर *किल्ली_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = रिक्त;

	/* Check parameters: only support allocation-form *bn */
	जर (pkey == रिक्त || किल्ली_name == रिक्त || bn == रिक्त || *bn != रिक्त)
		परत 0;

	/* Only support RSA keys */
	जर (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		परत 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	जर (rsa == रिक्त)
		परत 0;

	जर (म_भेद(किल्ली_name, "n") == 0) {
		स्थिरांक BIGNUM *n = ns_RSA_get0_n(rsa);
		जर (n != रिक्त)
			result = BN_dup(n);
	} अन्यथा जर (म_भेद(किल्ली_name, "e") == 0) {
		स्थिरांक BIGNUM *e = ns_RSA_get0_e(rsa);
		जर (e != रिक्त)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	परत (result != रिक्त) ? 1 : 0;
}

स्थिर पूर्णांक ns_EVP_PKEY_get_utf8_string_param(स्थिरांक EVP_PKEY *pkey,
		स्थिरांक अक्षर *किल्ली_name, अक्षर *str, माप_प्रकार max_len,
		माप_प्रकार *out_len)
{
	स्थिरांक EC_GROUP *ecgroup;
	स्थिरांक अक्षर *group = "";
	माप_प्रकार group_len = 0;
	EC_किल्ली *ec;
	पूर्णांक ret = 0;

	जर (pkey == रिक्त || किल्ली_name == रिक्त)
		परत 0;

	/* Only support EC keys */
	जर (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		परत 0;

	/* Only support fetching the group */
	जर (म_भेद(किल्ली_name, "group") != 0)
		परत 0;

	ec = EVP_PKEY_get1_EC_किल्ली((EVP_PKEY *) pkey);

	ecgroup = EC_किल्ली_get0_group(ec);
	जर (ecgroup != रिक्त) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = म_माप(group);
	}

	जर (str != रिक्त && max_len > group_len) {
		स_नकल(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	जर (out_len != रिक्त) {
		*out_len = group_len;
	}

	EC_किल्ली_free(ec);

	परत ret;
}

स्थिर पूर्णांक ns_EVP_PKEY_get_octet_string_param(स्थिरांक EVP_PKEY *pkey,
		स्थिरांक अक्षर *किल्ली_name, अचिन्हित अक्षर *buf, माप_प्रकार max_len,
		माप_प्रकार *out_len)
{
	स्थिरांक EC_GROUP *ecgroup;
	स्थिरांक EC_POINT *ecpoint;
	माप_प्रकार len;
	BN_CTX *bnctx;
	EC_किल्ली *ec;
	पूर्णांक ret = 0;

	जर (pkey == रिक्त || किल्ली_name == रिक्त)
		परत 0;

	/* Only support EC keys */
	जर (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		परत 0;

	जर (म_भेद(किल्ली_name, "encoded-pub-key") != 0)
		परत 0;

	ec = EVP_PKEY_get1_EC_किल्ली((EVP_PKEY *) pkey);
	जर (ec == रिक्त)
		परत 0;

	ecgroup = EC_किल्ली_get0_group(ec);
	जर (ecgroup != रिक्त) {
		ecpoint = EC_किल्ली_get0_public_किल्ली(ec);
		जर (ecpoint != रिक्त) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 रिक्त,
						 0,
						 bnctx);
			जर (len != 0 && len <= max_len) {
				जर (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			जर (out_len != रिक्त)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_किल्ली_free(ec);

	परत ret;
}
#अन्यथा
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
	पूर्णांक entryidx;
	पूर्णांक entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *मूल्य;
	स्थिरांक अचिन्हित अक्षर *मूल्य_str;
	ASN1_OBJECT *name;
	पूर्णांक name_nid;
	अक्षर **field;

	entrycnt = X509_NAME_entry_मोजणी(xname);

	साठी (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		मूल्य = X509_NAME_ENTRY_get_data(entry);
		मूल्य_str = ns_ASN1_STRING_get0_data(मूल्य);
		निवड (name_nid) {
		स्थिती NID_commonName:
			field = &iname->common_name;
			थांबा;
		स्थिती NID_countryName:
			field = &iname->country;
			थांबा;
		स्थिती NID_localityName:
			field = &iname->locality;
			थांबा;
		स्थिती NID_stateOrProvinceName:
			field = &iname->province;
			थांबा;
		स्थिती NID_organizationName:
			field = &iname->organisation;
			थांबा;
		स्थिती NID_organizationalUnitName:
			field = &iname->organisation_unit;
			थांबा;
		पूर्वनिर्धारित :
			field = रिक्त;
			थांबा;
		}
		जर (field != रिक्त) {
			*field = strdup((स्थिरांक अक्षर *)मूल्य_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} अन्यथा {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	जर (iname->common_name == रिक्त) {
		iname->common_name = strdup("Unknown");
	}

	परत NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
स्थिर अक्षर *hexdup(स्थिरांक अक्षर *hex)
{
	पूर्णांक hexlen;
	अक्षर *dst;
	अक्षर *out;
	पूर्णांक cn = 0;

	hexlen = म_माप(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = दो_स्मृति(((hexlen * 7) + 6) / 2);

	जर (dst != रिक्त) {
		साठी (out = dst; *hex != 0; hex++) {
			जर (cn == 2) {
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
	परत dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
स्थिर अक्षर *bindup(अचिन्हित अक्षर *bin, अचिन्हित पूर्णांक binlen)
{
	अक्षर *dst;
	अक्षर *out;
	अचिन्हित पूर्णांक idx;
	स्थिरांक अक्षर hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = दो_स्मृति(binlen * 7);

	जर (dst != रिक्त) {
		out = dst;
		साठी (idx = 0; idx < binlen; idx++) {
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
	परत dst;
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
	अक्षर *tmp;

	जर (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		परत NSERROR_BAD_PARAMETER;
	}

	जर (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		परत NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	जर (tmp != रिक्त) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	जर (tmp != रिक्त) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	परत NSERROR_OK;
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

	परत NSERROR_OK;
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

	परत NSERROR_OK;
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
	माप_प्रकार len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", रिक्त, 0, &len);
	जर (len != 0) {
		ikey->curve = दो_स्मृति(len + 1);
		जर (ikey->curve != रिक्त) {
			जर (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, रिक्त) == 0) {
				मुक्त(ikey->curve);
				ikey->curve = रिक्त;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			रिक्त, 0, &len);
	जर (len != 0) {
		अचिन्हित अक्षर *point = दो_स्मृति(len);
		जर (point != रिक्त) {
			जर (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					रिक्त) == 1) {
				ikey->सार्वजनिक = bindup(point, len);
			}
			मुक्त(point);
		}
	}

	परत NSERROR_OK;
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

	जर (pkey == रिक्त) {
		परत NSERROR_BAD_PARAMETER;
	}

	निवड (EVP_PKEY_base_id(pkey)) {
	स्थिती EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		थांबा;

	स्थिती EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		थांबा;

	स्थिती EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		थांबा;

	स्थिती EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		थांबा;

	पूर्वनिर्धारित:
		res = NSERROR_NOT_IMPLEMENTED;
		थांबा;
	}

	EVP_PKEY_free(pkey);

	परत res;
}

स्थिर nserror san_to_info(X509 *cert, संरचना ns_cert_san **prev_next)
{
	पूर्णांक idx;
	पूर्णांक san_names_nb = -1;
	स्थिरांक GENERAL_NAME *current_name;
	स्थिरांक अचिन्हित अक्षर *dns_name;
	संरचना ns_cert_san *isan;

	चळत_OF(GENERAL_NAME) *san_names = रिक्त;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, रिक्त, रिक्त);
	जर (san_names == रिक्त) {
		परत NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_अंक(san_names);

	/* Check each name within the extension */
	साठी (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_मूल्य(san_names, idx);

		जर (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = दो_स्मृति(आकार(संरचना ns_cert_san));
			जर (isan != रिक्त) {
				isan->name = strdup((स्थिरांक अक्षर *)dns_name);
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
	साठी (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_काढा(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	परत NSERROR_OK;
}

स्थिर nserror
der_to_certinfo(स्थिरांक uint8_t *der,
		माप_प्रकार der_length,
		संरचना ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	स्थिरांक ASN1_INTEGER *asn1_अंक;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	जर (der == रिक्त) {
		परत NSERROR_OK;
	}

	cert = d2i_X509(रिक्त, &der, der_length);
	जर (cert == रिक्त) {
		परत NSERROR_INVALID;
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
	info->not_before = सुस्मृति(1, buf->length + 1);
	जर (info->not_before != रिक्त) {
		स_नकल(info->not_before, buf->data, (अचिन्हित)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(शून्य) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = सुस्मृति(1, buf->length + 1);
	जर (info->not_after != रिक्त) {
		स_नकल(info->not_after, buf->data, (अचिन्हित)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	पूर्णांक pkey_nid = ns_X509_get_signature_nid(cert);
	जर (pkey_nid != NID_undef) {
		स्थिरांक अक्षर* sslbuf = OBJ_nid2ln(pkey_nid);
		जर (sslbuf != रिक्त) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_अंक = X509_get_serial_संख्या(cert);
	जर (asn1_अंक != रिक्त) {
		bignum = ASN1_INTEGER_to_BN(asn1_अंक, रिक्त);
		जर (bignum != रिक्त) {
			अक्षर *tmp = BN_bn2hex(bignum);
			जर (tmp != रिक्त) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = रिक्त;
		}
	}

	/* fingerprints */
	स्थिरांक EVP_MD *digest;
	अचिन्हित पूर्णांक dig_len;
	अचिन्हित अक्षर *buff;
	पूर्णांक rc;

	digest = EVP_sha1();
	buff = दो_स्मृति(EVP_MD_size(digest));
	जर (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		जर ((rc == 1) && (dig_len == (अचिन्हित पूर्णांक)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		मुक्त(buff);
	}

	digest = EVP_sha256();
	buff = दो_स्मृति(EVP_MD_size(digest));
	जर (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		जर ((rc == 1) && (dig_len == (अचिन्हित पूर्णांक)EVP_MD_size(digest))) {
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
	pkey_to_info(X509_get_pubkey(cert), &info->public_किल्ली);

	X509_free(cert);

	परत NSERROR_OK;
}

/* copy certificate data */
स्थिर nserror
convert_chain_to_cert_info(स्थिरांक संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_out)
{
	संरचना ns_cert_info *certs;
	माप_प्रकार depth;
	nserror res;

	certs = सुस्मृति(chain->depth, आकार(संरचना ns_cert_info));
	जर (certs == रिक्त) {
		परत NSERROR_NOMEM;
	}

	साठी (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		जर (res != NSERROR_OK) {
			मुक्त(certs);
			परत res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	परत NSERROR_OK;
}

#अन्यथा
स्थिर nserror
convert_chain_to_cert_info(स्थिरांक संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_out)
{
	परत NSERROR_NOT_IMPLEMENTED;
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
	जर (res != NSERROR_OK) {
		परत res;
	}

	जर (cert_name->organisation != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (cert_name->organisation_unit != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (cert_name->locality != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (cert_name->province != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (cert_name->country != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	परत res;
}

/**
 * output formatted certificate subject alternate names
 */
स्थिर nserror
format_certificate_san(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_san *san)
{
	nserror res;

	जर (san == रिक्त) {
		परत NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	जर (res != NSERROR_OK) {
		परत res;
	}

	पर्यंत (san != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		जर (res != NSERROR_OK) {
			परत res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	परत res;

}


स्थिर nserror
format_certificate_public_किल्ली(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_pkey *public_किल्ली)
{
	nserror res;

	जर (public_किल्ली->algor == रिक्त) {
		/* skip the table if no algorithm name */
		परत NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_किल्ली->algor,
			 public_किल्ली->size);
	जर (res != NSERROR_OK) {
		परत res;
	}


	जर (public_किल्ली->exponent != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_किल्ली->exponent);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (public_किल्ली->modulus != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_किल्ली->modulus);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (public_किल्ली->curve != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_किल्ली->curve);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (public_किल्ली->सार्वजनिक != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_किल्ली->सार्वजनिक);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	परत res;
}

स्थिर nserror
format_certificate_fingerprint(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_info *cert_info)
{
	nserror res;

	जर ((cert_info->sha1fingerprint == रिक्त) &&
	    (cert_info->sha256fingerprint == रिक्त))  {
		/* skip the table if no fingerprints */
		परत NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	जर (res != NSERROR_OK) {
		परत res;
	}

	जर (cert_info->sha256fingerprint != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (cert_info->sha1fingerprint != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	परत res;
}

स्थिर nserror
format_certificate(संरचना fetch_about_context *ctx,
		   संरचना ns_cert_info *cert_info,
		   माप_प्रकार depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	जर (res != NSERROR_OK) {
		परत res;
	}

	जर (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = format_certificate_public_किल्ली(ctx, &cert_info->public_किल्ली);
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	जर (res != NSERROR_OK) {
		परत res;
	}

	जर (cert_info->serialnum != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	जर (cert_info->sig_algor != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	जर (res != NSERROR_OK) {
		परत res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	जर (res != NSERROR_OK) {
		परत res;
	}

	परत res;
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
	पूर्णांक code = 200;
	nserror res;
	संरचना cert_chain *chain = रिक्त;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	जर (fetch_about_send_header(ctx, "Content-Type: text/html"))
		जा fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	जर (res != NSERROR_OK) {
		जा fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	जर (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		जर (res != NSERROR_OK) {
			जा fetch_about_certificate_handler_aborted;
		}
	} अन्यथा {
		संरचना ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		जर (res == NSERROR_OK) {
			माप_प्रकार depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			जर (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				जा fetch_about_certificate_handler_aborted;
			}

			साठी (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				जर (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					जा fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			जर (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				जा fetch_about_certificate_handler_aborted;
			}

			साठी (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				जर (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					जा fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} अन्यथा {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			जर (res != NSERROR_OK) {
				जा fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	जर (res != NSERROR_OK) {
		जा fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	परत true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	परत false;
}
