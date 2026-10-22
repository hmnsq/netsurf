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

#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <stdio.h>
#समावेश <string.h>

#समावेश "utils/errors.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/inttypes.h"
#समावेश "netsurf/ssl_certs.h"

#समावेश "private.h"
#समावेश "certificate.h"

/**
 * certificate name parameters
 */
काष्ठा ns_cert_name {
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
काष्ठा ns_cert_pkey {
	अक्षर *algor;
	पूर्णांक आकार;
	अक्षर *modulus;
	अक्षर *exponent;
	अक्षर *curve;
	अक्षर *खुला;
};

/**
 * Certificate subject alternative name
 */
काष्ठा ns_cert_san {
	काष्ठा ns_cert_san *अग्र;
	अक्षर *name;
};

/**
 * certificate information for certificate chain
 */
काष्ठा ns_cert_info {
	काष्ठा ns_cert_name subject_name; /**< Subject details */
	काष्ठा ns_cert_name issuer_name; /**< Issuer details */
	काष्ठा ns_cert_pkey public_कुंजी; /**< public key details */
	दीर्घ version;		/**< Certificate version */
	अक्षर *not_before;	/**< Valid from date */
	अक्षर *not_after;	/**< Valid to date */
	पूर्णांक sig_type;		/**< Signature type */
	अक्षर *sig_algor;        /**< Signature Algorithm */
	अक्षर *serialnum;	/**< Serial number */
	अक्षर *sha1fingerprint; /**< fingerprint shar1 encoded */
	अक्षर *sha256fingerprint; /**< fingerprint shar256 encoded */
	काष्ठा ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
अटल nserror free_ns_cert_info(काष्ठा ns_cert_info *cinfo)
{
	काष्ठा ns_cert_san *san;

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
	मुक्त(cinfo->public_कुंजी.खुला);
	मुक्त(cinfo->not_before);
	मुक्त(cinfo->not_after);
	मुक्त(cinfo->sig_algor);
	मुक्त(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	जबतक (san != शून्य) {
		काष्ठा ns_cert_san *अग्र;
		अग्र = san->अग्र;
		मुक्त(san);
		san = अग्र;
	}

	मुक्त(cinfo);

	वापस NSERROR_OK;
}

#अगर_घोषित WITH_OPENSSL

#समावेश <openssl/ssl.h>
#समावेश <openssl/x509v3.h>

#अगर (OPENSSL_VERSION_संख्या < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# अगर defined(LIBRESSL_VERSION_संख्या)
  /* LibreSSL */
#  अगर (LIBRESSL_VERSION_संख्या < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   अगर (LIBRESSL_VERSION_संख्या < 0x2070000fL)
    /* LibreSSL <2.7.0 */
अटल पूर्णांक ns_X509_get_signature_nid(X509 *cert)
{
	वापस OBJ_obj2nid(cert->cert_info->कुंजी->algor->algorithm);
}

अटल स्थिर अचिन्हित अक्षर *ns_ASN1_STRING_get0_तथ्य(ASN1_STRING *asn1str)
{
	वापस (स्थिर अचिन्हित अक्षर *)ASN1_STRING_तथ्य(asn1str);
}
#   अन्यथा
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_तथ्य ASN1_STRING_get0_तथ्य
#   endif

अटल स्थिर BIGNUM *ns_RSA_get0_n(स्थिर RSA *d)
{
	वापस d->n;
}

अटल स्थिर BIGNUM *ns_RSA_get0_e(स्थिर RSA *d)
{
	वापस d->e;
}
#  अन्यथा
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_तथ्य ASN1_STRING_get0_तथ्य
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# अन्यथा
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_तथ्य ASN1_STRING_get0_तथ्य
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

अटल पूर्णांक ns_EVP_PKEY_get_bn_param(स्थिर EVP_PKEY *pkey,
		स्थिर अक्षर *कुंजी_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *परिणाम = शून्य;

	/* Check parameters: only support allocation-form *bn */
	अगर (pkey == शून्य || कुंजी_name == शून्य || bn == शून्य || *bn != शून्य)
		वापस 0;

	/* Only support RSA keys */
	अगर (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		वापस 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	अगर (rsa == शून्य)
		वापस 0;

	अगर (म_भेद(कुंजी_name, "n") == 0) {
		स्थिर BIGNUM *n = ns_RSA_get0_n(rsa);
		अगर (n != शून्य)
			परिणाम = BN_dup(n);
	} अन्यथा अगर (म_भेद(कुंजी_name, "e") == 0) {
		स्थिर BIGNUM *e = ns_RSA_get0_e(rsa);
		अगर (e != शून्य)
			परिणाम = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = परिणाम;

	वापस (परिणाम != शून्य) ? 1 : 0;
}

अटल पूर्णांक ns_EVP_PKEY_get_utf8_string_param(स्थिर EVP_PKEY *pkey,
		स्थिर अक्षर *कुंजी_name, अक्षर *str, माप_प्रकार max_len,
		माप_प्रकार *out_len)
{
	स्थिर EC_GROUP *ecgroup;
	स्थिर अक्षर *group = "";
	माप_प्रकार group_len = 0;
	EC_कुंजी *ec;
	पूर्णांक ret = 0;

	अगर (pkey == शून्य || कुंजी_name == शून्य)
		वापस 0;

	/* Only support EC keys */
	अगर (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		वापस 0;

	/* Only support fetching the group */
	अगर (म_भेद(कुंजी_name, "group") != 0)
		वापस 0;

	ec = EVP_PKEY_get1_EC_कुंजी((EVP_PKEY *) pkey);

	ecgroup = EC_कुंजी_get0_group(ec);
	अगर (ecgroup != शून्य) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = म_माप(group);
	}

	अगर (str != शून्य && max_len > group_len) {
		स_नकल(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	अगर (out_len != शून्य) {
		*out_len = group_len;
	}

	EC_कुंजी_free(ec);

	वापस ret;
}

अटल पूर्णांक ns_EVP_PKEY_get_octet_string_param(स्थिर EVP_PKEY *pkey,
		स्थिर अक्षर *कुंजी_name, अचिन्हित अक्षर *buf, माप_प्रकार max_len,
		माप_प्रकार *out_len)
{
	स्थिर EC_GROUP *ecgroup;
	स्थिर EC_POINT *ecpoint;
	माप_प्रकार len;
	BN_CTX *bnctx;
	EC_कुंजी *ec;
	पूर्णांक ret = 0;

	अगर (pkey == शून्य || कुंजी_name == शून्य)
		वापस 0;

	/* Only support EC keys */
	अगर (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		वापस 0;

	अगर (म_भेद(कुंजी_name, "encoded-pub-key") != 0)
		वापस 0;

	ec = EVP_PKEY_get1_EC_कुंजी((EVP_PKEY *) pkey);
	अगर (ec == शून्य)
		वापस 0;

	ecgroup = EC_कुंजी_get0_group(ec);
	अगर (ecgroup != शून्य) {
		ecpoint = EC_कुंजी_get0_public_कुंजी(ec);
		अगर (ecpoint != शून्य) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 शून्य,
						 0,
						 bnctx);
			अगर (len != 0 && len <= max_len) {
				अगर (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			अगर (out_len != शून्य)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_कुंजी_free(ec);

	वापस ret;
}
#अन्यथा
/* OpenSSL 3.x and later */
#घोषणा ns_X509_get_signature_nid X509_get_signature_nid
#घोषणा ns_ASN1_STRING_get0_तथ्य ASN1_STRING_get0_तथ्य
#घोषणा ns_RSA_get0_n RSA_get0_n
#घोषणा ns_RSA_get0_e RSA_get0_e
#घोषणा ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#घोषणा ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#घोषणा ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#पूर्ण_अगर

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
अटल nserror
xname_to_info(X509_NAME *xname, काष्ठा ns_cert_name *iname)
{
	पूर्णांक entryidx;
	पूर्णांक entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *मूल्य;
	स्थिर अचिन्हित अक्षर *मूल्य_str;
	ASN1_OBJECT *name;
	पूर्णांक name_nid;
	अक्षर **field;

	entrycnt = X509_NAME_entry_गिनती(xname);

	क्रम (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		मूल्य = X509_NAME_ENTRY_get_तथ्य(entry);
		मूल्य_str = ns_ASN1_STRING_get0_तथ्य(मूल्य);
		चयन (name_nid) {
		हाल NID_commonName:
			field = &iname->common_name;
			अवरोध;
		हाल NID_countryName:
			field = &iname->country;
			अवरोध;
		हाल NID_localityName:
			field = &iname->locality;
			अवरोध;
		हाल NID_stateOrProvinceName:
			field = &iname->province;
			अवरोध;
		हाल NID_organizationName:
			field = &iname->organisation;
			अवरोध;
		हाल NID_organizationalUnitName:
			field = &iname->organisation_unit;
			अवरोध;
		शेष :
			field = शून्य;
			अवरोध;
		}
		अगर (field != शून्य) {
			*field = strdup((स्थिर अक्षर *)मूल्य_str);
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
	अगर (iname->common_name == शून्य) {
		iname->common_name = strdup("Unknown");
	}

	वापस NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
अटल अक्षर *hexdup(स्थिर अक्षर *hex)
{
	पूर्णांक hexlen;
	अक्षर *dst;
	अक्षर *out;
	पूर्णांक cn = 0;

	hexlen = म_माप(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = दो_स्मृति(((hexlen * 7) + 6) / 2);

	अगर (dst != शून्य) {
		क्रम (out = dst; *hex != 0; hex++) {
			अगर (cn == 2) {
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
	वापस dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
अटल अक्षर *bindup(अचिन्हित अक्षर *bin, अचिन्हित पूर्णांक binlen)
{
	अक्षर *dst;
	अक्षर *out;
	अचिन्हित पूर्णांक idx;
	स्थिर अक्षर hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = दो_स्मृति(binlen * 7);

	अगर (dst != शून्य) {
		out = dst;
		क्रम (idx = 0; idx < binlen; idx++) {
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
	वापस dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
अटल nserror
rsa_to_info(EVP_PKEY *pkey, काष्ठा ns_cert_pkey *ikey)
{
	BIGNUM *n = शून्य, *e = शून्य;
	अक्षर *tmp;

	अगर (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		वापस NSERROR_BAD_PARAMETER;
	}

	अगर (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		वापस NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->आकार = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	अगर (tmp != शून्य) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	अगर (tmp != शून्य) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	वापस NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
अटल nserror
dsa_to_info(EVP_PKEY *pkey, काष्ठा ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->आकार = EVP_PKEY_bits(pkey);

	वापस NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
अटल nserror
dh_to_info(EVP_PKEY *pkey, काष्ठा ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->आकार = EVP_PKEY_bits(pkey);

	वापस NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
अटल nserror
ec_to_info(EVP_PKEY *pkey, काष्ठा ns_cert_pkey *ikey)
{
	माप_प्रकार len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->आकार = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", शून्य, 0, &len);
	अगर (len != 0) {
		ikey->curve = दो_स्मृति(len + 1);
		अगर (ikey->curve != शून्य) {
			अगर (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, शून्य) == 0) {
				मुक्त(ikey->curve);
				ikey->curve = शून्य;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			शून्य, 0, &len);
	अगर (len != 0) {
		अचिन्हित अक्षर *point = दो_स्मृति(len);
		अगर (point != शून्य) {
			अगर (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					शून्य) == 1) {
				ikey->खुला = bindup(point, len);
			}
			मुक्त(point);
		}
	}

	वापस NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
अटल nserror
pkey_to_info(EVP_PKEY *pkey, काष्ठा ns_cert_pkey *ikey)
{
	nserror res;

	अगर (pkey == शून्य) {
		वापस NSERROR_BAD_PARAMETER;
	}

	चयन (EVP_PKEY_base_id(pkey)) {
	हाल EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		अवरोध;

	हाल EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		अवरोध;

	हाल EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		अवरोध;

	हाल EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		अवरोध;

	शेष:
		res = NSERROR_NOT_IMPLEMENTED;
		अवरोध;
	}

	EVP_PKEY_free(pkey);

	वापस res;
}

अटल nserror san_to_info(X509 *cert, काष्ठा ns_cert_san **पूर्व_अग्र)
{
	पूर्णांक idx;
	पूर्णांक san_names_nb = -1;
	स्थिर GENERAL_NAME *current_name;
	स्थिर अचिन्हित अक्षर *dns_name;
	काष्ठा ns_cert_san *isan;

	ढेर_OF(GENERAL_NAME) *san_names = शून्य;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, शून्य, शून्य);
	अगर (san_names == शून्य) {
		वापस NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_अंक(san_names);

	/* Check each name within the extension */
	क्रम (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_मूल्य(san_names, idx);

		अगर (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_तथ्य(current_name->d.dNSName);

			isan = दो_स्मृति(माप(काष्ठा ns_cert_san));
			अगर (isan != शून्य) {
				isan->name = strdup((स्थिर अक्षर *)dns_name);
				isan->अग्र = शून्य;
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
	क्रम (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_निकालो(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	वापस NSERROR_OK;
}

अटल nserror
der_to_certinfo(स्थिर uint8_t *der,
		माप_प्रकार der_लंबाई,
		काष्ठा ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	स्थिर ASN1_INTEGER *asn1_अंक;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	अगर (der == शून्य) {
		वापस NSERROR_OK;
	}

	cert = d2i_X509(शून्य, &der, der_लंबाई);
	अगर (cert == शून्य) {
		वापस NSERROR_INVALID;
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
	(व्योम) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = सुस्मृति(1, buf->लंबाई + 1);
	अगर (info->not_before != शून्य) {
		स_नकल(info->not_before, buf->तथ्य, (अचिन्हित)buf->लंबाई);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(व्योम) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = सुस्मृति(1, buf->लंबाई + 1);
	अगर (info->not_after != शून्य) {
		स_नकल(info->not_after, buf->तथ्य, (अचिन्हित)buf->लंबाई);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	पूर्णांक pkey_nid = ns_X509_get_signature_nid(cert);
	अगर (pkey_nid != NID_undef) {
		स्थिर अक्षर* sslbuf = OBJ_nid2ln(pkey_nid);
		अगर (sslbuf != शून्य) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_अंक = X509_get_serial_संख्या(cert);
	अगर (asn1_अंक != शून्य) {
		bignum = ASN1_INTEGER_to_BN(asn1_अंक, शून्य);
		अगर (bignum != शून्य) {
			अक्षर *tmp = BN_bn2hex(bignum);
			अगर (tmp != शून्य) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = शून्य;
		}
	}

	/* fingerprints */
	स्थिर EVP_MD *digest;
	अचिन्हित पूर्णांक dig_len;
	अचिन्हित अक्षर *buff;
	पूर्णांक rc;

	digest = EVP_sha1();
	buff = दो_स्मृति(EVP_MD_आकार(digest));
	अगर (buff != शून्य) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		अगर ((rc == 1) && (dig_len == (अचिन्हित पूर्णांक)EVP_MD_आकार(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		मुक्त(buff);
	}

	digest = EVP_sha256();
	buff = दो_स्मृति(EVP_MD_आकार(digest));
	अगर (buff != शून्य) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		अगर ((rc == 1) && (dig_len == (अचिन्हित पूर्णांक)EVP_MD_आकार(digest))) {
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

	वापस NSERROR_OK;
}

/* copy certificate data */
अटल nserror
convert_chain_to_cert_info(स्थिर काष्ठा cert_chain *chain,
			   काष्ठा ns_cert_info **cert_info_out)
{
	काष्ठा ns_cert_info *certs;
	माप_प्रकार depth;
	nserror res;

	certs = सुस्मृति(chain->depth, माप(काष्ठा ns_cert_info));
	अगर (certs == शून्य) {
		वापस NSERROR_NOMEM;
	}

	क्रम (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_लंबाई,
				      certs + depth);
		अगर (res != NSERROR_OK) {
			मुक्त(certs);
			वापस res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	वापस NSERROR_OK;
}

#अन्यथा
अटल nserror
convert_chain_to_cert_info(स्थिर काष्ठा cert_chain *chain,
			   काष्ठा ns_cert_info **cert_info_out)
{
	वापस NSERROR_NOT_IMPLEMENTED;
}
#पूर्ण_अगर


अटल nserror
format_certificate_name(काष्ठा fetch_about_context *ctx,
			काष्ठा ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	अगर (cert_name->organisation != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (cert_name->organisation_unit != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (cert_name->locality != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (cert_name->province != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (cert_name->country != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	वापस res;
}

/**
 * output formatted certificate subject alternate names
 */
अटल nserror
format_certificate_san(काष्ठा fetch_about_context *ctx,
			      काष्ठा ns_cert_san *san)
{
	nserror res;

	अगर (san == शून्य) {
		वापस NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	जबतक (san != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		अगर (res != NSERROR_OK) {
			वापस res;
		}

		san = san->अग्र;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	वापस res;

}


अटल nserror
format_certificate_public_कुंजी(काष्ठा fetch_about_context *ctx,
			      काष्ठा ns_cert_pkey *public_कुंजी)
{
	nserror res;

	अगर (public_कुंजी->algor == शून्य) {
		/* skip the table if no algorithm name */
		वापस NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_कुंजी->algor,
			 public_कुंजी->आकार);
	अगर (res != NSERROR_OK) {
		वापस res;
	}


	अगर (public_कुंजी->exponent != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_कुंजी->exponent);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (public_कुंजी->modulus != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_कुंजी->modulus);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (public_कुंजी->curve != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_कुंजी->curve);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (public_कुंजी->खुला != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_कुंजी->खुला);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	वापस res;
}

अटल nserror
format_certificate_fingerprint(काष्ठा fetch_about_context *ctx,
			      काष्ठा ns_cert_info *cert_info)
{
	nserror res;

	अगर ((cert_info->sha1fingerprint == शून्य) &&
	    (cert_info->sha256fingerprint == शून्य))  {
		/* skip the table if no fingerprints */
		वापस NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	अगर (cert_info->sha256fingerprint != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (cert_info->sha1fingerprint != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	वापस res;
}

अटल nserror
format_certificate(काष्ठा fetch_about_context *ctx,
		   काष्ठा ns_cert_info *cert_info,
		   माप_प्रकार depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	अगर (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = format_certificate_public_कुंजी(ctx, &cert_info->public_कुंजी);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	अगर (cert_info->serialnum != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	अगर (cert_info->sig_algor != शून्य) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	वापस res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(काष्ठा fetch_about_context *ctx)
{
	पूर्णांक code = 200;
	nserror res;
	काष्ठा cert_chain *chain = शून्य;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	अगर (fetch_about_send_header(ctx, "Content-Type: text/html"))
		जाओ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	अगर (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		अगर (res != NSERROR_OK) {
			जाओ fetch_about_certificate_handler_aborted;
		}
	} अन्यथा {
		काष्ठा ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		अगर (res == NSERROR_OK) {
			माप_प्रकार depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			अगर (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				जाओ fetch_about_certificate_handler_aborted;
			}

			क्रम (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				अगर (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					जाओ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			अगर (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				जाओ fetch_about_certificate_handler_aborted;
			}

			क्रम (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				अगर (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					जाओ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} अन्यथा {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			अगर (res != NSERROR_OK) {
				जाओ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	वापस true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	वापस false;
}
