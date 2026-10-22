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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <stdio.h>
#ضمن <string.h>

#ضمن "utils/errors.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/inttypes.h"
#ضمن "netsurf/ssl_certs.h"

#ضمن "private.h"
#ضمن "certificate.h"

/**
 * certificate name parameters
 */
بنية ns_cert_name {
	حرف *common_name;
	حرف *organisation;
	حرف *organisation_unit;
	حرف *locality;
	حرف *province;
	حرف *country;
};

/**
 * Certificate public key parameters
 */
بنية ns_cert_pkey {
	حرف *algor;
	صحيح اندازه;
	حرف *modulus;
	حرف *exponent;
	حرف *منحنی;
	حرف *عام;
};

/**
 * Certificate subject alternative name
 */
بنية ns_cert_san {
	بنية ns_cert_san *بعدی;
	حرف *name;
};

/**
 * certificate information for certificate chain
 */
بنية ns_cert_info {
	بنية ns_cert_name subject_name; /**< Subject details */
	بنية ns_cert_name issuer_name; /**< Issuer details */
	بنية ns_cert_pkey عمومی_کلید; /**< public key details */
	دراز نسخه;		/**< Certificate version */
	حرف *not_before;	/**< Valid from date */
	حرف *not_after;	/**< Valid to date */
	صحيح sig_type;		/**< Signature type */
	حرف *sig_algor;        /**< Signature Algorithm */
	حرف *serialnum;	/**< Serial number */
	حرف *sha1fingerprint; /**< fingerprint shar1 encoded */
	حرف *sha256fingerprint; /**< fingerprint shar256 encoded */
	بنية ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
ساكن nserror free_ns_cert_info(بنية ns_cert_info *cinfo)
{
	بنية ns_cert_san *san;

	حرر(cinfo->subject_name.common_name);
	حرر(cinfo->subject_name.organisation);
	حرر(cinfo->subject_name.organisation_unit);
	حرر(cinfo->subject_name.locality);
	حرر(cinfo->subject_name.province);
	حرر(cinfo->subject_name.country);
	حرر(cinfo->issuer_name.common_name);
	حرر(cinfo->issuer_name.organisation);
	حرر(cinfo->issuer_name.organisation_unit);
	حرر(cinfo->issuer_name.locality);
	حرر(cinfo->issuer_name.province);
	حرر(cinfo->issuer_name.country);
	حرر(cinfo->عمومی_کلید.algor);
	حرر(cinfo->عمومی_کلید.modulus);
	حرر(cinfo->عمومی_کلید.exponent);
	حرر(cinfo->عمومی_کلید.منحنی);
	حرر(cinfo->عمومی_کلید.عام);
	حرر(cinfo->not_before);
	حرر(cinfo->not_after);
	حرر(cinfo->sig_algor);
	حرر(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	طالما (san != عدم) {
		بنية ns_cert_san *بعدی;
		بعدی = san->بعدی;
		حرر(san);
		san = بعدی;
	}

	حرر(cinfo);

	أرجع NSERROR_OK;
}

#اگر_تعریف_شده WITH_OPENSSL

#ضمن <openssl/ssl.h>
#ضمن <openssl/x509v3.h>

#إذا (OPENSSL_نسخه_عدد < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# إذا defined(LIBRESSL_نسخه_عدد)
  /* LibreSSL */
#  إذا (LIBRESSL_نسخه_عدد < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   إذا (LIBRESSL_نسخه_عدد < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ساكن صحيح ns_X509_get_signature_nid(X509 *cert)
{
	أرجع OBJ_obj2nid(cert->cert_info->کلید->algor->algorithm);
}

ساكن ثابت بیعلامت حرف *ns_ASN1_STRING_get0_داده(ASN1_STRING *asn1str)
{
	أرجع (ثابت بیعلامت حرف *)ASN1_STRING_داده(asn1str);
}
#   وإلا
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_داده ASN1_STRING_get0_داده
#   endif

ساكن ثابت BIGNUM *ns_RSA_get0_n(ثابت RSA *d)
{
	أرجع d->n;
}

ساكن ثابت BIGNUM *ns_RSA_get0_e(ثابت RSA *d)
{
	أرجع d->e;
}
#  وإلا
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_داده ASN1_STRING_get0_داده
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# وإلا
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_داده ASN1_STRING_get0_داده
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ساكن صحيح ns_EVP_PKEY_get_bn_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *نتیجه = عدم;

	/* Check parameters: only support allocation-form *bn */
	إذا (pkey == عدم || کلید_name == عدم || bn == عدم || *bn != عدم)
		أرجع 0;

	/* Only support RSA keys */
	إذا (EVP_PKEY_پایه_id(pkey) != EVP_PKEY_RSA)
		أرجع 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	إذا (rsa == عدم)
		أرجع 0;

	إذا (strcmp(کلید_name, "n") == 0) {
		ثابت BIGNUM *n = ns_RSA_get0_n(rsa);
		إذا (n != عدم)
			نتیجه = BN_dup(n);
	} وإلا إذا (strcmp(کلید_name, "e") == 0) {
		ثابت BIGNUM *e = ns_RSA_get0_e(rsa);
		إذا (e != عدم)
			نتیجه = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = نتیجه;

	أرجع (نتیجه != عدم) ? 1 : 0;
}

ساكن صحيح ns_EVP_PKEY_get_utf8_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, حرف *str, size_t max_len,
		size_t *بیرون_len)
{
	ثابت EC_گروه *ecgroup;
	ثابت حرف *گروه = "";
	size_t گروه_len = 0;
	EC_کلید *ec;
	صحيح ret = 0;

	إذا (pkey == عدم || کلید_name == عدم)
		أرجع 0;

	/* Only support EC keys */
	إذا (EVP_PKEY_پایه_id(pkey) != EVP_PKEY_EC)
		أرجع 0;

	/* Only support fetching the group */
	إذا (strcmp(کلید_name, "group") != 0)
		أرجع 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);

	ecgroup = EC_کلید_get0_گروه(ec);
	إذا (ecgroup != عدم) {
		گروه = OBJ_nid2ln(EC_گروه_get_منحنی_name(ecgroup));
		گروه_len = طول_رشته(گروه);
	}

	إذا (str != عدم && max_len > گروه_len) {
		کپی_حافظه(str, گروه, گروه_len);
		str[گروه_len] = '\0';
		ret = 1;
	}

	إذا (بیرون_len != عدم) {
		*بیرون_len = گروه_len;
	}

	EC_کلید_free(ec);

	أرجع ret;
}

ساكن صحيح ns_EVP_PKEY_get_octet_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, بیعلامت حرف *buf, size_t max_len,
		size_t *بیرون_len)
{
	ثابت EC_گروه *ecgroup;
	ثابت EC_نقطه *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_کلید *ec;
	صحيح ret = 0;

	إذا (pkey == عدم || کلید_name == عدم)
		أرجع 0;

	/* Only support EC keys */
	إذا (EVP_PKEY_پایه_id(pkey) != EVP_PKEY_EC)
		أرجع 0;

	إذا (strcmp(کلید_name, "encoded-pub-key") != 0)
		أرجع 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);
	إذا (ec == عدم)
		أرجع 0;

	ecgroup = EC_کلید_get0_گروه(ec);
	إذا (ecgroup != عدم) {
		ecpoint = EC_کلید_get0_عمومی_کلید(ec);
		إذا (ecpoint != عدم) {
			bnctx = BN_CTX_new();
			len = EC_نقطه_point2oct(ecgroup,
						 ecpoint,
						 نقطه_CONVERSION_UNCOMPRESSED,
						 عدم,
						 0,
						 bnctx);
			إذا (len != 0 && len <= max_len) {
				إذا (EC_نقطه_point2oct(ecgroup,
						       ecpoint,
						       نقطه_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			إذا (بیرون_len != عدم)
				*بیرون_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_کلید_free(ec);

	أرجع ret;
}
#وإلا
/* OpenSSL 3.x and later */
#تعریف_کن ns_X509_get_signature_nid X509_get_signature_nid
#تعریف_کن ns_ASN1_STRING_get0_داده ASN1_STRING_get0_داده
#تعریف_کن ns_RSA_get0_n RSA_get0_n
#تعریف_کن ns_RSA_get0_e RSA_get0_e
#تعریف_کن ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#تعریف_کن ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#تعریف_کن ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#پایان_اگر

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
ساكن nserror
xname_مقصد_info(X509_NAME *xname, بنية ns_cert_name *iname)
{
	صحيح entryidx;
	صحيح entrycnt;
	X509_NAME_مدخل *مدخل; /* current name entry */
	ASN1_STRING *مقدار;
	ثابت بیعلامت حرف *مقدار_str;
	ASN1_OBJECT *name;
	صحيح name_nid;
	حرف **فیلد;

	entrycnt = X509_NAME_مدخل_شمار(xname);

	لكل (entryidx = 0; entryidx < entrycnt; entryidx++) {
		مدخل = X509_NAME_get_مدخل(xname, entryidx);
		name = X509_NAME_مدخل_get_object(مدخل);
		name_nid = OBJ_obj2nid(name);
		مقدار = X509_NAME_مدخل_get_داده(مدخل);
		مقدار_str = ns_ASN1_STRING_get0_داده(مقدار);
		بدل (name_nid) {
		حالة NID_commonName:
			فیلد = &iname->common_name;
			اكسر;
		حالة NID_countryName:
			فیلد = &iname->country;
			اكسر;
		حالة NID_localityName:
			فیلد = &iname->locality;
			اكسر;
		حالة NID_stateOrProvinceName:
			فیلد = &iname->province;
			اكسر;
		حالة NID_organizationName:
			فیلد = &iname->organisation;
			اكسر;
		حالة NID_organizationalUnitName:
			فیلد = &iname->organisation_unit;
			اكسر;
		افتراضي :
			فیلد = عدم;
			اكسر;
		}
		إذا (فیلد != عدم) {
			*فیلد = strdup((ثابت حرف *)مقدار_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *فیلد);
		} وإلا {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	إذا (iname->common_name == عدم) {
		iname->common_name = strdup("Unknown");
	}

	أرجع NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ساكن حرف *hexdup(ثابت حرف *hex)
{
	صحيح hexlen;
	حرف *dst;
	حرف *بیرون;
	صحيح cn = 0;

	hexlen = طول_رشته(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = حافظه_بده(((hexlen * 7) + 6) / 2);

	إذا (dst != عدم) {
		لكل (بیرون = dst; *hex != 0; hex++) {
			إذا (cn == 2) {
				cn = 0;
				*بیرون++ = '&';
				*بیرون++ = '#';
				*بیرون++ = '5';
				*بیرون++ = '8';
				*بیرون++ = ';';
			}
			*بیرون++ = *hex;
			cn++;
		}
		*بیرون = 0;
	}
	أرجع dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ساكن حرف *bindup(بیعلامت حرف *bin, بیعلامت صحيح binlen)
{
	حرف *dst;
	حرف *بیرون;
	بیعلامت صحيح idx;
	ثابت حرف hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = حافظه_بده(binlen * 7);

	إذا (dst != عدم) {
		بیرون = dst;
		لكل (idx = 0; idx < binlen; idx++) {
			*بیرون++ = hex[(bin[idx] & 0xf0) >> 4];
			*بیرون++ = hex[bin[idx] & 0xf];

			*بیرون++ = '&';
			*بیرون++ = '#';
			*بیرون++ = '5';
			*بیرون++ = '8';
			*بیرون++ = ';';
		}
		بیرون -= 5;
		*بیرون = 0;
	}
	أرجع dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساكن nserror
rsa_مقصد_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	BIGNUM *n = عدم, *e = عدم;
	حرف *tmp;

	إذا (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	إذا (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		أرجع NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->اندازه = EVP_PKEY_بیتها(pkey);

	tmp = BN_bn2hex(n);
	إذا (tmp != عدم) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	إذا (tmp != عدم) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	أرجع NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساكن nserror
dsa_مقصد_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->اندازه = EVP_PKEY_بیتها(pkey);

	أرجع NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساكن nserror
dh_مقصد_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->اندازه = EVP_PKEY_بیتها(pkey);

	أرجع NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساكن nserror
ec_مقصد_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->اندازه = EVP_PKEY_بیتها(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", عدم, 0, &len);
	إذا (len != 0) {
		ikey->منحنی = حافظه_بده(len + 1);
		إذا (ikey->منحنی != عدم) {
			إذا (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->منحنی, len + 1, عدم) == 0) {
				حرر(ikey->منحنی);
				ikey->منحنی = عدم;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			عدم, 0, &len);
	إذا (len != 0) {
		بیعلامت حرف *نقطه = حافظه_بده(len);
		إذا (نقطه != عدم) {
			إذا (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", نقطه, len,
					عدم) == 1) {
				ikey->عام = bindup(نقطه, len);
			}
			حرر(نقطه);
		}
	}

	أرجع NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساكن nserror
pkey_مقصد_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	nserror res;

	إذا (pkey == عدم) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	بدل (EVP_PKEY_پایه_id(pkey)) {
	حالة EVP_PKEY_RSA:
		res = rsa_مقصد_info(pkey, ikey);
		اكسر;

	حالة EVP_PKEY_DSA:
		res = dsa_مقصد_info(pkey, ikey);
		اكسر;

	حالة EVP_PKEY_DH:
		res = dh_مقصد_info(pkey, ikey);
		اكسر;

	حالة EVP_PKEY_EC:
		res = ec_مقصد_info(pkey, ikey);
		اكسر;

	افتراضي:
		res = NSERROR_NOT_IMPLEMENTED;
		اكسر;
	}

	EVP_PKEY_free(pkey);

	أرجع res;
}

ساكن nserror san_مقصد_info(X509 *cert, بنية ns_cert_san **prev_بعدی)
{
	صحيح idx;
	صحيح san_نامها_nb = -1;
	ثابت GENERAL_NAME *current_name;
	ثابت بیعلامت حرف *dns_name;
	بنية ns_cert_san *isan;

	پشته_از(GENERAL_NAME) *san_نامها = عدم;

	san_نامها = X509_get_ext_d2i(cert, NID_subject_alt_name, عدم, عدم);
	إذا (san_نامها == عدم) {
		أرجع NSERROR_OK;
	}

	san_نامها_nb = sk_GENERAL_NAME_رقم(san_نامها);

	/* Check each name within the extension */
	لكل (idx = 0; idx < san_نامها_nb; idx++) {
		current_name = sk_GENERAL_NAME_مقدار(san_نامها, idx);

		إذا (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_داده(current_name->d.dNSName);

			isan = حافظه_بده(حجم(بنية ns_cert_san));
			إذا (isan != عدم) {
				isan->name = strdup((ثابت حرف *)dns_name);
				isan->بعدی = عدم;
				*prev_بعدی = isan;
				prev_بعدی = &isan->بعدی;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	لكل (idx = 0; idx < san_نامها_nb; idx++) {
		GENERAL_NAME *مدخل = sk_GENERAL_NAME_برداشت(san_نامها);
		GENERAL_NAME_free(مدخل);
	}
	sk_GENERAL_NAME_free(san_نامها);

	أرجع NSERROR_OK;
}

ساكن nserror
der_مقصد_certinfo(ثابت uint8_t *der,
		size_t der_درازی,
		بنية ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ثابت ASN1_INTEGER *asn1_رقم;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	إذا (der == عدم) {
		أرجع NSERROR_OK;
	}

	cert = d2i_X509(عدم, &der, der_درازی);
	إذا (cert == عدم) {
		أرجع NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->نسخه = X509_get_نسخه(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_زمان_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(فراغ) BIO_مجموعه_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = حافظه_صفر_بده(1, buf->درازی + 1);
	إذا (info->not_before != عدم) {
		کپی_حافظه(info->not_before, buf->داده, (بیعلامت)buf->درازی);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_زمان_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(فراغ) BIO_مجموعه_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = حافظه_صفر_بده(1, buf->درازی + 1);
	إذا (info->not_after != عدم) {
		کپی_حافظه(info->not_after, buf->داده, (بیعلامت)buf->درازی);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	صحيح pkey_nid = ns_X509_get_signature_nid(cert);
	إذا (pkey_nid != NID_undef) {
		ثابت حرف* sslbuf = OBJ_nid2ln(pkey_nid);
		إذا (sslbuf != عدم) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_رقم = X509_get_serial_عدد(cert);
	إذا (asn1_رقم != عدم) {
		bignum = ASN1_INTEGER_مقصد_BN(asn1_رقم, عدم);
		إذا (bignum != عدم) {
			حرف *tmp = BN_bn2hex(bignum);
			إذا (tmp != عدم) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = عدم;
		}
	}

	/* fingerprints */
	ثابت EVP_MD *digest;
	بیعلامت صحيح dig_len;
	بیعلامت حرف *buff;
	صحيح rc;

	digest = EVP_sha1();
	buff = حافظه_بده(EVP_MD_اندازه(digest));
	إذا (buff != عدم) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		إذا ((rc == 1) && (dig_len == (بیعلامت صحيح)EVP_MD_اندازه(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		حرر(buff);
	}

	digest = EVP_sha256();
	buff = حافظه_بده(EVP_MD_اندازه(digest));
	إذا (buff != عدم) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		إذا ((rc == 1) && (dig_len == (بیعلامت صحيح)EVP_MD_اندازه(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		حرر(buff);
	}

	/* subject alternative names */
	san_مقصد_info(cert, &info->san);

	/* issuer name */
	xname_مقصد_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_مقصد_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_مقصد_info(X509_get_pubkey(cert), &info->عمومی_کلید);

	X509_free(cert);

	أرجع NSERROR_OK;
}

/* copy certificate data */
ساكن nserror
convert_chain_مقصد_cert_info(ثابت بنية cert_chain *chain,
			   بنية ns_cert_info **cert_info_بیرون)
{
	بنية ns_cert_info *certs;
	size_t عمق;
	nserror res;

	certs = حافظه_صفر_بده(chain->عمق, حجم(بنية ns_cert_info));
	إذا (certs == عدم) {
		أرجع NSERROR_NOMEM;
	}

	لكل (عمق = 0; عمق < chain->عمق;عمق++) {
		res = der_مقصد_certinfo(chain->certs[عمق].der,
				      chain->certs[عمق].der_درازی,
				      certs + عمق);
		إذا (res != NSERROR_OK) {
			حرر(certs);
			أرجع res;
		}
		certs[عمق].err = chain->certs[عمق].err;
	}

	*cert_info_بیرون = certs;
	أرجع NSERROR_OK;
}

#وإلا
ساكن nserror
convert_chain_مقصد_cert_info(ثابت بنية cert_chain *chain,
			   بنية ns_cert_info **cert_info_بیرون)
{
	أرجع NSERROR_NOT_IMPLEMENTED;
}
#پایان_اگر


ساكن nserror
format_certificate_name(بنية fetch_about_context *ctx,
			بنية ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	إذا (cert_name->organisation != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (cert_name->organisation_unit != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (cert_name->locality != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (cert_name->province != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (cert_name->country != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	أرجع res;
}

/**
 * output formatted certificate subject alternate names
 */
ساكن nserror
format_certificate_san(بنية fetch_about_context *ctx,
			      بنية ns_cert_san *san)
{
	nserror res;

	إذا (san == عدم) {
		أرجع NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	طالما (san != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}

		san = san->بعدی;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	أرجع res;

}


ساكن nserror
format_certificate_عمومی_کلید(بنية fetch_about_context *ctx,
			      بنية ns_cert_pkey *عمومی_کلید)
{
	nserror res;

	إذا (عمومی_کلید->algor == عدم) {
		/* skip the table if no algorithm name */
		أرجع NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 عمومی_کلید->algor,
			 عمومی_کلید->اندازه);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}


	إذا (عمومی_کلید->exponent != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 عمومی_کلید->exponent);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عمومی_کلید->modulus != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 عمومی_کلید->modulus);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عمومی_کلید->منحنی != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 عمومی_کلید->منحنی);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عمومی_کلید->عام != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 عمومی_کلید->عام);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	أرجع res;
}

ساكن nserror
format_certificate_fingerprint(بنية fetch_about_context *ctx,
			      بنية ns_cert_info *cert_info)
{
	nserror res;

	إذا ((cert_info->sha1fingerprint == عدم) &&
	    (cert_info->sha256fingerprint == عدم))  {
		/* skip the table if no fingerprints */
		أرجع NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	إذا (cert_info->sha256fingerprint != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (cert_info->sha1fingerprint != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	أرجع res;
}

ساكن nserror
format_certificate(بنية fetch_about_context *ctx,
		   بنية ns_cert_info *cert_info,
		   size_t عمق)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 عمق, cert_info->subject_name.common_name);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	إذا (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = format_certificate_عمومی_کلید(ctx, &cert_info->عمومی_کلید);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	إذا (cert_info->serialnum != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (cert_info->sig_algor != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->نسخه);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	أرجع res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(بنية fetch_about_context *ctx)
{
	صحيح کد = 200;
	nserror res;
	بنية cert_chain *chain = عدم;

	/* content is going to return ok */
	fetch_about_مجموعه_http_کد(ctx, کد);

	/* content type */
	إذا (fetch_about_send_سرآیند(ctx, "Content-Type: text/html"))
		اذهب fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	إذا (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		إذا (res != NSERROR_OK) {
			اذهب fetch_about_certificate_handler_aborted;
		}
	} وإلا {
		بنية ns_cert_info *cert_info;
		res = convert_chain_مقصد_cert_info(chain, &cert_info);
		إذا (res == NSERROR_OK) {
			size_t عمق;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			إذا (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				اذهب fetch_about_certificate_handler_aborted;
			}

			لكل (عمق = 0; عمق < chain->عمق; عمق++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						عمق, (cert_info + عمق)
							->subject_name
								.common_name);
				إذا (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					اذهب fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			إذا (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				اذهب fetch_about_certificate_handler_aborted;
			}

			لكل (عمق = 0; عمق < chain->عمق; عمق++) {
				res = format_certificate(ctx, cert_info + عمق,
						عمق);
				إذا (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					اذهب fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} وإلا {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			إذا (res != NSERROR_OK) {
				اذهب fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_پایانیافته(ctx);

	cert_chain_free(chain);

	أرجع true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	أرجع false;
}
