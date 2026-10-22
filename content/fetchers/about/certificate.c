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
	صحيح ناپ;
	حرف *modulus;
	حرف *exponent;
	حرف *ونگ;
	حرف *عام;
};

/**
 * Certificate subject alternative name
 */
بنية ns_cert_san {
	بنية ns_cert_san *اگلا;
	حرف *name;
};

/**
 * certificate information for certificate chain
 */
بنية ns_cert_info {
	بنية ns_cert_name subject_name; /**< Subject details */
	بنية ns_cert_name issuer_name; /**< Issuer details */
	بنية ns_cert_pkey عوامی_کلید; /**< public key details */
	لما نسخہ;		/**< Certificate version */
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
	حرر(cinfo->عوامی_کلید.algor);
	حرر(cinfo->عوامی_کلید.modulus);
	حرر(cinfo->عوامی_کلید.exponent);
	حرر(cinfo->عوامی_کلید.ونگ);
	حرر(cinfo->عوامی_کلید.عام);
	حرر(cinfo->not_before);
	حرر(cinfo->not_after);
	حرر(cinfo->sig_algor);
	حرر(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	طالما (san != عدم) {
		بنية ns_cert_san *اگلا;
		اگلا = san->اگلا;
		حرر(san);
		san = اگلا;
	}

	حرر(cinfo);

	أرجع NSERROR_OK;
}

#جے_تعریف_شدہ WITH_OPENSSL

#ضمن <openssl/ssl.h>
#ضمن <openssl/x509v3.h>

#إذا (OPENSSL_نسخہ_نمبر < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# إذا defined(LIBRESSL_نسخہ_نمبر)
  /* LibreSSL */
#  إذا (LIBRESSL_نسخہ_نمبر < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   إذا (LIBRESSL_نسخہ_نمبر < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ساكن صحيح ns_X509_get_signature_nid(X509 *cert)
{
	أرجع OBJ_obj2nid(cert->cert_info->کلید->algor->algorithm);
}

ساكن ثابت بنانشانی حرف *ns_ASN1_STRING_get0_انکڑے(ASN1_STRING *asn1str)
{
	أرجع (ثابت بنانشانی حرف *)ASN1_STRING_انکڑے(asn1str);
}
#   وإلا
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_انکڑے ASN1_STRING_get0_انکڑے
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
#   define ns_ASN1_STRING_get0_انکڑے ASN1_STRING_get0_انکڑے
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# وإلا
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_انکڑے ASN1_STRING_get0_انکڑے
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ساكن صحيح ns_EVP_PKEY_get_bn_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *نتیجہ = عدم;

	/* Check parameters: only support allocation-form *bn */
	إذا (pkey == عدم || کلید_name == عدم || bn == عدم || *bn != عدم)
		أرجع 0;

	/* Only support RSA keys */
	إذا (EVP_PKEY_ادھار_id(pkey) != EVP_PKEY_RSA)
		أرجع 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	إذا (rsa == عدم)
		أرجع 0;

	إذا (strcmp(کلید_name, "n") == 0) {
		ثابت BIGNUM *n = ns_RSA_get0_n(rsa);
		إذا (n != عدم)
			نتیجہ = BN_dup(n);
	} وإلا إذا (strcmp(کلید_name, "e") == 0) {
		ثابت BIGNUM *e = ns_RSA_get0_e(rsa);
		إذا (e != عدم)
			نتیجہ = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = نتیجہ;

	أرجع (نتیجہ != عدم) ? 1 : 0;
}

ساكن صحيح ns_EVP_PKEY_get_utf8_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, حرف *str, size_t max_len,
		size_t *باہرلا_len)
{
	ثابت EC_گروہ *ecgroup;
	ثابت حرف *گروہ = "";
	size_t گروہ_len = 0;
	EC_کلید *ec;
	صحيح ret = 0;

	إذا (pkey == عدم || کلید_name == عدم)
		أرجع 0;

	/* Only support EC keys */
	إذا (EVP_PKEY_ادھار_id(pkey) != EVP_PKEY_EC)
		أرجع 0;

	/* Only support fetching the group */
	إذا (strcmp(کلید_name, "group") != 0)
		أرجع 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);

	ecgroup = EC_کلید_get0_گروہ(ec);
	إذا (ecgroup != عدم) {
		گروہ = OBJ_nid2ln(EC_گروہ_get_ونگ_name(ecgroup));
		گروہ_len = لڑی_لمبائی(گروہ);
	}

	إذا (str != عدم && max_len > گروہ_len) {
		حافظہ_نقل(str, گروہ, گروہ_len);
		str[گروہ_len] = '\0';
		ret = 1;
	}

	إذا (باہرلا_len != عدم) {
		*باہرلا_len = گروہ_len;
	}

	EC_کلید_free(ec);

	أرجع ret;
}

ساكن صحيح ns_EVP_PKEY_get_octet_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *کلید_name, بنانشانی حرف *buf, size_t max_len,
		size_t *باہرلا_len)
{
	ثابت EC_گروہ *ecgroup;
	ثابت EC_نقطہ *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_کلید *ec;
	صحيح ret = 0;

	إذا (pkey == عدم || کلید_name == عدم)
		أرجع 0;

	/* Only support EC keys */
	إذا (EVP_PKEY_ادھار_id(pkey) != EVP_PKEY_EC)
		أرجع 0;

	إذا (strcmp(کلید_name, "encoded-pub-key") != 0)
		أرجع 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);
	إذا (ec == عدم)
		أرجع 0;

	ecgroup = EC_کلید_get0_گروہ(ec);
	إذا (ecgroup != عدم) {
		ecpoint = EC_کلید_get0_عوامی_کلید(ec);
		إذا (ecpoint != عدم) {
			bnctx = BN_CTX_new();
			len = EC_نقطہ_point2oct(ecgroup,
						 ecpoint,
						 نقطہ_CONVERSION_UNCOMPRESSED,
						 عدم,
						 0,
						 bnctx);
			إذا (len != 0 && len <= max_len) {
				إذا (EC_نقطہ_point2oct(ecgroup,
						       ecpoint,
						       نقطہ_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			إذا (باہرلا_len != عدم)
				*باہرلا_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_کلید_free(ec);

	أرجع ret;
}
#وإلا
/* OpenSSL 3.x and later */
#تعریف_کرو ns_X509_get_signature_nid X509_get_signature_nid
#تعریف_کرو ns_ASN1_STRING_get0_انکڑے ASN1_STRING_get0_انکڑے
#تعریف_کرو ns_RSA_get0_n RSA_get0_n
#تعریف_کرو ns_RSA_get0_e RSA_get0_e
#تعریف_کرو ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#تعریف_کرو ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#تعریف_کرو ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#جے_انت

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
ساكن nserror
xname_منزل_info(X509_NAME *xname, بنية ns_cert_name *iname)
{
	صحيح entryidx;
	صحيح entrycnt;
	X509_NAME_اندراج *اندراج; /* current name entry */
	ASN1_STRING *قدر;
	ثابت بنانشانی حرف *قدر_str;
	ASN1_OBJECT *name;
	صحيح name_nid;
	حرف **میدان;

	entrycnt = X509_NAME_اندراج_گنتی(xname);

	لكل (entryidx = 0; entryidx < entrycnt; entryidx++) {
		اندراج = X509_NAME_get_اندراج(xname, entryidx);
		name = X509_NAME_اندراج_get_object(اندراج);
		name_nid = OBJ_obj2nid(name);
		قدر = X509_NAME_اندراج_get_انکڑے(اندراج);
		قدر_str = ns_ASN1_STRING_get0_انکڑے(قدر);
		بدل (name_nid) {
		حالة NID_commonName:
			میدان = &iname->common_name;
			اكسر;
		حالة NID_countryName:
			میدان = &iname->country;
			اكسر;
		حالة NID_localityName:
			میدان = &iname->locality;
			اكسر;
		حالة NID_stateOrProvinceName:
			میدان = &iname->province;
			اكسر;
		حالة NID_organizationName:
			میدان = &iname->organisation;
			اكسر;
		حالة NID_organizationalUnitName:
			میدان = &iname->organisation_unit;
			اكسر;
		افتراضي :
			میدان = عدم;
			اكسر;
		}
		إذا (میدان != عدم) {
			*میدان = strdup((ثابت حرف *)قدر_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *میدان);
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
	حرف *باہرلا;
	صحيح cn = 0;

	hexlen = لڑی_لمبائی(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = حافظہ_دیو(((hexlen * 7) + 6) / 2);

	إذا (dst != عدم) {
		لكل (باہرلا = dst; *hex != 0; hex++) {
			إذا (cn == 2) {
				cn = 0;
				*باہرلا++ = '&';
				*باہرلا++ = '#';
				*باہرلا++ = '5';
				*باہرلا++ = '8';
				*باہرلا++ = ';';
			}
			*باہرلا++ = *hex;
			cn++;
		}
		*باہرلا = 0;
	}
	أرجع dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ساكن حرف *bindup(بنانشانی حرف *bin, بنانشانی صحيح binlen)
{
	حرف *dst;
	حرف *باہرلا;
	بنانشانی صحيح idx;
	ثابت حرف hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = حافظہ_دیو(binlen * 7);

	إذا (dst != عدم) {
		باہرلا = dst;
		لكل (idx = 0; idx < binlen; idx++) {
			*باہرلا++ = hex[(bin[idx] & 0xf0) >> 4];
			*باہرلا++ = hex[bin[idx] & 0xf];

			*باہرلا++ = '&';
			*باہرلا++ = '#';
			*باہرلا++ = '5';
			*باہرلا++ = '8';
			*باہرلا++ = ';';
		}
		باہرلا -= 5;
		*باہرلا = 0;
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
rsa_منزل_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
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

	ikey->ناپ = EVP_PKEY_بٹ(pkey);

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
dsa_منزل_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->ناپ = EVP_PKEY_بٹ(pkey);

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
dh_منزل_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->ناپ = EVP_PKEY_بٹ(pkey);

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
ec_منزل_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->ناپ = EVP_PKEY_بٹ(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", عدم, 0, &len);
	إذا (len != 0) {
		ikey->ونگ = حافظہ_دیو(len + 1);
		إذا (ikey->ونگ != عدم) {
			إذا (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->ونگ, len + 1, عدم) == 0) {
				حرر(ikey->ونگ);
				ikey->ونگ = عدم;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			عدم, 0, &len);
	إذا (len != 0) {
		بنانشانی حرف *نقطہ = حافظہ_دیو(len);
		إذا (نقطہ != عدم) {
			إذا (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", نقطہ, len,
					عدم) == 1) {
				ikey->عام = bindup(نقطہ, len);
			}
			حرر(نقطہ);
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
pkey_منزل_info(EVP_PKEY *pkey, بنية ns_cert_pkey *ikey)
{
	nserror res;

	إذا (pkey == عدم) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	بدل (EVP_PKEY_ادھار_id(pkey)) {
	حالة EVP_PKEY_RSA:
		res = rsa_منزل_info(pkey, ikey);
		اكسر;

	حالة EVP_PKEY_DSA:
		res = dsa_منزل_info(pkey, ikey);
		اكسر;

	حالة EVP_PKEY_DH:
		res = dh_منزل_info(pkey, ikey);
		اكسر;

	حالة EVP_PKEY_EC:
		res = ec_منزل_info(pkey, ikey);
		اكسر;

	افتراضي:
		res = NSERROR_NOT_IMPLEMENTED;
		اكسر;
	}

	EVP_PKEY_free(pkey);

	أرجع res;
}

ساكن nserror san_منزل_info(X509 *cert, بنية ns_cert_san **prev_اگلا)
{
	صحيح idx;
	صحيح san_ناں_nb = -1;
	ثابت GENERAL_NAME *current_name;
	ثابت بنانشانی حرف *dns_name;
	بنية ns_cert_san *isan;

	ڈھیر_دا(GENERAL_NAME) *san_ناں = عدم;

	san_ناں = X509_get_ext_d2i(cert, NID_subject_alt_name, عدم, عدم);
	إذا (san_ناں == عدم) {
		أرجع NSERROR_OK;
	}

	san_ناں_nb = sk_GENERAL_NAME_عدد(san_ناں);

	/* Check each name within the extension */
	لكل (idx = 0; idx < san_ناں_nb; idx++) {
		current_name = sk_GENERAL_NAME_قدر(san_ناں, idx);

		إذا (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_انکڑے(current_name->d.dNSName);

			isan = حافظہ_دیو(حجم(بنية ns_cert_san));
			إذا (isan != عدم) {
				isan->name = strdup((ثابت حرف *)dns_name);
				isan->اگلا = عدم;
				*prev_اگلا = isan;
				prev_اگلا = &isan->اگلا;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	لكل (idx = 0; idx < san_ناں_nb; idx++) {
		GENERAL_NAME *اندراج = sk_GENERAL_NAME_نکالو(san_ناں);
		GENERAL_NAME_free(اندراج);
	}
	sk_GENERAL_NAME_free(san_ناں);

	أرجع NSERROR_OK;
}

ساكن nserror
der_منزل_certinfo(ثابت uint8_t *der,
		size_t der_لمائی,
		بنية ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ثابت ASN1_INTEGER *asn1_عدد;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	إذا (der == عدم) {
		أرجع NSERROR_OK;
	}

	cert = d2i_X509(عدم, &der, der_لمائی);
	إذا (cert == عدم) {
		أرجع NSERROR_INVALID;
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
	ASN1_ویلا_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(فراغ) BIO_سیٹ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = صفر_حافظہ_دیو(1, buf->لمائی + 1);
	إذا (info->not_before != عدم) {
		حافظہ_نقل(info->not_before, buf->انکڑے, (بنانشانی)buf->لمائی);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_ویلا_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(فراغ) BIO_سیٹ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = صفر_حافظہ_دیو(1, buf->لمائی + 1);
	إذا (info->not_after != عدم) {
		حافظہ_نقل(info->not_after, buf->انکڑے, (بنانشانی)buf->لمائی);
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
	asn1_عدد = X509_get_serial_نمبر(cert);
	إذا (asn1_عدد != عدم) {
		bignum = ASN1_INTEGER_منزل_BN(asn1_عدد, عدم);
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
	بنانشانی صحيح dig_len;
	بنانشانی حرف *buff;
	صحيح rc;

	digest = EVP_sha1();
	buff = حافظہ_دیو(EVP_MD_ناپ(digest));
	إذا (buff != عدم) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		إذا ((rc == 1) && (dig_len == (بنانشانی صحيح)EVP_MD_ناپ(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		حرر(buff);
	}

	digest = EVP_sha256();
	buff = حافظہ_دیو(EVP_MD_ناپ(digest));
	إذا (buff != عدم) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		إذا ((rc == 1) && (dig_len == (بنانشانی صحيح)EVP_MD_ناپ(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		حرر(buff);
	}

	/* subject alternative names */
	san_منزل_info(cert, &info->san);

	/* issuer name */
	xname_منزل_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_منزل_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_منزل_info(X509_get_pubkey(cert), &info->عوامی_کلید);

	X509_free(cert);

	أرجع NSERROR_OK;
}

/* copy certificate data */
ساكن nserror
convert_chain_منزل_cert_info(ثابت بنية cert_chain *chain,
			   بنية ns_cert_info **cert_info_باہرلا)
{
	بنية ns_cert_info *certs;
	size_t ڈونگھائی;
	nserror res;

	certs = صفر_حافظہ_دیو(chain->ڈونگھائی, حجم(بنية ns_cert_info));
	إذا (certs == عدم) {
		أرجع NSERROR_NOMEM;
	}

	لكل (ڈونگھائی = 0; ڈونگھائی < chain->ڈونگھائی;ڈونگھائی++) {
		res = der_منزل_certinfo(chain->certs[ڈونگھائی].der,
				      chain->certs[ڈونگھائی].der_لمائی,
				      certs + ڈونگھائی);
		إذا (res != NSERROR_OK) {
			حرر(certs);
			أرجع res;
		}
		certs[ڈونگھائی].err = chain->certs[ڈونگھائی].err;
	}

	*cert_info_باہرلا = certs;
	أرجع NSERROR_OK;
}

#وإلا
ساكن nserror
convert_chain_منزل_cert_info(ثابت بنية cert_chain *chain,
			   بنية ns_cert_info **cert_info_باہرلا)
{
	أرجع NSERROR_NOT_IMPLEMENTED;
}
#جے_انت


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

		san = san->اگلا;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	أرجع res;

}


ساكن nserror
format_certificate_عوامی_کلید(بنية fetch_about_context *ctx,
			      بنية ns_cert_pkey *عوامی_کلید)
{
	nserror res;

	إذا (عوامی_کلید->algor == عدم) {
		/* skip the table if no algorithm name */
		أرجع NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 عوامی_کلید->algor,
			 عوامی_کلید->ناپ);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}


	إذا (عوامی_کلید->exponent != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 عوامی_کلید->exponent);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عوامی_کلید->modulus != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 عوامی_کلید->modulus);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عوامی_کلید->ونگ != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 عوامی_کلید->ونگ);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عوامی_کلید->عام != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 عوامی_کلید->عام);
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
		   size_t ڈونگھائی)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 ڈونگھائی, cert_info->subject_name.common_name);
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

	res = format_certificate_عوامی_کلید(ctx, &cert_info->عوامی_کلید);
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
			 cert_info->نسخہ);
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
	صحيح کوڈ = 200;
	nserror res;
	بنية cert_chain *chain = عدم;

	/* content is going to return ok */
	fetch_about_سیٹ_http_کوڈ(ctx, کوڈ);

	/* content type */
	إذا (fetch_about_send_سرخی(ctx, "Content-Type: text/html"))
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
		res = convert_chain_منزل_cert_info(chain, &cert_info);
		إذا (res == NSERROR_OK) {
			size_t ڈونگھائی;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			إذا (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				اذهب fetch_about_certificate_handler_aborted;
			}

			لكل (ڈونگھائی = 0; ڈونگھائی < chain->ڈونگھائی; ڈونگھائی++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						ڈونگھائی, (cert_info + ڈونگھائی)
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

			لكل (ڈونگھائی = 0; ڈونگھائی < chain->ڈونگھائی; ڈونگھائی++) {
				res = format_certificate(ctx, cert_info + ڈونگھائی,
						ڈونگھائی);
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

	fetch_about_send_مکمل(ctx);

	cert_chain_free(chain);

	أرجع true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	أرجع false;
}
