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
	पूर्णाङ्क आकार;
	वर्ण *modulus;
	वर्ण *exponent;
	वर्ण *वक्रम्;
	वर्ण *सार्वजनिक;
};

/**
 * Certificate subject alternative name
 */
संरचना ns_cert_san {
	संरचना ns_cert_san *अग्र;
	वर्ण *name;
};

/**
 * certificate information for certificate chain
 */
संरचना ns_cert_info {
	संरचना ns_cert_name subject_name; /**< Subject details */
	संरचना ns_cert_name issuer_name; /**< Issuer details */
	संरचना ns_cert_pkey लौकिकम्_कुञ्चिका; /**< public key details */
	दीर्घ संस्करणम्;		/**< Certificate version */
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
	मुक्त(cinfo->लौकिकम्_कुञ्चिका.algor);
	मुक्त(cinfo->लौकिकम्_कुञ्चिका.modulus);
	मुक्त(cinfo->लौकिकम्_कुञ्चिका.exponent);
	मुक्त(cinfo->लौकिकम्_कुञ्चिका.वक्रम्);
	मुक्त(cinfo->लौकिकम्_कुञ्चिका.सार्वजनिक);
	मुक्त(cinfo->not_before);
	मुक्त(cinfo->not_after);
	मुक्त(cinfo->sig_algor);
	मुक्त(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	यावत् (san != रिक्त) {
		संरचना ns_cert_san *अग्र;
		अग्र = san->अग्र;
		मुक्त(san);
		san = अग्र;
	}

	मुक्त(cinfo);

	प्रत्यागम NSERROR_OK;
}

#यदि_परिभाषितम् WITH_OPENSSL

#अन्तर्भाव <openssl/ssl.h>
#अन्तर्भाव <openssl/x509v3.h>

#यदि (OPENSSL_संस्करणम्_सङ्ख्या < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# यदि defined(LIBRESSL_संस्करणम्_सङ्ख्या)
  /* LibreSSL */
#  यदि (LIBRESSL_संस्करणम्_सङ्ख्या < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   यदि (LIBRESSL_संस्करणम्_सङ्ख्या < 0x2070000fL)
    /* LibreSSL <2.7.0 */
स्थिर पूर्णाङ्क ns_X509_get_signature_nid(X509 *cert)
{
	प्रत्यागम OBJ_obj2nid(cert->cert_info->कुञ्चिका->algor->algorithm);
}

स्थिर ध्रुव अचिह्नित वर्ण *ns_ASN1_STRING_get0_दत्त(ASN1_STRING *asn1str)
{
	प्रत्यागम (ध्रुव अचिह्नित वर्ण *)ASN1_STRING_दत्त(asn1str);
}
#   अन्यथा
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_दत्त ASN1_STRING_get0_दत्त
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
#   define ns_ASN1_STRING_get0_दत्त ASN1_STRING_get0_दत्त
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# अन्यथा
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_दत्त ASN1_STRING_get0_दत्त
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_bn_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुञ्चिका_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *परिणाम = रिक्त;

	/* Check parameters: only support allocation-form *bn */
	यदि (pkey == रिक्त || कुञ्चिका_name == रिक्त || bn == रिक्त || *bn != रिक्त)
		प्रत्यागम 0;

	/* Only support RSA keys */
	यदि (EVP_PKEY_आधारः_id(pkey) != EVP_PKEY_RSA)
		प्रत्यागम 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	यदि (rsa == रिक्त)
		प्रत्यागम 0;

	यदि (वाक्यं_तुलय(कुञ्चिका_name, "n") == 0) {
		ध्रुव BIGNUM *n = ns_RSA_get0_n(rsa);
		यदि (n != रिक्त)
			परिणाम = BN_dup(n);
	} अन्यथा यदि (वाक्यं_तुलय(कुञ्चिका_name, "e") == 0) {
		ध्रुव BIGNUM *e = ns_RSA_get0_e(rsa);
		यदि (e != रिक्त)
			परिणाम = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = परिणाम;

	प्रत्यागम (परिणाम != रिक्त) ? 1 : 0;
}

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_utf8_string_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुञ्चिका_name, वर्ण *str, size_t max_len,
		size_t *बहिर्गमनम्_len)
{
	ध्रुव EC_गणः *ecgroup;
	ध्रुव वर्ण *गणः = "";
	size_t गणः_len = 0;
	EC_कुञ्चिका *ec;
	पूर्णाङ्क ret = 0;

	यदि (pkey == रिक्त || कुञ्चिका_name == रिक्त)
		प्रत्यागम 0;

	/* Only support EC keys */
	यदि (EVP_PKEY_आधारः_id(pkey) != EVP_PKEY_EC)
		प्रत्यागम 0;

	/* Only support fetching the group */
	यदि (वाक्यं_तुलय(कुञ्चिका_name, "group") != 0)
		प्रत्यागम 0;

	ec = EVP_PKEY_get1_EC_कुञ्चिका((EVP_PKEY *) pkey);

	ecgroup = EC_कुञ्चिका_get0_गणः(ec);
	यदि (ecgroup != रिक्त) {
		गणः = OBJ_nid2ln(EC_गणः_get_वक्रम्_name(ecgroup));
		गणः_len = वाक्यदैर्घ्यम्(गणः);
	}

	यदि (str != रिक्त && max_len > गणः_len) {
		स्मृतिं_प्रतिलिख(str, गणः, गणः_len);
		str[गणः_len] = '\0';
		ret = 1;
	}

	यदि (बहिर्गमनम्_len != रिक्त) {
		*बहिर्गमनम्_len = गणः_len;
	}

	EC_कुञ्चिका_free(ec);

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क ns_EVP_PKEY_get_octet_string_param(ध्रुव EVP_PKEY *pkey,
		ध्रुव वर्ण *कुञ्चिका_name, अचिह्नित वर्ण *buf, size_t max_len,
		size_t *बहिर्गमनम्_len)
{
	ध्रुव EC_गणः *ecgroup;
	ध्रुव EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_कुञ्चिका *ec;
	पूर्णाङ्क ret = 0;

	यदि (pkey == रिक्त || कुञ्चिका_name == रिक्त)
		प्रत्यागम 0;

	/* Only support EC keys */
	यदि (EVP_PKEY_आधारः_id(pkey) != EVP_PKEY_EC)
		प्रत्यागम 0;

	यदि (वाक्यं_तुलय(कुञ्चिका_name, "encoded-pub-key") != 0)
		प्रत्यागम 0;

	ec = EVP_PKEY_get1_EC_कुञ्चिका((EVP_PKEY *) pkey);
	यदि (ec == रिक्त)
		प्रत्यागम 0;

	ecgroup = EC_कुञ्चिका_get0_गणः(ec);
	यदि (ecgroup != रिक्त) {
		ecpoint = EC_कुञ्चिका_get0_लौकिकम्_कुञ्चिका(ec);
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
			यदि (बहिर्गमनम्_len != रिक्त)
				*बहिर्गमनम्_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_कुञ्चिका_free(ec);

	प्रत्यागम ret;
}
#अन्यथा
/* OpenSSL 3.x and later */
#परिभाषय ns_X509_get_signature_nid X509_get_signature_nid
#परिभाषय ns_ASN1_STRING_get0_दत्त ASN1_STRING_get0_दत्त
#परिभाषय ns_RSA_get0_n RSA_get0_n
#परिभाषय ns_RSA_get0_e RSA_get0_e
#परिभाषय ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#परिभाषय ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#परिभाषय ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#यदि_अन्तः

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
स्थिर nserror
xname_गन्तव्यम्_info(X509_NAME *xname, संरचना ns_cert_name *iname)
{
	पूर्णाङ्क entryidx;
	पूर्णाङ्क entrycnt;
	X509_NAME_प्रविष्टिः *प्रविष्टिः; /* current name entry */
	ASN1_STRING *मानम्;
	ध्रुव अचिह्नित वर्ण *मानम्_str;
	ASN1_OBJECT *name;
	पूर्णाङ्क name_nid;
	वर्ण **क्षेत्रम्;

	entrycnt = X509_NAME_प्रविष्टिः_गणनम्(xname);

	प्रति (entryidx = 0; entryidx < entrycnt; entryidx++) {
		प्रविष्टिः = X509_NAME_get_प्रविष्टिः(xname, entryidx);
		name = X509_NAME_प्रविष्टिः_get_object(प्रविष्टिः);
		name_nid = OBJ_obj2nid(name);
		मानम् = X509_NAME_प्रविष्टिः_get_दत्त(प्रविष्टिः);
		मानम्_str = ns_ASN1_STRING_get0_दत्त(मानम्);
		विकल्प (name_nid) {
		पक्ष NID_commonName:
			क्षेत्रम् = &iname->common_name;
			भङ्ग;
		पक्ष NID_countryName:
			क्षेत्रम् = &iname->country;
			भङ्ग;
		पक्ष NID_localityName:
			क्षेत्रम् = &iname->locality;
			भङ्ग;
		पक्ष NID_stateOrProvinceName:
			क्षेत्रम् = &iname->province;
			भङ्ग;
		पक्ष NID_organizationName:
			क्षेत्रम् = &iname->organisation;
			भङ्ग;
		पक्ष NID_organizationalUnitName:
			क्षेत्रम् = &iname->organisation_unit;
			भङ्ग;
		स्वभाव :
			क्षेत्रम् = रिक्त;
			भङ्ग;
		}
		यदि (क्षेत्रम् != रिक्त) {
			*क्षेत्रम् = strdup((ध्रुव वर्ण *)मानम्_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *क्षेत्रम्);
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
	वर्ण *बहिर्गमनम्;
	पूर्णाङ्क cn = 0;

	hexlen = वाक्यदैर्घ्यम्(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = स्मृतिं_देहि(((hexlen * 7) + 6) / 2);

	यदि (dst != रिक्त) {
		प्रति (बहिर्गमनम् = dst; *hex != 0; hex++) {
			यदि (cn == 2) {
				cn = 0;
				*बहिर्गमनम्++ = '&';
				*बहिर्गमनम्++ = '#';
				*बहिर्गमनम्++ = '5';
				*बहिर्गमनम्++ = '8';
				*बहिर्गमनम्++ = ';';
			}
			*बहिर्गमनम्++ = *hex;
			cn++;
		}
		*बहिर्गमनम् = 0;
	}
	प्रत्यागम dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
स्थिर वर्ण *bindup(अचिह्नित वर्ण *bin, अचिह्नित पूर्णाङ्क binlen)
{
	वर्ण *dst;
	वर्ण *बहिर्गमनम्;
	अचिह्नित पूर्णाङ्क idx;
	ध्रुव वर्ण hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = स्मृतिं_देहि(binlen * 7);

	यदि (dst != रिक्त) {
		बहिर्गमनम् = dst;
		प्रति (idx = 0; idx < binlen; idx++) {
			*बहिर्गमनम्++ = hex[(bin[idx] & 0xf0) >> 4];
			*बहिर्गमनम्++ = hex[bin[idx] & 0xf];

			*बहिर्गमनम्++ = '&';
			*बहिर्गमनम्++ = '#';
			*बहिर्गमनम्++ = '5';
			*बहिर्गमनम्++ = '8';
			*बहिर्गमनम्++ = ';';
		}
		बहिर्गमनम् -= 5;
		*बहिर्गमनम् = 0;
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
rsa_गन्तव्यम्_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
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

	ikey->आकार = EVP_PKEY_द्वयङ्काः(pkey);

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
dsa_गन्तव्यम्_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->आकार = EVP_PKEY_द्वयङ्काः(pkey);

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
dh_गन्तव्यम्_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->आकार = EVP_PKEY_द्वयङ्काः(pkey);

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
ec_गन्तव्यम्_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->आकार = EVP_PKEY_द्वयङ्काः(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", रिक्त, 0, &len);
	यदि (len != 0) {
		ikey->वक्रम् = स्मृतिं_देहि(len + 1);
		यदि (ikey->वक्रम् != रिक्त) {
			यदि (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->वक्रम्, len + 1, रिक्त) == 0) {
				मुक्त(ikey->वक्रम्);
				ikey->वक्रम् = रिक्त;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			रिक्त, 0, &len);
	यदि (len != 0) {
		अचिह्नित वर्ण *point = स्मृतिं_देहि(len);
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
pkey_गन्तव्यम्_info(EVP_PKEY *pkey, संरचना ns_cert_pkey *ikey)
{
	nserror res;

	यदि (pkey == रिक्त) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	विकल्प (EVP_PKEY_आधारः_id(pkey)) {
	पक्ष EVP_PKEY_RSA:
		res = rsa_गन्तव्यम्_info(pkey, ikey);
		भङ्ग;

	पक्ष EVP_PKEY_DSA:
		res = dsa_गन्तव्यम्_info(pkey, ikey);
		भङ्ग;

	पक्ष EVP_PKEY_DH:
		res = dh_गन्तव्यम्_info(pkey, ikey);
		भङ्ग;

	पक्ष EVP_PKEY_EC:
		res = ec_गन्तव्यम्_info(pkey, ikey);
		भङ्ग;

	स्वभाव:
		res = NSERROR_NOT_IMPLEMENTED;
		भङ्ग;
	}

	EVP_PKEY_free(pkey);

	प्रत्यागम res;
}

स्थिर nserror san_गन्तव्यम्_info(X509 *cert, संरचना ns_cert_san **पूर्व_अग्र)
{
	पूर्णाङ्क idx;
	पूर्णाङ्क san_नामानि_nb = -1;
	ध्रुव GENERAL_NAME *current_name;
	ध्रुव अचिह्नित वर्ण *dns_name;
	संरचना ns_cert_san *isan;

	राशिः_सम्बन्धि(GENERAL_NAME) *san_नामानि = रिक्त;

	san_नामानि = X509_get_ext_d2i(cert, NID_subject_alt_name, रिक्त, रिक्त);
	यदि (san_नामानि == रिक्त) {
		प्रत्यागम NSERROR_OK;
	}

	san_नामानि_nb = sk_GENERAL_NAME_अङ्कः(san_नामानि);

	/* Check each name within the extension */
	प्रति (idx = 0; idx < san_नामानि_nb; idx++) {
		current_name = sk_GENERAL_NAME_मानम्(san_नामानि, idx);

		यदि (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_दत्त(current_name->d.dNSName);

			isan = स्मृतिं_देहि(परिमाण(संरचना ns_cert_san));
			यदि (isan != रिक्त) {
				isan->name = strdup((ध्रुव वर्ण *)dns_name);
				isan->अग्र = रिक्त;
				*पूर्व_अग्र = isan;
				पूर्व_अग्र = &isan->अग्र;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	प्रति (idx = 0; idx < san_नामानि_nb; idx++) {
		GENERAL_NAME *प्रविष्टिः = sk_GENERAL_NAME_निष्कासय(san_नामानि);
		GENERAL_NAME_free(प्रविष्टिः);
	}
	sk_GENERAL_NAME_free(san_नामानि);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror
der_गन्तव्यम्_certinfo(ध्रुव uint8_t *der,
		size_t der_दीर्घता,
		संरचना ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ध्रुव ASN1_INTEGER *asn1_अङ्कः;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	यदि (der == रिक्त) {
		प्रत्यागम NSERROR_OK;
	}

	cert = d2i_X509(रिक्त, &der, der_दीर्घता);
	यदि (cert == रिक्त) {
		प्रत्यागम NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->संस्करणम् = X509_get_संस्करणम्(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(शून्य) BIO_समुच्चयः_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = शून्यस्मृतिं_देहि(1, buf->दीर्घता + 1);
	यदि (info->not_before != रिक्त) {
		स्मृतिं_प्रतिलिख(info->not_before, buf->दत्त, (अचिह्नित)buf->दीर्घता);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(शून्य) BIO_समुच्चयः_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = शून्यस्मृतिं_देहि(1, buf->दीर्घता + 1);
	यदि (info->not_after != रिक्त) {
		स्मृतिं_प्रतिलिख(info->not_after, buf->दत्त, (अचिह्नित)buf->दीर्घता);
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
	asn1_अङ्कः = X509_get_serial_सङ्ख्या(cert);
	यदि (asn1_अङ्कः != रिक्त) {
		bignum = ASN1_INTEGER_गन्तव्यम्_BN(asn1_अङ्कः, रिक्त);
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
	अचिह्नित पूर्णाङ्क dig_len;
	अचिह्नित वर्ण *buff;
	पूर्णाङ्क rc;

	digest = EVP_sha1();
	buff = स्मृतिं_देहि(EVP_MD_आकार(digest));
	यदि (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		यदि ((rc == 1) && (dig_len == (अचिह्नित पूर्णाङ्क)EVP_MD_आकार(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		मुक्त(buff);
	}

	digest = EVP_sha256();
	buff = स्मृतिं_देहि(EVP_MD_आकार(digest));
	यदि (buff != रिक्त) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		यदि ((rc == 1) && (dig_len == (अचिह्नित पूर्णाङ्क)EVP_MD_आकार(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		मुक्त(buff);
	}

	/* subject alternative names */
	san_गन्तव्यम्_info(cert, &info->san);

	/* issuer name */
	xname_गन्तव्यम्_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_गन्तव्यम्_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_गन्तव्यम्_info(X509_get_pubkey(cert), &info->लौकिकम्_कुञ्चिका);

	X509_free(cert);

	प्रत्यागम NSERROR_OK;
}

/* copy certificate data */
स्थिर nserror
convert_chain_गन्तव्यम्_cert_info(ध्रुव संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_बहिर्गमनम्)
{
	संरचना ns_cert_info *certs;
	size_t गभीरता;
	nserror res;

	certs = शून्यस्मृतिं_देहि(chain->गभीरता, परिमाण(संरचना ns_cert_info));
	यदि (certs == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रति (गभीरता = 0; गभीरता < chain->गभीरता;गभीरता++) {
		res = der_गन्तव्यम्_certinfo(chain->certs[गभीरता].der,
				      chain->certs[गभीरता].der_दीर्घता,
				      certs + गभीरता);
		यदि (res != NSERROR_OK) {
			मुक्त(certs);
			प्रत्यागम res;
		}
		certs[गभीरता].err = chain->certs[गभीरता].err;
	}

	*cert_info_बहिर्गमनम् = certs;
	प्रत्यागम NSERROR_OK;
}

#अन्यथा
स्थिर nserror
convert_chain_गन्तव्यम्_cert_info(ध्रुव संरचना cert_chain *chain,
			   संरचना ns_cert_info **cert_info_बहिर्गमनम्)
{
	प्रत्यागम NSERROR_NOT_IMPLEMENTED;
}
#यदि_अन्तः


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

		san = san->अग्र;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	प्रत्यागम res;

}


स्थिर nserror
format_certificate_लौकिकम्_कुञ्चिका(संरचना fetch_about_context *ctx,
			      संरचना ns_cert_pkey *लौकिकम्_कुञ्चिका)
{
	nserror res;

	यदि (लौकिकम्_कुञ्चिका->algor == रिक्त) {
		/* skip the table if no algorithm name */
		प्रत्यागम NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 लौकिकम्_कुञ्चिका->algor,
			 लौकिकम्_कुञ्चिका->आकार);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}


	यदि (लौकिकम्_कुञ्चिका->exponent != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 लौकिकम्_कुञ्चिका->exponent);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (लौकिकम्_कुञ्चिका->modulus != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 लौकिकम्_कुञ्चिका->modulus);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (लौकिकम्_कुञ्चिका->वक्रम् != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 लौकिकम्_कुञ्चिका->वक्रम्);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	यदि (लौकिकम्_कुञ्चिका->सार्वजनिक != रिक्त) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 लौकिकम्_कुञ्चिका->सार्वजनिक);
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
		   size_t गभीरता)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 गभीरता, cert_info->subject_name.common_name);
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

	res = format_certificate_लौकिकम्_कुञ्चिका(ctx, &cert_info->लौकिकम्_कुञ्चिका);
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
			 cert_info->संस्करणम्);
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
	पूर्णाङ्क संहिता = 200;
	nserror res;
	संरचना cert_chain *chain = रिक्त;

	/* content is going to return ok */
	fetch_about_समुच्चयः_http_संहिता(ctx, संहिता);

	/* content type */
	यदि (fetch_about_send_शीर्षकम्(ctx, "Content-Type: text/html"))
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
		res = convert_chain_गन्तव्यम्_cert_info(chain, &cert_info);
		यदि (res == NSERROR_OK) {
			size_t गभीरता;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			यदि (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				गच्छ fetch_about_certificate_handler_aborted;
			}

			प्रति (गभीरता = 0; गभीरता < chain->गभीरता; गभीरता++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						गभीरता, (cert_info + गभीरता)
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

			प्रति (गभीरता = 0; गभीरता < chain->गभीरता; गभीरता++) {
				res = format_certificate(ctx, cert_info + गभीरता,
						गभीरता);
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

	fetch_about_send_परिसमाप्तम्(ctx);

	cert_chain_free(chain);

	प्रत्यागम true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	प्रत्यागम false;
}
