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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "netsurf/ssl_certs.h"

#அந்தர்பாவ "private.h"
#அந்தர்பாவ "certificate.h"

/**
 * certificate name parameters
 */
கட்டமைப்பு ns_cert_name {
	எழுத்து *common_name;
	எழுத்து *organisation;
	எழுத்து *organisation_unit;
	எழுத்து *locality;
	எழுத்து *province;
	எழுத்து *country;
};

/**
 * Certificate public key parameters
 */
கட்டமைப்பு ns_cert_pkey {
	எழுத்து *algor;
	முழு size;
	எழுத்து *modulus;
	எழுத்து *exponent;
	எழுத்து *curve;
	எழுத்து *பொது;
};

/**
 * Certificate subject alternative name
 */
கட்டமைப்பு ns_cert_san {
	கட்டமைப்பு ns_cert_san *next;
	எழுத்து *name;
};

/**
 * certificate information for certificate chain
 */
கட்டமைப்பு ns_cert_info {
	கட்டமைப்பு ns_cert_name subject_name; /**< Subject details */
	கட்டமைப்பு ns_cert_name issuer_name; /**< Issuer details */
	கட்டமைப்பு ns_cert_pkey public_சாவி; /**< public key details */
	long version;		/**< Certificate version */
	எழுத்து *not_before;	/**< Valid from date */
	எழுத்து *not_after;	/**< Valid to date */
	முழு sig_type;		/**< Signature type */
	எழுத்து *sig_algor;        /**< Signature Algorithm */
	எழுத்து *serialnum;	/**< Serial number */
	எழுத்து *sha1fingerprint; /**< fingerprint shar1 encoded */
	எழுத்து *sha256fingerprint; /**< fingerprint shar256 encoded */
	கட்டமைப்பு ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
நிலையான nserror free_ns_cert_info(கட்டமைப்பு ns_cert_info *cinfo)
{
	கட்டமைப்பு ns_cert_san *san;

	விடுவி(cinfo->subject_name.common_name);
	விடுவி(cinfo->subject_name.organisation);
	விடுவி(cinfo->subject_name.organisation_unit);
	விடுவி(cinfo->subject_name.locality);
	விடுவி(cinfo->subject_name.province);
	விடுவி(cinfo->subject_name.country);
	விடுவி(cinfo->issuer_name.common_name);
	விடுவி(cinfo->issuer_name.organisation);
	விடுவி(cinfo->issuer_name.organisation_unit);
	விடுவி(cinfo->issuer_name.locality);
	விடுவி(cinfo->issuer_name.province);
	விடுவி(cinfo->issuer_name.country);
	விடுவி(cinfo->public_சாவி.algor);
	விடுவி(cinfo->public_சாவி.modulus);
	விடுவி(cinfo->public_சாவி.exponent);
	விடுவி(cinfo->public_சாவி.curve);
	விடுவி(cinfo->public_சாவி.பொது);
	விடுவி(cinfo->not_before);
	விடுவி(cinfo->not_after);
	விடுவி(cinfo->sig_algor);
	விடுவி(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	போது (san != வெற்று) {
		கட்டமைப்பு ns_cert_san *next;
		next = san->next;
		விடுவி(san);
		san = next;
	}

	விடுவி(cinfo);

	திரும்பு NSERROR_OK;
}

#ifdef WITH_OPENSSL

#அந்தர்பாவ <openssl/ssl.h>
#அந்தர்பாவ <openssl/x509v3.h>

#எனில் (OPENSSL_VERSION_எண் < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# எனில் defined(LIBRESSL_VERSION_எண்)
  /* LibreSSL */
#  எனில் (LIBRESSL_VERSION_எண் < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   எனில் (LIBRESSL_VERSION_எண் < 0x2070000fL)
    /* LibreSSL <2.7.0 */
நிலையான முழு ns_X509_get_signature_nid(X509 *cert)
{
	திரும்பு OBJ_obj2nid(cert->cert_info->சாவி->algor->algorithm);
}

நிலையான மாறிலி unsigned எழுத்து *ns_ASN1_STRING_get0_data(ASN1_STRING *asn1str)
{
	திரும்பு (மாறிலி unsigned எழுத்து *)ASN1_STRING_data(asn1str);
}
#   இல்லையெனில்
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   endif

நிலையான மாறிலி BIGNUM *ns_RSA_get0_n(மாறிலி RSA *d)
{
	திரும்பு d->n;
}

நிலையான மாறிலி BIGNUM *ns_RSA_get0_e(மாறிலி RSA *d)
{
	திரும்பு d->e;
}
#  இல்லையெனில்
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# இல்லையெனில்
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_data ASN1_STRING_get0_data
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

நிலையான முழு ns_EVP_PKEY_get_bn_param(மாறிலி EVP_PKEY *pkey,
		மாறிலி எழுத்து *சாவி_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *result = வெற்று;

	/* Check parameters: only support allocation-form *bn */
	எனில் (pkey == வெற்று || சாவி_name == வெற்று || bn == வெற்று || *bn != வெற்று)
		திரும்பு 0;

	/* Only support RSA keys */
	எனில் (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA)
		திரும்பு 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	எனில் (rsa == வெற்று)
		திரும்பு 0;

	எனில் (strcmp(சாவி_name, "n") == 0) {
		மாறிலி BIGNUM *n = ns_RSA_get0_n(rsa);
		எனில் (n != வெற்று)
			result = BN_dup(n);
	} இல்லையெனில் எனில் (strcmp(சாவி_name, "e") == 0) {
		மாறிலி BIGNUM *e = ns_RSA_get0_e(rsa);
		எனில் (e != வெற்று)
			result = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = result;

	திரும்பு (result != வெற்று) ? 1 : 0;
}

நிலையான முழு ns_EVP_PKEY_get_utf8_string_param(மாறிலி EVP_PKEY *pkey,
		மாறிலி எழுத்து *சாவி_name, எழுத்து *str, size_t max_len,
		size_t *out_len)
{
	மாறிலி EC_GROUP *ecgroup;
	மாறிலி எழுத்து *group = "";
	size_t group_len = 0;
	EC_சாவி *ec;
	முழு ret = 0;

	எனில் (pkey == வெற்று || சாவி_name == வெற்று)
		திரும்பு 0;

	/* Only support EC keys */
	எனில் (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		திரும்பு 0;

	/* Only support fetching the group */
	எனில் (strcmp(சாவி_name, "group") != 0)
		திரும்பு 0;

	ec = EVP_PKEY_get1_EC_சாவி((EVP_PKEY *) pkey);

	ecgroup = EC_சாவி_get0_group(ec);
	எனில் (ecgroup != வெற்று) {
		group = OBJ_nid2ln(EC_GROUP_get_curve_name(ecgroup));
		group_len = strlen(group);
	}

	எனில் (str != வெற்று && max_len > group_len) {
		memcpy(str, group, group_len);
		str[group_len] = '\0';
		ret = 1;
	}

	எனில் (out_len != வெற்று) {
		*out_len = group_len;
	}

	EC_சாவி_free(ec);

	திரும்பு ret;
}

நிலையான முழு ns_EVP_PKEY_get_octet_string_param(மாறிலி EVP_PKEY *pkey,
		மாறிலி எழுத்து *சாவி_name, unsigned எழுத்து *buf, size_t max_len,
		size_t *out_len)
{
	மாறிலி EC_GROUP *ecgroup;
	மாறிலி EC_POINT *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_சாவி *ec;
	முழு ret = 0;

	எனில் (pkey == வெற்று || சாவி_name == வெற்று)
		திரும்பு 0;

	/* Only support EC keys */
	எனில் (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC)
		திரும்பு 0;

	எனில் (strcmp(சாவி_name, "encoded-pub-key") != 0)
		திரும்பு 0;

	ec = EVP_PKEY_get1_EC_சாவி((EVP_PKEY *) pkey);
	எனில் (ec == வெற்று)
		திரும்பு 0;

	ecgroup = EC_சாவி_get0_group(ec);
	எனில் (ecgroup != வெற்று) {
		ecpoint = EC_சாவி_get0_public_சாவி(ec);
		எனில் (ecpoint != வெற்று) {
			bnctx = BN_CTX_new();
			len = EC_POINT_point2oct(ecgroup,
						 ecpoint,
						 POINT_CONVERSION_UNCOMPRESSED,
						 வெற்று,
						 0,
						 bnctx);
			எனில் (len != 0 && len <= max_len) {
				எனில் (EC_POINT_point2oct(ecgroup,
						       ecpoint,
						       POINT_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			எனில் (out_len != வெற்று)
				*out_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_சாவி_free(ec);

	திரும்பு ret;
}
#இல்லையெனில்
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
நிலையான nserror
xname_to_info(X509_NAME *xname, கட்டமைப்பு ns_cert_name *iname)
{
	முழு entryidx;
	முழு entrycnt;
	X509_NAME_ENTRY *entry; /* current name entry */
	ASN1_STRING *மதிப்பு;
	மாறிலி unsigned எழுத்து *மதிப்பு_str;
	ASN1_OBJECT *name;
	முழு name_nid;
	எழுத்து **field;

	entrycnt = X509_NAME_entry_மொத்தம்(xname);

	க்கு (entryidx = 0; entryidx < entrycnt; entryidx++) {
		entry = X509_NAME_get_entry(xname, entryidx);
		name = X509_NAME_ENTRY_get_object(entry);
		name_nid = OBJ_obj2nid(name);
		மதிப்பு = X509_NAME_ENTRY_get_data(entry);
		மதிப்பு_str = ns_ASN1_STRING_get0_data(மதிப்பு);
		தேர்வு (name_nid) {
		நிலை NID_commonName:
			field = &iname->common_name;
			நிறுத்து;
		நிலை NID_countryName:
			field = &iname->country;
			நிறுத்து;
		நிலை NID_localityName:
			field = &iname->locality;
			நிறுத்து;
		நிலை NID_stateOrProvinceName:
			field = &iname->province;
			நிறுத்து;
		நிலை NID_organizationName:
			field = &iname->organisation;
			நிறுத்து;
		நிலை NID_organizationalUnitName:
			field = &iname->organisation_unit;
			நிறுத்து;
		இயல்பு :
			field = வெற்று;
			நிறுத்து;
		}
		எனில் (field != வெற்று) {
			*field = strdup((மாறிலி எழுத்து *)மதிப்பு_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *field);
		} இல்லையெனில் {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	எனில் (iname->common_name == வெற்று) {
		iname->common_name = strdup("Unknown");
	}

	திரும்பு NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
நிலையான எழுத்து *hexdup(மாறிலி எழுத்து *hex)
{
	முழு hexlen;
	எழுத்து *dst;
	எழுத்து *out;
	முழு cn = 0;

	hexlen = strlen(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = malloc(((hexlen * 7) + 6) / 2);

	எனில் (dst != வெற்று) {
		க்கு (out = dst; *hex != 0; hex++) {
			எனில் (cn == 2) {
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
	திரும்பு dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
நிலையான எழுத்து *bindup(unsigned எழுத்து *bin, unsigned முழு binlen)
{
	எழுத்து *dst;
	எழுத்து *out;
	unsigned முழு idx;
	மாறிலி எழுத்து hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = malloc(binlen * 7);

	எனில் (dst != வெற்று) {
		out = dst;
		க்கு (idx = 0; idx < binlen; idx++) {
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
	திரும்பு dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
நிலையான nserror
rsa_to_info(EVP_PKEY *pkey, கட்டமைப்பு ns_cert_pkey *ikey)
{
	BIGNUM *n = வெற்று, *e = வெற்று;
	எழுத்து *tmp;

	எனில் (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	எனில் (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->size = EVP_PKEY_bits(pkey);

	tmp = BN_bn2hex(n);
	எனில் (tmp != வெற்று) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	எனில் (tmp != வெற்று) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	திரும்பு NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
நிலையான nserror
dsa_to_info(EVP_PKEY *pkey, கட்டமைப்பு ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->size = EVP_PKEY_bits(pkey);

	திரும்பு NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
நிலையான nserror
dh_to_info(EVP_PKEY *pkey, கட்டமைப்பு ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->size = EVP_PKEY_bits(pkey);

	திரும்பு NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
நிலையான nserror
ec_to_info(EVP_PKEY *pkey, கட்டமைப்பு ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->size = EVP_PKEY_bits(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", வெற்று, 0, &len);
	எனில் (len != 0) {
		ikey->curve = malloc(len + 1);
		எனில் (ikey->curve != வெற்று) {
			எனில் (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->curve, len + 1, வெற்று) == 0) {
				விடுவி(ikey->curve);
				ikey->curve = வெற்று;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			வெற்று, 0, &len);
	எனில் (len != 0) {
		unsigned எழுத்து *point = malloc(len);
		எனில் (point != வெற்று) {
			எனில் (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", point, len,
					வெற்று) == 1) {
				ikey->பொது = bindup(point, len);
			}
			விடுவி(point);
		}
	}

	திரும்பு NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
நிலையான nserror
pkey_to_info(EVP_PKEY *pkey, கட்டமைப்பு ns_cert_pkey *ikey)
{
	nserror res;

	எனில் (pkey == வெற்று) {
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	தேர்வு (EVP_PKEY_base_id(pkey)) {
	நிலை EVP_PKEY_RSA:
		res = rsa_to_info(pkey, ikey);
		நிறுத்து;

	நிலை EVP_PKEY_DSA:
		res = dsa_to_info(pkey, ikey);
		நிறுத்து;

	நிலை EVP_PKEY_DH:
		res = dh_to_info(pkey, ikey);
		நிறுத்து;

	நிலை EVP_PKEY_EC:
		res = ec_to_info(pkey, ikey);
		நிறுத்து;

	இயல்பு:
		res = NSERROR_NOT_IMPLEMENTED;
		நிறுத்து;
	}

	EVP_PKEY_free(pkey);

	திரும்பு res;
}

நிலையான nserror san_to_info(X509 *cert, கட்டமைப்பு ns_cert_san **prev_next)
{
	முழு idx;
	முழு san_names_nb = -1;
	மாறிலி GENERAL_NAME *current_name;
	மாறிலி unsigned எழுத்து *dns_name;
	கட்டமைப்பு ns_cert_san *isan;

	அடுக்கு_OF(GENERAL_NAME) *san_names = வெற்று;

	san_names = X509_get_ext_d2i(cert, NID_subject_alt_name, வெற்று, வெற்று);
	எனில் (san_names == வெற்று) {
		திரும்பு NSERROR_OK;
	}

	san_names_nb = sk_GENERAL_NAME_இலக்கம்(san_names);

	/* Check each name within the extension */
	க்கு (idx = 0; idx < san_names_nb; idx++) {
		current_name = sk_GENERAL_NAME_மதிப்பு(san_names, idx);

		எனில் (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_data(current_name->d.dNSName);

			isan = malloc(அளவு(கட்டமைப்பு ns_cert_san));
			எனில் (isan != வெற்று) {
				isan->name = strdup((மாறிலி எழுத்து *)dns_name);
				isan->next = வெற்று;
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
	க்கு (idx = 0; idx < san_names_nb; idx++) {
		GENERAL_NAME *entry = sk_GENERAL_NAME_எடு(san_names);
		GENERAL_NAME_free(entry);
	}
	sk_GENERAL_NAME_free(san_names);

	திரும்பு NSERROR_OK;
}

நிலையான nserror
der_to_certinfo(மாறிலி uint8_t *der,
		size_t der_length,
		கட்டமைப்பு ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	மாறிலி ASN1_INTEGER *asn1_இலக்கம்;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	எனில் (der == வெற்று) {
		திரும்பு NSERROR_OK;
	}

	cert = d2i_X509(வெற்று, &der, der_length);
	எனில் (cert == வெற்று) {
		திரும்பு NSERROR_INVALID;
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
	(வெறுமை) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = calloc(1, buf->length + 1);
	எனில் (info->not_before != வெற்று) {
		memcpy(info->not_before, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_TIME_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(வெறுமை) BIO_set_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = calloc(1, buf->length + 1);
	எனில் (info->not_after != வெற்று) {
		memcpy(info->not_after, buf->data, (unsigned)buf->length);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	முழு pkey_nid = ns_X509_get_signature_nid(cert);
	எனில் (pkey_nid != NID_undef) {
		மாறிலி எழுத்து* sslbuf = OBJ_nid2ln(pkey_nid);
		எனில் (sslbuf != வெற்று) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_இலக்கம் = X509_get_serial_எண்(cert);
	எனில் (asn1_இலக்கம் != வெற்று) {
		bignum = ASN1_INTEGER_to_BN(asn1_இலக்கம், வெற்று);
		எனில் (bignum != வெற்று) {
			எழுத்து *tmp = BN_bn2hex(bignum);
			எனில் (tmp != வெற்று) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = வெற்று;
		}
	}

	/* fingerprints */
	மாறிலி EVP_MD *digest;
	unsigned முழு dig_len;
	unsigned எழுத்து *buff;
	முழு rc;

	digest = EVP_sha1();
	buff = malloc(EVP_MD_size(digest));
	எனில் (buff != வெற்று) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		எனில் ((rc == 1) && (dig_len == (unsigned முழு)EVP_MD_size(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		விடுவி(buff);
	}

	digest = EVP_sha256();
	buff = malloc(EVP_MD_size(digest));
	எனில் (buff != வெற்று) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		எனில் ((rc == 1) && (dig_len == (unsigned முழு)EVP_MD_size(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		விடுவி(buff);
	}

	/* subject alternative names */
	san_to_info(cert, &info->san);

	/* issuer name */
	xname_to_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_to_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_to_info(X509_get_pubkey(cert), &info->public_சாவி);

	X509_free(cert);

	திரும்பு NSERROR_OK;
}

/* copy certificate data */
நிலையான nserror
convert_chain_to_cert_info(மாறிலி கட்டமைப்பு cert_chain *chain,
			   கட்டமைப்பு ns_cert_info **cert_info_out)
{
	கட்டமைப்பு ns_cert_info *certs;
	size_t depth;
	nserror res;

	certs = calloc(chain->depth, அளவு(கட்டமைப்பு ns_cert_info));
	எனில் (certs == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	க்கு (depth = 0; depth < chain->depth;depth++) {
		res = der_to_certinfo(chain->certs[depth].der,
				      chain->certs[depth].der_length,
				      certs + depth);
		எனில் (res != NSERROR_OK) {
			விடுவி(certs);
			திரும்பு res;
		}
		certs[depth].err = chain->certs[depth].err;
	}

	*cert_info_out = certs;
	திரும்பு NSERROR_OK;
}

#இல்லையெனில்
நிலையான nserror
convert_chain_to_cert_info(மாறிலி கட்டமைப்பு cert_chain *chain,
			   கட்டமைப்பு ns_cert_info **cert_info_out)
{
	திரும்பு NSERROR_NOT_IMPLEMENTED;
}
#endif


நிலையான nserror
format_certificate_name(கட்டமைப்பு fetch_about_context *ctx,
			கட்டமைப்பு ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	எனில் (cert_name->organisation != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (cert_name->organisation_unit != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (cert_name->locality != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (cert_name->province != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (cert_name->country != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	திரும்பு res;
}

/**
 * output formatted certificate subject alternate names
 */
நிலையான nserror
format_certificate_san(கட்டமைப்பு fetch_about_context *ctx,
			      கட்டமைப்பு ns_cert_san *san)
{
	nserror res;

	எனில் (san == வெற்று) {
		திரும்பு NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	போது (san != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}

		san = san->next;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	திரும்பு res;

}


நிலையான nserror
format_certificate_public_சாவி(கட்டமைப்பு fetch_about_context *ctx,
			      கட்டமைப்பு ns_cert_pkey *public_சாவி)
{
	nserror res;

	எனில் (public_சாவி->algor == வெற்று) {
		/* skip the table if no algorithm name */
		திரும்பு NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 public_சாவி->algor,
			 public_சாவி->size);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}


	எனில் (public_சாவி->exponent != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 public_சாவி->exponent);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (public_சாவி->modulus != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 public_சாவி->modulus);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (public_சாவி->curve != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 public_சாவி->curve);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (public_சாவி->பொது != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 public_சாவி->பொது);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	திரும்பு res;
}

நிலையான nserror
format_certificate_fingerprint(கட்டமைப்பு fetch_about_context *ctx,
			      கட்டமைப்பு ns_cert_info *cert_info)
{
	nserror res;

	எனில் ((cert_info->sha1fingerprint == வெற்று) &&
	    (cert_info->sha256fingerprint == வெற்று))  {
		/* skip the table if no fingerprints */
		திரும்பு NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	எனில் (cert_info->sha256fingerprint != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (cert_info->sha1fingerprint != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	திரும்பு res;
}

நிலையான nserror
format_certificate(கட்டமைப்பு fetch_about_context *ctx,
		   கட்டமைப்பு ns_cert_info *cert_info,
		   size_t depth)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 depth, cert_info->subject_name.common_name);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	எனில் (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = format_certificate_public_சாவி(ctx, &cert_info->public_சாவி);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	எனில் (cert_info->serialnum != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	எனில் (cert_info->sig_algor != வெற்று) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->version);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	திரும்பு res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	முழு code = 200;
	nserror res;
	கட்டமைப்பு cert_chain *chain = வெற்று;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	எனில் (fetch_about_send_header(ctx, "Content-Type: text/html"))
		செல் fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	எனில் (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		எனில் (res != NSERROR_OK) {
			செல் fetch_about_certificate_handler_aborted;
		}
	} இல்லையெனில் {
		கட்டமைப்பு ns_cert_info *cert_info;
		res = convert_chain_to_cert_info(chain, &cert_info);
		எனில் (res == NSERROR_OK) {
			size_t depth;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			எனில் (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				செல் fetch_about_certificate_handler_aborted;
			}

			க்கு (depth = 0; depth < chain->depth; depth++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						depth, (cert_info + depth)
							->subject_name
								.common_name);
				எனில் (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					செல் fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			எனில் (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				செல் fetch_about_certificate_handler_aborted;
			}

			க்கு (depth = 0; depth < chain->depth; depth++) {
				res = format_certificate(ctx, cert_info + depth,
						depth);
				எனில் (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					செல் fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} இல்லையெனில் {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			எனில் (res != NSERROR_OK) {
				செல் fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	cert_chain_free(chain);

	திரும்பு true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	திரும்பு false;
}
