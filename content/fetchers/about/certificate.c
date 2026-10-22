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
ساختار ns_cert_name {
	نویسه *common_name;
	نویسه *organisation;
	نویسه *organisation_unit;
	نویسه *locality;
	نویسه *province;
	نویسه *country;
};

/**
 * Certificate public key parameters
 */
ساختار ns_cert_pkey {
	نویسه *algor;
	صحیح size;
	نویسه *modulus;
	نویسه *exponent;
	نویسه *curve;
	نویسه *عمومی;
};

/**
 * Certificate subject alternative name
 */
ساختار ns_cert_san {
	ساختار ns_cert_san *next;
	نویسه *name;
};

/**
 * certificate information for certificate chain
 */
ساختار ns_cert_info {
	ساختار ns_cert_name subject_name; /**< Subject details */
	ساختار ns_cert_name issuer_name; /**< Issuer details */
	ساختار ns_cert_pkey public_کلید; /**< public key details */
	long version;		/**< Certificate version */
	نویسه *not_before;	/**< Valid from date */
	نویسه *not_after;	/**< Valid to date */
	صحیح sig_type;		/**< Signature type */
	نویسه *sig_algor;        /**< Signature Algorithm */
	نویسه *serialnum;	/**< Serial number */
	نویسه *sha1fingerprint; /**< fingerprint shar1 encoded */
	نویسه *sha256fingerprint; /**< fingerprint shar256 encoded */
	ساختار ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
ایستا nserror free_ns_cert_info(ساختار ns_cert_info *cinfo)
{
	ساختار ns_cert_san *san;

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
	آزاد(cinfo->public_کلید.algor);
	آزاد(cinfo->public_کلید.modulus);
	آزاد(cinfo->public_کلید.exponent);
	آزاد(cinfo->public_کلید.curve);
	آزاد(cinfo->public_کلید.عمومی);
	آزاد(cinfo->not_before);
	آزاد(cinfo->not_after);
	آزاد(cinfo->sig_algor);
	آزاد(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	وقتی (san != پوچ) {
		ساختار ns_cert_san *next;
		next = san->next;
		آزاد(san);
		san = next;
	}

	آزاد(cinfo);

	بازگشت NSERROR_OK;
}

#ifdef WITH_OPENSSL

#شامل <openssl/ssl.h>
#شامل <openssl/x509v3.h>

#اگر (OPENSSL_VERSION_عدد < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# اگر defined(LIBRESSL_VERSION_عدد)
  /* LibreSSL */
#  اگر (LIBRESSL_VERSION_عدد < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   اگر (LIBRESSL_VERSION_عدد < 0x2070000fL)
    /* LibreSSL <2.7.0 */
ایستا صحیح ns_X509_get_signature_nid(X509 *cert)
{
	بازگشت OBJ_obj2nid(cert->cert_info->کلید->algor->algorithm);
}

ایستا ثابت unsigned نویسه *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	بازگشت (ثابت unsigned نویسه *)ASN1_STRING_data(asn1str);
}
#   وگرنه
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

ایستا ثابت BIGNUM *ns_RSA_get0_n(ثابت RSA *d)
{
	بازگشت d->n;
}

ایستا ثابت BIGNUM *ns_RSA_get0_e(ثابت RSA *d)
{
	بازگشت d->e;
}
#  وگرنه
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# وگرنه
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

ایستا صحیح ns_EVP_PKEY_get_bn_param(ثابت EVP_PKEY *pkey,
		ثابت نویسه *کلید_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = پوچ;

	/* Check parameters: only support allocation-form *bn */
	اگر (pkey == پوچ || کلید_name == پوچ || bn == پوچ || *bn != پوچ)
		بازگشت 0;

	/* Only support RSA keys */
	اگر (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		بازگشت 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	اگر (rsa == پوچ)
		بازگشت 0;

	اگر (strcmp(کلید_name, "n") == 0) {
		ثابت BIGNUM *n = ns_RSA_get0_n(rsa);
		اگر (n != پوچ)
			result = BN_dup(n);
	} وگرنه اگر (strcmp(کلید_name, "e") == 0) {
		ثابت BIGNUM *e = ns_RSA_get0_e(rsa);
		اگر (e != پوچ)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	بازگشت (result != پوچ) ? 1 : 0;
}

ایستا صحیح ns_EVP_PKEY_get_utf8_string_param(ثابت EVP_PKEY *pkey,
		ثابت نویسه *کلید_name, نویسه *str, size_t max_len,
		size_t *out_len)
{
	ثابت EC_GROUP *ecgroup;
	ثابت نویسه *group = "";
	size_t group_len = 0;
	EC_کلید *ec;
	صحیح ret = 0;

	اگر (pkey == پوچ || کلید_name == پوچ)
		بازگشت 0;

	/* Only support EC keys */
	اگر (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		بازگشت 0;

	/* Only support fetching the group */
	اگر (strcmp(کلید_name, "group") != 0)
		بازگشت 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);

	ecgroup = EC_کلید_get0_group(ec);
	اگر (ecgroup != پوچ) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	اگر (str != پوچ && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	اگر (out_len != پوچ) {
		*out_len = group_len;
	}

	EC_کلید_free(ec);

	بازگشت ret;
}

ایستا صحیح ns_EVP_PKEY_get_octet_string_param(ثابت EVP_PKEY *pkey,
		ثابت نویسه *کلید_name, unsigned نویسه *buf, size_t max_len,
		size_t *out_len)
{
	ثابت EC_GROUP *ecgroup;
	ثابت EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_کلید *ec;
	صحیح ret = 0;

	اگر (pkey == پوچ || کلید_name == پوچ)
		بازگشت 0;

	/* Only support EC keys */
	اگر (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		بازگشت 0;

	اگر (strcmp(کلید_name, "encoded-pub-key") != 0)
		بازگشت 0;

	ec = EVP_PKEY_get1_EC_کلید((EVP_PKEY *) pkey);
	اگر (ec == پوچ)
		بازگشت 0;

	ecgroup = EC_کلید_get0_group(ec);
	اگر (ecgroup != پوچ) {
		ecpoint = EC_کلید_get0_public_کلید(ec);
		اگر (ecpoint != پوچ) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 پوچ,
						 0,
						 bnctx);
			اگر (len != 0 && len <= max_len) {
				اگر (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			اگر (out_len != پوچ)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_کلید_free(ec);

	بازگشت ret;
}
#وگرنه
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
ایستا nserror
xname_to_info(X509_NAME *xname, ساختار ns_cert_name *iname)
{
	صحیح entryidx;
	صحیح entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *مقدار;
	ثابت unsigned نویسه *مقدار_str;
	ASN1_OBJECT *name;
	صحیح name_nid;
	نویسه **field;

	entrycnt = X509_NAME_entry_شمار(xname);

	برای (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		مقدار = X509_NAME_ENTRY_get_data(entry);
		مقدار_str = ns_ASN1_STRING_get0_data(مقدار);
		انتخاب (name_nid) {
		حالت NID_commonName:
			field = &iname->common_name;
			بشکن;
		حالت NID_countryName:
			field = &iname->country;
			بشکن;
		حالت NID_localityName:
			field = &iname->locality;
			بشکن;
		حالت NID_stateOrProvinceName:
			field = &iname->province;
			بشکن;
		حالت NID_organizationName:
			field = &iname->organisation;
			بشکن;
		حالت NID_organizationalUnitName:
			field = &iname->organisation_unit;
			بشکن;
		پیشفرض :
			field = پوچ;
			بشکن;
		}
		اگر (field != پوچ) {
			*field = strdup((ثابت نویسه *)مقدار_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} وگرنه {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	اگر (iname->common_name == پوچ) {
		iname->common_name = strdup("Unknown");
	}

	بازگشت NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ایستا نویسه *hexdup(ثابت نویسه *hex)
{
	صحیح hexlen;
	نویسه *dst;
	نویسه *out;
	صحیح cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	اگر (dst != پوچ) {
		برای (out = dst; *hex != 0; hex++) {
			اگر (cn == 2) {
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
	بازگشت dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
ایستا نویسه *bindup(unsigned نویسه *bin, unsigned صحیح binlen)
{
	نویسه *dst;
	نویسه *out;
	unsigned صحیح idx;
	ثابت نویسه hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	اگر (dst != پوچ) {
		out = dst;
		برای (idx = 0; idx < binlen; idx++) {
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
	بازگشت dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ایستا nserror
rsa_to_info(EVP_PKEY *pkey, ساختار ns_cert_pkey *ikey)
{
	BIGNUM *n = پوچ, *e = پوچ;
	نویسه *tmp;

	اگر (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		بازگشت NSERROR_BAD_PARAMETER;
	}

	اگر (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		بازگشت NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	اگر (tmp != پوچ) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	اگر (tmp != پوچ) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	بازگشت NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ایستا nserror
dsa_to_info(EVP_PKEY *pkey, ساختار ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	بازگشت NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ایستا nserror
dh_to_info(EVP_PKEY *pkey, ساختار ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	بازگشت NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ایستا nserror
ec_to_info(EVP_PKEY *pkey, ساختار ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", پوچ, 0, &len);
	اگر (len != 0) {
		ikey->curve = malloc(len + 1);
		اگر (ikey->curve != پوچ) {
			اگر (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, پوچ) == 0) {
				آزاد(ikey->curve);
				ikey->curve = پوچ;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			پوچ, 0, &len);
	اگر (len != 0) {
		unsigned نویسه *point = malloc(len);
		اگر (point != پوچ) {
			اگر (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					پوچ) == 1) {
				ikey->عمومی = bindup(point, len);
			}
			آزاد(point);
		}
	}

	بازگشت NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
ایستا nserror
pkey_to_info(EVP_PKEY *pkey, ساختار ns_cert_pkey *ikey)
{
	nserror res;

	اگر (pkey == پوچ) {
		بازگشت NSERROR_BAD_PARAMETER;
	}

	انتخاب (EVP_PKEY_base_id(pkey)) {
	حالت EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		بشکن;

	حالت EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		بشکن;

	حالت EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		بشکن;

	حالت EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		بشکن;

	پیشفرض:
		res = NSERROR_NOT_IMPLEMENTED;
		بشکن;
	}

	EVP_PKEY_free(pkey);

	بازگشت res;
}

ایستا nserror san_to_info(X509 *cert, ساختار ns_cert_san **prev_next)
{
	صحیح idx;
	صحیح san_names_nb = -1;
	ثابت GENERAL_NAME *current_name;
	ثابت unsigned نویسه *dns_name;
	ساختار ns_cert_san *isan;

	پشته_OF(GENERAL_NAME) *san_names = پوچ;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, پوچ, پوچ);
	اگر (san_names == پوچ) {
		بازگشت NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_رقم(san_names);

	/* Check each name within the extension */
	برای (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_مقدار(san_names, idx);

		اگر (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(اندازه(ساختار ns_cert_san));
			اگر (isan != پوچ) {
				isan->name = strdup((ثابت نویسه *)dns_name);
				isan->next = پوچ;
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
	برای (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_برداشت(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	بازگشت NSERROR_OK;
}

ایستا nserror
der_to_certinfo(ثابت uint8_t *der,
		size_t der_length,
		ساختار ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	ثابت ASN1_INTEGER *asn1_رقم;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	اگر (der == پوچ) {
		بازگشت NSERROR_OK;
	}

	cert = d2i_X509(پوچ, &der, der_length);
	اگر (cert == پوچ) {
		بازگشت NSERROR_INVALID;
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
	(تهی) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	اگر (info->not_before != پوچ) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(تهی) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	اگر (info->not_after != پوچ) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	صحیح pkey_nid = ns_X509_get_signature_nid(cert);
	اگر (pkey_nid != NID_undef) {
		ثابت نویسه* sslbuf = OBJ_nid2ln(pkey_nid);
		اگر (sslbuf != پوچ) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_رقم = X509_get_serial_عدد(cert);
	اگر (asn1_رقم != پوچ) {
		bignum = ASN1_INTEGER_to_BN(asn1_رقم, پوچ);
		اگر (bignum != پوچ) {
			نویسه *tmp = BN_bn2hex(bignum);
			اگر (tmp != پوچ) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = پوچ;
		}
	}

	/* fingerprints */
	ثابت EVP_MD *digest;
	unsigned صحیح dig_len;
	unsigned نویسه *buff;
	صحیح rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	اگر (buff != پوچ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		اگر ((rc == 1) && (dig_len == (unsigned صحیح)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		آزاد(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	اگر (buff != پوچ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		اگر ((rc == 1) && (dig_len == (unsigned صحیح)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		آزاد(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_کلید);

	X509_free(cert);

	بازگشت NSERROR_OK;
}

/* copy certificate data */
ایستا nserror
convert_chain_to_cert_info(ثابت ساختار cert_chain *chain,
			   ساختار ns_cert_info **cert_info_out)
{
	ساختار ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, اندازه(ساختار ns_cert_info));
	اگر (certs == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	برای (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		اگر (res != NSERROR_OK) {
			آزاد(certs);
			بازگشت res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	بازگشت NSERROR_OK;
}

#وگرنه
ایستا nserror
convert_chain_to_cert_info(ثابت ساختار cert_chain *chain,
			   ساختار ns_cert_info **cert_info_out)
{
	بازگشت NSERROR_NOT_IMPLEMENTED;
}
#endif


ایستا nserror
format_certificate_name(ساختار fetch_about_context *ctx,
			ساختار ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	اگر (cert_name->organisation != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (cert_name->organisation_unit != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (cert_name->locality != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (cert_name->province != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (cert_name->country != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	بازگشت res;
}

/**
 * output formatted certificate subject alternate names
 */
ایستا nserror
format_certificate_san(ساختار fetch_about_context *ctx,
			      ساختار ns_cert_san *san)
{
	nserror res;

	اگر (san == پوچ) {
		بازگشت NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	وقتی (san != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	بازگشت res;

}


ایستا nserror
format_certificate_public_کلید(ساختار fetch_about_context *ctx,
			      ساختار ns_cert_pkey *public_کلید)
{
	nserror res;

	اگر (public_کلید->algor == پوچ) {
		/* skip the table if no algorithm name */
		بازگشت NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_کلید->algor,
			 public_کلید->size);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}


	اگر (public_کلید->exponent != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_کلید->exponent);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (public_کلید->modulus != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_کلید->modulus);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (public_کلید->curve != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_کلید->curve);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (public_کلید->عمومی != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_کلید->عمومی);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	بازگشت res;
}

ایستا nserror
format_certificate_fingerprint(ساختار fetch_about_context *ctx,
			      ساختار ns_cert_info *cert_info)
{
	nserror res;

	اگر ((cert_info->sha1fingerprint == پوچ) &&
	    (cert_info->sha256fingerprint == پوچ))  {
		/* skip the table if no fingerprints */
		بازگشت NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	اگر (cert_info->sha256fingerprint != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (cert_info->sha1fingerprint != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	بازگشت res;
}

ایستا nserror
format_certificate(ساختار fetch_about_context *ctx,
		   ساختار ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	اگر (res != NSERROR_OK) {
		بازگشت res;
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
			بازگشت res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	اگر (res != NSERROR_OK) {
		بازگشت res;
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
		بازگشت res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = format_certificate_public_کلید(ctx, &cert_info->public_کلید);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	اگر (cert_info->serialnum != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	اگر (cert_info->sig_algor != پوچ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	بازگشت res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(ساختار fetch_about_context *ctx)
{
	صحیح code = 200;
	nserror res;
	ساختار cert_chain *chain = پوچ;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	اگر (fetch_about_send_header(ctx, "Content-Type: text/html"))
		برو fetch_about_certificate_handler_aborted;

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
		برو fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	اگر (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		اگر (res != NSERROR_OK) {
			برو fetch_about_certificate_handler_aborted;
		}
	} وگرنه {
		ساختار ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		اگر (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			اگر (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				برو fetch_about_certificate_handler_aborted;
			}

			برای (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				اگر (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					برو fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			اگر (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				برو fetch_about_certificate_handler_aborted;
			}

			برای (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				اگر (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					برو fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} وگرنه {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			اگر (res != NSERROR_OK) {
				برو fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	اگر (res != NSERROR_OK) {
		برو fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	بازگشت true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	بازگشت false;
}
