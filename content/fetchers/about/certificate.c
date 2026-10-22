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

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <stdio.h>
#شامل <string.h>

#شامل "utils/errors.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/inttypes.h"
#شامل "netsurf/ssl_certs.h"

#شامل "private.h"
#شامل "certificate.h"

/**
 * certificate name parameters
 */
ساخت ns_cert_name {
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
ساخت ns_cert_pkey {
	حرف *algor;
	صحیح سائز;
	حرف *modulus;
	حرف *exponent;
	حرف *خم;
	حرف *عوامی;
};

/**
 * Certificate subject alternative name
 */
ساخت ns_cert_san {
	ساخت ns_cert_san *اگلا;
	حرف *name;
};

/**
 * certificate information for certificate chain
 */
ساخت ns_cert_info {
	ساخت ns_cert_name subject_name; /**< Subject details */
	ساخت ns_cert_name issuer_name; /**< Issuer details */
	ساخت ns_cert_pkey عوامی_کلید; /**< public key details */
	طویل نسخہ;		/**< Certificate version */
	حرف *not_before;	/**< Valid from date */
	حرف *not_after;	/**< Valid to date */
	صحیح sig_type;		/**< Signature type */
	حرف *sig_algor;        /**< Signature Algorithm */
	حرف *serialnum;	/**< Serial number */
	حرف *sha1fingerprint; /**< fingerprint shar1 encoded */
	حرف *sha256fingerprint; /**< fingerprint shar256 encoded */
	ساخت ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
ساکن nserror free_ns_cert_info(ساخت ns_cert_info *cinfo)
{
	ساخت ns_cert_san *san;

	آزاد(cinfo->subject_name.common_name);
	آزاد(cinfo->subject_name.organisation);
	آزاد(cinfo->subject_name.organisation_unit);
	آزاد(cinfo->subject_name.locality);
	آزاد(cinfo->subject_name.province);
	آزاد(cinfo->subject_name.country);
	آزاد(cinfo->issuer_name.common_name);
	آزاد(cinfo->issuer_name.organisation);
	آزاد(cinfo->issuer_name.organisation_unit);
	آزاد(cinfo->issuer_name.locality);
	آزاد(cinfo->issuer_name.province);
	آزاد(cinfo->issuer_name.country);
	آزاد(cinfo->عوامی_کلید.algor);
	آزاد(cinfo->عوامی_کلید.modulus);
	آزاد(cinfo->عوامی_کلید.exponent);
	آزاد(cinfo->عوامی_کلید.خم);
	آزاد(cinfo->عوامی_کلید.عوامی);
	آزاد(cinfo->not_before);
	آزاد(cinfo->not_after);
	آزاد(cinfo->sig_algor);
	آزاد(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	جب (san != تہی) {
		ساخت ns_cert_san *اگلا;
		اگلا = san->اگلا;
		آزاد(san);
		san = اگلا;
	}

	آزاد(cinfo);

	واپس NSERROR_OK;
}

#اگر_تعریف_شدہ WITH_OPENSSL

#شامل <openssl/ssl.h>
#شامل <openssl/x509v3.h>

#اگر (OPENSSL_نسخہ_نمبر < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# اگر defined(LIBRESSL_نسخہ_نمبر)
  /* LibreSSL */
#  اگر (LIBRESSL_نسخہ_نمبر < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   اگر (LIBRESSL_نسخہ_نمبر < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ساکن صحیح ns_X509_get_signature_nid(X509 *cert)
{
	واپس OBJ_obj2nid(cert->cert_info->کلید->algor->algorithm);
}

ساکن ثابت غیرعلامتی حرف *ns_ASN1_STRING_get0_ڈیٹا(ASN1_STRING *asn1str)
{
	واپس (ثابت غیرعلامتی حرف *)ASN1_STRING_ڈیٹا(asn1str);
}
#   ورنہ
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_ڈیٹا ASN1_STRING_get0_ڈیٹا
#   endif

ساکن ثابت BIGNUM *ns_RSA_get0_n(ثابت RSA *d)
{
	واپس d->n;
}

ساکن ثابت BIGNUM *ns_RSA_get0_e(ثابت RSA *d)
{
	واپس d->e;
}
#  ورنہ
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_ڈیٹا ASN1_STRING_get0_ڈیٹا
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# ورنہ
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_ڈیٹا ASN1_STRING_get0_ڈیٹا
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ساکن صحیح ns_EVP_PKEY_get_bn_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *نتیجہ = تہی;

	/* Check parameters: only support allocation-form *bn */
	اگر (pkey == تہی || کلید_name == تہی || bn == تہی || *bn != تہی)
		واپس 0;

	/* Only support RSA keys */
	اگر (EVP_PKEY_بنیاد_id(pkey) != EVP_PKEY_RSA)
		واپس 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	اگر (rsa == تہی)
		واپس 0;

	اگر (strcmp(کلید_name, "n") == 0) {
		ثابت BIGNUM *n = ns_RSA_get0_n(rsa);
		اگر (n != تہی)
			نتیجہ = BN_dup(n);
	} ورنہ اگر (strcmp(کلید_name, "e") == 0) {
		ثابت BIGNUM *e = ns_RSA_get0_e(rsa);
		اگر (e != تہی)
			نتیجہ = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = نتیجہ;

	واپس (نتیجہ != تہی) ? 1 : 0;
}

ساکن صحیح ns_EVP_PKEY_get_utf8_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, حرف *str, size_t max_len,
		size_t *برآمد_len)
{
	ثابت EC_گروہ *ecgroup;
	ثابت حرف *گروہ = "";
	size_t گروہ_len = 0;
	EC_کلید *ec;
	صحیح ret = 0;

	اگر (pkey == تہی || کلید_name == تہی)
		واپس 0;

	/* Only support EC keys */
	اگر (EVP_PKEY_بنیاد_id(pkey) != EVP_PKEY_EC)
		واپس 0;

	/* Only support fetching the group */
	اگر (strcmp(کلید_name, "group") != 0)
		واپس 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);

	ecgroup = EC_کلید_get0_گروہ(ec);
	اگر (ecgroup != تہی) {
		گروہ = OBJ_nid2ln(EC_گروہ_get_خم_name(ecgroup));
		گروہ_len = لڑی_لمبائی(گروہ);
	}

	اگر (str != تہی && max_len > گروہ_len) {
		حافظہ_نقل(str, گروہ, گروہ_len);
		str[گروہ_len] = '\0';
		ret = 1;
	}

	اگر (برآمد_len != تہی) {
		*برآمد_len = گروہ_len;
	}

	EC_کلید_free(ec);

	واپس ret;
}

ساکن صحیح ns_EVP_PKEY_get_octet_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, غیرعلامتی حرف *buf, size_t max_len,
		size_t *برآمد_len)
{
	ثابت EC_گروہ *ecgroup;
	ثابت EC_نقطہ *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_کلید *ec;
	صحیح ret = 0;

	اگر (pkey == تہی || کلید_name == تہی)
		واپس 0;

	/* Only support EC keys */
	اگر (EVP_PKEY_بنیاد_id(pkey) != EVP_PKEY_EC)
		واپس 0;

	اگر (strcmp(کلید_name, "encoded-pub-key") != 0)
		واپس 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);
	اگر (ec == تہی)
		واپس 0;

	ecgroup = EC_کلید_get0_گروہ(ec);
	اگر (ecgroup != تہی) {
		ecpoint = EC_کلید_get0_عوامی_کلید(ec);
		اگر (ecpoint != تہی) {
			bnctx = BN_CTX_new();
			len = EC_نقطہ_point2oct(ecgroup,
						 ecpoint,
						 نقطہ_CONVERSION_UNCOMPRESSED,
						 تہی,
						 0,
						 bnctx);
			اگر (len != 0 && len <= max_len) {
				اگر (EC_نقطہ_point2oct(ecgroup,
						       ecpoint,
						       نقطہ_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			اگر (برآمد_len != تہی)
				*برآمد_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_کلید_free(ec);

	واپس ret;
}
#ورنہ
/* OpenSSL 3.x and later */
#تعریف_کرو ns_X509_get_signature_nid X509_get_signature_nid
#تعریف_کرو ns_ASN1_STRING_get0_ڈیٹا ASN1_STRING_get0_ڈیٹا
#تعریف_کرو ns_RSA_get0_n RSA_get0_n
#تعریف_کرو ns_RSA_get0_e RSA_get0_e
#تعریف_کرو ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#تعریف_کرو ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#تعریف_کرو ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#اگر_ختم

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
ساکن nserror
xname_مقصد_info(X509_NAME *xname, ساخت ns_cert_name *iname)
{
	صحیح entryidx;
	صحیح entrycnt;
	X509_NAME_اندراج *اندراج_2; /* current name entry */
	ASN1_STRING *قدر;
	ثابت غیرعلامتی حرف *قدر_str;
	ASN1_OBJECT *name;
	صحیح name_nid;
	حرف **میدان;

	entrycnt = X509_NAME_اندراج_گنتی(xname);

	برائے (entryidx = 0; entryidx < entrycnt; entryidx++) {
		اندراج_2 = X509_NAME_get_اندراج(xname, entryidx);
		name = X509_NAME_اندراج_get_object(اندراج_2);
		name_nid = OBJ_obj2nid(name);
		قدر = X509_NAME_اندراج_get_ڈیٹا(اندراج_2);
		قدر_str = ns_ASN1_STRING_get0_ڈیٹا(قدر);
		انتخاب (name_nid) {
		حالت NID_commonName:
			میدان = &iname->common_name;
			توڑو;
		حالت NID_countryName:
			میدان = &iname->country;
			توڑو;
		حالت NID_localityName:
			میدان = &iname->locality;
			توڑو;
		حالت NID_stateOrProvinceName:
			میدان = &iname->province;
			توڑو;
		حالت NID_organizationName:
			میدان = &iname->organisation;
			توڑو;
		حالت NID_organizationalUnitName:
			میدان = &iname->organisation_unit;
			توڑو;
		طےشدہ :
			میدان = تہی;
			توڑو;
		}
		اگر (میدان != تہی) {
			*میدان = strdup((ثابت حرف *)قدر_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *میدان);
		} ورنہ {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	اگر (iname->common_name == تہی) {
		iname->common_name = strdup("Unknown");
	}

	واپس NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ساکن حرف *hexdup(ثابت حرف *hex)
{
	صحیح hexlen;
	حرف *dst;
	حرف *برآمد;
	صحیح cn = 0;

	hexlen = لڑی_لمبائی(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = حافظہ_دو(((hexlen * 7) + 6) / 2);

	اگر (dst != تہی) {
		برائے (برآمد = dst; *hex != 0; hex++) {
			اگر (cn == 2) {
				cn = 0;
				*برآمد++ = '&';
				*برآمد++ = '#';
				*برآمد++ = '5';
				*برآمد++ = '8';
				*برآمد++ = ';';
			}
			*برآمد++ = *hex;
			cn++;
		}
		*برآمد = 0;
	}
	واپس dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ساکن حرف *bindup(غیرعلامتی حرف *bin, غیرعلامتی صحیح binlen)
{
	حرف *dst;
	حرف *برآمد;
	غیرعلامتی صحیح idx;
	ثابت حرف hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = حافظہ_دو(binlen * 7);

	اگر (dst != تہی) {
		برآمد = dst;
		برائے (idx = 0; idx < binlen; idx++) {
			*برآمد++ = hex[(bin[idx] & 0xf0) >> 4];
			*برآمد++ = hex[bin[idx] & 0xf];

			*برآمد++ = '&';
			*برآمد++ = '#';
			*برآمد++ = '5';
			*برآمد++ = '8';
			*برآمد++ = ';';
		}
		برآمد -= 5;
		*برآمد = 0;
	}
	واپس dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساکن nserror
rsa_مقصد_info(EVP_PKEY *pkey, ساخت ns_cert_pkey *ikey)
{
	BIGNUM *n = تہی, *e = تہی;
	حرف *tmp;

	اگر (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		واپس NSERROR_BAD_PARAMETER;
	}

	اگر (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		واپس NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->سائز = EVP_PKEY_بٹ(pkey);

	tmp = BN_bn2hex(n);
	اگر (tmp != تہی) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	اگر (tmp != تہی) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	واپس NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساکن nserror
dsa_مقصد_info(EVP_PKEY *pkey, ساخت ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->سائز = EVP_PKEY_بٹ(pkey);

	واپس NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساکن nserror
dh_مقصد_info(EVP_PKEY *pkey, ساخت ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->سائز = EVP_PKEY_بٹ(pkey);

	واپس NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساکن nserror
ec_مقصد_info(EVP_PKEY *pkey, ساخت ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->سائز = EVP_PKEY_بٹ(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", تہی, 0, &len);
	اگر (len != 0) {
		ikey->خم = حافظہ_دو(len + 1);
		اگر (ikey->خم != تہی) {
			اگر (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->خم, len + 1, تہی) == 0) {
				آزاد(ikey->خم);
				ikey->خم = تہی;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			تہی, 0, &len);
	اگر (len != 0) {
		غیرعلامتی حرف *نقطہ = حافظہ_دو(len);
		اگر (نقطہ != تہی) {
			اگر (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", نقطہ, len,
					تہی) == 1) {
				ikey->عوامی = bindup(نقطہ, len);
			}
			آزاد(نقطہ);
		}
	}

	واپس NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ساکن nserror
pkey_مقصد_info(EVP_PKEY *pkey, ساخت ns_cert_pkey *ikey)
{
	nserror res;

	اگر (pkey == تہی) {
		واپس NSERROR_BAD_PARAMETER;
	}

	انتخاب (EVP_PKEY_بنیاد_id(pkey)) {
	حالت EVP_PKEY_RSA:
		res = rsa_مقصد_info(pkey, ikey);
		توڑو;

	حالت EVP_PKEY_DSA:
		res = dsa_مقصد_info(pkey, ikey);
		توڑو;

	حالت EVP_PKEY_DH:
		res = dh_مقصد_info(pkey, ikey);
		توڑو;

	حالت EVP_PKEY_EC:
		res = ec_مقصد_info(pkey, ikey);
		توڑو;

	طےشدہ:
		res = NSERROR_NOT_IMPLEMENTED;
		توڑو;
	}

	EVP_PKEY_free(pkey);

	واپس res;
}

ساکن nserror san_مقصد_info(X509 *cert, ساخت ns_cert_san **پچھلا_اگلا)
{
	صحیح idx;
	صحیح san_نام_nb = -1;
	ثابت GENERAL_NAME *current_name;
	ثابت غیرعلامتی حرف *dns_name;
	ساخت ns_cert_san *isan;

	ڈھیر_کا(GENERAL_NAME) *san_نام = تہی;

	san_نام = X509_get_ext_d2i(cert, NID_subject_alt_name, تہی, تہی);
	اگر (san_نام == تہی) {
		واپس NSERROR_OK;
	}

	san_نام_nb = sk_GENERAL_NAME_عدد(san_نام);

	/* Check each name within the extension */
	برائے (idx = 0; idx < san_نام_nb; idx++) {
		current_name = sk_GENERAL_NAME_قدر(san_نام, idx);

		اگر (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_ڈیٹا(current_name->d.dNSName);

			isan = حافظہ_دو(حجم(ساخت ns_cert_san));
			اگر (isan != تہی) {
				isan->name = strdup((ثابت حرف *)dns_name);
				isan->اگلا = تہی;
				*پچھلا_اگلا = isan;
				پچھلا_اگلا = &isan->اگلا;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	برائے (idx = 0; idx < san_نام_nb; idx++) {
		GENERAL_NAME *اندراج_2 = sk_GENERAL_NAME_نکالو(san_نام);
		GENERAL_NAME_free(اندراج_2);
	}
	sk_GENERAL_NAME_free(san_نام);

	واپس NSERROR_OK;
}

ساکن nserror
der_مقصد_certinfo(ثابت uint8_t *der,
		size_t der_لمبائی,
		ساخت ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ثابت ASN1_INTEGER *asn1_عدد;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	اگر (der == تہی) {
		واپس NSERROR_OK;
	}

	cert = d2i_X509(تہی, &der, der_لمبائی);
	اگر (cert == تہی) {
		واپس NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->نسخہ = X509_get_نسخہ(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_وقت_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(خالی) BIO_سیٹ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = صفر_حافظہ_دو(1, buf->لمبائی + 1);
	اگر (info->not_before != تہی) {
		حافظہ_نقل(info->not_before, buf->ڈیٹا, (غیرعلامتی)buf->لمبائی);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_وقت_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(خالی) BIO_سیٹ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = صفر_حافظہ_دو(1, buf->لمبائی + 1);
	اگر (info->not_after != تہی) {
		حافظہ_نقل(info->not_after, buf->ڈیٹا, (غیرعلامتی)buf->لمبائی);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	صحیح pkey_nid = ns_X509_get_signature_nid(cert);
	اگر (pkey_nid != NID_undef) {
		ثابت حرف* sslbuf = OBJ_nid2ln(pkey_nid);
		اگر (sslbuf != تہی) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_عدد = X509_get_serial_نمبر(cert);
	اگر (asn1_عدد != تہی) {
		bignum = ASN1_INTEGER_مقصد_BN(asn1_عدد, تہی);
		اگر (bignum != تہی) {
			حرف *tmp = BN_bn2hex(bignum);
			اگر (tmp != تہی) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = تہی;
		}
	}

	/* fingerprints */
	ثابت EVP_MD *digest;
	غیرعلامتی صحیح dig_len;
	غیرعلامتی حرف *buff;
	صحیح rc;

	digest = EVP_sha1();
	buff = حافظہ_دو(EVP_MD_سائز(digest));
	اگر (buff != تہی) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		اگر ((rc == 1) && (dig_len == (غیرعلامتی صحیح)EVP_MD_سائز(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		آزاد(buff);
	}

	digest = EVP_sha256();
	buff = حافظہ_دو(EVP_MD_سائز(digest));
	اگر (buff != تہی) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		اگر ((rc == 1) && (dig_len == (غیرعلامتی صحیح)EVP_MD_سائز(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		آزاد(buff);
	}

	/* subject alternative names */
	san_مقصد_info(cert, &info->san);

	/* issuer name */
	xname_مقصد_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_مقصد_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_مقصد_info(X509_get_pubkey(cert), &info->عوامی_کلید);

	X509_free(cert);

	واپس NSERROR_OK;
}

/* copy certificate data */
ساکن nserror
convert_chain_مقصد_cert_info(ثابت ساخت cert_chain *chain,
			   ساخت ns_cert_info **cert_info_برآمد)
{
	ساخت ns_cert_info *certs;
	size_t گہرائی;
	nserror res;

	certs = صفر_حافظہ_دو(chain->گہرائی, حجم(ساخت ns_cert_info));
	اگر (certs == تہی) {
		واپس NSERROR_NOMEM;
	}

	برائے (گہرائی = 0; گہرائی < chain->گہرائی;گہرائی++) {
		res = der_مقصد_certinfo(chain->certs[گہرائی].der,
				      chain->certs[گہرائی].der_لمبائی,
				      certs + گہرائی);
		اگر (res != NSERROR_OK) {
			آزاد(certs);
			واپس res;
		}
		certs[گہرائی].err = chain->certs[گہرائی].err;
	}

	*cert_info_برآمد = certs;
	واپس NSERROR_OK;
}

#ورنہ
ساکن nserror
convert_chain_مقصد_cert_info(ثابت ساخت cert_chain *chain,
			   ساخت ns_cert_info **cert_info_برآمد)
{
	واپس NSERROR_NOT_IMPLEMENTED;
}
#اگر_ختم


ساکن nserror
format_certificate_name(ساخت fetch_about_context *ctx,
			ساخت ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	اگر (cert_name->organisation != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (cert_name->organisation_unit != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (cert_name->locality != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (cert_name->province != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (cert_name->country != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	واپس res;
}

/**
 * output formatted certificate subject alternate names
 */
ساکن nserror
format_certificate_san(ساخت fetch_about_context *ctx,
			      ساخت ns_cert_san *san)
{
	nserror res;

	اگر (san == تہی) {
		واپس NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	جب (san != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		اگر (res != NSERROR_OK) {
			واپس res;
		}

		san = san->اگلا;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	واپس res;

}


ساکن nserror
format_certificate_عوامی_کلید(ساخت fetch_about_context *ctx,
			      ساخت ns_cert_pkey *عوامی_کلید)
{
	nserror res;

	اگر (عوامی_کلید->algor == تہی) {
		/* skip the table if no algorithm name */
		واپس NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 عوامی_کلید->algor,
			 عوامی_کلید->سائز);
	اگر (res != NSERROR_OK) {
		واپس res;
	}


	اگر (عوامی_کلید->exponent != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 عوامی_کلید->exponent);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (عوامی_کلید->modulus != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 عوامی_کلید->modulus);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (عوامی_کلید->خم != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 عوامی_کلید->خم);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (عوامی_کلید->عوامی != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 عوامی_کلید->عوامی);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	واپس res;
}

ساکن nserror
format_certificate_fingerprint(ساخت fetch_about_context *ctx,
			      ساخت ns_cert_info *cert_info)
{
	nserror res;

	اگر ((cert_info->sha1fingerprint == تہی) &&
	    (cert_info->sha256fingerprint == تہی))  {
		/* skip the table if no fingerprints */
		واپس NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	اگر (cert_info->sha256fingerprint != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (cert_info->sha1fingerprint != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	واپس res;
}

ساکن nserror
format_certificate(ساخت fetch_about_context *ctx,
		   ساخت ns_cert_info *cert_info,
		   size_t گہرائی)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 گہرائی, cert_info->subject_name.common_name);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	اگر (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = format_certificate_عوامی_کلید(ctx, &cert_info->عوامی_کلید);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	اگر (cert_info->serialnum != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	اگر (cert_info->sig_algor != تہی) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->نسخہ);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	واپس res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(ساخت fetch_about_context *ctx)
{
	صحیح رمز = 200;
	nserror res;
	ساخت cert_chain *chain = تہی;

	/* content is going to return ok */
	fetch_about_سیٹ_http_رمز(ctx, رمز);

	/* content type */
	اگر (fetch_about_send_سرخی(ctx, "Content-Type: text/html"))
		جاؤ fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	اگر (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		اگر (res != NSERROR_OK) {
			جاؤ fetch_about_certificate_handler_aborted;
		}
	} ورنہ {
		ساخت ns_cert_info *cert_info;
		res = convert_chain_مقصد_cert_info(chain, &cert_info);
		اگر (res == NSERROR_OK) {
			size_t گہرائی;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			اگر (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				جاؤ fetch_about_certificate_handler_aborted;
			}

			برائے (گہرائی = 0; گہرائی < chain->گہرائی; گہرائی++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						گہرائی, (cert_info + گہرائی)
							->subject_name
								.common_name);
				اگر (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					جاؤ fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			اگر (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				جاؤ fetch_about_certificate_handler_aborted;
			}

			برائے (گہرائی = 0; گہرائی < chain->گہرائی; گہرائی++) {
				res = format_certificate(ctx, cert_info + گہرائی,
						گہرائی);
				اگر (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					جاؤ fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} ورنہ {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			اگر (res != NSERROR_OK) {
				جاؤ fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_ختم_شدہ(ctx);

	cert_chain_free(chain);

	واپس true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	واپس false;
}
