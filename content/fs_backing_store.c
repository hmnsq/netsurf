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

#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/filepath.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/file.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/hashmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/backing_store.h"

/** Backing store file format version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CONTROL_ਸੰਸਕਰਣ 202

/**
 * Number of milliseconds after a update before control data
 * maintenance is performed
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CONTROL_MAINT_ਸਮਾਂ 10000

/** Filename of serialised entries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENTRIES_FNAME "entries"

/** Filename of block file index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੰਡਾਂ_FNAME "blocks"

/** log2 block data address length (64k) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੰਡ_ADDR_LEN 16

/** log2 number of entries per block file(1024) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ 10

/** log2 number of data block files */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੰਡ_FILE_ਗਿਣਤੀ (ਖੰਡ_ADDR_LEN - ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ)

/** log2 size of data blocks (8k) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੰਡ_ਅੰਕੜੇ_ਮਾਪ 13

/** log2 size of metadata blocks (8k) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੰਡ_META_ਮਾਪ 13

/** length in bytes of a block files use map */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੰਡ_USE_MAP_ਮਾਪ (1 << (ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uint32_t ਇੰਦਰਾਜ_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uint16_t ਖੰਡ_ਸੂਚਕਾਂਕ_t;


/**
 * Entry element index values.
 */
ਗਿਣਤੀ store_ਇੰਦਰਾਜ_elem_idx {
	ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ = 0, /**< entry element is data */
	ਇੰਦਰਾਜ_ELEM_META = 1,  /**< entry element is metadata */
	ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
ਗਿਣਤੀ store_ਇੰਦਰਾਜ_elem_flags {
	/** store not managing any allocation on entry */
	ਇੰਦਰਾਜ_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ = 0x1,
	/** entry data allocation is mmaped */
	ਇੰਦਰਾਜ_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	ਇੰਦਰਾਜ_ELEM_FLAG_SMALL = 0x4,
};


ਗਿਣਤੀ store_ਇੰਦਰਾਜ_flags {
	/** entry is normal */
	ਇੰਦਰਾਜ_FLAGS_NONE = 0,
	/** entry has been invalidated but something still holding a reference */
	ਇੰਦਰਾਜ_FLAGS_INVALID = 1,
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
ਬਣਤਰ store_ਇੰਦਰਾਜ_element {
	uint8_t* ਅੰਕੜੇ; /**< data allocated */
	uint32_t ਮਾਪ; /**< size of entry element on disc */
	ਖੰਡ_ਸੂਚਕਾਂਕ_t ਖੰਡ; /**< small object data block */
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
ਬਣਤਰ store_ਇੰਦਰਾਜ {
	nsurl *url; /**< The URL for this entry */
	int64_t ਆਖਰੀ_ਵਰਤਿਆ; /**< UNIX time the entry was last used */
	uint16_t use_ਗਿਣਤੀ; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	ਬਣਤਰ store_ਇੰਦਰਾਜ_element elem[ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ];
};

/**
 * Small block file.
 */
ਬਣਤਰ ਖੰਡ_file {
	/** file descriptor of the block file */
	ਪੂਰਨ fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[ਖੰਡ_USE_MAP_ਮਾਪ];
};

/**
 * log2 of block size.
 */
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ log2_ਖੰਡ_ਮਾਪ[ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ] = {
	ਖੰਡ_ਅੰਕੜੇ_ਮਾਪ, /**< Data block size */
	ਖੰਡ_META_ਮਾਪ  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
ਬਣਤਰ store_state {
	/* store config */
	ਅੱਖਰ *ਰਸਤਾ; /**< The path to the backing store */
	size_t ਹੱਦਬੰਦੀ; /**< The backing store upper bound target size */
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
	ਬਣਤਰ ਖੰਡ_file ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ][ਖੰਡ_FILE_ਗਿਣਤੀ];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool ਖੰਡਾਂ_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool ਖੰਡਾਂ_opened;


	/* stats */
	uint64_t ਕੁੱਲ_alloc; /**< total size of all allocated storage. */

	size_t ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ; /**< number of cache hits */
	uint64_t ਪ੍ਰਾਪਤੀ_ਮਾਪ; /**< size of storage served */
	size_t ਖੁੰਝ_ਗਿਣਤੀ; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
ਬਣਤਰ store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

ਸਥਿਰ bool
entries_hashmap_ਕੁੰਜੀ_eq(ਖਾਲੀ *key1, ਖਾਲੀ *key2)
{
	ਵਾਪਸ nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

ਸਥਿਰ ਖਾਲੀ *
entries_hashmap_ਮੁੱਲ_alloc(ਖਾਲੀ *ਕੁੰਜੀ)
{
	ਬਣਤਰ store_ਇੰਦਰਾਜ *ent = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ store_ਇੰਦਰਾਜ));
	ਜੇ (ent != ਨਿਰਾ) {
		ent->url = nsurl_ref(ਕੁੰਜੀ);
	}
	ਵਾਪਸ ent;
}

ਸਥਿਰ ਖਾਲੀ
entries_hashmap_ਮੁੱਲ_destroy(ਖਾਲੀ *ਮੁੱਲ)
{
	ਬਣਤਰ store_ਇੰਦਰਾਜ *ent = ਮੁੱਲ;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	ਮੁਕਤ(ent);
}

ਸਥਿਰ hashmap_parameters_t entries_hashmap_parameters = {
	.ਕੁੰਜੀ_clone = (hashmap_ਕੁੰਜੀ_clone_t)nsurl_ref,
	.ਕੁੰਜੀ_destroy = (hashmap_ਕੁੰਜੀ_destroy_t)nsurl_unref,
	.ਕੁੰਜੀ_hash = (hashmap_ਕੁੰਜੀ_hash_t)nsurl_hash,
	.ਕੁੰਜੀ_eq = entries_hashmap_ਕੁੰਜੀ_eq,
	.ਮੁੱਲ_alloc = entries_hashmap_ਮੁੱਲ_alloc,
	.ਮੁੱਲ_destroy = entries_hashmap_ਮੁੱਲ_destroy,
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
ਸਥਿਰ ਅੱਖਰ *
store_fname(ਬਣਤਰ store_state *state,
	    ਇੰਦਰਾਜ_ident_t ident,
	    ਪੂਰਨ elem_idx)
{
	ਅੱਖਰ *fname = ਨਿਰਾ;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	ਅਚਲ uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ_dir_ਸਾਰਣੀ[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	ਅਚਲ uint8_t encoding_ਸਾਰਣੀ[64][3] = {
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
	b32u_i[0] = encoding_ਸਾਰਣੀ[(ident      ) & 0x1f][0];
	b32u_i[1] = encoding_ਸਾਰਣੀ[(ident >>  5) & 0x1f][0];
	b32u_i[2] = encoding_ਸਾਰਣੀ[(ident >> 10) & 0x1f][0];
	b32u_i[3] = encoding_ਸਾਰਣੀ[(ident >> 15) & 0x1f][0];
	b32u_i[4] = encoding_ਸਾਰਣੀ[(ident >> 20) & 0x1f][0];
	b32u_i[5] = encoding_ਸਾਰਣੀ[(ident >> 25) & 0x1f][0];
	b32u_i[6] = encoding_ਸਾਰਣੀ[(ident >> 30) & 0x1f][0];
	b32u_i[7] = 0; /* null terminate ident string */

	/* base32 encode directory separators */
	b32u_d[0] = (uint8_t*)ਅਧਾਰ_dir_ਸਾਰਣੀ[elem_idx];
	b32u_d[1] = &encoding_ਸਾਰਣੀ[(ident      ) & 0x3f][0];
	b32u_d[2] = &encoding_ਸਾਰਣੀ[(ident >>  6) & 0x3f][0];
	b32u_d[3] = &encoding_ਸਾਰਣੀ[(ident >> 12) & 0x3f][0];
	b32u_d[4] = &encoding_ਸਾਰਣੀ[(ident >> 18) & 0x3f][0];
	b32u_d[5] = &encoding_ਸਾਰਣੀ[(ident >> 24) & 0x3f][0];

	ਚੋਣ (elem_idx) {
	ਹਾਲਤ ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ:
	ਹਾਲਤ ਇੰਦਰਾਜ_ELEM_META:
		netsurf_mkpath(&fname, ਨਿਰਾ, 8,
			       state->ਰਸਤਾ, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		ਰੋਕੋ;

	ਹਾਲਤ (ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ + ਇੰਦਰਾਜ_ELEM_META):
	ਹਾਲਤ (ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ + ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ):
		netsurf_mkpath(&fname, ਨਿਰਾ, 3,
			       state->ਰਸਤਾ, b32u_d[0], b32u_d[1]);
		ਰੋਕੋ;

	ਮੂਲ:
		ਪ੍ਰਤਿਜ੍ਞਾ("bad element index" == ਨਿਰਾ);
		ਰੋਕੋ;
	}

	ਵਾਪਸ fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
invalidate_element(ਬਣਤਰ store_state *state,
		   ਬਣਤਰ store_ਇੰਦਰਾਜ *bse,
		   ਪੂਰਨ elem_idx)
{
	ਜੇ (bse->elem[elem_idx].ਖੰਡ != 0) {
		ਖੰਡ_ਸੂਚਕਾਂਕ_t bf;
		ਖੰਡ_ਸੂਚਕਾਂਕ_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].ਖੰਡ >> ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ) &
			((1 << ਖੰਡ_FILE_ਗਿਣਤੀ) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].ਖੰਡ & ((1U << ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ) -1);

		/* clear bit in use map */
		state->ਖੰਡਾਂ[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} ਨਹੀਂਤਾਂ {
		ਅੱਖਰ *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		ਜੇ (fname == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
		unlink(fname);
		ਮੁਕਤ(fname);
	}

	state->ਕੁੱਲ_alloc -= bse->elem[elem_idx].ਮਾਪ;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
invalidate_ਇੰਦਰਾਜ(ਬਣਤਰ store_state *state, ਬਣਤਰ store_ਇੰਦਰਾਜ *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= ਇੰਦਰਾਜ_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	ਜੇ (((bse->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].flags &
	     (ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ | ਇੰਦਰਾਜ_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[ਇੰਦਰਾਜ_ELEM_META].flags &
	      (ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ | ਇੰਦਰਾਜ_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		ਵਾਪਸ NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, ਇੰਦਰਾਜ_ELEM_META);
	ਜੇ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ);
	ਜੇ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
ਸਥਿਰ ਪੂਰਨ compar(ਅਚਲ ਖਾਲੀ *va, ਅਚਲ ਖਾਲੀ *vb)
{
	ਅਚਲ ਬਣਤਰ store_ਇੰਦਰਾਜ *a = *(ਅਚਲ ਬਣਤਰ store_ਇੰਦਰਾਜ **)va;
	ਅਚਲ ਬਣਤਰ store_ਇੰਦਰਾਜ *b = *(ਅਚਲ ਬਣਤਰ store_ਇੰਦਰਾਜ **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	ਜੇ ((a->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].flags == ਇੰਦਰਾਜ_ELEM_FLAG_NONE) &&
	    (b->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].flags != ਇੰਦਰਾਜ_ELEM_FLAG_NONE)) {
		ਵਾਪਸ -1;
	} ਨਹੀਂਤਾਂ ਜੇ ((a->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].flags != ਇੰਦਰਾਜ_ELEM_FLAG_NONE) &&
		   (b->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].flags == ਇੰਦਰਾਜ_ELEM_FLAG_NONE)) {
		ਵਾਪਸ 1;
	}

	ਜੇ ((a->elem[ਇੰਦਰਾਜ_ELEM_META].flags == ਇੰਦਰਾਜ_ELEM_FLAG_NONE) &&
	    (b->elem[ਇੰਦਰਾਜ_ELEM_META].flags != ਇੰਦਰਾਜ_ELEM_FLAG_NONE)) {
		ਵਾਪਸ -1;
	} ਨਹੀਂਤਾਂ ਜੇ ((a->elem[ਇੰਦਰਾਜ_ELEM_META].flags != ਇੰਦਰਾਜ_ELEM_FLAG_NONE) &&
		   (b->elem[ਇੰਦਰਾਜ_ELEM_META].flags == ਇੰਦਰਾਜ_ELEM_FLAG_NONE)) {
		ਵਾਪਸ 1;
	}

	ਜੇ (a->use_ਗਿਣਤੀ < b->use_ਗਿਣਤੀ) {
		ਵਾਪਸ -1;
	} ਨਹੀਂਤਾਂ ਜੇ (a->use_ਗਿਣਤੀ > b->use_ਗਿਣਤੀ) {
		ਵਾਪਸ 1;
	}
	/* use count is the same - now consider last use time */

	ਜੇ (a->ਆਖਰੀ_ਵਰਤਿਆ < b->ਆਖਰੀ_ਵਰਤਿਆ) {
		ਵਾਪਸ -1;
	} ਨਹੀਂਤਾਂ ਜੇ (a->ਆਖਰੀ_ਵਰਤਿਆ > b->ਆਖਰੀ_ਵਰਤਿਆ) {
		ਵਾਪਸ 1;
	}

	/* they are the same */
	ਵਾਪਸ 0;
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	ਬਣਤਰ store_ਇੰਦਰਾਜ **elist;
	size_t ent_ਗਿਣਤੀ;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
ਸਥਿਰ bool
ਇੰਦਰਾਜ_eviction_iterator_cb(ਖਾਲੀ *ਕੁੰਜੀ, ਖਾਲੀ *ਮੁੱਲ, ਖਾਲੀ *ctx)
{
	eviction_state_t *estate = ctx;
	ਬਣਤਰ store_ਇੰਦਰਾਜ *ent = ਮੁੱਲ;
	estate->elist[estate->ent_ਗਿਣਤੀ++] = ent;
	ਵਾਪਸ false;
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
ਸਥਿਰ nserror store_evict(ਬਣਤਰ store_state *state)
{
	size_t ent = 0;
	size_t removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	size_t old_ਗਿਣਤੀ;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	ਜੇ (state->ਕੁੱਲ_alloc < state->ਹੱਦਬੰਦੀ) {
		/* cache within limits */
		ਵਾਪਸ NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->ਕੁੱਲ_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_ਗਿਣਤੀ = hashmap_ਗਿਣਤੀ(state->entries);
	estate.ent_ਗਿਣਤੀ = 0;
	estate.elist = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ state_ਇੰਦਰਾਜ*) * old_ਗਿਣਤੀ);
	ਜੇ (estate.elist == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਜੇ (hashmap_iterate(state->entries, ਇੰਦਰਾਜ_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		ਮੁਕਤ(estate.elist);
		ਵਾਪਸ NSERROR_UNKNOWN;
	}

	ਜੇ (old_ਗਿਣਤੀ != estate.ent_ਗਿਣਤੀ) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		ਮੁਕਤ(estate.elist);
		ਵਾਪਸ NSERROR_UNKNOWN;
	}

	qsort(estate.elist, estate.ent_ਗਿਣਤੀ, ਆਕਾਰ(ਬਣਤਰ state_ਇੰਦਰਾਜ*), compar);

	/* evict entries in listed order */
	removed = 0;
	ਲਈ (ent = 0; ent < estate.ent_ਗਿਣਤੀ; ent++) {
		ਬਣਤਰ store_ਇੰਦਰਾਜ *bse = estate.elist[ent];

		removed += bse->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].ਮਾਪ;
		removed += bse->elem[ਇੰਦਰਾਜ_ELEM_META].ਮਾਪ;

		ret = invalidate_ਇੰਦਰਾਜ(state, bse);
		ਜੇ (ret != NSERROR_OK) {
			ਰੋਕੋ;
		}

		ਜੇ (removed > state->hysteresis) {
			ਰੋਕੋ;
		}
	}

	ਮੁਕਤ(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->ਕੁੱਲ_alloc, old_ਗਿਣਤੀ - ent);

	ਵਾਪਸ ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
ਸਥਿਰ nserror
write_ਇੰਦਰਾਜ(ਬਣਤਰ store_ਇੰਦਰਾਜ *ent, ਪੂਰਨ fd)
{
	uint32_t len = ਲੜੀ_ਲੰਬਾਈ(nsurl_access(ent->url));
	ਜੇ (ਲਿਖ(fd, &len, ਆਕਾਰ(len)) != ਆਕਾਰ(len))
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	ਜੇ (ਲਿਖ(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	ਜੇ (ਲਿਖ(fd, ent, ਆਕਾਰ(*ent)) != ਆਕਾਰ(*ent))
		ਵਾਪਸ NSERROR_SAVE_FAILED;

	ਵਾਪਸ NSERROR_OK;
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	ਪੂਰਨ fd;
	size_t written;
} write_ਇੰਦਰਾਜ_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
ਸਥਿਰ bool
write_ਇੰਦਰਾਜ_iterator(ਖਾਲੀ *ਕੁੰਜੀ, ਖਾਲੀ *ਮੁੱਲ, ਖਾਲੀ *ctx)
{
	/* We ignore the key */
	ਬਣਤਰ store_ਇੰਦਰਾਜ *ent = ਮੁੱਲ;
	write_ਇੰਦਰਾਜ_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	ਵਾਪਸ write_ਇੰਦਰਾਜ(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror write_entries(ਬਣਤਰ store_state *state)
{
	ਅੱਖਰ *tname = ਨਿਰਾ; /* temporary file name for atomic replace */
	ਅੱਖਰ *fname = ਨਿਰਾ; /* target filename */
	write_ਇੰਦਰਾਜ_iteration_state weistate;
	nserror ret;

	memset(&weistate, 0, ਆਕਾਰ(weistate));

	ਜੇ (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		ਵਾਪਸ NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ਨਿਰਾ, 2, state->ਰਸਤਾ, "t"ENTRIES_FNAME);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	weistate.fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ਜੇ (weistate.fd == -1) {
		ਮੁਕਤ(tname);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	ਜੇ (hashmap_iterate(state->entries, write_ਇੰਦਰਾਜ_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		close(weistate.fd);
		unlink(tname);
		ਮੁਕਤ(tname);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	close(weistate.fd);

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, ENTRIES_FNAME);
	ਜੇ (ret != NSERROR_OK) {
		unlink(tname);
		ਮੁਕਤ(tname);
		ਵਾਪਸ ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(ਖਾਲੀ)remove(fname);
	ਜੇ (rename(tname, fname) != 0) {
		unlink(tname);
		ਮੁਕਤ(tname);
		ਮੁਕਤ(fname);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror write_ਖੰਡਾਂ(ਬਣਤਰ store_state *state)
{
	ਪੂਰਨ fd;
	ਅੱਖਰ *tname = ਨਿਰਾ; /* temporary file name for atomic replace */
	ਅੱਖਰ *fname = ਨਿਰਾ; /* target filename */
	size_t ਖੰਡਾਂ_ਮਾਪ;
	size_t written = 0;
	size_t wr;
	nserror ret;
	ਪੂਰਨ bfidx; /* block file index */
	ਪੂਰਨ elem_idx;

	ਜੇ (state->ਖੰਡਾਂ_dirty == false) {
		/* blocks use maps have not been updated since last write */
		ਵਾਪਸ NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ਨਿਰਾ, 2, state->ਰਸਤਾ, "t"ਖੰਡਾਂ_FNAME);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ਜੇ (fd == -1) {
		ਮੁਕਤ(tname);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	ਖੰਡਾਂ_ਮਾਪ = (ਖੰਡ_FILE_ਗਿਣਤੀ * ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ) * ਖੰਡ_USE_MAP_ਮਾਪ;

	ਲਈ (elem_idx = 0; elem_idx < ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ; elem_idx++) {
		ਲਈ (bfidx = 0; bfidx < ਖੰਡ_FILE_ਗਿਣਤੀ; bfidx++) {
			wr = ਲਿਖ(fd,
				   &state->ਖੰਡਾਂ[elem_idx][bfidx].use_map[0],
				   ਖੰਡ_USE_MAP_ਮਾਪ);
			ਜੇ (wr != ਖੰਡ_USE_MAP_ਮਾਪ) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				ਜਾਓ wr_err;
			}
			written += wr;
		}
	}
wr_err:
	close(fd);

	/* check all data was written */
	ਜੇ (written != ਖੰਡਾਂ_ਮਾਪ) {
		unlink(tname);
		ਮੁਕਤ(tname);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, ਖੰਡਾਂ_FNAME);
	ਜੇ (ret != NSERROR_OK) {
		unlink(tname);
		ਮੁਕਤ(tname);
		ਵਾਪਸ ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(ਖਾਲੀ)remove(fname);
	ਜੇ (rename(tname, fname) != 0) {
		unlink(tname);
		ਮੁਕਤ(tname);
		ਮੁਕਤ(fname);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror ਸੈੱਟ_ਖੰਡ_extents(ਬਣਤਰ store_state *state)
{
	ਪੂਰਨ bfidx; /* block file index */
	ਪੂਰਨ elem_idx;
	ਪੂਰਨ ftr;

	ਜੇ (state->ਖੰਡਾਂ_opened == false) {
		/* no blocks have been opened since last write */
		ਵਾਪਸ NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	ਲਈ (elem_idx = 0; elem_idx < ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ; elem_idx++) {
		ਲਈ (bfidx = 0; bfidx < ਖੰਡ_FILE_ਗਿਣਤੀ; bfidx++) {
			ਜੇ (state->ਖੰਡਾਂ[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->ਖੰਡਾਂ[elem_idx][bfidx].fd, 1U << (log2_ਖੰਡ_ਮਾਪ[elem_idx] + ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ));
				ਜੇ (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      errno);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->ਖੰਡਾਂ_opened = false;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ ਖਾਲੀ control_maintenance(ਖਾਲੀ *s)
{
	ਬਣਤਰ store_state *state = s;

	write_entries(state);
	write_ਖੰਡਾਂ(state);
	ਸੈੱਟ_ਖੰਡ_extents(state);
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
ਸਥਿਰ nserror
get_store_ਇੰਦਰਾਜ(ਬਣਤਰ store_state *state, nsurl *url, ਬਣਤਰ store_ਇੰਦਰਾਜ **bse)
{
	ਬਣਤਰ store_ਇੰਦਰਾਜ *ent;

	ent = hashmap_lookup(state->entries, url);

	ਜੇ (ent == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	*bse = ent;

	ent->ਆਖਰੀ_ਵਰਤਿਆ = time(ਨਿਰਾ);
	ent->use_ਗਿਣਤੀ++;

	state->entries_dirty = true;

	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(CONTROL_MAINT_ਸਮਾਂ, control_maintenance, state);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Find next available small block.
 */
ਸਥਿਰ ਖੰਡ_ਸੂਚਕਾਂਕ_t alloc_ਖੰਡ(ਬਣਤਰ store_state *state, ਪੂਰਨ elem_idx)
{
	ਪੂਰਨ bf;
	ਪੂਰਨ idx;
	ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ;
	uint8_t *map;

	ਲਈ (bf = 0; bf < ਖੰਡ_FILE_ਗਿਣਤੀ; bf++) {
		map = &state->ਖੰਡਾਂ[elem_idx][bf].use_map[0];

		ਲਈ (idx = 0; idx < ਖੰਡ_USE_MAP_ਮਾਪ; idx++) {
			ਜੇ (*(map + idx) != 0xff) {
				/* located an unused block */
				ਲਈ (ਦੋਆਧਾਰੀ_ਅੰਕ = 0; ਦੋਆਧਾਰੀ_ਅੰਕ < 8;ਦੋਆਧਾਰੀ_ਅੰਕ++) {
					ਜੇ (((*(map + idx)) & (1U << ਦੋਆਧਾਰੀ_ਅੰਕ)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << ਦੋਆਧਾਰੀ_ਅੰਕ;
						state->ਖੰਡਾਂ_dirty = true;
						ਵਾਪਸ (((bf * ਖੰਡ_USE_MAP_ਮਾਪ) + idx) * 8) + ਦੋਆਧਾਰੀ_ਅੰਕ;
					}
				}
			}
		}
	}

	ਵਾਪਸ 0;
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
ਸਥਿਰ nserror
ਸੈੱਟ_store_ਇੰਦਰਾਜ(ਬਣਤਰ store_state *state,
		nsurl *url,
		ਪੂਰਨ elem_idx,
		uint8_t *ਅੰਕੜੇ,
		ਅਚਲ size_t datalen,
		ਬਣਤਰ store_ਇੰਦਰਾਜ **bse)
{
	ਬਣਤਰ store_ਇੰਦਰਾਜ *se;
	nserror ret;
	ਬਣਤਰ store_ਇੰਦਰਾਜ_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	se = hashmap_lookup(state->entries, url);
	ਜੇ (se == ਨਿਰਾ) {
		se = hashmap_ਪਾਓ(state->entries, url);
	}
	ਜੇ (se == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	ਜੇ ((elem->flags & (ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ | ਇੰਦਰਾਜ_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		ਵਾਪਸ NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_ਗਿਣਤੀ = 1;
	se->ਆਖਰੀ_ਵਰਤਿਆ = time(ਨਿਰਾ);

	/* store the data in the element */
	elem->flags |= ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ;
	elem->ਅੰਕੜੇ = ਅੰਕੜੇ;
	elem->ref = 1;

	/* account for size of entry element */
	state->ਕੁੱਲ_alloc -= elem->ਮਾਪ;
	elem->ਮਾਪ = datalen;
	state->ਕੁੱਲ_alloc += elem->ਮਾਪ;

	/* if the element will fit in a small block attempt to allocate one */
	ਜੇ (elem->ਮਾਪ <= (1U << log2_ਖੰਡ_ਮਾਪ[elem_idx])) {
		elem->ਖੰਡ = alloc_ਖੰਡ(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(CONTROL_MAINT_ਸਮਾਂ, control_maintenance, state);

	*bse = se;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ ਪੂਰਨ
store_open(ਬਣਤਰ store_state *state,
	   ਇੰਦਰਾਜ_ident_t ident,
	   ਪੂਰਨ elem_idx,
	   ਪੂਰਨ openflags)
{
	ਅੱਖਰ *fname;
	nserror ret;
	ਪੂਰਨ fd;

	fname = store_fname(state, ident, elem_idx);
	ਜੇ (fname == ਨਿਰਾ) {
		NSLOG(netsurf, ERROR, "filename error");
		ਵਾਪਸ -1;
	}

	/* ensure all path elements to file exist if creating file */
	ਜੇ (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		ਜੇ (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			ਮੁਕਤ(fname);
			ਵਾਪਸ -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = open(fname, openflags, S_IRUSR | S_IWUSR);

	ਮੁਕਤ(fname);

	ਵਾਪਸ fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
unlink_entries(ਬਣਤਰ store_state *state)
{
	ਅੱਖਰ *fname = ਨਿਰਾ;
	nserror ret;

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, ENTRIES_FNAME);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	unlink(fname);

	ਮੁਕਤ(fname);
	ਵਾਪਸ NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
ਸਥਿਰ nserror
read_entries(ਬਣਤਰ store_state *state)
{
	ਅੱਖਰ *fname = ਨਿਰਾ;
	ਅੱਖਰ *url;
	nsurl *nsurl;
	nserror ret;
	size_t read_entries = 0;
	ਬਣਤਰ store_ਇੰਦਰਾਜ *ent;
	ਪੂਰਨ fd;

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, ENTRIES_FNAME);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	ਜੇ (state->entries == ਨਿਰਾ) {
		ਮੁਕਤ(fname);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	fd = open(fname, O_RDWR);
	ਜੇ (fd != -1) {
		uint32_t urllen;
		ਜਦੋਂ (ਪਠ(fd, &urllen, ਆਕਾਰ(urllen)) == ਆਕਾਰ(urllen)) {
			url = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, urllen+1);
			ਜੇ (url == ਨਿਰਾ) {
				close(fd);
				ਮੁਕਤ(fname);
				ਵਾਪਸ NSERROR_NOMEM;
			}
			ਜੇ (ਪਠ(fd, url, urllen) != (ssize_t)urllen) {
				ਮੁਕਤ(url);
				close(fd);
				ਮੁਕਤ(fname);
				ਵਾਪਸ NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			ਜੇ (ret != NSERROR_OK) {
				ਮੁਕਤ(url);
				close(fd);
				ਮੁਕਤ(fname);
				ਵਾਪਸ ret;
			}
			ਮੁਕਤ(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_ਪਾਓ(state->entries, nsurl);
			ਜੇ (ent == ਨਿਰਾ) {
				nsurl_unref(nsurl);
				close(fd);
				ਮੁਕਤ(fname);
				ਵਾਪਸ NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			ਜੇ (ਪਠ(fd, ent, ਆਕਾਰ(*ent)) != ਆਕਾਰ(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				close(fd);
				ਮੁਕਤ(fname);
				ਵਾਪਸ NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->ਕੁੱਲ_alloc += ent->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].ਮਾਪ;
			state->ਕੁੱਲ_alloc += ent->elem[ਇੰਦਰਾਜ_ELEM_META].ਮਾਪ;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ].flags &= ~(ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ | ਇੰਦਰਾਜ_ELEM_FLAG_MMAP);
			ent->elem[ਇੰਦਰਾਜ_ELEM_META].flags &= ~(ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ | ਇੰਦਰਾਜ_ELEM_FLAG_MMAP);

		}
		close(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	ਮੁਕਤ(fname);
	ਵਾਪਸ NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
read_ਖੰਡਾਂ(ਬਣਤਰ store_state *state)
{
	ਪੂਰਨ bfidx; /* block file index */
	ਪੂਰਨ elem_idx;
	ਪੂਰਨ fd;
	ssize_t rd;
	ਅੱਖਰ *fname = ਨਿਰਾ;
	nserror ret;

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, ਖੰਡਾਂ_FNAME);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = open(fname, O_RDWR);
	ਮੁਕਤ(fname);
	ਜੇ (fd != -1) {
		/* initialise block file use array */
		ਲਈ (elem_idx = 0; elem_idx < ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ; elem_idx++) {
			ਲਈ (bfidx = 0; bfidx < ਖੰਡ_FILE_ਗਿਣਤੀ; bfidx++) {
				rd = ਪਠ(fd,
					  &state->ਖੰਡਾਂ[elem_idx][bfidx].use_map[0],
					  ਖੰਡ_USE_MAP_ਮਾਪ);
				ਜੇ (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					ਜਾਓ rd_err;
				}
			}
		}
	rd_err:
		close(fd);

	} ਨਹੀਂਤਾਂ {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ][0].use_map[0] = 1;
		state->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	ਲਈ (bfidx = 0; bfidx < ਖੰਡ_FILE_ਗਿਣਤੀ; bfidx++) {
		state->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ][bfidx].fd = -1;
		state->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_META][bfidx].fd = -1;
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
write_cache_ਟੈਗ(ਬਣਤਰ store_state *state)
{
	FILE *fcachetag;
	nserror ret;
	ਅੱਖਰ *fname = ਨਿਰਾ;

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, "CACHEDIR.TAG");
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	fcachetag = fopen(fname, "wb");

	ਮੁਕਤ(fname);

	ਜੇ (fcachetag == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	fprintf(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	fclose(fcachetag);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
write_control(ਬਣਤਰ store_state *state)
{
	FILE *fcontrol;
	nserror ret;
	ਅੱਖਰ *fname = ਨਿਰਾ;

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, "control");
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	ਜੇ (ret != NSERROR_OK) {
		ਮੁਕਤ(fname);
		ਵਾਪਸ ret;
	}

	fcontrol = fopen(fname, "wb");

	ਮੁਕਤ(fname);

	ਜੇ (fcontrol == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	fprintf(fcontrol, "%u%c", CONTROL_ਸੰਸਕਰਣ, 0);

	fclose(fcontrol);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
read_control(ਬਣਤਰ store_state *state)
{
	nserror ret;
	FILE *fcontrol;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ctrlversion;
	ਅੱਖਰ *fname = ਨਿਰਾ;

	ret = netsurf_mkpath(&fname, ਨਿਰਾ, 2, state->ਰਸਤਾ, "control");
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = fopen(fname, "rb");

	ਮੁਕਤ(fname);

	ਜੇ (fcontrol == ਨਿਰਾ) {
		/* unable to open control file */
		ਜੇ (errno == ENOENT) {
			ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
		} ਨਹੀਂਤਾਂ {
			ਵਾਪਸ NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	ਜੇ (fscanf(fcontrol, "%u", &ctrlversion) != 1) {
		ਜਾਓ control_error;
	}

	ਜੇ (ctrlversion != CONTROL_ਸੰਸਕਰਣ) {
		ਜਾਓ control_error;
	}

	ਜੇ (fgetc(fcontrol) != 0) {
		ਜਾਓ control_error;
	}

	fclose(fcontrol);

	ਵਾਪਸ NSERROR_OK;

control_error: /* problem with the control file */

	fclose(fcontrol);

	ਵਾਪਸ NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror
initialise(ਅਚਲ ਬਣਤਰ llcache_store_parameters *parameters)
{
	ਬਣਤਰ store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	ਜੇ (storestate != ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	ਜੇ (parameters->ਹੱਦਬੰਦੀ == 0) {
		ਵਾਪਸ NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	ਜੇ (parameters->ਰਸਤਾ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ store_state));
	ਜੇ (newstate == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	newstate->ਰਸਤਾ = strdup(parameters->ਰਸਤਾ);
	newstate->ਹੱਦਬੰਦੀ = parameters->ਹੱਦਬੰਦੀ;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	ਜੇ (ret != NSERROR_OK) {
		ਜੇ (ret == NSERROR_NOT_ਮਿਲਿਆ) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} ਨਹੀਂਤਾਂ {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->ਰਸਤਾ);
			ਜੇ (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->ਰਸਤਾ);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} ਨਹੀਂਤਾਂ {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->ਰਸਤਾ);
			}
		}
		ret = write_control(newstate);
		ਜੇ (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_ਟੈਗ(newstate);
		}
	}
	ਜੇ (ret != NSERROR_OK) {
		/* that went well obviously */
		ਮੁਕਤ(newstate->ਰਸਤਾ);
		ਮੁਕਤ(newstate);
		ਵਾਪਸ ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	ਜੇ (ret != NSERROR_OK) {
		/* that went well obviously */
		ਮੁਕਤ(newstate->ਰਸਤਾ);
		ਮੁਕਤ(newstate);
		ਵਾਪਸ ret;
	}

	/* read blocks */
	ret = read_ਖੰਡਾਂ(newstate);
	ਜੇ (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		ਮੁਕਤ(newstate->ਰਸਤਾ);
		ਮੁਕਤ(newstate);
		ਵਾਪਸ ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->ਰਸਤਾ,
	      newstate->ਹੱਦਬੰਦੀ,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->ਕੁੱਲ_alloc, newstate->ਹੱਦਬੰਦੀ);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
ਸਥਿਰ nserror
finalise(ਖਾਲੀ)
{
	ਪੂਰਨ bf; /* block file index */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ op_ਗਿਣਤੀ;

	ਜੇ (storestate != ਨਿਰਾ) {
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_ਖੰਡਾਂ(storestate);

		/* ensure all block files are closed */
		ਲਈ (bf = 0; bf < ਖੰਡ_FILE_ਗਿਣਤੀ; bf++) {
			ਜੇ (storestate->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ][bf].fd != -1) {
				close(storestate->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ][bf].fd);
			}
			ਜੇ (storestate->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_META][bf].fd != -1) {
				close(storestate->ਖੰਡਾਂ[ਇੰਦਰਾਜ_ELEM_META][bf].fd);
			}
		}

		op_ਗਿਣਤੀ = storestate->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ + storestate->ਖੁੰਝ_ਗਿਣਤੀ;

		/* avoid division by zero */
		ਜੇ (op_ਗਿਣਤੀ > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_ਗਿਣਤੀ,
			      storestate->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ,
			      storestate->ਖੁੰਝ_ਗਿਣਤੀ,
			      0,
			      (storestate->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ * 100) / op_ਗਿਣਤੀ,
			      (storestate->ਖੁੰਝ_ਗਿਣਤੀ * 100) / op_ਗਿਣਤੀ,
			      0);
		}

		hashmap_destroy(storestate->entries);
		ਮੁਕਤ(storestate->ਰਸਤਾ);
		ਮੁਕਤ(storestate);
		storestate = ਨਿਰਾ;
	}
	ਵਾਪਸ NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ਸਥਿਰ nserror store_write_ਖੰਡ(ਬਣਤਰ store_state *state,
			 ਬਣਤਰ store_ਇੰਦਰਾਜ *bse,
			 ਪੂਰਨ elem_idx)
{
	ਖੰਡ_ਸੂਚਕਾਂਕ_t bf = (bse->elem[elem_idx].ਖੰਡ >> ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ) &
		((1 << ਖੰਡ_FILE_ਗਿਣਤੀ) - 1); /* block file block resides in */
	ਖੰਡ_ਸੂਚਕਾਂਕ_t bi = bse->elem[elem_idx].ਖੰਡ & ((1U << ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	ਜੇ (state->ਖੰਡਾਂ[elem_idx][bf].fd == -1) {
		state->ਖੰਡਾਂ[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ, O_CREAT | O_RDWR);
		ਜੇ (state->ਖੰਡਾਂ[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			ਵਾਪਸ NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->ਖੰਡਾਂ_opened = true;
	}

	offst = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)bi << log2_ਖੰਡ_ਮਾਪ[elem_idx];

	wr = nsu_pwrite(state->ਖੰਡਾਂ[elem_idx][bf].fd,
			bse->elem[elem_idx].ਅੰਕੜੇ,
			bse->elem[elem_idx].ਮਾਪ,
			offst);
	ਜੇ (wr != (ssize_t)bse->elem[elem_idx].ਮਾਪ) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].ਮਾਪ,
		      bse->elem[elem_idx].ਅੰਕੜੇ,
		      (size_t)offst,
		      bse->elem[elem_idx].ਖੰਡ,
		      errno);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].ਅੰਕੜੇ, (size_t)offst,
	      bse->elem[elem_idx].ਖੰਡ);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ਸਥਿਰ nserror store_write_file(ਬਣਤਰ store_state *state,
			 ਬਣਤਰ store_ਇੰਦਰਾਜ *bse,
			 ਪੂਰਨ elem_idx)
{
	ssize_t wr;
	ਪੂਰਨ fd;
	ਪੂਰਨ err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	ਜੇ (fd < 0) {
		perror("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	wr = ਲਿਖ(fd, bse->elem[elem_idx].ਅੰਕੜੇ, bse->elem[elem_idx].ਮਾਪ);
	err = errno; /* close can change errno */

	close(fd);
	ਜੇ (wr != (ssize_t)bse->elem[elem_idx].ਮਾਪ) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].ਮਾਪ,
		      bse->elem[elem_idx].ਅੰਕੜੇ,
		      err);

		/** @todo Delete the file? */
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].ਅੰਕੜੇ);

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
store(nsurl *url,
      ਗਿਣਤੀ backing_store_flags bsflags,
      uint8_t *ਅੰਕੜੇ,
      ਅਚਲ size_t datalen)
{
	nserror ret;
	ਬਣਤਰ store_ਇੰਦਰਾਜ *bse;
	ਪੂਰਨ elem_idx;

	/* check backing store is initialised */
	ਜੇ (storestate == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	ਜੇ ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ਇੰਦਰਾਜ_ELEM_META;
	} ਨਹੀਂਤਾਂ {
		elem_idx = ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ;
	}

	/* set the store entry up */
	ret = ਸੈੱਟ_store_ਇੰਦਰਾਜ(storestate, url, elem_idx, ਅੰਕੜੇ, datalen, &bse);
	ਜੇ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		ਵਾਪਸ ret;
	}

	ਜੇ (bse->elem[elem_idx].ਖੰਡ != 0) {
		/* small block storage */
		ret = store_write_ਖੰਡ(storestate, bse, elem_idx);
	} ਨਹੀਂਤਾਂ {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	ਵਾਪਸ ret;
}

/**
 * release any allocation for an entry
 */
ਸਥਿਰ nserror ਇੰਦਰਾਜ_release_alloc(ਬਣਤਰ store_ਇੰਦਰਾਜ_element *elem)
{
	ਜੇ ((elem->flags & ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ) != 0) {
		elem->ref--;
		ਜੇ (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->ਅੰਕੜੇ);
			ਮੁਕਤ(elem->ਅੰਕੜੇ);
			elem->flags &= ~ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ;
		}
	}
	ਵਾਪਸ NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ਸਥਿਰ nserror store_read_ਖੰਡ(ਬਣਤਰ store_state *state,
			 ਬਣਤਰ store_ਇੰਦਰਾਜ *bse,
			 ਪੂਰਨ elem_idx)
{
	ਖੰਡ_ਸੂਚਕਾਂਕ_t bf = (bse->elem[elem_idx].ਖੰਡ >> ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ) &
		((1 << ਖੰਡ_FILE_ਗਿਣਤੀ) - 1); /* block file block resides in */
	ਖੰਡ_ਸੂਚਕਾਂਕ_t bi = bse->elem[elem_idx].ਖੰਡ & ((1 << ਖੰਡ_ਇੰਦਰਾਜ_ਗਿਣਤੀ) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	ਜੇ (state->ਖੰਡਾਂ[elem_idx][bf].fd == -1) {
		state->ਖੰਡਾਂ[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ਇੰਦਰਾਜ_ELEM_ਗਿਣਤੀ, O_CREAT | O_RDWR);
		ਜੇ (state->ਖੰਡਾਂ[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			ਵਾਪਸ NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->ਖੰਡਾਂ_opened = true;
	}

	offst = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)bi << log2_ਖੰਡ_ਮਾਪ[elem_idx];

	rd = nsu_pread(state->ਖੰਡਾਂ[elem_idx][bf].fd,
		       bse->elem[elem_idx].ਅੰਕੜੇ,
		       bse->elem[elem_idx].ਮਾਪ,
		       offst);
	ਜੇ (rd != (ssize_t)bse->elem[elem_idx].ਮਾਪ) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].ਮਾਪ,
		      bse->elem[elem_idx].ਅੰਕੜੇ,
		      (size_t)offst,
		      bse->elem[elem_idx].ਖੰਡ,
		      errno);
		ਵਾਪਸ NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].ਅੰਕੜੇ, (size_t)offst,
	      bse->elem[elem_idx].ਖੰਡ);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ਸਥਿਰ nserror store_read_file(ਬਣਤਰ store_state *state,
			 ਬਣਤਰ store_ਇੰਦਰਾਜ *bse,
			 ਪੂਰਨ elem_idx)
{
	ਪੂਰਨ fd;
	ssize_t rd; /* return from read */
	ਪੂਰਨ ret = NSERROR_OK;
	size_t tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	ਜੇ (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		/** @todo should this invalidate the entry? */
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	ਜਦੋਂ (tot < bse->elem[elem_idx].ਮਾਪ) {
		rd = ਪਠ(fd,
			  bse->elem[elem_idx].ਅੰਕੜੇ + tot,
			  bse->elem[elem_idx].ਮਾਪ - tot);
		ਜੇ (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      errno);
			ret = NSERROR_NOT_ਮਿਲਿਆ;
			ਰੋਕੋ;
		}
		tot += rd;
	}

	close(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].ਅੰਕੜੇ);

	ਵਾਪਸ ret;
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
ਸਥਿਰ nserror
fetch(nsurl *url,
      ਗਿਣਤੀ backing_store_flags bsflags,
      uint8_t **ਅੰਕੜੇ_ਬਾਹਰਲਾ,
      size_t *datalen_ਬਾਹਰਲਾ)
{
	nserror ret;
	ਬਣਤਰ store_ਇੰਦਰਾਜ *bse;
	ਬਣਤਰ store_ਇੰਦਰਾਜ_element *elem;
	ਪੂਰਨ elem_idx;

	/* check backing store is initialised */
	ਜੇ (storestate == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_ਇੰਦਰਾਜ(storestate, url, &bse);
	ਜੇ (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->ਖੁੰਝ_ਗਿਣਤੀ++;
		ਵਾਪਸ ret;
	}
	storestate->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	ਜੇ ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ਇੰਦਰਾਜ_ELEM_META;
	} ਨਹੀਂਤਾਂ {
		elem_idx = ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	ਜੇ ((elem->flags & ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->ਅੰਕੜੇ, elem->ref);

	} ਨਹੀਂਤਾਂ {
		/* allocate from the heap */
		elem->ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(elem->ਮਾਪ);
		ਜੇ (elem->ਅੰਕੜੇ == ਨਿਰਾ) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			ਵਾਪਸ NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->ਅੰਕੜੇ);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= ਇੰਦਰਾਜ_ELEM_FLAG_ਢੇਰ;
		elem->ref = 1;

		/* fill the new block */
		ਜੇ (elem->ਖੰਡ != 0) {
			ret = store_read_ਖੰਡ(storestate, bse, elem_idx);
		} ਨਹੀਂਤਾਂ {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	ਜੇ (ret != NSERROR_OK) {
		ਇੰਦਰਾਜ_release_alloc(elem);
	} ਨਹੀਂਤਾਂ {
		/* update stats and setup return pointers */
		storestate->ਪ੍ਰਾਪਤੀ_ਮਾਪ += elem->ਮਾਪ;

		*ਅੰਕੜੇ_ਬਾਹਰਲਾ = elem->ਅੰਕੜੇ;
		*datalen_ਬਾਹਰਲਾ = elem->ਮਾਪ;
	}

	ਵਾਪਸ ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
ਸਥਿਰ nserror release(nsurl *url, ਗਿਣਤੀ backing_store_flags bsflags)
{
	nserror ret;
	ਬਣਤਰ store_ਇੰਦਰਾਜ *bse;
	ਬਣਤਰ store_ਇੰਦਰਾਜ_element *elem;

	/* check backing store is initialised */
	ਜੇ (storestate == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	ret = get_store_ਇੰਦਰਾਜ(storestate, url, &bse);
	ਜੇ (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		ਵਾਪਸ ret;
	}

	/* the entry element */
	ਜੇ ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[ਇੰਦਰਾਜ_ELEM_META];
	} ਨਹੀਂਤਾਂ {
		elem = &bse->elem[ਇੰਦਰਾਜ_ELEM_ਅੰਕੜੇ];
	}

	ret = ਇੰਦਰਾਜ_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	ਜੇ ((ret == NSERROR_OK) &&
	    ((bse->flags & ਇੰਦਰਾਜ_FLAGS_INVALID) != 0)) {
		ret = invalidate_ਇੰਦਰਾਜ(storestate, bse);
	}

	ਵਾਪਸ ret;
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
ਸਥਿਰ nserror
invalidate(nsurl *url)
{
	nserror ret;
	ਬਣਤਰ store_ਇੰਦਰਾਜ *bse;

	/* check backing store is initialised */
	ਜੇ (storestate == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	ret = get_store_ਇੰਦਰਾਜ(storestate, url, &bse);
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	ਵਾਪਸ invalidate_ਇੰਦਰਾਜ(storestate, bse);
}


ਸਥਿਰ ਬਣਤਰ gui_llcache_ਸਾਰਣੀ llcache_ਸਾਰਣੀ = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

ਬਣਤਰ gui_llcache_ਸਾਰਣੀ *filesystem_llcache_ਸਾਰਣੀ = &llcache_ਸਾਰਣੀ;
