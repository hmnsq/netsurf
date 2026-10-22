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
	संरचना ns_cert_pkey public_कुंजी; /**< public key details */
	दीर्घ version;		/**< Certificate version */
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
	मुक्त(cinfo->public_कुंजी.algor);
	मुक्त(cinfo->public_कुंजी.modulus);
	मुक्त(cinfo->public_कुंजी.exponent);
	मुक्त(cinfo->public_कुंजी.curve);
	मुक्त(cinfo->public_कुंजी.सार्वजनिक);
	मुक्त(cinfo->not_before);
	मुक्त(cinfo->not_after);
	मुक्त(cinfo->sig_algor);
	मुक्त(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	यावत् (san != रिक्त) {
		संरचना ns_cert_san *next;
		next = san->next;
		मुक्त(san);
		san = next;
	}

	मुक्त(cinfo);

	प्रत्यागम NSERROR_OK;
}

#ifdef WITH_OPENSSL

#अन्तर्भाव <openssl/ssl.h>
#अन्तर्भाव <openssl/x509v3.h>

#यदि (OPENSSL_VERSION_संख्या < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# यदि defined(LIBRESSL_VERSION_संख्या)
  /* LibreSSL */
#  यदि (LIBRESSL_VERSION_संख्या < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   यदि (LIBRESSL_VERSION_संख्या < 0x2070000fL)
    /* LibreSSL <2.7.0 */
स्थिर पूर्णाङ्क ns_X509_get_signature_nid(X509 *cert)
{
	प्रत्यागम OBJ_obj2nid(cert->cert_info->कुंजी->algor->algorithm);
}

स्थिर ध्रुव अचिन्हित वर्ण *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	प्रत्यागम (ध्रुव अचिन्हित वर्ण *)ASN1_STRING_data(asn1str);
}
#   अन्यथा
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

स्थिर ध्रुव BIGNUM *ns_RSA_get0_n(ध्रुव RSA *d)
{
	प्रत्यागम d->n;
}

स्थिर ध्रुव BIGNUM *ns_RSA_get0_e(ध्रुव RSA *d)
{
	प्रत्यागम d->e;
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

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_bn_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुंजी_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = रिक्त;

	/* Check parameters: only support allocation-form *bn */
	यदि (pkey == रिक्त || कुंजी_name == रिक्त || bn == रिक्त || *bn != रिक्त)
		प्रत्यागम 0;

	/* Only support RSA keys */
	यदि (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		प्रत्यागम 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	यदि (rsa == रिक्त)
		प्रत्यागम 0;

	यदि (म_भेद(कुंजी_name, "n") == 0) {
		ध्रुव BIGNUM *n = ns_RSA_get0_n(rsa);
		यदि (n != रिक्त)
			result = BN_dup(n);
	} अन्यथा यदि (म_भेद(कुंजी_name, "e") == 0) {
		ध्रुव BIGNUM *e = ns_RSA_get0_e(rsa);
		यदि (e != रिक्त)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	प्रत्यागम (result != रिक्त) ? 1 : 0;
}

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_utf8_string_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुंजी_name, वर्ण *str, माप_प्रकार max_len,
		माप_प्रकार *out_len)
{
	ध्रुव EC_GROUP *ecgroup;
	ध्रुव वर्ण *group = "";
	माप_प्रकार group_len = 0;
	EC_कुंजी *ec;
	पूर्णाङ्क ret = 0;

	यदि (pkey == रिक्त || कुंजी_name == रिक्त)
		प्रत्यागम 0;

	/* Only support EC keys */
	यदि (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		प्रत्यागम 0;

	/* Only support fetching the group */
	यदि (म_भेद(कुंजी_name, "group") != 0)
		प्रत्यागम 0;

	ec = EVP_PKEY_get1_EC_कुंजी((EVP_PKEY *) pkey);

	ecgroup = EC_कुंजी_get0_group(ec);
	यदि (ecgroup != रिक्त) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = म_माप(group);
	}

	यदि (str != रिक्त && max_len > group_len) {
		स_नकल(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	यदि (out_len != रिक्त) {
		*out_len = group_len;
	}

	EC_कुंजी_free(ec);

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_octet_string_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुंजी_name, अचिन्हित वर्ण *buf, माप_प्रकार max_len,
		माप_प्रकार *out_len)
{
	ध्रुव EC_GROUP *ecgroup;
	ध्रुव EC_POINT *ecpoint;
	माप_प्रकार len;
	BN_CTX *bnctx;
	EC_कुंजी *ec;
	पूर्णाङ्क ret = 0;

	यदि (pkey == रिक्त || कुंजी_name == रिक्त)
		प्रत्यागम 0;

	/* Only support EC keys */
	यदि (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		प्रत्यागम 0;

	यदि (म_भेद(कुंजी_name, "encoded-pub-key") != 0)
		प्रत्यागम 0;

	ec = EVP_PKEY_get1_EC_कुंजी((EVP_PKEY *) pkey);
	यदि (ec == रिक्त)
		प्रत्यागम 0;

	ecgroup = EC_कुंजी_get0_group(ec);
	यदि (ecgroup != रिक्त) {
		ecpoint = EC_कुंजी_get0_public_कुंजी(ec);
		यदि (ecpoint != रिक्त) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 रिक्त,
						 0,
						 bnctx);
			यदि (len != 0 && len <= max_len) {
				यदि (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			यदि (out_len != रिक्त)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_कुंजी_free(ec);

	प्रत्यागम ret;
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
	पूर्णाङ्क entryidx;
	पूर्णाङ्क entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *मूल्य;
	ध्रुव अचिन्हित वर्ण *मूल्य_str;
	ASN1_OBJECT *name;
	पूर्णाङ्क name_nid;
	वर्ण **field;

	entrycnt = X509_NAME_entry_गिनती(xname);

	प्रति (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		मूल्य = X509_NAME_ENTRY_get_data(entry);
		मूल्य_str = ns_ASN1_STRING_get0_data(मूल्य);
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
		यदि (field != रिक्त) {
			*field = strdup((ध्रुव वर्ण *)मूल्य_str);
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
	यदि (iname->common_name == रिक्त) {
		iname->common_name = strdup("Unknown");
	}

	प्रत्यागम NSERROR_OK;
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

	hexlen = म_माप(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = दो_स्मृति(((hexlen * 7) + 6) / 2);

	यदि (dst != रिक्त) {
		प्रति (out = dst; *hex != 0; hex++) {
			यदि (cn == 2) {
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
	प्रत्यागम dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
स्थिर वर्ण *bindup(अचिन्हित वर्ण *bin, अचिन्हित पूर्णाङ्क binlen)
{
	वर्ण *dst;
	वर्ण *out;
	अचिन्हित पूर्णाङ्क idx;
	ध्रुव वर्ण hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = दो_स्मृति(binlen * 7);

	यदि (dst != रिक्त) {
		out = dst;
		प्रति (idx = 0; idx < binlen; idx++) {
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
	प्रत्यागम dst;
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

	यदि (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	यदि (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	यदि (tmp != रिक्त) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	यदि (tmp != रिक्त) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	प्रत्यागम NSERROR_OK;
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

	प्रत्यागम NSERROR_OK;
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

	प्रत्यागम NSERROR_OK;
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
	यदि (len != 0) {
		ikey->curve = दो_स्मृति(len + 1);
		यदि (ikey->curve != रिक्त) {
			यदि (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, रिक्त) == 0) {
				मुक्त(ikey->curve);
				ikey->curve = रिक्त;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			रिक्त, 0, &len);
	यदि (len != 0) {
		अचिन्हित वर्ण *point = दो_स्मृति(len);
		यदि (point != रिक्त) {
			यदि (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					रिक्त) == 1) {
				ikey->सार्वजनिक = bindup(point, len);
			}
			मुक्त(point);
		}
	}

	प्रत्यागम NSERROR_OK;
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

	यदि (pkey == रिक्त) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
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

	प्रत्यागम res;
}

स्थिर nserror san_to_info(X509 *cert, संरचना ns_cert_san **prev_next)
{
	पूर्णाङ्क idx;
	पूर्णाङ्क san_names_nb = -1;
	ध्रुव GENERAL_NAME *current_name;
	ध्रुव अचिन्हित वर्ण *dns_name;
	संरचना ns_cert_san *isan;

	ढेर_OF(GENERAL_NAME) *san_names = रिक्त;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, रिक्त, रिक्त);
	यदि (san_names == रिक्त) {
		प्रत्यागम NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_अंक(san_names);

	/* Check each name within the extension */
	प्रति (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_मूल्य(san_names, idx);

		यदि (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = दो_स्मृति(परिमाण(संरचना ns_cert_san));
			यदि (isan != रिक्त) {
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
	प्रति (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_निकालो(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror
der_to_certinfo(ध्रुव uint8_t *der,
		माप_प्रकार der_length,
		संरचना ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ध्रुव ASN1_INTEGER *asn1_अंक;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	यदि (der == रिक्त) {
		प्रत्यागम NSERROR_OK;
	}

	cert = d2i_X509(रिक्त, &der, der_length);
	यदि (cert == रिक्त) {
		प्रत्यागम NSERROR_INVALID;
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
	यदि (info->not_before != रिक्त) {
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
	यदि (info->not_after != रिक्त) {
		स_नकल(info->not_after, buf->data, (अचिन्हित)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	पूर्णाङ्क pkey_nid = ns_X509_get_signature_nid(cert);
	यदि (pkey_nid != NID_undef) {
		ध्रुव वर्ण* sslbuf = OBJ_nid2ln(pkey_nid);
		यदि (sslbuf != रिक्त) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_अंक = X509_get_serial_संख्या(cert);
	यदि (asn1_अंक != रिक्त) {
		bignum = ASN1_INTEGER_to_BN(asn1_अंक, रिक्त);
		यदि (bignum != रिक्त) {
			वर्ण *tmp = BN_bn2hex(bignum);
			यदि (tmp != रिक्त) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = रिक्त;
		}
	}

	/* fingerprints */
	ध्रुव EVP_MD *digest;
	अचिन्हित पूर्णाङ्क dig_len;
	अचिन्हित वर्ण *buff;
	पूर्णाङ्क rc;

	digest = EVP_sha1();
	buff = दो_स्मृति(EVP_MD_size(digest));
	यदि (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		यदि ((rc == 1) && (dig_len == (अचिन्हित पूर्णाङ्क)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		मुक्त(buff);
	}

	digest = EVP_sha256();
	buff = दो_स्मृति(EVP_MD_size(digest));
	यदि (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		यदि ((rc == 1) && (dig_len == (अचिन्हित पूर्णाङ्क)EVP_MD_size(digest))) {
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
	pkey_to_info(X509_get_pubkey(cert), &info->public_कुंजी);

	X509_free(cert);

	प्रत्यागम NSERROR_OK;
}

/* copy certificate data */
स्थिर nserror
convert_chain_to_cert_info(ध्रुव संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_out)
{
	संरचना ns_cert_info *certs;
	माप_प्रकार depth;
	nserror res;

	certs = सुस्मृति(chain->depth, परिमाण(संरचना ns_cert_info));
	यदि (certs == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रति (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		यदि (res != NSERROR_OK) {
			मुक्त(certs);
			प्रत्यागम res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	प्रत्यागम NSERROR_OK;
}

#अन्यथा
स्थिर nserror
convert_chain_to_cert_info(ध्रुव संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_out)
{
	प्रत्यागम NSERROR_NOT_IMPLEMENTED;
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
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	यदि (cert_name->organisation != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (cert_name->organisation_unit != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (cert_name->locality != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (cert_name->province != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (cert_name->country != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	प्रत्यागम res;
}

/**
 * output formatted certificate subject alternate names
 */
स्थिर nserror
format_certificate_san(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_san *san)
{
	nserror res;

	यदि (san == रिक्त) {
		प्रत्यागम NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	यावत् (san != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	प्रत्यागम res;

}


स्थिर nserror
format_certificate_public_कुंजी(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_pkey *public_कुंजी)
{
	nserror res;

	यदि (public_कुंजी->algor == रिक्त) {
		/* skip the table if no algorithm name */
		प्रत्यागम NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_कुंजी->algor,
			 public_कुंजी->size);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}


	यदि (public_कुंजी->exponent != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_कुंजी->exponent);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (public_कुंजी->modulus != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_कुंजी->modulus);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (public_कुंजी->curve != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_कुंजी->curve);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (public_कुंजी->सार्वजनिक != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_कुंजी->सार्वजनिक);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	प्रत्यागम res;
}

स्थिर nserror
format_certificate_fingerprint(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_info *cert_info)
{
	nserror res;

	यदि ((cert_info->sha1fingerprint == रिक्त) &&
	    (cert_info->sha256fingerprint == रिक्त))  {
		/* skip the table if no fingerprints */
		प्रत्यागम NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	यदि (cert_info->sha256fingerprint != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (cert_info->sha1fingerprint != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	प्रत्यागम res;
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
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	यदि (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = format_certificate_public_कुंजी(ctx, &cert_info->public_कुंजी);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	यदि (cert_info->serialnum != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (cert_info->sig_algor != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	प्रत्यागम res;
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
	यदि (fetch_about_send_header(ctx, "Content-Type: text/html"))
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
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	यदि (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		यदि (res != NSERROR_OK) {
			गच्छ fetch_about_certificate_handler_aborted;
		}
	} अन्यथा {
		संरचना ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		यदि (res == NSERROR_OK) {
			माप_प्रकार depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			यदि (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				गच्छ fetch_about_certificate_handler_aborted;
			}

			प्रति (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				यदि (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					गच्छ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			यदि (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				गच्छ fetch_about_certificate_handler_aborted;
			}

			प्रति (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				यदि (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					गच्छ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} अन्यथा {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			यदि (res != NSERROR_OK) {
				गच्छ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	प्रत्यागम true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	प्रत्यागम false;
}
