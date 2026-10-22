/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
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
 * Low-level resource cache persistent storage implementation.
 *
 * file based backing store.
 *
 * \todo Consider improving eviction sorting to include objects size
 *         and remaining lifetime and other cost metrics.
 *
 * \todo Implement mmap retrieval where supported.
 *
 * \todo Implement static retrieval for metadata objects as their heap
 *         lifetime is typically very short, though this may be obsoleted
 *         by a small object storage strategy.
 *
 */

#अन्तर्भाव <unistd.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <nsutils/unistd.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/filepath.h"
#अन्तर्भाव "utils/file.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/hashmap.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "netsurf/misc.h"

#अन्तर्भाव "content/backing_store.h"

/** Backing store file format version */
#परिभाषय CONTROL_संस्करणम् 202

/**
 * Number of milliseconds after a update before control data
 * maintenance is performed
 */
#परिभाषय CONTROL_MAINT_TIME 10000

/** Filename of serialised entries */
#परिभाषय ENTRIES_FNAME "entries"

/** Filename of block file index */
#परिभाषय खण्डानि_FNAME "blocks"

/** log2 block data address length (64k) */
#परिभाषय खण्डम्_ADDR_LEN 16

/** log2 number of entries per block file(1024) */
#परिभाषय खण्डम्_प्रविष्टिः_गणनम् 10

/** log2 number of data block files */
#परिभाषय खण्डम्_FILE_गणनम् (खण्डम्_ADDR_LEN - खण्डम्_प्रविष्टिः_गणनम्)

/** log2 size of data blocks (8k) */
#परिभाषय खण्डम्_दत्त_आकार 13

/** log2 size of metadata blocks (8k) */
#परिभाषय खण्डम्_META_आकार 13

/** length in bytes of a block files use map */
#परिभाषय खण्डम्_USE_MAP_आकार (1 << (खण्डम्_प्रविष्टिः_गणनम् - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
प्रकारनिर्देश uint32_t प्रविष्टिः_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
प्रकारनिर्देश uint16_t खण्डम्_सूचकांक_t;


/**
 * Entry element index values.
 */
गणना store_प्रविष्टिः_elem_idx {
	प्रविष्टिः_ELEM_दत्त = 0, /**< entry element is data */
	प्रविष्टिः_ELEM_META = 1,  /**< entry element is metadata */
	प्रविष्टिः_ELEM_गणनम् = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
गणना store_प्रविष्टिः_elem_flags {
	/** store not managing any allocation on entry */
	प्रविष्टिः_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	प्रविष्टिः_ELEM_FLAG_HEAP = 0x1,
	/** entry data allocation is mmaped */
	प्रविष्टिः_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	प्रविष्टिः_ELEM_FLAG_SMALL = 0x4,
};


गणना store_प्रविष्टिः_flags {
	/** entry is normal */
	प्रविष्टिः_FLAGS_NONE = 0,
	/** entry has been invalidated but something still holding a reference */
	प्रविष्टिः_FLAGS_INVALID = 1,
};

/**
 * Backing store entry element.
 *
 * An element keeps data about:
 *  - the current memory allocation
 *  - the number of outstanding references to the memory
 *  - the size of the element data
 *  - flags controlling how the memory and element are handled
 *
 * @note Order is important to avoid excessive structure packing overhead.
 */
संरचना store_प्रविष्टिः_element {
	uint8_t* दत्त; /**< data allocated */
	uint32_t आकार; /**< size of entry element on disc */
	खण्डम्_सूचकांक_t खण्डम्; /**< small object data block */
	uint8_t ref; /**< element data reference count */
	uint8_t flags; /**< entry flags */
};

/**
 * Backing store object index entry.
 *
 * An entry in the backing store contains two elements for the actual
 * data and the metadata. The two elements are treated identically for
 * storage lifetime but as a collective whole for expiration and
 * indexing.
 *
 * @note Order is important to avoid excessive structure packing overhead.
 */
संरचना store_प्रविष्टिः {
	nsurl *url; /**< The URL for this entry */
	int64_t चरमः_प्रयुक्तम्; /**< UNIX time the entry was last used */
	uint16_t use_गणनम्; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	संरचना store_प्रविष्टिः_element elem[प्रविष्टिः_ELEM_गणनम्];
};

/**
 * Small block file.
 */
संरचना खण्डम्_file {
	/** file descriptor of the block file */
	पूर्णाङ्क fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[खण्डम्_USE_MAP_आकार];
};

/**
 * log2 of block size.
 */
स्थिर ध्रुव अचिह्नित पूर्णाङ्क log2_खण्डम्_आकार[प्रविष्टिः_ELEM_गणनम्] = {
	खण्डम्_दत्त_आकार, /**< Data block size */
	खण्डम्_META_आकार  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
संरचना store_state {
	/* store config */
	वर्ण *मार्गः; /**< The path to the backing store */
	size_t सीमा; /**< The backing store upper bound target size */
	size_t hysteresis; /**< The hysteresis around the target size */

	/**
	 * The cache object hash
	 */
	hashmap_t *entries;

	/** flag indicating if the entries have been made persistent
	 * since they were last changed.
	 */
	bool entries_dirty;

	/** small block indexes */
	संरचना खण्डम्_file खण्डानि[प्रविष्टिः_ELEM_गणनम्][खण्डम्_FILE_गणनम्];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool खण्डानि_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool खण्डानि_opened;


	/* stats */
	uint64_t योग_alloc; /**< total size of all allocated storage. */

	size_t लब्धम्_गणनम्; /**< number of cache hits */
	uint64_t लब्धम्_आकार; /**< size of storage served */
	size_t अलब्धिः_गणनम्; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
संरचना store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

स्थिर bool
entries_hashmap_कुञ्चिका_eq(शून्य *key1, शून्य *key2)
{
	प्रत्यागम nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

स्थिर शून्य *
entries_hashmap_मानम्_alloc(शून्य *कुञ्चिका)
{
	संरचना store_प्रविष्टिः *ent = शून्यस्मृतिं_देहि(1, परिमाण(संरचना store_प्रविष्टिः));
	यदि (ent != रिक्त) {
		ent->url = nsurl_ref(कुञ्चिका);
	}
	प्रत्यागम ent;
}

स्थिर शून्य
entries_hashmap_मानम्_destroy(शून्य *मानम्)
{
	संरचना store_प्रविष्टिः *ent = मानम्;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	मुक्त(ent);
}

स्थिर hashmap_parameters_t entries_hashmap_parameters = {
	.कुञ्चिका_clone = (hashmap_कुञ्चिका_clone_t)nsurl_ref,
	.कुञ्चिका_destroy = (hashmap_कुञ्चिका_destroy_t)nsurl_unref,
	.कुञ्चिका_hash = (hashmap_कुञ्चिका_hash_t)nsurl_hash,
	.कुञ्चिका_eq = entries_hashmap_कुञ्चिका_eq,
	.मानम्_alloc = entries_hashmap_मानम्_alloc,
	.मानम्_destroy = entries_hashmap_मानम्_destroy,
};

/**
 * Generate a filename for an object.
 *
 * this generates the filename for an object on disc. It is necessary
 * for this to generate a filename which conforms to the limitations
 * of all the filesystems the cache can be placed upon.
 *
 * From http://en.wikipedia.org/wiki/Comparison_of_file_systems#Limits
 * the relevant subset is:
 *  - path elements no longer than 8 characters
 *  - acceptable characters are A-Z, 0-9
 *  - short total path lengths (255 or less)
 *  - no more than 77 entries per directory (6bits worth)
 *
 * The short total path lengths mean the encoding must represent as
 * much data as possible in the least number of characters.
 *
 * To achieve all these goals we use RFC4648 base32 encoding which
 * packs 5bits into each character of the filename. To represent a 32
 * bit ident this requires a total path length of between 17 and 22
 * bytes (including directory separators) BA/BB/BC/BD/BE/ABCDEFG
 *
 * @note Version 1.00 of the cache implementation used base64 to
 * encode this, however that did not meet the requirement for only
 * using uppercase characters.
 *
 * @note Versions prior to 1.30 only packed 5 bits per directory level
 * A/B/C/D/E/F/ABCDEFG which only required 19 characters to represent
 * but resulted in requiring an extra level of directory which is less
 * desirable than the three extra characters using six bits.
 *
 * @param state The store state to use.
 * @param ident The identifier to use.
 * @param elem_idx The element index.
 * @return The filename string or NULL on allocation error.
 */
स्थिर वर्ण *
store_fname(संरचना store_state *state,
	    प्रविष्टिः_ident_t ident,
	    पूर्णाङ्क elem_idx)
{
	वर्ण *fname = रिक्त;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	ध्रुव uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	ध्रुव वर्ण *आधारः_dir_सारणी[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	ध्रुव uint8_t encoding_सारणी[64][3] = {
		{ 'A',   0, 0 }, { 'B',   0, 0 }, /*  0 */
		{ 'C',   0, 0 }, { 'D',   0, 0 }, /*  2 */
		{ 'E',   0, 0 }, { 'F',   0, 0 }, /*  4 */
		{ 'G',   0, 0 }, { 'H',   0, 0 }, /*  6 */
		{ 'I',   0, 0 }, { 'J',   0, 0 }, /*  8 */
		{ 'K',   0, 0 }, { 'L',   0, 0 }, /* 10 */
		{ 'M',   0, 0 }, { 'N',   0, 0 }, /* 12 */
		{ 'O',   0, 0 }, { 'P',   0, 0 }, /* 14 */
		{ 'Q',   0, 0 }, { 'R',   0, 0 }, /* 16 */
		{ 'S',   0, 0 }, { 'T',   0, 0 }, /* 18 */
		{ 'U',   0, 0 }, { 'V',   0, 0 }, /* 20 */
		{ 'W',   0, 0 }, { 'X',   0, 0 }, /* 22 */
		{ 'Y',   0, 0 }, { 'Z',   0, 0 }, /* 24 */
		{ '2',   0, 0 }, { '3',   0, 0 }, /* 26 */
		{ '4',   0, 0 }, { '5',   0, 0 }, /* 28 */
		{ '6',   0, 0 }, { '7',   0, 0 }, /* 30 */
		{ 'B', 'A', 0 }, { 'B', 'B', 0 }, /* 32 */
		{ 'B', 'C', 0 }, { 'B', 'D', 0 }, /* 34 */
		{ 'B', 'E', 0 }, { 'B', 'F', 0 }, /* 36 */
		{ 'B', 'G', 0 }, { 'B', 'H', 0 }, /* 38 */
		{ 'B', 'I', 0 }, { 'B', 'J', 0 }, /* 40 */
		{ 'B', 'K', 0 }, { 'B', 'L', 0 }, /* 42 */
		{ 'B', 'M', 0 }, { 'B', 'N', 0 }, /* 44 */
		{ 'B', 'O', 0 }, { 'B', 'P', 0 }, /* 46 */
		{ 'B', 'Q', 0 }, { 'B', 'R', 0 }, /* 48 */
		{ 'B', 'S', 0 }, { 'B', 'T', 0 }, /* 50 */
		{ 'B', 'U', 0 }, { 'B', 'V', 0 }, /* 52 */
		{ 'B', 'W', 0 }, { 'B', 'X', 0 }, /* 54 */
		{ 'B', 'Y', 0 }, { 'B', 'Z', 0 }, /* 56 */
		{ 'B', '2', 0 }, { 'B', '3', 0 }, /* 58 */
		{ 'B', '4', 0 }, { 'B', '5', 0 }, /* 60 */
		{ 'B', '6', 0 }, { 'B', '7', 0 }  /* 62 */
	};

	/* base32 encode ident */
	b32u_i[0] = encoding_सारणी[(ident      ) & 0x1f][0];
	b32u_i[1] = encoding_सारणी[(ident >>  5) & 0x1f][0];
	b32u_i[2] = encoding_सारणी[(ident >> 10) & 0x1f][0];
	b32u_i[3] = encoding_सारणी[(ident >> 15) & 0x1f][0];
	b32u_i[4] = encoding_सारणी[(ident >> 20) & 0x1f][0];
	b32u_i[5] = encoding_सारणी[(ident >> 25) & 0x1f][0];
	b32u_i[6] = encoding_सारणी[(ident >> 30) & 0x1f][0];
	b32u_i[7] = 0; /* null terminate ident string */

	/* base32 encode directory separators */
	b32u_d[0] = (uint8_t*)आधारः_dir_सारणी[elem_idx];
	b32u_d[1] = &encoding_सारणी[(ident      ) & 0x3f][0];
	b32u_d[2] = &encoding_सारणी[(ident >>  6) & 0x3f][0];
	b32u_d[3] = &encoding_सारणी[(ident >> 12) & 0x3f][0];
	b32u_d[4] = &encoding_सारणी[(ident >> 18) & 0x3f][0];
	b32u_d[5] = &encoding_सारणी[(ident >> 24) & 0x3f][0];

	विकल्प (elem_idx) {
	पक्ष प्रविष्टिः_ELEM_दत्त:
	पक्ष प्रविष्टिः_ELEM_META:
		netsurf_mkpath(&fname, रिक्त, 8,
			       state->मार्गः, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		भङ्ग;

	पक्ष (प्रविष्टिः_ELEM_गणनम् + प्रविष्टिः_ELEM_META):
	पक्ष (प्रविष्टिः_ELEM_गणनम् + प्रविष्टिः_ELEM_दत्त):
		netsurf_mkpath(&fname, रिक्त, 3,
			       state->मार्गः, b32u_d[0], b32u_d[1]);
		भङ्ग;

	स्वभाव:
		प्रतिज्ञा("bad element index" == रिक्त);
		भङ्ग;
	}

	प्रत्यागम fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
invalidate_element(संरचना store_state *state,
		   संरचना store_प्रविष्टिः *bse,
		   पूर्णाङ्क elem_idx)
{
	यदि (bse->elem[elem_idx].खण्डम् != 0) {
		खण्डम्_सूचकांक_t bf;
		खण्डम्_सूचकांक_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].खण्डम् >> खण्डम्_प्रविष्टिः_गणनम्) &
			((1 << खण्डम्_FILE_गणनम्) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].खण्डम् & ((1U << खण्डम्_प्रविष्टिः_गणनम्) -1);

		/* clear bit in use map */
		state->खण्डानि[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} अन्यथा {
		वर्ण *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		यदि (fname == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
		unlink(fname);
		मुक्त(fname);
	}

	state->योग_alloc -= bse->elem[elem_idx].आकार;

	प्रत्यागम NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
invalidate_प्रविष्टिः(संरचना store_state *state, संरचना store_प्रविष्टिः *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= प्रविष्टिः_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	यदि (((bse->elem[प्रविष्टिः_ELEM_दत्त].flags &
	     (प्रविष्टिः_ELEM_FLAG_HEAP | प्रविष्टिः_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[प्रविष्टिः_ELEM_META].flags &
	      (प्रविष्टिः_ELEM_FLAG_HEAP | प्रविष्टिः_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		प्रत्यागम NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, प्रविष्टिः_ELEM_META);
	यदि (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, प्रविष्टिः_ELEM_दत्त);
	यदि (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	प्रत्यागम NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
स्थिर पूर्णाङ्क compar(ध्रुव शून्य *va, ध्रुव शून्य *vb)
{
	ध्रुव संरचना store_प्रविष्टिः *a = *(ध्रुव संरचना store_प्रविष्टिः **)va;
	ध्रुव संरचना store_प्रविष्टिः *b = *(ध्रुव संरचना store_प्रविष्टिः **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	यदि ((a->elem[प्रविष्टिः_ELEM_दत्त].flags == प्रविष्टिः_ELEM_FLAG_NONE) &&
	    (b->elem[प्रविष्टिः_ELEM_दत्त].flags != प्रविष्टिः_ELEM_FLAG_NONE)) {
		प्रत्यागम -1;
	} अन्यथा यदि ((a->elem[प्रविष्टिः_ELEM_दत्त].flags != प्रविष्टिः_ELEM_FLAG_NONE) &&
		   (b->elem[प्रविष्टिः_ELEM_दत्त].flags == प्रविष्टिः_ELEM_FLAG_NONE)) {
		प्रत्यागम 1;
	}

	यदि ((a->elem[प्रविष्टिः_ELEM_META].flags == प्रविष्टिः_ELEM_FLAG_NONE) &&
	    (b->elem[प्रविष्टिः_ELEM_META].flags != प्रविष्टिः_ELEM_FLAG_NONE)) {
		प्रत्यागम -1;
	} अन्यथा यदि ((a->elem[प्रविष्टिः_ELEM_META].flags != प्रविष्टिः_ELEM_FLAG_NONE) &&
		   (b->elem[प्रविष्टिः_ELEM_META].flags == प्रविष्टिः_ELEM_FLAG_NONE)) {
		प्रत्यागम 1;
	}

	यदि (a->use_गणनम् < b->use_गणनम्) {
		प्रत्यागम -1;
	} अन्यथा यदि (a->use_गणनम् > b->use_गणनम्) {
		प्रत्यागम 1;
	}
	/* use count is the same - now consider last use time */

	यदि (a->चरमः_प्रयुक्तम् < b->चरमः_प्रयुक्तम्) {
		प्रत्यागम -1;
	} अन्यथा यदि (a->चरमः_प्रयुक्तम् > b->चरमः_प्रयुक्तम्) {
		प्रत्यागम 1;
	}

	/* they are the same */
	प्रत्यागम 0;
}

प्रकारनिर्देश संरचना {
	संरचना store_प्रविष्टिः **elist;
	size_t ent_गणनम्;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
स्थिर bool
प्रविष्टिः_eviction_iterator_cb(शून्य *कुञ्चिका, शून्य *मानम्, शून्य *ctx)
{
	eviction_state_t *estate = ctx;
	संरचना store_प्रविष्टिः *ent = मानम्;
	estate->elist[estate->ent_गणनम्++] = ent;
	प्रत्यागम false;
}

/**
 * Evict entries from backing store as per configuration.
 *
 * Entries are evicted to ensure the cache remains within the
 * configured limits on size and number of entries.
 *
 * The approach is to check if the cache limits have been exceeded and
 * if so build and sort list of entries to evict. The list is sorted
 * by use count and then by age, so oldest object with least number of uses
 * get evicted first.
 *
 * @param state The store state to use.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror store_evict(संरचना store_state *state)
{
	size_t ent = 0;
	size_t removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	size_t old_गणनम्;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	यदि (state->योग_alloc < state->सीमा) {
		/* cache within limits */
		प्रत्यागम NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->योग_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_गणनम् = hashmap_गणनम्(state->entries);
	estate.ent_गणनम् = 0;
	estate.elist = स्मृतिं_देहि(परिमाण(संरचना state_प्रविष्टिः*) * old_गणनम्);
	यदि (estate.elist == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	यदि (hashmap_iterate(state->entries, प्रविष्टिः_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		मुक्त(estate.elist);
		प्रत्यागम NSERROR_UNKNOWN;
	}

	यदि (old_गणनम् != estate.ent_गणनम्) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		मुक्त(estate.elist);
		प्रत्यागम NSERROR_UNKNOWN;
	}

	शीघ्रं_क्रमय(estate.elist, estate.ent_गणनम्, परिमाण(संरचना state_प्रविष्टिः*), compar);

	/* evict entries in listed order */
	removed = 0;
	प्रति (ent = 0; ent < estate.ent_गणनम्; ent++) {
		संरचना store_प्रविष्टिः *bse = estate.elist[ent];

		removed += bse->elem[प्रविष्टिः_ELEM_दत्त].आकार;
		removed += bse->elem[प्रविष्टिः_ELEM_META].आकार;

		ret = invalidate_प्रविष्टिः(state, bse);
		यदि (ret != NSERROR_OK) {
			भङ्ग;
		}

		यदि (removed > state->hysteresis) {
			भङ्ग;
		}
	}

	मुक्त(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->योग_alloc, old_गणनम् - ent);

	प्रत्यागम ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
स्थिर nserror
write_प्रविष्टिः(संरचना store_प्रविष्टिः *ent, पूर्णाङ्क fd)
{
	uint32_t len = वाक्यदैर्घ्यम्(nsurl_access(ent->url));
	यदि (लिख(fd, &len, परिमाण(len)) != परिमाण(len))
		प्रत्यागम NSERROR_SAVE_FAILED;
	यदि (लिख(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		प्रत्यागम NSERROR_SAVE_FAILED;
	यदि (लिख(fd, ent, परिमाण(*ent)) != परिमाण(*ent))
		प्रत्यागम NSERROR_SAVE_FAILED;

	प्रत्यागम NSERROR_OK;
}

प्रकारनिर्देश संरचना {
	पूर्णाङ्क fd;
	size_t written;
} write_प्रविष्टिः_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
स्थिर bool
write_प्रविष्टिः_iterator(शून्य *कुञ्चिका, शून्य *मानम्, शून्य *ctx)
{
	/* We ignore the key */
	संरचना store_प्रविष्टिः *ent = मानम्;
	write_प्रविष्टिः_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	प्रत्यागम write_प्रविष्टिः(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror write_entries(संरचना store_state *state)
{
	वर्ण *tname = रिक्त; /* temporary file name for atomic replace */
	वर्ण *fname = रिक्त; /* target filename */
	write_प्रविष्टिः_iteration_state weistate;
	nserror ret;

	स्मृतिं_पूरय(&weistate, 0, परिमाण(weistate));

	यदि (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		प्रत्यागम NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, रिक्त, 2, state->मार्गः, "t"ENTRIES_FNAME);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	weistate.fd = उद्घाटय(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	यदि (weistate.fd == -1) {
		मुक्त(tname);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	यदि (hashmap_iterate(state->entries, write_प्रविष्टिः_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		close(weistate.fd);
		unlink(tname);
		मुक्त(tname);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	close(weistate.fd);

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, ENTRIES_FNAME);
	यदि (ret != NSERROR_OK) {
		unlink(tname);
		मुक्त(tname);
		प्रत्यागम ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(शून्य)remove(fname);
	यदि (rename(tname, fname) != 0) {
		unlink(tname);
		मुक्त(tname);
		मुक्त(fname);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	प्रत्यागम NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror write_खण्डानि(संरचना store_state *state)
{
	पूर्णाङ्क fd;
	वर्ण *tname = रिक्त; /* temporary file name for atomic replace */
	वर्ण *fname = रिक्त; /* target filename */
	size_t खण्डानि_आकार;
	size_t written = 0;
	size_t wr;
	nserror ret;
	पूर्णाङ्क bfidx; /* block file index */
	पूर्णाङ्क elem_idx;

	यदि (state->खण्डानि_dirty == false) {
		/* blocks use maps have not been updated since last write */
		प्रत्यागम NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, रिक्त, 2, state->मार्गः, "t"खण्डानि_FNAME);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	fd = उद्घाटय(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	यदि (fd == -1) {
		मुक्त(tname);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	खण्डानि_आकार = (खण्डम्_FILE_गणनम् * प्रविष्टिः_ELEM_गणनम्) * खण्डम्_USE_MAP_आकार;

	प्रति (elem_idx = 0; elem_idx < प्रविष्टिः_ELEM_गणनम्; elem_idx++) {
		प्रति (bfidx = 0; bfidx < खण्डम्_FILE_गणनम्; bfidx++) {
			wr = लिख(fd,
				   &state->खण्डानि[elem_idx][bfidx].use_map[0],
				   खण्डम्_USE_MAP_आकार);
			यदि (wr != खण्डम्_USE_MAP_आकार) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				गच्छ wr_err;
			}
			written += wr;
		}
	}
wr_err:
	close(fd);

	/* check all data was written */
	यदि (written != खण्डानि_आकार) {
		unlink(tname);
		मुक्त(tname);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, खण्डानि_FNAME);
	यदि (ret != NSERROR_OK) {
		unlink(tname);
		मुक्त(tname);
		प्रत्यागम ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(शून्य)remove(fname);
	यदि (rename(tname, fname) != 0) {
		unlink(tname);
		मुक्त(tname);
		मुक्त(fname);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * Ensures block files are of the correct extent
 *
 * block files have their extent set to their maximum size to ensure
 * subsequent reads and writes do not need to extend the file and are
 * therefore faster.
 *
 * \param state The backing store state to set block extent for.
 * \return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror समुच्चयः_खण्डम्_extents(संरचना store_state *state)
{
	पूर्णाङ्क bfidx; /* block file index */
	पूर्णाङ्क elem_idx;
	पूर्णाङ्क ftr;

	यदि (state->खण्डानि_opened == false) {
		/* no blocks have been opened since last write */
		प्रत्यागम NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	प्रति (elem_idx = 0; elem_idx < प्रविष्टिः_ELEM_गणनम्; elem_idx++) {
		प्रति (bfidx = 0; bfidx < खण्डम्_FILE_गणनम्; bfidx++) {
			यदि (state->खण्डानि[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->खण्डानि[elem_idx][bfidx].fd, 1U << (log2_खण्डम्_आकार[elem_idx] + खण्डम्_प्रविष्टिः_गणनम्));
				यदि (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      errno);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->खण्डानि_opened = false;

	प्रत्यागम NSERROR_OK;
}

/**
 * maintenance of control structures.
 *
 * callback scheduled when control data has been update. Currently
 * this is for when the entries table is dirty and requires
 * serialising.
 *
 * \param s store state to maintain.
 */
स्थिर शून्य control_maintenance(शून्य *s)
{
	संरचना store_state *state = s;

	write_entries(state);
	write_खण्डानि(state);
	समुच्चयः_खण्डम्_extents(state);
}


/**
 * Lookup a backing store entry in the entry table from a url.
 *
 * This finds the store entry associated with the given
 * key. Additionally if an entry is found it updates the usage data
 * about the entry.
 *
 * @param state The store state to use.
 * @param url The value used as the unique key to search entries for.
 * @param bse Pointer used to return value.
 * @return NSERROR_OK and bse updated on success or NSERROR_NOT_FOUND
 *         if no entry corresponds to the url.
 */
स्थिर nserror
get_store_प्रविष्टिः(संरचना store_state *state, nsurl *url, संरचना store_प्रविष्टिः **bse)
{
	संरचना store_प्रविष्टिः *ent;

	ent = hashmap_lookup(state->entries, url);

	यदि (ent == रिक्त) {
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	*bse = ent;

	ent->चरमः_प्रयुक्तम् = time(रिक्त);
	ent->use_गणनम्++;

	state->entries_dirty = true;

	guit->misc->कालसारणी(CONTROL_MAINT_TIME, control_maintenance, state);

	प्रत्यागम NSERROR_OK;
}

/**
 * Find next available small block.
 */
स्थिर खण्डम्_सूचकांक_t alloc_खण्डम्(संरचना store_state *state, पूर्णाङ्क elem_idx)
{
	पूर्णाङ्क bf;
	पूर्णाङ्क idx;
	पूर्णाङ्क bit;
	uint8_t *map;

	प्रति (bf = 0; bf < खण्डम्_FILE_गणनम्; bf++) {
		map = &state->खण्डानि[elem_idx][bf].use_map[0];

		प्रति (idx = 0; idx < खण्डम्_USE_MAP_आकार; idx++) {
			यदि (*(map + idx) != 0xff) {
				/* located an unused block */
				प्रति (bit = 0; bit < 8;bit++) {
					यदि (((*(map + idx)) & (1U << bit)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << bit;
						state->खण्डानि_dirty = true;
						प्रत्यागम (((bf * खण्डम्_USE_MAP_आकार) + idx) * 8) + bit;
					}
				}
			}
		}
	}

	प्रत्यागम 0;
}

/**
 * Set a backing store entry in the entry table from a url.
 *
 * This creates a backing store entry in the entry table for a url.
 *
 * @param state The store state to use.
 * @param url The value used as the unique key to search entries for.
 * @param elem_idx The index of the entry element to use.
 * @param data The data to store
 * @param datalen The length of data in \a data
 * @param bse Pointer used to return value.
 * @return NSERROR_OK and \a bse updated on success or NSERROR_NOT_FOUND
 *         if no entry corresponds to the url.
 */
स्थिर nserror
समुच्चयः_store_प्रविष्टिः(संरचना store_state *state,
		nsurl *url,
		पूर्णाङ्क elem_idx,
		uint8_t *दत्त,
		ध्रुव size_t datalen,
		संरचना store_प्रविष्टिः **bse)
{
	संरचना store_प्रविष्टिः *se;
	nserror ret;
	संरचना store_प्रविष्टिः_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	se = hashmap_lookup(state->entries, url);
	यदि (se == रिक्त) {
		se = hashmap_निवेशय(state->entries, url);
	}
	यदि (se == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	यदि ((elem->flags & (प्रविष्टिः_ELEM_FLAG_HEAP | प्रविष्टिः_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		प्रत्यागम NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_गणनम् = 1;
	se->चरमः_प्रयुक्तम् = time(रिक्त);

	/* store the data in the element */
	elem->flags |= प्रविष्टिः_ELEM_FLAG_HEAP;
	elem->दत्त = दत्त;
	elem->ref = 1;

	/* account for size of entry element */
	state->योग_alloc -= elem->आकार;
	elem->आकार = datalen;
	state->योग_alloc += elem->आकार;

	/* if the element will fit in a small block attempt to allocate one */
	यदि (elem->आकार <= (1U << log2_खण्डम्_आकार[elem_idx])) {
		elem->खण्डम् = alloc_खण्डम्(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->कालसारणी(CONTROL_MAINT_TIME, control_maintenance, state);

	*bse = se;

	प्रत्यागम NSERROR_OK;
}


/**
 * Open a file using a store ident.
 *
 * @param state The store state to use.
 * @param ident The identifier to open file for.
 * @param elem_idx The element within the store entry to open. The
 *                 value should be be one of the values in the
 *                 store_entry_elem_idx enum. Additionally it may have
 *                 ENTRY_ELEM_COUNT added to it to indicate block file
 *                 names.
 * @param openflags The flags used with the open call.
 * @return An fd from the open call or -1 on error.
 */
स्थिर पूर्णाङ्क
store_open(संरचना store_state *state,
	   प्रविष्टिः_ident_t ident,
	   पूर्णाङ्क elem_idx,
	   पूर्णाङ्क openflags)
{
	वर्ण *fname;
	nserror ret;
	पूर्णाङ्क fd;

	fname = store_fname(state, ident, elem_idx);
	यदि (fname == रिक्त) {
		NSLOG(netsurf, ERROR, "filename error");
		प्रत्यागम -1;
	}

	/* ensure all path elements to file exist if creating file */
	यदि (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		यदि (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			मुक्त(fname);
			प्रत्यागम -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = उद्घाटय(fname, openflags, S_IRUSR | S_IWUSR);

	मुक्त(fname);

	प्रत्यागम fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
unlink_entries(संरचना store_state *state)
{
	वर्ण *fname = रिक्त;
	nserror ret;

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, ENTRIES_FNAME);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	unlink(fname);

	मुक्त(fname);
	प्रत्यागम NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
स्थिर nserror
read_entries(संरचना store_state *state)
{
	वर्ण *fname = रिक्त;
	वर्ण *url;
	nsurl *nsurl;
	nserror ret;
	size_t read_entries = 0;
	संरचना store_प्रविष्टिः *ent;
	पूर्णाङ्क fd;

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, ENTRIES_FNAME);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	यदि (state->entries == रिक्त) {
		मुक्त(fname);
		प्रत्यागम NSERROR_NOMEM;
	}

	fd = उद्घाटय(fname, O_RDWR);
	यदि (fd != -1) {
		uint32_t urllen;
		यावत् (पठ(fd, &urllen, परिमाण(urllen)) == परिमाण(urllen)) {
			url = शून्यस्मृतिं_देहि(1, urllen+1);
			यदि (url == रिक्त) {
				close(fd);
				मुक्त(fname);
				प्रत्यागम NSERROR_NOMEM;
			}
			यदि (पठ(fd, url, urllen) != (ssize_t)urllen) {
				मुक्त(url);
				close(fd);
				मुक्त(fname);
				प्रत्यागम NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			यदि (ret != NSERROR_OK) {
				मुक्त(url);
				close(fd);
				मुक्त(fname);
				प्रत्यागम ret;
			}
			मुक्त(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_निवेशय(state->entries, nsurl);
			यदि (ent == रिक्त) {
				nsurl_unref(nsurl);
				close(fd);
				मुक्त(fname);
				प्रत्यागम NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			यदि (पठ(fd, ent, परिमाण(*ent)) != परिमाण(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				close(fd);
				मुक्त(fname);
				प्रत्यागम NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->योग_alloc += ent->elem[प्रविष्टिः_ELEM_दत्त].आकार;
			state->योग_alloc += ent->elem[प्रविष्टिः_ELEM_META].आकार;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[प्रविष्टिः_ELEM_दत्त].flags &= ~(प्रविष्टिः_ELEM_FLAG_HEAP | प्रविष्टिः_ELEM_FLAG_MMAP);
			ent->elem[प्रविष्टिः_ELEM_META].flags &= ~(प्रविष्टिः_ELEM_FLAG_HEAP | प्रविष्टिः_ELEM_FLAG_MMAP);

		}
		close(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	मुक्त(fname);
	प्रत्यागम NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
read_खण्डानि(संरचना store_state *state)
{
	पूर्णाङ्क bfidx; /* block file index */
	पूर्णाङ्क elem_idx;
	पूर्णाङ्क fd;
	ssize_t rd;
	वर्ण *fname = रिक्त;
	nserror ret;

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, खण्डानि_FNAME);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = उद्घाटय(fname, O_RDWR);
	मुक्त(fname);
	यदि (fd != -1) {
		/* initialise block file use array */
		प्रति (elem_idx = 0; elem_idx < प्रविष्टिः_ELEM_गणनम्; elem_idx++) {
			प्रति (bfidx = 0; bfidx < खण्डम्_FILE_गणनम्; bfidx++) {
				rd = पठ(fd,
					  &state->खण्डानि[elem_idx][bfidx].use_map[0],
					  खण्डम्_USE_MAP_आकार);
				यदि (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					गच्छ rd_err;
				}
			}
		}
	rd_err:
		close(fd);

	} अन्यथा {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->खण्डानि[प्रविष्टिः_ELEM_दत्त][0].use_map[0] = 1;
		state->खण्डानि[प्रविष्टिः_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	प्रति (bfidx = 0; bfidx < खण्डम्_FILE_गणनम्; bfidx++) {
		state->खण्डानि[प्रविष्टिः_ELEM_दत्त][bfidx].fd = -1;
		state->खण्डानि[प्रविष्टिः_ELEM_META][bfidx].fd = -1;
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
write_cache_चिह्नकम्(संरचना store_state *state)
{
	FILE *fcachetag;
	nserror ret;
	वर्ण *fname = रिक्त;

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, "CACHEDIR.TAG");
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	fcachetag = fopen(fname, "wb");

	मुक्त(fname);

	यदि (fcachetag == रिक्त) {
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	fprintf(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	fclose(fcachetag);

	प्रत्यागम NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
write_control(संरचना store_state *state)
{
	FILE *fcontrol;
	nserror ret;
	वर्ण *fname = रिक्त;

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, "control");
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	यदि (ret != NSERROR_OK) {
		मुक्त(fname);
		प्रत्यागम ret;
	}

	fcontrol = fopen(fname, "wb");

	मुक्त(fname);

	यदि (fcontrol == रिक्त) {
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	fprintf(fcontrol, "%u%c", CONTROL_संस्करणम्, 0);

	fclose(fcontrol);

	प्रत्यागम NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
read_control(संरचना store_state *state)
{
	nserror ret;
	FILE *fcontrol;
	अचिह्नित पूर्णाङ्क ctrlversion;
	वर्ण *fname = रिक्त;

	ret = netsurf_mkpath(&fname, रिक्त, 2, state->मार्गः, "control");
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = fopen(fname, "rb");

	मुक्त(fname);

	यदि (fcontrol == रिक्त) {
		/* unable to open control file */
		यदि (errno == ENOENT) {
			प्रत्यागम NSERROR_NOT_प्राप्तम्;
		} अन्यथा {
			प्रत्यागम NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	यदि (fscanf(fcontrol, "%u", &ctrlversion) != 1) {
		गच्छ control_error;
	}

	यदि (ctrlversion != CONTROL_संस्करणम्) {
		गच्छ control_error;
	}

	यदि (fgetc(fcontrol) != 0) {
		गच्छ control_error;
	}

	fclose(fcontrol);

	प्रत्यागम NSERROR_OK;

control_error: /* problem with the control file */

	fclose(fcontrol);

	प्रत्यागम NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
initialise(ध्रुव संरचना llcache_store_parameters *parameters)
{
	संरचना store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	यदि (storestate != रिक्त) {
		प्रत्यागम NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	यदि (parameters->सीमा == 0) {
		प्रत्यागम NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	यदि (parameters->मार्गः == रिक्त) {
		प्रत्यागम NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = शून्यस्मृतिं_देहि(1, परिमाण(संरचना store_state));
	यदि (newstate == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	newstate->मार्गः = strdup(parameters->मार्गः);
	newstate->सीमा = parameters->सीमा;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	यदि (ret != NSERROR_OK) {
		यदि (ret == NSERROR_NOT_प्राप्तम्) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} अन्यथा {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->मार्गः);
			यदि (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->मार्गः);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} अन्यथा {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->मार्गः);
			}
		}
		ret = write_control(newstate);
		यदि (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_चिह्नकम्(newstate);
		}
	}
	यदि (ret != NSERROR_OK) {
		/* that went well obviously */
		मुक्त(newstate->मार्गः);
		मुक्त(newstate);
		प्रत्यागम ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	यदि (ret != NSERROR_OK) {
		/* that went well obviously */
		मुक्त(newstate->मार्गः);
		मुक्त(newstate);
		प्रत्यागम ret;
	}

	/* read blocks */
	ret = read_खण्डानि(newstate);
	यदि (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		मुक्त(newstate->मार्गः);
		मुक्त(newstate);
		प्रत्यागम ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->मार्गः,
	      newstate->सीमा,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->योग_alloc, newstate->सीमा);

	प्रत्यागम NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
स्थिर nserror
finalise(शून्य)
{
	पूर्णाङ्क bf; /* block file index */
	अचिह्नित पूर्णाङ्क op_गणनम्;

	यदि (storestate != रिक्त) {
		guit->misc->कालसारणी(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_खण्डानि(storestate);

		/* ensure all block files are closed */
		प्रति (bf = 0; bf < खण्डम्_FILE_गणनम्; bf++) {
			यदि (storestate->खण्डानि[प्रविष्टिः_ELEM_दत्त][bf].fd != -1) {
				close(storestate->खण्डानि[प्रविष्टिः_ELEM_दत्त][bf].fd);
			}
			यदि (storestate->खण्डानि[प्रविष्टिः_ELEM_META][bf].fd != -1) {
				close(storestate->खण्डानि[प्रविष्टिः_ELEM_META][bf].fd);
			}
		}

		op_गणनम् = storestate->लब्धम्_गणनम् + storestate->अलब्धिः_गणनम्;

		/* avoid division by zero */
		यदि (op_गणनम् > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_गणनम्,
			      storestate->लब्धम्_गणनम्,
			      storestate->अलब्धिः_गणनम्,
			      0,
			      (storestate->लब्धम्_गणनम् * 100) / op_गणनम्,
			      (storestate->अलब्धिः_गणनम् * 100) / op_गणनम्,
			      0);
		}

		hashmap_destroy(storestate->entries);
		मुक्त(storestate->मार्गः);
		मुक्त(storestate);
		storestate = रिक्त;
	}
	प्रत्यागम NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
स्थिर nserror store_write_खण्डम्(संरचना store_state *state,
			 संरचना store_प्रविष्टिः *bse,
			 पूर्णाङ्क elem_idx)
{
	खण्डम्_सूचकांक_t bf = (bse->elem[elem_idx].खण्डम् >> खण्डम्_प्रविष्टिः_गणनम्) &
		((1 << खण्डम्_FILE_गणनम्) - 1); /* block file block resides in */
	खण्डम्_सूचकांक_t bi = bse->elem[elem_idx].खण्डम् & ((1U << खण्डम्_प्रविष्टिः_गणनम्) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	यदि (state->खण्डानि[elem_idx][bf].fd == -1) {
		state->खण्डानि[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + प्रविष्टिः_ELEM_गणनम्, O_CREAT | O_RDWR);
		यदि (state->खण्डानि[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			प्रत्यागम NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->खण्डानि_opened = true;
	}

	offst = (अचिह्नित पूर्णाङ्क)bi << log2_खण्डम्_आकार[elem_idx];

	wr = nsu_pwrite(state->खण्डानि[elem_idx][bf].fd,
			bse->elem[elem_idx].दत्त,
			bse->elem[elem_idx].आकार,
			offst);
	यदि (wr != (ssize_t)bse->elem[elem_idx].आकार) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].आकार,
		      bse->elem[elem_idx].दत्त,
		      (size_t)offst,
		      bse->elem[elem_idx].खण्डम्,
		      errno);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].दत्त, (size_t)offst,
	      bse->elem[elem_idx].खण्डम्);

	प्रत्यागम NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
स्थिर nserror store_write_file(संरचना store_state *state,
			 संरचना store_प्रविष्टिः *bse,
			 पूर्णाङ्क elem_idx)
{
	ssize_t wr;
	पूर्णाङ्क fd;
	पूर्णाङ्क err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	यदि (fd < 0) {
		perror("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	wr = लिख(fd, bse->elem[elem_idx].दत्त, bse->elem[elem_idx].आकार);
	err = errno; /* close can change errno */

	close(fd);
	यदि (wr != (ssize_t)bse->elem[elem_idx].आकार) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].आकार,
		      bse->elem[elem_idx].दत्त,
		      err);

		/** @todo Delete the file? */
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].दत्त);

	प्रत्यागम NSERROR_OK;
}

/**
 * Place an object in the backing store.
 *
 * takes ownership of the heap block passed in.
 *
 * @param url The url is used as the unique primary key for the data.
 * @param bsflags The flags to control how the object is stored.
 * @param data The objects source data.
 * @param datalen The length of the \a data.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
store(nsurl *url,
      गणना backing_store_flags bsflags,
      uint8_t *दत्त,
      ध्रुव size_t datalen)
{
	nserror ret;
	संरचना store_प्रविष्टिः *bse;
	पूर्णाङ्क elem_idx;

	/* check backing store is initialised */
	यदि (storestate == रिक्त) {
		प्रत्यागम NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	यदि ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = प्रविष्टिः_ELEM_META;
	} अन्यथा {
		elem_idx = प्रविष्टिः_ELEM_दत्त;
	}

	/* set the store entry up */
	ret = समुच्चयः_store_प्रविष्टिः(storestate, url, elem_idx, दत्त, datalen, &bse);
	यदि (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		प्रत्यागम ret;
	}

	यदि (bse->elem[elem_idx].खण्डम् != 0) {
		/* small block storage */
		ret = store_write_खण्डम्(storestate, bse, elem_idx);
	} अन्यथा {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	प्रत्यागम ret;
}

/**
 * release any allocation for an entry
 */
स्थिर nserror प्रविष्टिः_release_alloc(संरचना store_प्रविष्टिः_element *elem)
{
	यदि ((elem->flags & प्रविष्टिः_ELEM_FLAG_HEAP) != 0) {
		elem->ref--;
		यदि (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->दत्त);
			मुक्त(elem->दत्त);
			elem->flags &= ~प्रविष्टिः_ELEM_FLAG_HEAP;
		}
	}
	प्रत्यागम NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
स्थिर nserror store_read_खण्डम्(संरचना store_state *state,
			 संरचना store_प्रविष्टिः *bse,
			 पूर्णाङ्क elem_idx)
{
	खण्डम्_सूचकांक_t bf = (bse->elem[elem_idx].खण्डम् >> खण्डम्_प्रविष्टिः_गणनम्) &
		((1 << खण्डम्_FILE_गणनम्) - 1); /* block file block resides in */
	खण्डम्_सूचकांक_t bi = bse->elem[elem_idx].खण्डम् & ((1 << खण्डम्_प्रविष्टिः_गणनम्) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	यदि (state->खण्डानि[elem_idx][bf].fd == -1) {
		state->खण्डानि[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + प्रविष्टिः_ELEM_गणनम्, O_CREAT | O_RDWR);
		यदि (state->खण्डानि[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			प्रत्यागम NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->खण्डानि_opened = true;
	}

	offst = (अचिह्नित पूर्णाङ्क)bi << log2_खण्डम्_आकार[elem_idx];

	rd = nsu_pread(state->खण्डानि[elem_idx][bf].fd,
		       bse->elem[elem_idx].दत्त,
		       bse->elem[elem_idx].आकार,
		       offst);
	यदि (rd != (ssize_t)bse->elem[elem_idx].आकार) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].आकार,
		      bse->elem[elem_idx].दत्त,
		      (size_t)offst,
		      bse->elem[elem_idx].खण्डम्,
		      errno);
		प्रत्यागम NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].दत्त, (size_t)offst,
	      bse->elem[elem_idx].खण्डम्);

	प्रत्यागम NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
स्थिर nserror store_read_file(संरचना store_state *state,
			 संरचना store_प्रविष्टिः *bse,
			 पूर्णाङ्क elem_idx)
{
	पूर्णाङ्क fd;
	ssize_t rd; /* return from read */
	पूर्णाङ्क ret = NSERROR_OK;
	size_t tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	यदि (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		/** @todo should this invalidate the entry? */
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	यावत् (tot < bse->elem[elem_idx].आकार) {
		rd = पठ(fd,
			  bse->elem[elem_idx].दत्त + tot,
			  bse->elem[elem_idx].आकार - tot);
		यदि (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      errno);
			ret = NSERROR_NOT_प्राप्तम्;
			भङ्ग;
		}
		tot += rd;
	}

	close(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].दत्त);

	प्रत्यागम ret;
}

/**
 * Retrieve an object from the backing store.
 *
 * @param[in] url The url is used as the unique primary key for the data.
 * @param[in] bsflags The flags to control how the object is retrieved.
 * @param[out] data_out The objects data.
 * @param[out] datalen_out The length of the \a data retrieved.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
fetch(nsurl *url,
      गणना backing_store_flags bsflags,
      uint8_t **दत्त_बहिर्गमनम्,
      size_t *datalen_बहिर्गमनम्)
{
	nserror ret;
	संरचना store_प्रविष्टिः *bse;
	संरचना store_प्रविष्टिः_element *elem;
	पूर्णाङ्क elem_idx;

	/* check backing store is initialised */
	यदि (storestate == रिक्त) {
		प्रत्यागम NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_प्रविष्टिः(storestate, url, &bse);
	यदि (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->अलब्धिः_गणनम्++;
		प्रत्यागम ret;
	}
	storestate->लब्धम्_गणनम्++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	यदि ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = प्रविष्टिः_ELEM_META;
	} अन्यथा {
		elem_idx = प्रविष्टिः_ELEM_दत्त;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	यदि ((elem->flags & प्रविष्टिः_ELEM_FLAG_HEAP) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->दत्त, elem->ref);

	} अन्यथा {
		/* allocate from the heap */
		elem->दत्त = स्मृतिं_देहि(elem->आकार);
		यदि (elem->दत्त == रिक्त) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			प्रत्यागम NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->दत्त);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= प्रविष्टिः_ELEM_FLAG_HEAP;
		elem->ref = 1;

		/* fill the new block */
		यदि (elem->खण्डम् != 0) {
			ret = store_read_खण्डम्(storestate, bse, elem_idx);
		} अन्यथा {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	यदि (ret != NSERROR_OK) {
		प्रविष्टिः_release_alloc(elem);
	} अन्यथा {
		/* update stats and setup return pointers */
		storestate->लब्धम्_आकार += elem->आकार;

		*दत्त_बहिर्गमनम् = elem->दत्त;
		*datalen_बहिर्गमनम् = elem->आकार;
	}

	प्रत्यागम ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror release(nsurl *url, गणना backing_store_flags bsflags)
{
	nserror ret;
	संरचना store_प्रविष्टिः *bse;
	संरचना store_प्रविष्टिः_element *elem;

	/* check backing store is initialised */
	यदि (storestate == रिक्त) {
		प्रत्यागम NSERROR_INIT_FAILED;
	}

	ret = get_store_प्रविष्टिः(storestate, url, &bse);
	यदि (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		प्रत्यागम ret;
	}

	/* the entry element */
	यदि ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[प्रविष्टिः_ELEM_META];
	} अन्यथा {
		elem = &bse->elem[प्रविष्टिः_ELEM_दत्त];
	}

	ret = प्रविष्टिः_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	यदि ((ret == NSERROR_OK) &&
	    ((bse->flags & प्रविष्टिः_FLAGS_INVALID) != 0)) {
		ret = invalidate_प्रविष्टिः(storestate, bse);
	}

	प्रत्यागम ret;
}


/**
 * Invalidate a source object from the backing store.
 *
 * The entry (if present in the backing store) must no longer
 * be returned as a result to the fetch or meta operations.
 *
 * @param url The url is used as the unique primary key to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
स्थिर nserror
invalidate(nsurl *url)
{
	nserror ret;
	संरचना store_प्रविष्टिः *bse;

	/* check backing store is initialised */
	यदि (storestate == रिक्त) {
		प्रत्यागम NSERROR_INIT_FAILED;
	}

	ret = get_store_प्रविष्टिः(storestate, url, &bse);
	यदि (ret != NSERROR_OK) {
		प्रत्यागम ret;
	}

	प्रत्यागम invalidate_प्रविष्टिः(storestate, bse);
}


स्थिर संरचना gui_llcache_सारणी llcache_सारणी = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

संरचना gui_llcache_सारणी *filesystem_llcache_सारणी = &llcache_सारणी;
