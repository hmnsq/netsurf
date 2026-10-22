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

#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <nsutils/unistd.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/filepath.h"
#ಅನ್ತರ್ಭಾವ "utils/file.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/hashmap.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"

#ಅನ್ತರ್ಭಾವ "content/backing_store.h"

/** Backing store file format version */
#ವ್ಯಾಖ್ಯಾನಿಸು CONTROL_ಆವೃತ್ತಿ 202

/**
 * Number of milliseconds after a update before control data
 * maintenance is performed
 */
#ವ್ಯಾಖ್ಯಾನಿಸು CONTROL_MAINT_ಸಮಯ 10000

/** Filename of serialised entries */
#ವ್ಯಾಖ್ಯಾನಿಸು ENTRIES_FNAME "entries"

/** Filename of block file index */
#ವ್ಯಾಖ್ಯಾನಿಸು ಖಂಡಗಳು_FNAME "blocks"

/** log2 block data address length (64k) */
#ವ್ಯಾಖ್ಯಾನಿಸು ಖಂಡ_ADDR_LEN 16

/** log2 number of entries per block file(1024) */
#ವ್ಯಾಖ್ಯಾನಿಸು ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ 10

/** log2 number of data block files */
#ವ್ಯಾಖ್ಯಾನಿಸು ಖಂಡ_FILE_ಎಣಿಕೆ (ಖಂಡ_ADDR_LEN - ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ)

/** log2 size of data blocks (8k) */
#ವ್ಯಾಖ್ಯಾನಿಸು ಖಂಡ_ದತ್ತಾಂಶ_ಗಾತ್ರ 13

/** log2 size of metadata blocks (8k) */
#ವ್ಯಾಖ್ಯಾನಿಸು ಖಂಡ_META_ಗಾತ್ರ 13

/** length in bytes of a block files use map */
#ವ್ಯಾಖ್ಯಾನಿಸು ಖಂಡ_USE_MAP_ಗಾತ್ರ (1 << (ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uint32_t ನಮೂದು_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uint16_t ಖಂಡ_ಸೂಚ್ಯಂಕ_t;


/**
 * Entry element index values.
 */
ಎಣಿಕೆ store_ನಮೂದು_elem_idx {
	ನಮೂದು_ELEM_ದತ್ತಾಂಶ = 0, /**< entry element is data */
	ನಮೂದು_ELEM_META = 1,  /**< entry element is metadata */
	ನಮೂದು_ELEM_ಎಣಿಕೆ = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
ಎಣಿಕೆ store_ನಮೂದು_elem_flags {
	/** store not managing any allocation on entry */
	ನಮೂದು_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	ನಮೂದು_ELEM_FLAG_ರಾಶಿ = 0x1,
	/** entry data allocation is mmaped */
	ನಮೂದು_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	ನಮೂದು_ELEM_FLAG_SMALL = 0x4,
};


ಎಣಿಕೆ store_ನಮೂದು_flags {
	/** entry is normal */
	ನಮೂದು_FLAGS_NONE = 0,
	/** entry has been invalidated but something still holding a reference */
	ನಮೂದು_FLAGS_INVALID = 1,
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
ರಚನೆ store_ನಮೂದು_element {
	uint8_t* ದತ್ತಾಂಶ; /**< data allocated */
	uint32_t ಗಾತ್ರ_2; /**< size of entry element on disc */
	ಖಂಡ_ಸೂಚ್ಯಂಕ_t ಖಂಡ; /**< small object data block */
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
ರಚನೆ store_ನಮೂದು {
	nsurl *url; /**< The URL for this entry */
	int64_t ಕೊನೆಯ_ಬಳಸಿದ; /**< UNIX time the entry was last used */
	uint16_t use_ಎಣಿಕೆ; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	ರಚನೆ store_ನಮೂದು_element elem[ನಮೂದು_ELEM_ಎಣಿಕೆ];
};

/**
 * Small block file.
 */
ರಚನೆ ಖಂಡ_file {
	/** file descriptor of the block file */
	ಪೂರ್ಣಾಂಕ fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[ಖಂಡ_USE_MAP_ಗಾತ್ರ];
};

/**
 * log2 of block size.
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ log2_ಖಂಡ_ಗಾತ್ರ[ನಮೂದು_ELEM_ಎಣಿಕೆ] = {
	ಖಂಡ_ದತ್ತಾಂಶ_ಗಾತ್ರ, /**< Data block size */
	ಖಂಡ_META_ಗಾತ್ರ  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
ರಚನೆ store_state {
	/* store config */
	ಅಕ್ಷರ *ಮಾರ್ಗ; /**< The path to the backing store */
	size_t ಎಲ್ಲೆ; /**< The backing store upper bound target size */
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
	ರಚನೆ ಖಂಡ_file ಖಂಡಗಳು[ನಮೂದು_ELEM_ಎಣಿಕೆ][ಖಂಡ_FILE_ಎಣಿಕೆ];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool ಖಂಡಗಳು_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool ಖಂಡಗಳು_opened;


	/* stats */
	uint64_t ಒಟ್ಟು_alloc; /**< total size of all allocated storage. */

	size_t ದೊರಕಿತು_ಎಣಿಕೆ; /**< number of cache hits */
	uint64_t ದೊರಕಿತು_ಗಾತ್ರ; /**< size of storage served */
	size_t ತಪ್ಪಿದ_ಎಣಿಕೆ; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
ರಚನೆ store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

ಸ್ಥಿರ bool
entries_hashmap_ಕೀಲಿ_eq(ಶೂನ್ಯ *key1, ಶೂನ್ಯ *key2)
{
	ಹಿಂತಿರುಗಿ nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

ಸ್ಥಿರ ಶೂನ್ಯ *
entries_hashmap_ಮೌಲ್ಯ_alloc(ಶೂನ್ಯ *ಕೀಲಿ)
{
	ರಚನೆ store_ನಮೂದು *ent = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ store_ನಮೂದು));
	ಆದರೆ (ent != ಖಾಲಿ) {
		ent->url = nsurl_ref(ಕೀಲಿ);
	}
	ಹಿಂತಿರುಗಿ ent;
}

ಸ್ಥಿರ ಶೂನ್ಯ
entries_hashmap_ಮೌಲ್ಯ_destroy(ಶೂನ್ಯ *ಮೌಲ್ಯ)
{
	ರಚನೆ store_ನಮೂದು *ent = ಮೌಲ್ಯ;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	ಬಿಡುಗಡೆ(ent);
}

ಸ್ಥಿರ hashmap_parameters_t entries_hashmap_parameters = {
	.ಕೀಲಿ_clone = (hashmap_ಕೀಲಿ_clone_t)nsurl_ref,
	.ಕೀಲಿ_destroy = (hashmap_ಕೀಲಿ_destroy_t)nsurl_unref,
	.ಕೀಲಿ_hash = (hashmap_ಕೀಲಿ_hash_t)nsurl_hash,
	.ಕೀಲಿ_eq = entries_hashmap_ಕೀಲಿ_eq,
	.ಮೌಲ್ಯ_alloc = entries_hashmap_ಮೌಲ್ಯ_alloc,
	.ಮೌಲ್ಯ_destroy = entries_hashmap_ಮೌಲ್ಯ_destroy,
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
ಸ್ಥಿರ ಅಕ್ಷರ *
store_fname(ರಚನೆ store_state *state,
	    ನಮೂದು_ident_t ident,
	    ಪೂರ್ಣಾಂಕ elem_idx)
{
	ಅಕ್ಷರ *fname = ಖಾಲಿ;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	ಸ್ಥಿರಾಂಕ uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ_dir_ಕೋಷ್ಟಕ[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	ಸ್ಥಿರಾಂಕ uint8_t encoding_ಕೋಷ್ಟಕ[64][3] = {
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
	b32u_i[0] = encoding_ಕೋಷ್ಟಕ[(ident      ) & 0x1f][0];
	b32u_i[1] = encoding_ಕೋಷ್ಟಕ[(ident >>  5) & 0x1f][0];
	b32u_i[2] = encoding_ಕೋಷ್ಟಕ[(ident >> 10) & 0x1f][0];
	b32u_i[3] = encoding_ಕೋಷ್ಟಕ[(ident >> 15) & 0x1f][0];
	b32u_i[4] = encoding_ಕೋಷ್ಟಕ[(ident >> 20) & 0x1f][0];
	b32u_i[5] = encoding_ಕೋಷ್ಟಕ[(ident >> 25) & 0x1f][0];
	b32u_i[6] = encoding_ಕೋಷ್ಟಕ[(ident >> 30) & 0x1f][0];
	b32u_i[7] = 0; /* null terminate ident string */

	/* base32 encode directory separators */
	b32u_d[0] = (uint8_t*)ತಳ_dir_ಕೋಷ್ಟಕ[elem_idx];
	b32u_d[1] = &encoding_ಕೋಷ್ಟಕ[(ident      ) & 0x3f][0];
	b32u_d[2] = &encoding_ಕೋಷ್ಟಕ[(ident >>  6) & 0x3f][0];
	b32u_d[3] = &encoding_ಕೋಷ್ಟಕ[(ident >> 12) & 0x3f][0];
	b32u_d[4] = &encoding_ಕೋಷ್ಟಕ[(ident >> 18) & 0x3f][0];
	b32u_d[5] = &encoding_ಕೋಷ್ಟಕ[(ident >> 24) & 0x3f][0];

	ಆಯ್ಕೆ (elem_idx) {
	ಸ್ಥಿತಿ ನಮೂದು_ELEM_ದತ್ತಾಂಶ:
	ಸ್ಥಿತಿ ನಮೂದು_ELEM_META:
		netsurf_mkpath(&fname, ಖಾಲಿ, 8,
			       state->ಮಾರ್ಗ, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ (ನಮೂದು_ELEM_ಎಣಿಕೆ + ನಮೂದು_ELEM_META):
	ಸ್ಥಿತಿ (ನಮೂದು_ELEM_ಎಣಿಕೆ + ನಮೂದು_ELEM_ದತ್ತಾಂಶ):
		netsurf_mkpath(&fname, ಖಾಲಿ, 3,
			       state->ಮಾರ್ಗ, b32u_d[0], b32u_d[1]);
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ಪ್ರತಿಜ್ಞಾ("bad element index" == ಖಾಲಿ);
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
invalidate_element(ರಚನೆ store_state *state,
		   ರಚನೆ store_ನಮೂದು *bse,
		   ಪೂರ್ಣಾಂಕ elem_idx)
{
	ಆದರೆ (bse->elem[elem_idx].ಖಂಡ != 0) {
		ಖಂಡ_ಸೂಚ್ಯಂಕ_t bf;
		ಖಂಡ_ಸೂಚ್ಯಂಕ_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].ಖಂಡ >> ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ) &
			((1 << ಖಂಡ_FILE_ಎಣಿಕೆ) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].ಖಂಡ & ((1U << ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ) -1);

		/* clear bit in use map */
		state->ಖಂಡಗಳು[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಕ್ಷರ *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		ಆದರೆ (fname == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		unlink(fname);
		ಬಿಡುಗಡೆ(fname);
	}

	state->ಒಟ್ಟು_alloc -= bse->elem[elem_idx].ಗಾತ್ರ_2;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
invalidate_ನಮೂದು(ರಚನೆ store_state *state, ರಚನೆ store_ನಮೂದು *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= ನಮೂದು_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	ಆದರೆ (((bse->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].flags &
	     (ನಮೂದು_ELEM_FLAG_ರಾಶಿ | ನಮೂದು_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[ನಮೂದು_ELEM_META].flags &
	      (ನಮೂದು_ELEM_FLAG_ರಾಶಿ | ನಮೂದು_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, ನಮೂದು_ELEM_META);
	ಆದರೆ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, ನಮೂದು_ELEM_ದತ್ತಾಂಶ);
	ಆದರೆ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ compar(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *va, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *vb)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ store_ನಮೂದು *a = *(ಸ್ಥಿರಾಂಕ ರಚನೆ store_ನಮೂದು **)va;
	ಸ್ಥಿರಾಂಕ ರಚನೆ store_ನಮೂದು *b = *(ಸ್ಥಿರಾಂಕ ರಚನೆ store_ನಮೂದು **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	ಆದರೆ ((a->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].flags == ನಮೂದು_ELEM_FLAG_NONE) &&
	    (b->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].flags != ನಮೂದು_ELEM_FLAG_NONE)) {
		ಹಿಂತಿರುಗಿ -1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((a->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].flags != ನಮೂದು_ELEM_FLAG_NONE) &&
		   (b->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].flags == ನಮೂದು_ELEM_FLAG_NONE)) {
		ಹಿಂತಿರುಗಿ 1;
	}

	ಆದರೆ ((a->elem[ನಮೂದು_ELEM_META].flags == ನಮೂದು_ELEM_FLAG_NONE) &&
	    (b->elem[ನಮೂದು_ELEM_META].flags != ನಮೂದು_ELEM_FLAG_NONE)) {
		ಹಿಂತಿರುಗಿ -1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((a->elem[ನಮೂದು_ELEM_META].flags != ನಮೂದು_ELEM_FLAG_NONE) &&
		   (b->elem[ನಮೂದು_ELEM_META].flags == ನಮೂದು_ELEM_FLAG_NONE)) {
		ಹಿಂತಿರುಗಿ 1;
	}

	ಆದರೆ (a->use_ಎಣಿಕೆ < b->use_ಎಣಿಕೆ) {
		ಹಿಂತಿರುಗಿ -1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (a->use_ಎಣಿಕೆ > b->use_ಎಣಿಕೆ) {
		ಹಿಂತಿರುಗಿ 1;
	}
	/* use count is the same - now consider last use time */

	ಆದರೆ (a->ಕೊನೆಯ_ಬಳಸಿದ < b->ಕೊನೆಯ_ಬಳಸಿದ) {
		ಹಿಂತಿರುಗಿ -1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (a->ಕೊನೆಯ_ಬಳಸಿದ > b->ಕೊನೆಯ_ಬಳಸಿದ) {
		ಹಿಂತಿರುಗಿ 1;
	}

	/* they are the same */
	ಹಿಂತಿರುಗಿ 0;
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	ರಚನೆ store_ನಮೂದು **elist;
	size_t ent_ಎಣಿಕೆ;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
ಸ್ಥಿರ bool
ನಮೂದು_eviction_iterator_cb(ಶೂನ್ಯ *ಕೀಲಿ, ಶೂನ್ಯ *ಮೌಲ್ಯ, ಶೂನ್ಯ *ctx)
{
	eviction_state_t *estate = ctx;
	ರಚನೆ store_ನಮೂದು *ent = ಮೌಲ್ಯ;
	estate->elist[estate->ent_ಎಣಿಕೆ++] = ent;
	ಹಿಂತಿರುಗಿ false;
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
ಸ್ಥಿರ nserror store_evict(ರಚನೆ store_state *state)
{
	size_t ent = 0;
	size_t removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	size_t old_ಎಣಿಕೆ;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	ಆದರೆ (state->ಒಟ್ಟು_alloc < state->ಎಲ್ಲೆ) {
		/* cache within limits */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->ಒಟ್ಟು_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_ಎಣಿಕೆ = hashmap_ಎಣಿಕೆ(state->entries);
	estate.ent_ಎಣಿಕೆ = 0;
	estate.elist = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ state_ನಮೂದು*) * old_ಎಣಿಕೆ);
	ಆದರೆ (estate.elist == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಆದರೆ (hashmap_iterate(state->entries, ನಮೂದು_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		ಬಿಡುಗಡೆ(estate.elist);
		ಹಿಂತಿರುಗಿ NSERROR_UNKNOWN;
	}

	ಆದರೆ (old_ಎಣಿಕೆ != estate.ent_ಎಣಿಕೆ) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		ಬಿಡುಗಡೆ(estate.elist);
		ಹಿಂತಿರುಗಿ NSERROR_UNKNOWN;
	}

	qsort(estate.elist, estate.ent_ಎಣಿಕೆ, ಗಾತ್ರ(ರಚನೆ state_ನಮೂದು*), compar);

	/* evict entries in listed order */
	removed = 0;
	ಗಾಗಿ (ent = 0; ent < estate.ent_ಎಣಿಕೆ; ent++) {
		ರಚನೆ store_ನಮೂದು *bse = estate.elist[ent];

		removed += bse->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].ಗಾತ್ರ_2;
		removed += bse->elem[ನಮೂದು_ELEM_META].ಗಾತ್ರ_2;

		ret = invalidate_ನಮೂದು(state, bse);
		ಆದರೆ (ret != NSERROR_OK) {
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (removed > state->hysteresis) {
			ನಿಲ್ಲಿಸು;
		}
	}

	ಬಿಡುಗಡೆ(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->ಒಟ್ಟು_alloc, old_ಎಣಿಕೆ - ent);

	ಹಿಂತಿರುಗಿ ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
ಸ್ಥಿರ nserror
write_ನಮೂದು(ರಚನೆ store_ನಮೂದು *ent, ಪೂರ್ಣಾಂಕ fd)
{
	uint32_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(nsurl_access(ent->url));
	ಆದರೆ (ಲಿಖ(fd, &len, ಗಾತ್ರ(len)) != ಗಾತ್ರ(len))
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	ಆದರೆ (ಲಿಖ(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	ಆದರೆ (ಲಿಖ(fd, ent, ಗಾತ್ರ(*ent)) != ಗಾತ್ರ(*ent))
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	ಪೂರ್ಣಾಂಕ fd;
	size_t written;
} write_ನಮೂದು_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
ಸ್ಥಿರ bool
write_ನಮೂದು_iterator(ಶೂನ್ಯ *ಕೀಲಿ, ಶೂನ್ಯ *ಮೌಲ್ಯ, ಶೂನ್ಯ *ctx)
{
	/* We ignore the key */
	ರಚನೆ store_ನಮೂದು *ent = ಮೌಲ್ಯ;
	write_ನಮೂದು_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	ಹಿಂತಿರುಗಿ write_ನಮೂದು(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror write_entries(ರಚನೆ store_state *state)
{
	ಅಕ್ಷರ *tname = ಖಾಲಿ; /* temporary file name for atomic replace */
	ಅಕ್ಷರ *fname = ಖಾಲಿ; /* target filename */
	write_ನಮೂದು_iteration_state weistate;
	nserror ret;

	memset(&weistate, 0, ಗಾತ್ರ(weistate));

	ಆದರೆ (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, "t"ENTRIES_FNAME);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	weistate.fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ಆದರೆ (weistate.fd == -1) {
		ಬಿಡುಗಡೆ(tname);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	ಆದರೆ (hashmap_iterate(state->entries, write_ನಮೂದು_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		close(weistate.fd);
		unlink(tname);
		ಬಿಡುಗಡೆ(tname);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	close(weistate.fd);

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, ENTRIES_FNAME);
	ಆದರೆ (ret != NSERROR_OK) {
		unlink(tname);
		ಬಿಡುಗಡೆ(tname);
		ಹಿಂತಿರುಗಿ ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(ಶೂನ್ಯ)remove(fname);
	ಆದರೆ (rename(tname, fname) != 0) {
		unlink(tname);
		ಬಿಡುಗಡೆ(tname);
		ಬಿಡುಗಡೆ(fname);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror write_ಖಂಡಗಳು(ರಚನೆ store_state *state)
{
	ಪೂರ್ಣಾಂಕ fd;
	ಅಕ್ಷರ *tname = ಖಾಲಿ; /* temporary file name for atomic replace */
	ಅಕ್ಷರ *fname = ಖಾಲಿ; /* target filename */
	size_t ಖಂಡಗಳು_ಗಾತ್ರ;
	size_t written = 0;
	size_t wr;
	nserror ret;
	ಪೂರ್ಣಾಂಕ bfidx; /* block file index */
	ಪೂರ್ಣಾಂಕ elem_idx;

	ಆದರೆ (state->ಖಂಡಗಳು_dirty == false) {
		/* blocks use maps have not been updated since last write */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, "t"ಖಂಡಗಳು_FNAME);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ಆದರೆ (fd == -1) {
		ಬಿಡುಗಡೆ(tname);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	ಖಂಡಗಳು_ಗಾತ್ರ = (ಖಂಡ_FILE_ಎಣಿಕೆ * ನಮೂದು_ELEM_ಎಣಿಕೆ) * ಖಂಡ_USE_MAP_ಗಾತ್ರ;

	ಗಾಗಿ (elem_idx = 0; elem_idx < ನಮೂದು_ELEM_ಎಣಿಕೆ; elem_idx++) {
		ಗಾಗಿ (bfidx = 0; bfidx < ಖಂಡ_FILE_ಎಣಿಕೆ; bfidx++) {
			wr = ಲಿಖ(fd,
				   &state->ಖಂಡಗಳು[elem_idx][bfidx].use_map[0],
				   ಖಂಡ_USE_MAP_ಗಾತ್ರ);
			ಆದರೆ (wr != ಖಂಡ_USE_MAP_ಗಾತ್ರ) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				ಹೋಗು wr_err;
			}
			written += wr;
		}
	}
wr_err:
	close(fd);

	/* check all data was written */
	ಆದರೆ (written != ಖಂಡಗಳು_ಗಾತ್ರ) {
		unlink(tname);
		ಬಿಡುಗಡೆ(tname);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, ಖಂಡಗಳು_FNAME);
	ಆದರೆ (ret != NSERROR_OK) {
		unlink(tname);
		ಬಿಡುಗಡೆ(tname);
		ಹಿಂತಿರುಗಿ ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(ಶೂನ್ಯ)remove(fname);
	ಆದರೆ (rename(tname, fname) != 0) {
		unlink(tname);
		ಬಿಡುಗಡೆ(tname);
		ಬಿಡುಗಡೆ(fname);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror ಗಣ_ಖಂಡ_extents(ರಚನೆ store_state *state)
{
	ಪೂರ್ಣಾಂಕ bfidx; /* block file index */
	ಪೂರ್ಣಾಂಕ elem_idx;
	ಪೂರ್ಣಾಂಕ ftr;

	ಆದರೆ (state->ಖಂಡಗಳು_opened == false) {
		/* no blocks have been opened since last write */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	ಗಾಗಿ (elem_idx = 0; elem_idx < ನಮೂದು_ELEM_ಎಣಿಕೆ; elem_idx++) {
		ಗಾಗಿ (bfidx = 0; bfidx < ಖಂಡ_FILE_ಎಣಿಕೆ; bfidx++) {
			ಆದರೆ (state->ಖಂಡಗಳು[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->ಖಂಡಗಳು[elem_idx][bfidx].fd, 1U << (log2_ಖಂಡ_ಗಾತ್ರ[elem_idx] + ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ));
				ಆದರೆ (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      errno);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->ಖಂಡಗಳು_opened = false;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ ಶೂನ್ಯ control_maintenance(ಶೂನ್ಯ *s)
{
	ರಚನೆ store_state *state = s;

	write_entries(state);
	write_ಖಂಡಗಳು(state);
	ಗಣ_ಖಂಡ_extents(state);
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
ಸ್ಥಿರ nserror
get_store_ನಮೂದು(ರಚನೆ store_state *state, nsurl *url, ರಚನೆ store_ನಮೂದು **bse)
{
	ರಚನೆ store_ನಮೂದು *ent;

	ent = hashmap_lookup(state->entries, url);

	ಆದರೆ (ent == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	*bse = ent;

	ent->ಕೊನೆಯ_ಬಳಸಿದ = time(ಖಾಲಿ);
	ent->use_ಎಣಿಕೆ++;

	state->entries_dirty = true;

	guit->misc->ವೇಳಾಪಟ್ಟಿ(CONTROL_MAINT_ಸಮಯ, control_maintenance, state);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Find next available small block.
 */
ಸ್ಥಿರ ಖಂಡ_ಸೂಚ್ಯಂಕ_t alloc_ಖಂಡ(ರಚನೆ store_state *state, ಪೂರ್ಣಾಂಕ elem_idx)
{
	ಪೂರ್ಣಾಂಕ bf;
	ಪೂರ್ಣಾಂಕ idx;
	ಪೂರ್ಣಾಂಕ ಬಿಟ್;
	uint8_t *map;

	ಗಾಗಿ (bf = 0; bf < ಖಂಡ_FILE_ಎಣಿಕೆ; bf++) {
		map = &state->ಖಂಡಗಳು[elem_idx][bf].use_map[0];

		ಗಾಗಿ (idx = 0; idx < ಖಂಡ_USE_MAP_ಗಾತ್ರ; idx++) {
			ಆದರೆ (*(map + idx) != 0xff) {
				/* located an unused block */
				ಗಾಗಿ (ಬಿಟ್ = 0; ಬಿಟ್ < 8;ಬಿಟ್++) {
					ಆದರೆ (((*(map + idx)) & (1U << ಬಿಟ್)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << ಬಿಟ್;
						state->ಖಂಡಗಳು_dirty = true;
						ಹಿಂತಿರುಗಿ (((bf * ಖಂಡ_USE_MAP_ಗಾತ್ರ) + idx) * 8) + ಬಿಟ್;
					}
				}
			}
		}
	}

	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ nserror
ಗಣ_store_ನಮೂದು(ರಚನೆ store_state *state,
		nsurl *url,
		ಪೂರ್ಣಾಂಕ elem_idx,
		uint8_t *ದತ್ತಾಂಶ,
		ಸ್ಥಿರಾಂಕ size_t datalen,
		ರಚನೆ store_ನಮೂದು **bse)
{
	ರಚನೆ store_ನಮೂದು *se;
	nserror ret;
	ರಚನೆ store_ನಮೂದು_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	se = hashmap_lookup(state->entries, url);
	ಆದರೆ (se == ಖಾಲಿ) {
		se = hashmap_ಸೇರಿಸು(state->entries, url);
	}
	ಆದರೆ (se == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	ಆದರೆ ((elem->flags & (ನಮೂದು_ELEM_FLAG_ರಾಶಿ | ನಮೂದು_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		ಹಿಂತಿರುಗಿ NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_ಎಣಿಕೆ = 1;
	se->ಕೊನೆಯ_ಬಳಸಿದ = time(ಖಾಲಿ);

	/* store the data in the element */
	elem->flags |= ನಮೂದು_ELEM_FLAG_ರಾಶಿ;
	elem->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
	elem->ref = 1;

	/* account for size of entry element */
	state->ಒಟ್ಟು_alloc -= elem->ಗಾತ್ರ_2;
	elem->ಗಾತ್ರ_2 = datalen;
	state->ಒಟ್ಟು_alloc += elem->ಗಾತ್ರ_2;

	/* if the element will fit in a small block attempt to allocate one */
	ಆದರೆ (elem->ಗಾತ್ರ_2 <= (1U << log2_ಖಂಡ_ಗಾತ್ರ[elem_idx])) {
		elem->ಖಂಡ = alloc_ಖಂಡ(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->ವೇಳಾಪಟ್ಟಿ(CONTROL_MAINT_ಸಮಯ, control_maintenance, state);

	*bse = se;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
store_open(ರಚನೆ store_state *state,
	   ನಮೂದು_ident_t ident,
	   ಪೂರ್ಣಾಂಕ elem_idx,
	   ಪೂರ್ಣಾಂಕ openflags)
{
	ಅಕ್ಷರ *fname;
	nserror ret;
	ಪೂರ್ಣಾಂಕ fd;

	fname = store_fname(state, ident, elem_idx);
	ಆದರೆ (fname == ಖಾಲಿ) {
		NSLOG(netsurf, ERROR, "filename error");
		ಹಿಂತಿರುಗಿ -1;
	}

	/* ensure all path elements to file exist if creating file */
	ಆದರೆ (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		ಆದರೆ (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			ಬಿಡುಗಡೆ(fname);
			ಹಿಂತಿರುಗಿ -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = open(fname, openflags, S_IRUSR | S_IWUSR);

	ಬಿಡುಗಡೆ(fname);

	ಹಿಂತಿರುಗಿ fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
unlink_entries(ರಚನೆ store_state *state)
{
	ಅಕ್ಷರ *fname = ಖಾಲಿ;
	nserror ret;

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, ENTRIES_FNAME);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	unlink(fname);

	ಬಿಡುಗಡೆ(fname);
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
ಸ್ಥಿರ nserror
read_entries(ರಚನೆ store_state *state)
{
	ಅಕ್ಷರ *fname = ಖಾಲಿ;
	ಅಕ್ಷರ *url;
	nsurl *nsurl;
	nserror ret;
	size_t read_entries = 0;
	ರಚನೆ store_ನಮೂದು *ent;
	ಪೂರ್ಣಾಂಕ fd;

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, ENTRIES_FNAME);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	ಆದರೆ (state->entries == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(fname);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	fd = open(fname, O_RDWR);
	ಆದರೆ (fd != -1) {
		uint32_t urllen;
		ಆಗ (ಪಠ(fd, &urllen, ಗಾತ್ರ(urllen)) == ಗಾತ್ರ(urllen)) {
			url = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, urllen+1);
			ಆದರೆ (url == ಖಾಲಿ) {
				close(fd);
				ಬಿಡುಗಡೆ(fname);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}
			ಆದರೆ (ಪಠ(fd, url, urllen) != (ssize_t)urllen) {
				ಬಿಡುಗಡೆ(url);
				close(fd);
				ಬಿಡುಗಡೆ(fname);
				ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			ಆದರೆ (ret != NSERROR_OK) {
				ಬಿಡುಗಡೆ(url);
				close(fd);
				ಬಿಡುಗಡೆ(fname);
				ಹಿಂತಿರುಗಿ ret;
			}
			ಬಿಡುಗಡೆ(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_ಸೇರಿಸು(state->entries, nsurl);
			ಆದರೆ (ent == ಖಾಲಿ) {
				nsurl_unref(nsurl);
				close(fd);
				ಬಿಡುಗಡೆ(fname);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			ಆದರೆ (ಪಠ(fd, ent, ಗಾತ್ರ(*ent)) != ಗಾತ್ರ(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				close(fd);
				ಬಿಡುಗಡೆ(fname);
				ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->ಒಟ್ಟು_alloc += ent->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].ಗಾತ್ರ_2;
			state->ಒಟ್ಟು_alloc += ent->elem[ನಮೂದು_ELEM_META].ಗಾತ್ರ_2;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ].flags &= ~(ನಮೂದು_ELEM_FLAG_ರಾಶಿ | ನಮೂದು_ELEM_FLAG_MMAP);
			ent->elem[ನಮೂದು_ELEM_META].flags &= ~(ನಮೂದು_ELEM_FLAG_ರಾಶಿ | ನಮೂದು_ELEM_FLAG_MMAP);

		}
		close(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	ಬಿಡುಗಡೆ(fname);
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
read_ಖಂಡಗಳು(ರಚನೆ store_state *state)
{
	ಪೂರ್ಣಾಂಕ bfidx; /* block file index */
	ಪೂರ್ಣಾಂಕ elem_idx;
	ಪೂರ್ಣಾಂಕ fd;
	ssize_t rd;
	ಅಕ್ಷರ *fname = ಖಾಲಿ;
	nserror ret;

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, ಖಂಡಗಳು_FNAME);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = open(fname, O_RDWR);
	ಬಿಡುಗಡೆ(fname);
	ಆದರೆ (fd != -1) {
		/* initialise block file use array */
		ಗಾಗಿ (elem_idx = 0; elem_idx < ನಮೂದು_ELEM_ಎಣಿಕೆ; elem_idx++) {
			ಗಾಗಿ (bfidx = 0; bfidx < ಖಂಡ_FILE_ಎಣಿಕೆ; bfidx++) {
				rd = ಪಠ(fd,
					  &state->ಖಂಡಗಳು[elem_idx][bfidx].use_map[0],
					  ಖಂಡ_USE_MAP_ಗಾತ್ರ);
				ಆದರೆ (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					ಹೋಗು rd_err;
				}
			}
		}
	rd_err:
		close(fd);

	} ಇಲ್ಲದಿದ್ದರೆ {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->ಖಂಡಗಳು[ನಮೂದು_ELEM_ದತ್ತಾಂಶ][0].use_map[0] = 1;
		state->ಖಂಡಗಳು[ನಮೂದು_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	ಗಾಗಿ (bfidx = 0; bfidx < ಖಂಡ_FILE_ಎಣಿಕೆ; bfidx++) {
		state->ಖಂಡಗಳು[ನಮೂದು_ELEM_ದತ್ತಾಂಶ][bfidx].fd = -1;
		state->ಖಂಡಗಳು[ನಮೂದು_ELEM_META][bfidx].fd = -1;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
write_cache_ಟ್ಯಾಗ್(ರಚನೆ store_state *state)
{
	FILE *fcachetag;
	nserror ret;
	ಅಕ್ಷರ *fname = ಖಾಲಿ;

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, "CACHEDIR.TAG");
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	fcachetag = fopen(fname, "wb");

	ಬಿಡುಗಡೆ(fname);

	ಆದರೆ (fcachetag == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	fprintf(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	fclose(fcachetag);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
write_control(ರಚನೆ store_state *state)
{
	FILE *fcontrol;
	nserror ret;
	ಅಕ್ಷರ *fname = ಖಾಲಿ;

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, "control");
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	ಆದರೆ (ret != NSERROR_OK) {
		ಬಿಡುಗಡೆ(fname);
		ಹಿಂತಿರುಗಿ ret;
	}

	fcontrol = fopen(fname, "wb");

	ಬಿಡುಗಡೆ(fname);

	ಆದರೆ (fcontrol == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	fprintf(fcontrol, "%u%c", CONTROL_ಆವೃತ್ತಿ, 0);

	fclose(fcontrol);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
read_control(ರಚನೆ store_state *state)
{
	nserror ret;
	FILE *fcontrol;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ctrlversion;
	ಅಕ್ಷರ *fname = ಖಾಲಿ;

	ret = netsurf_mkpath(&fname, ಖಾಲಿ, 2, state->ಮಾರ್ಗ, "control");
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = fopen(fname, "rb");

	ಬಿಡುಗಡೆ(fname);

	ಆದರೆ (fcontrol == ಖಾಲಿ) {
		/* unable to open control file */
		ಆದರೆ (errno == ENOENT) {
			ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	ಆದರೆ (fscanf(fcontrol, "%u", &ctrlversion) != 1) {
		ಹೋಗು control_error;
	}

	ಆದರೆ (ctrlversion != CONTROL_ಆವೃತ್ತಿ) {
		ಹೋಗು control_error;
	}

	ಆದರೆ (fgetc(fcontrol) != 0) {
		ಹೋಗು control_error;
	}

	fclose(fcontrol);

	ಹಿಂತಿರುಗಿ NSERROR_OK;

control_error: /* problem with the control file */

	fclose(fcontrol);

	ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror
initialise(ಸ್ಥಿರಾಂಕ ರಚನೆ llcache_store_parameters *parameters)
{
	ರಚನೆ store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	ಆದರೆ (storestate != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	ಆದರೆ (parameters->ಎಲ್ಲೆ == 0) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	ಆದರೆ (parameters->ಮಾರ್ಗ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ store_state));
	ಆದರೆ (newstate == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	newstate->ಮಾರ್ಗ = strdup(parameters->ಮಾರ್ಗ);
	newstate->ಎಲ್ಲೆ = parameters->ಎಲ್ಲೆ;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	ಆದರೆ (ret != NSERROR_OK) {
		ಆದರೆ (ret == NSERROR_NOT_ಸಿಕ್ಕಿತು) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} ಇಲ್ಲದಿದ್ದರೆ {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->ಮಾರ್ಗ);
			ಆದರೆ (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->ಮಾರ್ಗ);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} ಇಲ್ಲದಿದ್ದರೆ {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->ಮಾರ್ಗ);
			}
		}
		ret = write_control(newstate);
		ಆದರೆ (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_ಟ್ಯಾಗ್(newstate);
		}
	}
	ಆದರೆ (ret != NSERROR_OK) {
		/* that went well obviously */
		ಬಿಡುಗಡೆ(newstate->ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(newstate);
		ಹಿಂತಿರುಗಿ ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	ಆದರೆ (ret != NSERROR_OK) {
		/* that went well obviously */
		ಬಿಡುಗಡೆ(newstate->ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(newstate);
		ಹಿಂತಿರುಗಿ ret;
	}

	/* read blocks */
	ret = read_ಖಂಡಗಳು(newstate);
	ಆದರೆ (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		ಬಿಡುಗಡೆ(newstate->ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(newstate);
		ಹಿಂತಿರುಗಿ ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->ಮಾರ್ಗ,
	      newstate->ಎಲ್ಲೆ,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->ಒಟ್ಟು_alloc, newstate->ಎಲ್ಲೆ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
ಸ್ಥಿರ nserror
finalise(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ bf; /* block file index */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ op_ಎಣಿಕೆ;

	ಆದರೆ (storestate != ಖಾಲಿ) {
		guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_ಖಂಡಗಳು(storestate);

		/* ensure all block files are closed */
		ಗಾಗಿ (bf = 0; bf < ಖಂಡ_FILE_ಎಣಿಕೆ; bf++) {
			ಆದರೆ (storestate->ಖಂಡಗಳು[ನಮೂದು_ELEM_ದತ್ತಾಂಶ][bf].fd != -1) {
				close(storestate->ಖಂಡಗಳು[ನಮೂದು_ELEM_ದತ್ತಾಂಶ][bf].fd);
			}
			ಆದರೆ (storestate->ಖಂಡಗಳು[ನಮೂದು_ELEM_META][bf].fd != -1) {
				close(storestate->ಖಂಡಗಳು[ನಮೂದು_ELEM_META][bf].fd);
			}
		}

		op_ಎಣಿಕೆ = storestate->ದೊರಕಿತು_ಎಣಿಕೆ + storestate->ತಪ್ಪಿದ_ಎಣಿಕೆ;

		/* avoid division by zero */
		ಆದರೆ (op_ಎಣಿಕೆ > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_ಎಣಿಕೆ,
			      storestate->ದೊರಕಿತು_ಎಣಿಕೆ,
			      storestate->ತಪ್ಪಿದ_ಎಣಿಕೆ,
			      0,
			      (storestate->ದೊರಕಿತು_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ,
			      (storestate->ತಪ್ಪಿದ_ಎಣಿಕೆ * 100) / op_ಎಣಿಕೆ,
			      0);
		}

		hashmap_destroy(storestate->entries);
		ಬಿಡುಗಡೆ(storestate->ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(storestate);
		storestate = ಖಾಲಿ;
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ಸ್ಥಿರ nserror store_write_ಖಂಡ(ರಚನೆ store_state *state,
			 ರಚನೆ store_ನಮೂದು *bse,
			 ಪೂರ್ಣಾಂಕ elem_idx)
{
	ಖಂಡ_ಸೂಚ್ಯಂಕ_t bf = (bse->elem[elem_idx].ಖಂಡ >> ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ) &
		((1 << ಖಂಡ_FILE_ಎಣಿಕೆ) - 1); /* block file block resides in */
	ಖಂಡ_ಸೂಚ್ಯಂಕ_t bi = bse->elem[elem_idx].ಖಂಡ & ((1U << ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	ಆದರೆ (state->ಖಂಡಗಳು[elem_idx][bf].fd == -1) {
		state->ಖಂಡಗಳು[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ನಮೂದು_ELEM_ಎಣಿಕೆ, O_CREAT | O_RDWR);
		ಆದರೆ (state->ಖಂಡಗಳು[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->ಖಂಡಗಳು_opened = true;
	}

	offst = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)bi << log2_ಖಂಡ_ಗಾತ್ರ[elem_idx];

	wr = nsu_pwrite(state->ಖಂಡಗಳು[elem_idx][bf].fd,
			bse->elem[elem_idx].ದತ್ತಾಂಶ,
			bse->elem[elem_idx].ಗಾತ್ರ_2,
			offst);
	ಆದರೆ (wr != (ssize_t)bse->elem[elem_idx].ಗಾತ್ರ_2) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].ಗಾತ್ರ_2,
		      bse->elem[elem_idx].ದತ್ತಾಂಶ,
		      (size_t)offst,
		      bse->elem[elem_idx].ಖಂಡ,
		      errno);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].ದತ್ತಾಂಶ, (size_t)offst,
	      bse->elem[elem_idx].ಖಂಡ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ಸ್ಥಿರ nserror store_write_file(ರಚನೆ store_state *state,
			 ರಚನೆ store_ನಮೂದು *bse,
			 ಪೂರ್ಣಾಂಕ elem_idx)
{
	ssize_t wr;
	ಪೂರ್ಣಾಂಕ fd;
	ಪೂರ್ಣಾಂಕ err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	ಆದರೆ (fd < 0) {
		perror("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	wr = ಲಿಖ(fd, bse->elem[elem_idx].ದತ್ತಾಂಶ, bse->elem[elem_idx].ಗಾತ್ರ_2);
	err = errno; /* close can change errno */

	close(fd);
	ಆದರೆ (wr != (ssize_t)bse->elem[elem_idx].ಗಾತ್ರ_2) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].ಗಾತ್ರ_2,
		      bse->elem[elem_idx].ದತ್ತಾಂಶ,
		      err);

		/** @todo Delete the file? */
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].ದತ್ತಾಂಶ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
store(nsurl *url,
      ಎಣಿಕೆ backing_store_flags bsflags,
      uint8_t *ದತ್ತಾಂಶ,
      ಸ್ಥಿರಾಂಕ size_t datalen)
{
	nserror ret;
	ರಚನೆ store_ನಮೂದು *bse;
	ಪೂರ್ಣಾಂಕ elem_idx;

	/* check backing store is initialised */
	ಆದರೆ (storestate == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	ಆದರೆ ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ನಮೂದು_ELEM_META;
	} ಇಲ್ಲದಿದ್ದರೆ {
		elem_idx = ನಮೂದು_ELEM_ದತ್ತಾಂಶ;
	}

	/* set the store entry up */
	ret = ಗಣ_store_ನಮೂದು(storestate, url, elem_idx, ದತ್ತಾಂಶ, datalen, &bse);
	ಆದರೆ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		ಹಿಂತಿರುಗಿ ret;
	}

	ಆದರೆ (bse->elem[elem_idx].ಖಂಡ != 0) {
		/* small block storage */
		ret = store_write_ಖಂಡ(storestate, bse, elem_idx);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	ಹಿಂತಿರುಗಿ ret;
}

/**
 * release any allocation for an entry
 */
ಸ್ಥಿರ nserror ನಮೂದು_release_alloc(ರಚನೆ store_ನಮೂದು_element *elem)
{
	ಆದರೆ ((elem->flags & ನಮೂದು_ELEM_FLAG_ರಾಶಿ) != 0) {
		elem->ref--;
		ಆದರೆ (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->ದತ್ತಾಂಶ);
			ಬಿಡುಗಡೆ(elem->ದತ್ತಾಂಶ);
			elem->flags &= ~ನಮೂದು_ELEM_FLAG_ರಾಶಿ;
		}
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ಸ್ಥಿರ nserror store_read_ಖಂಡ(ರಚನೆ store_state *state,
			 ರಚನೆ store_ನಮೂದು *bse,
			 ಪೂರ್ಣಾಂಕ elem_idx)
{
	ಖಂಡ_ಸೂಚ್ಯಂಕ_t bf = (bse->elem[elem_idx].ಖಂಡ >> ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ) &
		((1 << ಖಂಡ_FILE_ಎಣಿಕೆ) - 1); /* block file block resides in */
	ಖಂಡ_ಸೂಚ್ಯಂಕ_t bi = bse->elem[elem_idx].ಖಂಡ & ((1 << ಖಂಡ_ನಮೂದು_ಎಣಿಕೆ) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	ಆದರೆ (state->ಖಂಡಗಳು[elem_idx][bf].fd == -1) {
		state->ಖಂಡಗಳು[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ನಮೂದು_ELEM_ಎಣಿಕೆ, O_CREAT | O_RDWR);
		ಆದರೆ (state->ಖಂಡಗಳು[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->ಖಂಡಗಳು_opened = true;
	}

	offst = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)bi << log2_ಖಂಡ_ಗಾತ್ರ[elem_idx];

	rd = nsu_pread(state->ಖಂಡಗಳು[elem_idx][bf].fd,
		       bse->elem[elem_idx].ದತ್ತಾಂಶ,
		       bse->elem[elem_idx].ಗಾತ್ರ_2,
		       offst);
	ಆದರೆ (rd != (ssize_t)bse->elem[elem_idx].ಗಾತ್ರ_2) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].ಗಾತ್ರ_2,
		      bse->elem[elem_idx].ದತ್ತಾಂಶ,
		      (size_t)offst,
		      bse->elem[elem_idx].ಖಂಡ,
		      errno);
		ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].ದತ್ತಾಂಶ, (size_t)offst,
	      bse->elem[elem_idx].ಖಂಡ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ಸ್ಥಿರ nserror store_read_file(ರಚನೆ store_state *state,
			 ರಚನೆ store_ನಮೂದು *bse,
			 ಪೂರ್ಣಾಂಕ elem_idx)
{
	ಪೂರ್ಣಾಂಕ fd;
	ssize_t rd; /* return from read */
	ಪೂರ್ಣಾಂಕ ret = NSERROR_OK;
	size_t tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	ಆದರೆ (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		/** @todo should this invalidate the entry? */
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	ಆಗ (tot < bse->elem[elem_idx].ಗಾತ್ರ_2) {
		rd = ಪಠ(fd,
			  bse->elem[elem_idx].ದತ್ತಾಂಶ + tot,
			  bse->elem[elem_idx].ಗಾತ್ರ_2 - tot);
		ಆದರೆ (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      errno);
			ret = NSERROR_NOT_ಸಿಕ್ಕಿತು;
			ನಿಲ್ಲಿಸು;
		}
		tot += rd;
	}

	close(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].ದತ್ತಾಂಶ);

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ nserror
fetch(nsurl *url,
      ಎಣಿಕೆ backing_store_flags bsflags,
      uint8_t **ದತ್ತಾಂಶ_ಹೊರಹೋಗು,
      size_t *datalen_ಹೊರಹೋಗು)
{
	nserror ret;
	ರಚನೆ store_ನಮೂದು *bse;
	ರಚನೆ store_ನಮೂದು_element *elem;
	ಪೂರ್ಣಾಂಕ elem_idx;

	/* check backing store is initialised */
	ಆದರೆ (storestate == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_ನಮೂದು(storestate, url, &bse);
	ಆದರೆ (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->ತಪ್ಪಿದ_ಎಣಿಕೆ++;
		ಹಿಂತಿರುಗಿ ret;
	}
	storestate->ದೊರಕಿತು_ಎಣಿಕೆ++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	ಆದರೆ ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ನಮೂದು_ELEM_META;
	} ಇಲ್ಲದಿದ್ದರೆ {
		elem_idx = ನಮೂದು_ELEM_ದತ್ತಾಂಶ;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	ಆದರೆ ((elem->flags & ನಮೂದು_ELEM_FLAG_ರಾಶಿ) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->ದತ್ತಾಂಶ, elem->ref);

	} ಇಲ್ಲದಿದ್ದರೆ {
		/* allocate from the heap */
		elem->ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(elem->ಗಾತ್ರ_2);
		ಆದರೆ (elem->ದತ್ತಾಂಶ == ಖಾಲಿ) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->ದತ್ತಾಂಶ);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= ನಮೂದು_ELEM_FLAG_ರಾಶಿ;
		elem->ref = 1;

		/* fill the new block */
		ಆದರೆ (elem->ಖಂಡ != 0) {
			ret = store_read_ಖಂಡ(storestate, bse, elem_idx);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	ಆದರೆ (ret != NSERROR_OK) {
		ನಮೂದು_release_alloc(elem);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* update stats and setup return pointers */
		storestate->ದೊರಕಿತು_ಗಾತ್ರ += elem->ಗಾತ್ರ_2;

		*ದತ್ತಾಂಶ_ಹೊರಹೋಗು = elem->ದತ್ತಾಂಶ;
		*datalen_ಹೊರಹೋಗು = elem->ಗಾತ್ರ_2;
	}

	ಹಿಂತಿರುಗಿ ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
ಸ್ಥಿರ nserror release(nsurl *url, ಎಣಿಕೆ backing_store_flags bsflags)
{
	nserror ret;
	ರಚನೆ store_ನಮೂದು *bse;
	ರಚನೆ store_ನಮೂದು_element *elem;

	/* check backing store is initialised */
	ಆದರೆ (storestate == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	ret = get_store_ನಮೂದು(storestate, url, &bse);
	ಆದರೆ (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		ಹಿಂತಿರುಗಿ ret;
	}

	/* the entry element */
	ಆದರೆ ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[ನಮೂದು_ELEM_META];
	} ಇಲ್ಲದಿದ್ದರೆ {
		elem = &bse->elem[ನಮೂದು_ELEM_ದತ್ತಾಂಶ];
	}

	ret = ನಮೂದು_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	ಆದರೆ ((ret == NSERROR_OK) &&
	    ((bse->flags & ನಮೂದು_FLAGS_INVALID) != 0)) {
		ret = invalidate_ನಮೂದು(storestate, bse);
	}

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ nserror
invalidate(nsurl *url)
{
	nserror ret;
	ರಚನೆ store_ನಮೂದು *bse;

	/* check backing store is initialised */
	ಆದರೆ (storestate == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	ret = get_store_ನಮೂದು(storestate, url, &bse);
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ invalidate_ನಮೂದು(storestate, bse);
}


ಸ್ಥಿರ ರಚನೆ gui_llcache_ಕೋಷ್ಟಕ llcache_ಕೋಷ್ಟಕ = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

ರಚನೆ gui_llcache_ಕೋಷ್ಟಕ *filesystem_llcache_ಕೋಷ್ಟಕ = &llcache_ಕೋಷ್ಟಕ;
