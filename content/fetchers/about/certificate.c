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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "netsurf/ssl_certs.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "certificate.h"

/**
 * certificate name parameters
 */
నిర్మాణం ns_cert_name {
	అక్షరం *common_name;
	అక్షరం *organisation;
	అక్షరం *organisation_unit;
	అక్షరం *locality;
	అక్షరం *province;
	అక్షరం *country;
};

/**
 * Certificate public key parameters
 */
నిర్మాణం ns_cert_pkey {
	అక్షరం *algor;
	పూర్ణాంకం కొలత;
	అక్షరం *modulus;
	అక్షరం *exponent;
	అక్షరం *వక్రం;
	అక్షరం *బహిరంగ;
};

/**
 * Certificate subject alternative name
 */
నిర్మాణం ns_cert_san {
	నిర్మాణం ns_cert_san *తదుపరి;
	అక్షరం *name;
};

/**
 * certificate information for certificate chain
 */
నిర్మాణం ns_cert_info {
	నిర్మాణం ns_cert_name subject_name; /**< Subject details */
	నిర్మాణం ns_cert_name issuer_name; /**< Issuer details */
	నిర్మాణం ns_cert_pkey ప్రజా_కీలకం; /**< public key details */
	దీర్ఘ సంస్కరణ;		/**< Certificate version */
	అక్షరం *not_before;	/**< Valid from date */
	అక్షరం *not_after;	/**< Valid to date */
	పూర్ణాంకం sig_type;		/**< Signature type */
	అక్షరం *sig_algor;        /**< Signature Algorithm */
	అక్షరం *serialnum;	/**< Serial number */
	అక్షరం *sha1fingerprint; /**< fingerprint shar1 encoded */
	అక్షరం *sha256fingerprint; /**< fingerprint shar256 encoded */
	నిర్మాణం ns_cert_san *san; /**< subject alternative names */
	ssl_cert_err err;       /**< Whatever is wrong with this certificate */
};

/**
 * free all resources associated with a certificate information structure
 */
స్థిర nserror free_ns_cert_info(నిర్మాణం ns_cert_info *cinfo)
{
	నిర్మాణం ns_cert_san *san;

	విడుదల(cinfo->subject_name.common_name);
	విడుదల(cinfo->subject_name.organisation);
	విడుదల(cinfo->subject_name.organisation_unit);
	విడుదల(cinfo->subject_name.locality);
	విడుదల(cinfo->subject_name.province);
	విడుదల(cinfo->subject_name.country);
	విడుదల(cinfo->issuer_name.common_name);
	విడుదల(cinfo->issuer_name.organisation);
	విడుదల(cinfo->issuer_name.organisation_unit);
	విడుదల(cinfo->issuer_name.locality);
	విడుదల(cinfo->issuer_name.province);
	విడుదల(cinfo->issuer_name.country);
	విడుదల(cinfo->ప్రజా_కీలకం.algor);
	విడుదల(cinfo->ప్రజా_కీలకం.modulus);
	విడుదల(cinfo->ప్రజా_కీలకం.exponent);
	విడుదల(cinfo->ప్రజా_కీలకం.వక్రం);
	విడుదల(cinfo->ప్రజా_కీలకం.బహిరంగ);
	విడుదల(cinfo->not_before);
	విడుదల(cinfo->not_after);
	విడుదల(cinfo->sig_algor);
	విడుదల(cinfo->serialnum);

	/* free san list avoiding use after free */
	san = cinfo->san;
	అయితే (san != ఖాళీ) {
		నిర్మాణం ns_cert_san *తదుపరి;
		తదుపరి = san->తదుపరి;
		విడుదల(san);
		san = తదుపరి;
	}

	విడుదల(cinfo);

	తిరిగి NSERROR_OK;
}

#నిర్వచితమైతే WITH_OPENSSL

#అన్తర్భావ <openssl/ssl.h>
#అన్తర్భావ <openssl/x509v3.h>

#ఒకవేళ (OPENSSL_సంస్కరణ_సంఖ్య < 0x30000000L)
/* OpenSSL 1.1.1 or LibreSSL */

# ఒకవేళ defined(LIBRESSL_సంస్కరణ_సంఖ్య)
  /* LibreSSL */
#  ఒకవేళ (LIBRESSL_సంస్కరణ_సంఖ్య < 0x3050000fL)
   /* LibreSSL <3.5.0 */

#   ఒకవేళ (LIBRESSL_సంస్కరణ_సంఖ్య < 0x2070000fL)
    /* LibreSSL <2.7.0 */
స్థిర పూర్ణాంకం ns_X509_get_signature_nid(X509 *cert)
{
	తిరిగి OBJ_obj2nid(cert->cert_info->కీలకం->algor->algorithm);
}

స్థిర స్థిరరాశి గుర్తులేని అక్షరం *ns_ASN1_STRING_get0_దత్తాంశం(ASN1_STRING *asn1str)
{
	తిరిగి (స్థిరరాశి గుర్తులేని అక్షరం *)ASN1_STRING_దత్తాంశం(asn1str);
}
#   లేకపోతే
#    define ns_X509_get_signature_nid X509_get_signature_nid
#    define ns_ASN1_STRING_get0_దత్తాంశం ASN1_STRING_get0_దత్తాంశం
#   endif

స్థిర స్థిరరాశి BIGNUM *ns_RSA_get0_n(స్థిరరాశి RSA *d)
{
	తిరిగి d->n;
}

స్థిర స్థిరరాశి BIGNUM *ns_RSA_get0_e(స్థిరరాశి RSA *d)
{
	తిరిగి d->e;
}
#  లేకపోతే
   /* LibreSSL >= 3.5.0 */
#   define ns_X509_get_signature_nid X509_get_signature_nid
#   define ns_ASN1_STRING_get0_దత్తాంశం ASN1_STRING_get0_దత్తాంశం
#   define ns_RSA_get0_n RSA_get0_n
#   define ns_RSA_get0_e RSA_get0_e
#  endif
# లేకపోతే
  /* OpenSSL 1.1.1 */
#  define ns_X509_get_signature_nid X509_get_signature_nid
#  define ns_ASN1_STRING_get0_దత్తాంశం ASN1_STRING_get0_దత్తాంశం
#  define ns_RSA_get0_n RSA_get0_n
#  define ns_RSA_get0_e RSA_get0_e
# endif

స్థిర పూర్ణాంకం ns_EVP_PKEY_get_bn_param(స్థిరరాశి EVP_PKEY *pkey,
		స్థిరరాశి అక్షరం *కీలకం_name, BIGNUM **bn) {
	RSA *rsa;
	BIGNUM *ఫలితం = ఖాళీ;

	/* Check parameters: only support allocation-form *bn */
	ఒకవేళ (pkey == ఖాళీ || కీలకం_name == ఖాళీ || bn == ఖాళీ || *bn != ఖాళీ)
		తిరిగి 0;

	/* Only support RSA keys */
	ఒకవేళ (EVP_PKEY_పునాది_id(pkey) != EVP_PKEY_RSA)
		తిరిగి 0;

	rsa = EVP_PKEY_get1_RSA((EVP_PKEY *) pkey);
	ఒకవేళ (rsa == ఖాళీ)
		తిరిగి 0;

	ఒకవేళ (strcmp(కీలకం_name, "n") == 0) {
		స్థిరరాశి BIGNUM *n = ns_RSA_get0_n(rsa);
		ఒకవేళ (n != ఖాళీ)
			ఫలితం = BN_dup(n);
	} లేకపోతే ఒకవేళ (strcmp(కీలకం_name, "e") == 0) {
		స్థిరరాశి BIGNUM *e = ns_RSA_get0_e(rsa);
		ఒకవేళ (e != ఖాళీ)
			ఫలితం = BN_dup(e);
	}

	RSA_free(rsa);

	*bn = ఫలితం;

	తిరిగి (ఫలితం != ఖాళీ) ? 1 : 0;
}

స్థిర పూర్ణాంకం ns_EVP_PKEY_get_utf8_string_param(స్థిరరాశి EVP_PKEY *pkey,
		స్థిరరాశి అక్షరం *కీలకం_name, అక్షరం *str, size_t max_len,
		size_t *బహిర్గతం_len)
{
	స్థిరరాశి EC_సమూహం *ecgroup;
	స్థిరరాశి అక్షరం *సమూహం = "";
	size_t సమూహం_len = 0;
	EC_కీలకం *ec;
	పూర్ణాంకం ret = 0;

	ఒకవేళ (pkey == ఖాళీ || కీలకం_name == ఖాళీ)
		తిరిగి 0;

	/* Only support EC keys */
	ఒకవేళ (EVP_PKEY_పునాది_id(pkey) != EVP_PKEY_EC)
		తిరిగి 0;

	/* Only support fetching the group */
	ఒకవేళ (strcmp(కీలకం_name, "group") != 0)
		తిరిగి 0;

	ec = EVP_PKEY_get1_EC_కీలకం((EVP_PKEY *) pkey);

	ecgroup = EC_కీలకం_get0_సమూహం(ec);
	ఒకవేళ (ecgroup != ఖాళీ) {
		సమూహం = OBJ_nid2ln(EC_సమూహం_get_వక్రం_name(ecgroup));
		సమూహం_len = అక్షరశ్రేణి_పొడవు(సమూహం);
	}

	ఒకవేళ (str != ఖాళీ && max_len > సమూహం_len) {
		జ్ఞాపకం_నకలు(str, సమూహం, సమూహం_len);
		str[సమూహం_len] = '\0';
		ret = 1;
	}

	ఒకవేళ (బహిర్గతం_len != ఖాళీ) {
		*బహిర్గతం_len = సమూహం_len;
	}

	EC_కీలకం_free(ec);

	తిరిగి ret;
}

స్థిర పూర్ణాంకం ns_EVP_PKEY_get_octet_string_param(స్థిరరాశి EVP_PKEY *pkey,
		స్థిరరాశి అక్షరం *కీలకం_name, గుర్తులేని అక్షరం *buf, size_t max_len,
		size_t *బహిర్గతం_len)
{
	స్థిరరాశి EC_సమూహం *ecgroup;
	స్థిరరాశి EC_బిందువు *ecpoint;
	size_t len;
	BN_CTX *bnctx;
	EC_కీలకం *ec;
	పూర్ణాంకం ret = 0;

	ఒకవేళ (pkey == ఖాళీ || కీలకం_name == ఖాళీ)
		తిరిగి 0;

	/* Only support EC keys */
	ఒకవేళ (EVP_PKEY_పునాది_id(pkey) != EVP_PKEY_EC)
		తిరిగి 0;

	ఒకవేళ (strcmp(కీలకం_name, "encoded-pub-key") != 0)
		తిరిగి 0;

	ec = EVP_PKEY_get1_EC_కీలకం((EVP_PKEY *) pkey);
	ఒకవేళ (ec == ఖాళీ)
		తిరిగి 0;

	ecgroup = EC_కీలకం_get0_సమూహం(ec);
	ఒకవేళ (ecgroup != ఖాళీ) {
		ecpoint = EC_కీలకం_get0_ప్రజా_కీలకం(ec);
		ఒకవేళ (ecpoint != ఖాళీ) {
			bnctx = BN_CTX_new();
			len = EC_బిందువు_point2oct(ecgroup,
						 ecpoint,
						 బిందువు_CONVERSION_UNCOMPRESSED,
						 ఖాళీ,
						 0,
						 bnctx);
			ఒకవేళ (len != 0 && len <= max_len) {
				ఒకవేళ (EC_బిందువు_point2oct(ecgroup,
						       ecpoint,
						       బిందువు_CONVERSION_UNCOMPRESSED,
						       buf,
						       len,
						       bnctx) == len)
					ret = 1;
			}
			ఒకవేళ (బహిర్గతం_len != ఖాళీ)
				*బహిర్గతం_len = len;
			BN_CTX_free(bnctx);
		}
	}

	EC_కీలకం_free(ec);

	తిరిగి ret;
}
#లేకపోతే
/* OpenSSL 3.x and later */
#నిర్వచించు ns_X509_get_signature_nid X509_get_signature_nid
#నిర్వచించు ns_ASN1_STRING_get0_దత్తాంశం ASN1_STRING_get0_దత్తాంశం
#నిర్వచించు ns_RSA_get0_n RSA_get0_n
#నిర్వచించు ns_RSA_get0_e RSA_get0_e
#నిర్వచించు ns_EVP_PKEY_get_bn_param EVP_PKEY_get_bn_param
#నిర్వచించు ns_EVP_PKEY_get_octet_string_param EVP_PKEY_get_octet_string_param
#నిర్వచించు ns_EVP_PKEY_get_utf8_string_param EVP_PKEY_get_utf8_string_param
#అయితే_ముగింపు

/**
 * extract certificate name information
 *
 * \param xname The X509 name to convert. The reference is borrowed so is not freeed
 * \param iname The info structure to recive the extracted parameters.
 * \return NSERROR_OK on success else error code
 */
స్థిర nserror
xname_గమ్యం_info(X509_NAME *xname, నిర్మాణం ns_cert_name *iname)
{
	పూర్ణాంకం entryidx;
	పూర్ణాంకం entrycnt;
	X509_NAME_నమోదు *నమోదు_2; /* current name entry */
	ASN1_STRING *విలువ;
	స్థిరరాశి గుర్తులేని అక్షరం *విలువ_str;
	ASN1_OBJECT *name;
	పూర్ణాంకం name_nid;
	అక్షరం **క్షేత్రం;

	entrycnt = X509_NAME_నమోదు_లెక్క(xname);

	కొరకు (entryidx = 0; entryidx < entrycnt; entryidx++) {
		నమోదు_2 = X509_NAME_get_నమోదు(xname, entryidx);
		name = X509_NAME_నమోదు_get_object(నమోదు_2);
		name_nid = OBJ_obj2nid(name);
		విలువ = X509_NAME_నమోదు_get_దత్తాంశం(నమోదు_2);
		విలువ_str = ns_ASN1_STRING_get0_దత్తాంశం(విలువ);
		ఎంపిక (name_nid) {
		స్థితి NID_commonName:
			క్షేత్రం = &iname->common_name;
			ఆపు;
		స్థితి NID_countryName:
			క్షేత్రం = &iname->country;
			ఆపు;
		స్థితి NID_localityName:
			క్షేత్రం = &iname->locality;
			ఆపు;
		స్థితి NID_stateOrProvinceName:
			క్షేత్రం = &iname->province;
			ఆపు;
		స్థితి NID_organizationName:
			క్షేత్రం = &iname->organisation;
			ఆపు;
		స్థితి NID_organizationalUnitName:
			క్షేత్రం = &iname->organisation_unit;
			ఆపు;
		అప్రమేయ :
			క్షేత్రం = ఖాళీ;
			ఆపు;
		}
		ఒకవేళ (క్షేత్రం != ఖాళీ) {
			*క్షేత్రం = strdup((స్థిరరాశి అక్షరం *)విలువ_str);
			NSLOG(netsurf, DEEPDEBUG,
			      "NID:%d value: %s", name_nid, *క్షేత్రం);
		} లేకపోతే {
			NSLOG(netsurf, DEEPDEBUG, "NID:%d", name_nid);
		}
	}

	/*
	 * ensure the common name is set to something, this being
	 *  missing means the certificate is broken but this should be
	 *  robust in the face of bad data
	 */
	ఒకవేళ (iname->common_name == ఖాళీ) {
		iname->common_name = strdup("Unknown");
	}

	తిరిగి NSERROR_OK;
}


/**
 * duplicate a hex formatted string inserting the colons
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
స్థిర అక్షరం *hexdup(స్థిరరాశి అక్షరం *hex)
{
	పూర్ణాంకం hexlen;
	అక్షరం *dst;
	అక్షరం *బహిర్గతం;
	పూర్ణాంకం cn = 0;

	hexlen = అక్షరశ్రేణి_పొడవు(hex);
	/* allow space fox XXYY to XX&#58;YY&#58; */
	dst = జ్ఞాపకం_ఇవ్వు(((hexlen * 7) + 6) / 2);

	ఒకవేళ (dst != ఖాళీ) {
		కొరకు (బహిర్గతం = dst; *hex != 0; hex++) {
			ఒకవేళ (cn == 2) {
				cn = 0;
				*బహిర్గతం++ = '&';
				*బహిర్గతం++ = '#';
				*బహిర్గతం++ = '5';
				*బహిర్గతం++ = '8';
				*బహిర్గతం++ = ';';
			}
			*బహిర్గతం++ = *hex;
			cn++;
		}
		*బహిర్గతం = 0;
	}
	తిరిగి dst;
}


/**
 * create a hex formatted string inserting the colons from binary data
 *
 * \todo only uses html entity as separator because netsurfs line breaking
 *       fails otherwise.
 */
స్థిర అక్షరం *bindup(గుర్తులేని అక్షరం *bin, గుర్తులేని పూర్ణాంకం binlen)
{
	అక్షరం *dst;
	అక్షరం *బహిర్గతం;
	గుర్తులేని పూర్ణాంకం idx;
	స్థిరరాశి అక్షరం hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	/* allow space fox XY to expand to XX&#58;YY&#58; */
	dst = జ్ఞాపకం_ఇవ్వు(binlen * 7);

	ఒకవేళ (dst != ఖాళీ) {
		బహిర్గతం = dst;
		కొరకు (idx = 0; idx < binlen; idx++) {
			*బహిర్గతం++ = hex[(bin[idx] & 0xf0) >> 4];
			*బహిర్గతం++ = hex[bin[idx] & 0xf];

			*బహిర్గతం++ = '&';
			*బహిర్గతం++ = '#';
			*బహిర్గతం++ = '5';
			*బహిర్గతం++ = '8';
			*బహిర్గతం++ = ';';
		}
		బహిర్గతం -= 5;
		*బహిర్గతం = 0;
	}
	తిరిగి dst;
}


/**
 * extract RSA key information to info structure
 *
 * \param pkey The RSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
స్థిర nserror
rsa_గమ్యం_info(EVP_PKEY *pkey, నిర్మాణం ns_cert_pkey *ikey)
{
	BIGNUM *n = ఖాళీ, *e = ఖాళీ;
	అక్షరం *tmp;

	ఒకవేళ (ns_EVP_PKEY_get_bn_param(pkey, "n", &n) != 1) {
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	ఒకవేళ (ns_EVP_PKEY_get_bn_param(pkey, "e", &e) != 1) {
		BN_free(n);
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	ikey->algor = strdup("RSA");

	ikey->కొలత = EVP_PKEY_బిట్లు(pkey);

	tmp = BN_bn2hex(n);
	ఒకవేళ (tmp != ఖాళీ) {
		ikey->modulus = hexdup(tmp);
		OPENSSL_free(tmp);
	}

	tmp = BN_bn2dec(e);
	ఒకవేళ (tmp != ఖాళీ) {
		ikey->exponent = strdup(tmp);
		OPENSSL_free(tmp);
	}

	BN_free(e);
	BN_free(n);

	తిరిగి NSERROR_OK;
}


/**
 * extract DSA key information to info structure
 *
 * \param pkey The DSA key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
స్థిర nserror
dsa_గమ్యం_info(EVP_PKEY *pkey, నిర్మాణం ns_cert_pkey *ikey)
{
	ikey->algor = strdup("DSA");

	ikey->కొలత = EVP_PKEY_బిట్లు(pkey);

	తిరిగి NSERROR_OK;
}


/**
 * extract DH key information to info structure
 *
 * \param pkey The DH key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
స్థిర nserror
dh_గమ్యం_info(EVP_PKEY *pkey, నిర్మాణం ns_cert_pkey *ikey)
{
	ikey->algor = strdup("Diffie Hellman");

	ikey->కొలత = EVP_PKEY_బిట్లు(pkey);

	తిరిగి NSERROR_OK;
}


/**
 * extract EC key information to info structure
 *
 * \param pkey The EC key to examine.
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
స్థిర nserror
ec_గమ్యం_info(EVP_PKEY *pkey, నిర్మాణం ns_cert_pkey *ikey)
{
	size_t len;

	ikey->algor = strdup("Elliptic Curve");

	ikey->కొలత = EVP_PKEY_బిట్లు(pkey);

	len = 0;
	ns_EVP_PKEY_get_utf8_string_param(pkey, "group", ఖాళీ, 0, &len);
	ఒకవేళ (len != 0) {
		ikey->వక్రం = జ్ఞాపకం_ఇవ్వు(len + 1);
		ఒకవేళ (ikey->వక్రం != ఖాళీ) {
			ఒకవేళ (ns_EVP_PKEY_get_utf8_string_param(pkey, "group",
					ikey->వక్రం, len + 1, ఖాళీ) == 0) {
				విడుదల(ikey->వక్రం);
				ikey->వక్రం = ఖాళీ;
			}
		}
	}

	len = 0;
	ns_EVP_PKEY_get_octet_string_param(pkey, "encoded-pub-key",
			ఖాళీ, 0, &len);
	ఒకవేళ (len != 0) {
		గుర్తులేని అక్షరం *బిందువు = జ్ఞాపకం_ఇవ్వు(len);
		ఒకవేళ (బిందువు != ఖాళీ) {
			ఒకవేళ (ns_EVP_PKEY_get_octet_string_param(pkey,
					"encoded-pub-key", బిందువు, len,
					ఖాళీ) == 1) {
				ikey->బహిరంగ = bindup(బిందువు, len);
			}
			విడుదల(బిందువు);
		}
	}

	తిరిగి NSERROR_OK;
}

/**
 * extract public key information to info structure
 *
 * \param pkey the public key to examine. The reference is dropped on return
 * \param ikey The public key info structure to fill
 * \rerun NSERROR_OK on success else error code.
 */
స్థిర nserror
pkey_గమ్యం_info(EVP_PKEY *pkey, నిర్మాణం ns_cert_pkey *ikey)
{
	nserror res;

	ఒకవేళ (pkey == ఖాళీ) {
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	ఎంపిక (EVP_PKEY_పునాది_id(pkey)) {
	స్థితి EVP_PKEY_RSA:
		res = rsa_గమ్యం_info(pkey, ikey);
		ఆపు;

	స్థితి EVP_PKEY_DSA:
		res = dsa_గమ్యం_info(pkey, ikey);
		ఆపు;

	స్థితి EVP_PKEY_DH:
		res = dh_గమ్యం_info(pkey, ikey);
		ఆపు;

	స్థితి EVP_PKEY_EC:
		res = ec_గమ్యం_info(pkey, ikey);
		ఆపు;

	అప్రమేయ:
		res = NSERROR_NOT_IMPLEMENTED;
		ఆపు;
	}

	EVP_PKEY_free(pkey);

	తిరిగి res;
}

స్థిర nserror san_గమ్యం_info(X509 *cert, నిర్మాణం ns_cert_san **prev_తదుపరి)
{
	పూర్ణాంకం idx;
	పూర్ణాంకం san_పేర్లు_nb = -1;
	స్థిరరాశి GENERAL_NAME *current_name;
	స్థిరరాశి గుర్తులేని అక్షరం *dns_name;
	నిర్మాణం ns_cert_san *isan;

	దొంతర_యొక్క(GENERAL_NAME) *san_పేర్లు = ఖాళీ;

	san_పేర్లు = X509_get_ext_d2i(cert, NID_subject_alt_name, ఖాళీ, ఖాళీ);
	ఒకవేళ (san_పేర్లు == ఖాళీ) {
		తిరిగి NSERROR_OK;
	}

	san_పేర్లు_nb = sk_GENERAL_NAME_అంకె(san_పేర్లు);

	/* Check each name within the extension */
	కొరకు (idx = 0; idx < san_పేర్లు_nb; idx++) {
		current_name = sk_GENERAL_NAME_విలువ(san_పేర్లు, idx);

		ఒకవేళ (current_name->type == GEN_DNS) {
			/* extract DNS name into info structure */
			dns_name = ns_ASN1_STRING_get0_దత్తాంశం(current_name->d.dNSName);

			isan = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం ns_cert_san));
			ఒకవేళ (isan != ఖాళీ) {
				isan->name = strdup((స్థిరరాశి అక్షరం *)dns_name);
				isan->తదుపరి = ఖాళీ;
				*prev_తదుపరి = isan;
				prev_తదుపరి = &isan->తదుపరి;
			}
		}
	}

	/* AmiSSL can't cope with the "correct" mechanism of freeing
	 * the GENERAL_NAME stack, which is:
	 * sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
	 * So instead we do this open-coded loop which does the same:
	 */
	కొరకు (idx = 0; idx < san_పేర్లు_nb; idx++) {
		GENERAL_NAME *నమోదు_2 = sk_GENERAL_NAME_తీయి(san_పేర్లు);
		GENERAL_NAME_free(నమోదు_2);
	}
	sk_GENERAL_NAME_free(san_పేర్లు);

	తిరిగి NSERROR_OK;
}

స్థిర nserror
der_గమ్యం_certinfo(స్థిరరాశి uint8_t *der,
		size_t der_నిడివి,
		నిర్మాణం ns_cert_info *info)
{
	BIO *mem;
	BUF_MEM *buf;
	స్థిరరాశి ASN1_INTEGER *asn1_అంకె;
	BIGNUM *bignum;
	X509 *cert;		/**< Pointer to certificate */

	ఒకవేళ (der == ఖాళీ) {
		తిరిగి NSERROR_OK;
	}

	cert = d2i_X509(ఖాళీ, &der, der_నిడివి);
	ఒకవేళ (cert == ఖాళీ) {
		తిరిగి NSERROR_INVALID;
	}

	/*
	 * get certificate version
	 *
	 * \note this is defined by standards (X.509 et al) to be one
	 *        less than the certificate version.
	 */
	info->సంస్కరణ = X509_get_సంస్కరణ(cert) + 1;

	/* not before date */
	mem = BIO_new(BIO_s_mem());
	ASN1_సమయం_print(mem, X509_get_notBefore(cert));
	BIO_get_mem_ptr(mem, &buf);
	(శూన్య) BIO_సమితి_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_before = సున్నా_జ్ఞాపకం_ఇవ్వు(1, buf->నిడివి + 1);
	ఒకవేళ (info->not_before != ఖాళీ) {
		జ్ఞాపకం_నకలు(info->not_before, buf->దత్తాంశం, (గుర్తులేని)buf->నిడివి);
	}
	BUF_MEM_free(buf);

	/* not after date */
	mem = BIO_new(BIO_s_mem());
	ASN1_సమయం_print(mem,
			X509_get_notAfter(cert));
	BIO_get_mem_ptr(mem, &buf);
	(శూన్య) BIO_సమితి_close(mem, BIO_NOCLOSE);
	BIO_free(mem);
	info->not_after = సున్నా_జ్ఞాపకం_ఇవ్వు(1, buf->నిడివి + 1);
	ఒకవేళ (info->not_after != ఖాళీ) {
		జ్ఞాపకం_నకలు(info->not_after, buf->దత్తాంశం, (గుర్తులేని)buf->నిడివి);
	}
	BUF_MEM_free(buf);

	/* signature type */
	info->sig_type = X509_get_signature_type(cert);

	/* signature algorithm */
	పూర్ణాంకం pkey_nid = ns_X509_get_signature_nid(cert);
	ఒకవేళ (pkey_nid != NID_undef) {
		స్థిరరాశి అక్షరం* sslbuf = OBJ_nid2ln(pkey_nid);
		ఒకవేళ (sslbuf != ఖాళీ) {
			info->sig_algor = strdup(sslbuf);
		}
	}

	/* serial number */
	asn1_అంకె = X509_get_serial_సంఖ్య(cert);
	ఒకవేళ (asn1_అంకె != ఖాళీ) {
		bignum = ASN1_INTEGER_గమ్యం_BN(asn1_అంకె, ఖాళీ);
		ఒకవేళ (bignum != ఖాళీ) {
			అక్షరం *tmp = BN_bn2hex(bignum);
			ఒకవేళ (tmp != ఖాళీ) {
				info->serialnum = hexdup(tmp);
				OPENSSL_free(tmp);
			}
			BN_free(bignum);
			bignum = ఖాళీ;
		}
	}

	/* fingerprints */
	స్థిరరాశి EVP_MD *digest;
	గుర్తులేని పూర్ణాంకం dig_len;
	గుర్తులేని అక్షరం *buff;
	పూర్ణాంకం rc;

	digest = EVP_sha1();
	buff = జ్ఞాపకం_ఇవ్వు(EVP_MD_కొలత(digest));
	ఒకవేళ (buff != ఖాళీ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ఒకవేళ ((rc == 1) && (dig_len == (గుర్తులేని పూర్ణాంకం)EVP_MD_కొలత(digest))) {
			info->sha1fingerprint = bindup(buff, dig_len);
		}
		విడుదల(buff);
	}

	digest = EVP_sha256();
	buff = జ్ఞాపకం_ఇవ్వు(EVP_MD_కొలత(digest));
	ఒకవేళ (buff != ఖాళీ) {
		rc = X509_digest(cert, digest, buff, &dig_len);
		ఒకవేళ ((rc == 1) && (dig_len == (గుర్తులేని పూర్ణాంకం)EVP_MD_కొలత(digest))) {
			info->sha256fingerprint = bindup(buff, dig_len);
		}
		విడుదల(buff);
	}

	/* subject alternative names */
	san_గమ్యం_info(cert, &info->san);

	/* issuer name */
	xname_గమ్యం_info(X509_get_issuer_name(cert), &info->issuer_name);

	/* subject */
	xname_గమ్యం_info(X509_get_subject_name(cert), &info->subject_name);

	/* public key */
	pkey_గమ్యం_info(X509_get_pubkey(cert), &info->ప్రజా_కీలకం);

	X509_free(cert);

	తిరిగి NSERROR_OK;
}

/* copy certificate data */
స్థిర nserror
convert_chain_గమ్యం_cert_info(స్థిరరాశి నిర్మాణం cert_chain *chain,
			   నిర్మాణం ns_cert_info **cert_info_బహిర్గతం)
{
	నిర్మాణం ns_cert_info *certs;
	size_t లోతు;
	nserror res;

	certs = సున్నా_జ్ఞాపకం_ఇవ్వు(chain->లోతు, పరిమాణం(నిర్మాణం ns_cert_info));
	ఒకవేళ (certs == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	కొరకు (లోతు = 0; లోతు < chain->లోతు;లోతు++) {
		res = der_గమ్యం_certinfo(chain->certs[లోతు].der,
				      chain->certs[లోతు].der_నిడివి,
				      certs + లోతు);
		ఒకవేళ (res != NSERROR_OK) {
			విడుదల(certs);
			తిరిగి res;
		}
		certs[లోతు].err = chain->certs[లోతు].err;
	}

	*cert_info_బహిర్గతం = certs;
	తిరిగి NSERROR_OK;
}

#లేకపోతే
స్థిర nserror
convert_chain_గమ్యం_cert_info(స్థిరరాశి నిర్మాణం cert_chain *chain,
			   నిర్మాణం ns_cert_info **cert_info_బహిర్గతం)
{
	తిరిగి NSERROR_NOT_IMPLEMENTED;
}
#అయితే_ముగింపు


స్థిర nserror
format_certificate_name(నిర్మాణం fetch_about_context *ctx,
			నిర్మాణం ns_cert_name *cert_name)
{
	nserror res;
	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Common Name</th><td>%s</td></tr>\n",
			 cert_name->common_name);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	ఒకవేళ (cert_name->organisation != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation</th><td>%s</td></tr>\n",
				 cert_name->organisation);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (cert_name->organisation_unit != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Organisation Unit</th><td>%s</td></tr>\n",
				 cert_name->organisation_unit);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (cert_name->locality != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Locality</th><td>%s</td></tr>\n",
				 cert_name->locality);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (cert_name->province != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Province</th><td>%s</td></tr>\n",
				 cert_name->province);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (cert_name->country != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Country</th><td>%s</td></tr>\n",
				 cert_name->country);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	తిరిగి res;
}

/**
 * output formatted certificate subject alternate names
 */
స్థిర nserror
format_certificate_san(నిర్మాణం fetch_about_context *ctx,
			      నిర్మాణం ns_cert_san *san)
{
	nserror res;

	ఒకవేళ (san == ఖాళీ) {
		తిరిగి NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Alternative Names</th><td><hr></td></tr>\n");
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	అయితే (san != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>DNS Name</th><td>%s</td></tr>\n",
				 san->name);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}

		san = san->తదుపరి;
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	తిరిగి res;

}


స్థిర nserror
format_certificate_ప్రజా_కీలకం(నిర్మాణం fetch_about_context *ctx,
			      నిర్మాణం ns_cert_pkey *ప్రజా_కీలకం)
{
	nserror res;

	ఒకవేళ (ప్రజా_కీలకం->algor == ఖాళీ) {
		/* skip the table if no algorithm name */
		తిరిగి NSERROR_OK;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Public Key</th><td><hr></td></tr>\n"
			 "<tr><th>Algorithm</th><td>%s</td></tr>\n"
			 "<tr><th>Key Size</th><td>%d</td></tr>\n",
			 ప్రజా_కీలకం->algor,
			 ప్రజా_కీలకం->కొలత);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}


	ఒకవేళ (ప్రజా_కీలకం->exponent != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Exponent</th><td>%s</td></tr>\n",
				 ప్రజా_కీలకం->exponent);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (ప్రజా_కీలకం->modulus != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Modulus</th><td class=\"data\">%s</td></tr>\n",
				 ప్రజా_కీలకం->modulus);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (ప్రజా_కీలకం->వక్రం != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Curve</th><td>%s</td></tr>\n",
				 ప్రజా_కీలకం->వక్రం);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (ప్రజా_కీలకం->బహిరంగ != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Public Value</th><td>%s</td></tr>\n",
				 ప్రజా_కీలకం->బహిరంగ);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	తిరిగి res;
}

స్థిర nserror
format_certificate_fingerprint(నిర్మాణం fetch_about_context *ctx,
			      నిర్మాణం ns_cert_info *cert_info)
{
	nserror res;

	ఒకవేళ ((cert_info->sha1fingerprint == ఖాళీ) &&
	    (cert_info->sha256fingerprint == ఖాళీ))  {
		/* skip the table if no fingerprints */
		తిరిగి NSERROR_OK;
	}


	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Fingerprints</th><td><hr></td></tr>\n");
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	ఒకవేళ (cert_info->sha256fingerprint != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-256</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha256fingerprint);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (cert_info->sha1fingerprint != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>SHA-1</th><td class=\"data\">%s</td></tr>\n",
				 cert_info->sha1fingerprint);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</table>\n");

	తిరిగి res;
}

స్థిర nserror
format_certificate(నిర్మాణం fetch_about_context *ctx,
		   నిర్మాణం ns_cert_info *cert_info,
		   size_t లోతు)
{
	nserror res;

	res = fetch_about_ssenddataf(ctx,
			 "<h2 id=\"%"PRIsizet"\" class=\"ns-border\">%s</h2>\n",
			 లోతు, cert_info->subject_name.common_name);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	ఒకవేళ (cert_info->err != SSL_CERT_ERR_OK) {
		res = fetch_about_ssenddataf(ctx,
				 "<table class=\"info\">\n"
				 "<tr class=\"ns-even-fg-bad\">"
				 "<th>Fault</th>"
				 "<td>%s</td>"
				 "</tr>"
				 "</table>\n",
				 messages_get_sslcode(cert_info->err));
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued To</th><td><hr></td></tr>\n");
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = format_certificate_name(ctx, &cert_info->subject_name);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Issued By</th><td><hr></td></tr>\n");
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = format_certificate_name(ctx, &cert_info->issuer_name);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "</table>\n");
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Validity</th><td><hr></td></tr>\n"
			 "<tr><th>Valid From</th><td>%s</td></tr>\n"
			 "<tr><th>Valid Until</th><td>%s</td></tr>\n"
			 "</table>\n",
			 cert_info->not_before,
			 cert_info->not_after);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = format_certificate_san(ctx, cert_info->san);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = format_certificate_ప్రజా_కీలకం(ctx, &cert_info->ప్రజా_కీలకం);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = fetch_about_ssenddataf(ctx,
			 "<table class=\"info\">\n"
			 "<tr><th>Miscellaneous</th><td><hr></td></tr>\n");
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	ఒకవేళ (cert_info->serialnum != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Serial Number</th><td>%s</td></tr>\n",
				 cert_info->serialnum);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	ఒకవేళ (cert_info->sig_algor != ఖాళీ) {
		res = fetch_about_ssenddataf(ctx,
				 "<tr><th>Signature Algorithm</th>"
				 "<td>%s</td></tr>\n",
				 cert_info->sig_algor);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	res = fetch_about_ssenddataf(ctx,
			 "<tr><th>Version</th><td>%ld</td></tr>\n"
			 "</table>\n",
			 cert_info->సంస్కరణ);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	res = format_certificate_fingerprint(ctx, cert_info);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	తిరిగి res;
}

/**
 * Handler to generate about:certificate page.
 *
 * Shows details of a certificate chain
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_certificate_handler(నిర్మాణం fetch_about_context *ctx)
{
	పూర్ణాంకం సంకేతం_2 = 200;
	nserror res;
	నిర్మాణం cert_chain *chain = ఖాళీ;

	/* content is going to return ok */
	fetch_about_సమితి_http_సంకేతం(ctx, సంకేతం_2);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/html"))
		వెళ్ళు fetch_about_certificate_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Certificate Viewer</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
					"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body id=\"certificate\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">Certificate</h1>\n");
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_certificate_handler_aborted;
	}

	res = cert_chain_from_query(fetch_about_get_url(ctx), &chain);
	ఒకవేళ (res != NSERROR_OK) {
		res = fetch_about_ssenddataf(ctx, "<p>Could not process that</p>\n");
		ఒకవేళ (res != NSERROR_OK) {
			వెళ్ళు fetch_about_certificate_handler_aborted;
		}
	} లేకపోతే {
		నిర్మాణం ns_cert_info *cert_info;
		res = convert_chain_గమ్యం_cert_info(chain, &cert_info);
		ఒకవేళ (res == NSERROR_OK) {
			size_t లోతు;
			res = fetch_about_ssenddataf(ctx, "<ul>\n");
			ఒకవేళ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				వెళ్ళు fetch_about_certificate_handler_aborted;
			}

			కొరకు (లోతు = 0; లోతు < chain->లోతు; లోతు++) {
				res = fetch_about_ssenddataf(ctx, "<li><a href=\"#%"PRIsizet"\">%s</a></li>\n",
						లోతు, (cert_info + లోతు)
							->subject_name
								.common_name);
				ఒకవేళ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					వెళ్ళు fetch_about_certificate_handler_aborted;
				}

			}

			res = fetch_about_ssenddataf(ctx, "</ul>\n");
			ఒకవేళ (res != NSERROR_OK) {
				free_ns_cert_info(cert_info);
				వెళ్ళు fetch_about_certificate_handler_aborted;
			}

			కొరకు (లోతు = 0; లోతు < chain->లోతు; లోతు++) {
				res = format_certificate(ctx, cert_info + లోతు,
						లోతు);
				ఒకవేళ (res != NSERROR_OK) {
					free_ns_cert_info(cert_info);
					వెళ్ళు fetch_about_certificate_handler_aborted;
				}

			}
			free_ns_cert_info(cert_info);

		} లేకపోతే {
			res = fetch_about_ssenddataf(ctx,
					 "<p>Invalid certificate data</p>\n");
			ఒకవేళ (res != NSERROR_OK) {
				వెళ్ళు fetch_about_certificate_handler_aborted;
			}
		}
	}


	/* page footer */
	res = fetch_about_ssenddataf(ctx, "</body>\n</html>\n");
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_certificate_handler_aborted;
	}

	fetch_about_send_ముగిసిన(ctx);

	cert_chain_free(chain);

	తిరిగి true;

fetch_about_certificate_handler_aborted:
	cert_chain_free(chain);
	తిరిగి false;
}
