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
	صحيح سائيز;
	حرف *modulus;
	حرف *exponent;
	حرف *وڪر;
	حرف *عام;
};

/**
 * Certificate subject alternative name
 */
بنية ns_cert_san {
	بنية ns_cert_san *ايندڙ;
	حرف *name;
};

/**
 * certificate information for certificate chain
 */
بنية ns_cert_info {
	بنية ns_cert_name subject_name; /**< Subject details */
	بنية ns_cert_name issuer_name; /**< Issuer details */
	بنية ns_cert_pkey عوامي_ڪنجي; /**< public key details */
	ڊگهو نسخو;		/**< Certificate version */
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
	حرر(cinfo->عوامي_ڪنجي.algor);
	حرر(cinfo->عوامي_ڪنجي.modulus);
	حرر(cinfo->عوامي_ڪنجي.exponent);
	حرر(cinfo->عوامي_ڪنجي.وڪر);
	حرر(cinfo->عوامي_ڪنجي.عام);
	حرر(cinfo->not_before);
	حرر(cinfo->not_after);
	حرر(cinfo->sig_algor);
	حرر(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	طالما (san != عدم) {
		بنية ns_cert_san *ايندڙ;
		ايندڙ = san->ايندڙ;
		حرر(san);
		san = ايندڙ;
	}

	حرر(cinfo);

	أرجع NSERROR_OK;
}

#جيڪڏهن_وصف_ٿيل WITH_OPENSSL

#ضمن <openssl/ssl.h>
#ضمن <openssl/x509v3.h>

#إذا (OPENSSL_نسخو_عدد < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# إذا defined(LIBRESSL_نسخو_عدد)
  /* LibreSSL */
#  إذا (LIBRESSL_نسخو_عدد < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   إذا (LIBRESSL_نسخو_عدد < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ساكن صحيح ns_X509_get_signature_nid(X509 *cert)
{
	أرجع OBJ_obj2nid(cert->cert_info->ڪنجي->algor->algorithm);
}

ساكن ثابت بنانشان حرف *ns_ASN1_STRING_get0_معلومات(ASN1_STRING *asn1str)
{
	أرجع (ثابت بنانشان حرف *)ASN1_STRING_معلومات(asn1str);
}
#   وإلا
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_معلومات ASN1_STRING_get0_معلومات
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
#   define ns_ASN1_STRING_get0_معلومات ASN1_STRING_get0_معلومات
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# وإلا
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_معلومات ASN1_STRING_get0_معلومات
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ساكن صحيح ns_EVP_PKEY_get_bn_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *ڪنجي_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *نتيجو = عدم;

	/* Check parameters: only support allocation-form *bn */
	إذا (pkey == عدم || ڪنجي_name == عدم || bn == عدم || *bn != عدم)
		أرجع 0;

	/* Only support RSA keys */
	إذا (EVP_PKEY_بنياد_id(pkey) != EVP_PKEY_RSA)
		أرجع 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	إذا (rsa == عدم)
		أرجع 0;

	إذا (strcmp(ڪنجي_name, "n") == 0) {
		ثابت BIGNUM *n = ns_RSA_get0_n(rsa);
		إذا (n != عدم)
			نتيجو = BN_dup(n);
	} وإلا إذا (strcmp(ڪنجي_name, "e") == 0) {
		ثابت BIGNUM *e = ns_RSA_get0_e(rsa);
		إذا (e != عدم)
			نتيجو = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = نتيجو;

	أرجع (نتيجو != عدم) ? 1 : 0;
}

ساكن صحيح ns_EVP_PKEY_get_utf8_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *ڪنجي_name, حرف *str, size_t max_len,
		size_t *ٻاهريون_len)
{
	ثابت EC_گروهه *ecgroup;
	ثابت حرف *گروهه = "";
	size_t گروهه_len = 0;
	EC_ڪنجي *ec;
	صحيح ret = 0;

	إذا (pkey == عدم || ڪنجي_name == عدم)
		أرجع 0;

	/* Only support EC keys */
	إذا (EVP_PKEY_بنياد_id(pkey) != EVP_PKEY_EC)
		أرجع 0;

	/* Only support fetching the group */
	إذا (strcmp(ڪنجي_name, "group") != 0)
		أرجع 0;

	ec = EVP_PKEY_get1_EC_ڪنجي((EVP_PKEY *) pkey);

	ecgroup = EC_ڪنجي_get0_گروهه(ec);
	إذا (ecgroup != عدم) {
		گروهه = OBJ_nid2ln(EC_گروهه_get_وڪر_name(ecgroup));
		گروهه_len = لڙي_ڊيگهه(گروهه);
	}

	إذا (str != عدم && max_len > گروهه_len) {
		يادگيري_نقل(str, گروهه, گروهه_len);
		str[گروهه_len] = '\0';
		ret = 1;
	}

	إذا (ٻاهريون_len != عدم) {
		*ٻاهريون_len = گروهه_len;
	}

	EC_ڪنجي_free(ec);

	أرجع ret;
}

ساكن صحيح ns_EVP_PKEY_get_octet_string_param(ثابت EVP_PKEY *pkey,
		ثابت حرف *ڪنجي_name, بنانشان حرف *buf, size_t max_len,
		size_t *ٻاهريون_len)
{
	ثابت EC_گروهه *ecgroup;
	ثابت EC_نقطو *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_ڪنجي *ec;
	صحيح ret = 0;

	إذا (pkey == عدم || ڪنجي_name == عدم)
		أرجع 0;

	/* Only support EC keys */
	إذا (EVP_PKEY_بنياد_id(pkey) != EVP_PKEY_EC)
		أرجع 0;

	إذا (strcmp(ڪنجي_name, "encoded-pub-key") != 0)
		أرجع 0;

	ec = EVP_PKEY_get1_EC_ڪنجي((EVP_PKEY *) pkey);
	إذا (ec == عدم)
		أرجع 0;

	ecgroup = EC_ڪنجي_get0_گروهه(ec);
	إذا (ecgroup != عدم) {
		ecpoint = EC_ڪنجي_get0_عوامي_ڪنجي(ec);
		إذا (ecpoint != عدم) {
			bnctx = BN_CTX_new();
			len = EC_نقطو_point2oct(ecgroup,
						 ecpoint,
						 نقطو_CONVERSION_UNCOMPRESSED,
						 عدم,
						 0,
						 bnctx);
			إذا (len != 0 && len <= max_len) {
				إذا (EC_نقطو_point2oct(ecgroup,
						       ecpoint,
						       نقطو_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			إذا (ٻاهريون_len != عدم)
				*ٻاهريون_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_ڪنجي_free(ec);

	أرجع ret;
}
#وإلا
/* OpenSSL 3.x and later */
#وصف_ڪريو ns_X509_get_signature_nid X509_get_signature_nid
#وصف_ڪريو ns_ASN1_STRING_get0_معلومات ASN1_STRING_get0_معلومات
#وصف_ڪريو ns_RSA_get0_n RSA_get0_n
#وصف_ڪريو ns_RSA_get0_e RSA_get0_e
#وصف_ڪريو ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#وصف_ڪريو ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#وصف_ڪريو ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#جيڪڏهن_پورو

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
	X509_NAME_داخلا *داخلا; /* current name entry */
	ASN1_STRING *قدر;
	ثابت بنانشان حرف *قدر_str;
	ASN1_OBJECT *name;
	صحيح name_nid;
	حرف **ميدان;

	entrycnt = X509_NAME_داخلا_ڳڻپ(xname);

	لكل (entryidx = 0; entryidx < entrycnt; entryidx++) {
		داخلا = X509_NAME_get_داخلا(xname, entryidx);
		name = X509_NAME_داخلا_get_object(داخلا);
		name_nid = OBJ_obj2nid(name);
		قدر = X509_NAME_داخلا_get_معلومات(داخلا);
		قدر_str = ns_ASN1_STRING_get0_معلومات(قدر);
		بدل (name_nid) {
		حالة NID_commonName:
			ميدان = &iname->common_name;
			اكسر;
		حالة NID_countryName:
			ميدان = &iname->country;
			اكسر;
		حالة NID_localityName:
			ميدان = &iname->locality;
			اكسر;
		حالة NID_stateOrProvinceName:
			ميدان = &iname->province;
			اكسر;
		حالة NID_organizationName:
			ميدان = &iname->organisation;
			اكسر;
		حالة NID_organizationalUnitName:
			ميدان = &iname->organisation_unit;
			اكسر;
		افتراضي :
			ميدان = عدم;
			اكسر;
		}
		إذا (ميدان != عدم) {
			*ميدان = strdup((ثابت حرف *)قدر_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *ميدان);
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
	حرف *ٻاهريون;
	صحيح cn = 0;

	hexlen = لڙي_ڊيگهه(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = يادگيري_ڏيو(((hexlen * 7) + 6) / 2);

	إذا (dst != عدم) {
		لكل (ٻاهريون = dst; *hex != 0; hex++) {
			إذا (cn == 2) {
				cn = 0;
				*ٻاهريون++ = '&';
				*ٻاهريون++ = '#';
				*ٻاهريون++ = '5';
				*ٻاهريون++ = '8';
				*ٻاهريون++ = ';';
			}
			*ٻاهريون++ = *hex;
			cn++;
		}
		*ٻاهريون = 0;
	}
	أرجع dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ساكن حرف *bindup(بنانشان حرف *bin, بنانشان صحيح binlen)
{
	حرف *dst;
	حرف *ٻاهريون;
	بنانشان صحيح idx;
	ثابت حرف hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = يادگيري_ڏيو(binlen * 7);

	إذا (dst != عدم) {
		ٻاهريون = dst;
		لكل (idx = 0; idx < binlen; idx++) {
			*ٻاهريون++ = hex[(bin[idx] & 0xf0) >> 4];
			*ٻاهريون++ = hex[bin[idx] & 0xf];

			*ٻاهريون++ = '&';
			*ٻاهريون++ = '#';
			*ٻاهريون++ = '5';
			*ٻاهريون++ = '8';
			*ٻاهريون++ = ';';
		}
		ٻاهريون -= 5;
		*ٻاهريون = 0;
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

	ikey->سائيز = EVP_PKEY_بٽ(pkey);

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

	ikey->سائيز = EVP_PKEY_بٽ(pkey);

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

	ikey->سائيز = EVP_PKEY_بٽ(pkey);

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

	ikey->سائيز = EVP_PKEY_بٽ(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", عدم, 0, &len);
	إذا (len != 0) {
		ikey->وڪر = يادگيري_ڏيو(len + 1);
		إذا (ikey->وڪر != عدم) {
			إذا (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->وڪر, len + 1, عدم) == 0) {
				حرر(ikey->وڪر);
				ikey->وڪر = عدم;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			عدم, 0, &len);
	إذا (len != 0) {
		بنانشان حرف *نقطو = يادگيري_ڏيو(len);
		إذا (نقطو != عدم) {
			إذا (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", نقطو, len,
					عدم) == 1) {
				ikey->عام = bindup(نقطو, len);
			}
			حرر(نقطو);
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

	بدل (EVP_PKEY_بنياد_id(pkey)) {
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

ساكن nserror san_منزل_info(X509 *cert, بنية ns_cert_san **prev_ايندڙ)
{
	صحيح idx;
	صحيح san_نالا_nb = -1;
	ثابت GENERAL_NAME *current_name;
	ثابت بنانشان حرف *dns_name;
	بنية ns_cert_san *isan;

	ڍير_جو(GENERAL_NAME) *san_نالا = عدم;

	san_نالا = X509_get_ext_d2i(cert, NID_subject_alt_name, عدم, عدم);
	إذا (san_نالا == عدم) {
		أرجع NSERROR_OK;
	}

	san_نالا_nb = sk_GENERAL_NAME_انگ(san_نالا);

	/* Check each name within the extension */
	لكل (idx = 0; idx < san_نالا_nb; idx++) {
		current_name = sk_GENERAL_NAME_قدر(san_نالا, idx);

		إذا (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_معلومات(current_name->d.dNSName);

			isan = يادگيري_ڏيو(حجم(بنية ns_cert_san));
			إذا (isan != عدم) {
				isan->name = strdup((ثابت حرف *)dns_name);
				isan->ايندڙ = عدم;
				*prev_ايندڙ = isan;
				prev_ايندڙ = &isan->ايندڙ;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	لكل (idx = 0; idx < san_نالا_nb; idx++) {
		GENERAL_NAME *داخلا = sk_GENERAL_NAME_ڪڍو(san_نالا);
		GENERAL_NAME_free(داخلا);
	}
	sk_GENERAL_NAME_free(san_نالا);

	أرجع NSERROR_OK;
}

ساكن nserror
der_منزل_certinfo(ثابت uint8_t *der,
		size_t der_ڊيگهائي,
		بنية ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ثابت ASN1_INTEGER *asn1_انگ;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	إذا (der == عدم) {
		أرجع NSERROR_OK;
	}

	cert = d2i_X509(عدم, &der, der_ڊيگهائي);
	إذا (cert == عدم) {
		أرجع NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->نسخو = X509_get_نسخو(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_وقت_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(فراغ) BIO_سيٽ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = صفر_يادگيري_ڏيو(1, buf->ڊيگهائي + 1);
	إذا (info->not_before != عدم) {
		يادگيري_نقل(info->not_before, buf->معلومات, (بنانشان)buf->ڊيگهائي);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_وقت_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(فراغ) BIO_سيٽ_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = صفر_يادگيري_ڏيو(1, buf->ڊيگهائي + 1);
	إذا (info->not_after != عدم) {
		يادگيري_نقل(info->not_after, buf->معلومات, (بنانشان)buf->ڊيگهائي);
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
	asn1_انگ = X509_get_serial_عدد(cert);
	إذا (asn1_انگ != عدم) {
		bignum = ASN1_INTEGER_منزل_BN(asn1_انگ, عدم);
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
	بنانشان صحيح dig_len;
	بنانشان حرف *buff;
	صحيح rc;

	digest = EVP_sha1();
	buff = يادگيري_ڏيو(EVP_MD_سائيز(digest));
	إذا (buff != عدم) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		إذا ((rc == 1) && (dig_len == (بنانشان صحيح)EVP_MD_سائيز(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		حرر(buff);
	}

	digest = EVP_sha256();
	buff = يادگيري_ڏيو(EVP_MD_سائيز(digest));
	إذا (buff != عدم) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		إذا ((rc == 1) && (dig_len == (بنانشان صحيح)EVP_MD_سائيز(digest))) {
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
	pkey_منزل_info(X509_get_pubkey(cert), &info->عوامي_ڪنجي);

	X509_free(cert);

	أرجع NSERROR_OK;
}

/* copy certificate data */
ساكن nserror
convert_chain_منزل_cert_info(ثابت بنية cert_chain *chain,
			   بنية ns_cert_info **cert_info_ٻاهريون)
{
	بنية ns_cert_info *certs;
	size_t اونهائي;
	nserror res;

	certs = صفر_يادگيري_ڏيو(chain->اونهائي, حجم(بنية ns_cert_info));
	إذا (certs == عدم) {
		أرجع NSERROR_NOMEM;
	}

	لكل (اونهائي = 0; اونهائي < chain->اونهائي;اونهائي++) {
		res = der_منزل_certinfo(chain->certs[اونهائي].der,
				      chain->certs[اونهائي].der_ڊيگهائي,
				      certs + اونهائي);
		إذا (res != NSERROR_OK) {
			حرر(certs);
			أرجع res;
		}
		certs[اونهائي].err = chain->certs[اونهائي].err;
	}

	*cert_info_ٻاهريون = certs;
	أرجع NSERROR_OK;
}

#وإلا
ساكن nserror
convert_chain_منزل_cert_info(ثابت بنية cert_chain *chain,
			   بنية ns_cert_info **cert_info_ٻاهريون)
{
	أرجع NSERROR_NOT_IMPLEMENTED;
}
#جيڪڏهن_پورو


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

		san = san->ايندڙ;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	أرجع res;

}


ساكن nserror
format_certificate_عوامي_ڪنجي(بنية fetch_about_context *ctx,
			      بنية ns_cert_pkey *عوامي_ڪنجي)
{
	nserror res;

	إذا (عوامي_ڪنجي->algor == عدم) {
		/* skip the table if no algorithm name */
		أرجع NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 عوامي_ڪنجي->algor,
			 عوامي_ڪنجي->سائيز);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}


	إذا (عوامي_ڪنجي->exponent != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 عوامي_ڪنجي->exponent);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عوامي_ڪنجي->modulus != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 عوامي_ڪنجي->modulus);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عوامي_ڪنجي->وڪر != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 عوامي_ڪنجي->وڪر);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	إذا (عوامي_ڪنجي->عام != عدم) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 عوامي_ڪنجي->عام);
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
		   size_t اونهائي)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 اونهائي, cert_info->subject_name.common_name);
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

	res = format_certificate_عوامي_ڪنجي(ctx, &cert_info->عوامي_ڪنجي);
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
			 cert_info->نسخو);
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
	صحيح ڪوڊ = 200;
	nserror res;
	بنية cert_chain *chain = عدم;

	/* content is going to return ok */
	fetch_about_سيٽ_http_ڪوڊ(ctx, ڪوڊ);

	/* content type */
	إذا (fetch_about_send_سرو(ctx, "Content-Type: text/html"))
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
			size_t اونهائي;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			إذا (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				اذهب fetch_about_certificate_handler_aborted;
			}

			لكل (اونهائي = 0; اونهائي < chain->اونهائي; اونهائي++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						اونهائي, (cert_info + اونهائي)
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

			لكل (اونهائي = 0; اونهائي < chain->اونهائي; اونهائي++) {
				res = format_certificate(ctx, cert_info + اونهائي,
						اونهائي);
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

	fetch_about_send_ختم(ctx);

	cert_chain_free(chain);

	أرجع true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	أرجع false;
}
