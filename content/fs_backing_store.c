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

#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <nsutils/unistd.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/filepath.h"
#অন্তর্ভাব "utils/file.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/hashmap.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "netsurf/misc.h"

#অন্তর্ভাব "content/backing_store.h"

/** Backing store file format version */
#define CONTROL_VERSION 202

/**
 * Number of milliseconds after a update before control data
 * maintenance is performed
 */
#define CONTROL_MAINT_TIME 10000

/** Filename of serialised entries */
#define ENTRIES_FNAME "entries"

/** Filename of block file index */
#define BLOCKS_FNAME "blocks"

/** log2 block data address length (64k) */
#define BLOCK_ADDR_LEN 16

/** log2 number of entries per block file(1024) */
#define BLOCK_ENTRY_গুনতি 10

/** log2 number of data block files */
#define BLOCK_FILE_গুনতি (BLOCK_ADDR_LEN - BLOCK_ENTRY_গুনতি)

/** log2 size of data blocks (8k) */
#define BLOCK_তথ্য_আকার 13

/** log2 size of metadata blocks (8k) */
#define BLOCK_META_আকার 13

/** length in bytes of a block files use map */
#define BLOCK_USE_MAP_আকার (1 << (BLOCK_ENTRY_গুনতি - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
প্রকারলেখ uint32_t entry_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
প্রকারলেখ uint16_t block_সূচক_t;


/**
 * Entry element index values.
 */
গণনা store_entry_elem_idx {
	ENTRY_ELEM_তথ্য = 0, /**< entry element is data */
	ENTRY_ELEM_META = 1,  /**< entry element is metadata */
	ENTRY_ELEM_গুনতি = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
গণনা store_entry_elem_flags {
	/** store not managing any allocation on entry */
	ENTRY_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	ENTRY_ELEM_FLAG_HEAP = 0x1,
	/** entry data allocation is mmaped */
	ENTRY_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	ENTRY_ELEM_FLAG_SMALL = 0x4,
};


গণনা store_entry_flags {
	/** entry is normal */
	ENTRY_FLAGS_NONE = 0,
	/** entry has been invalidated but something still holding a reference */
	ENTRY_FLAGS_INVALID = 1,
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
কাঠামো store_entry_element {
	uint8_t* তথ্য; /**< data allocated */
	uint32_t আকার_2; /**< size of entry element on disc */
	block_সূচক_t block; /**< small object data block */
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
কাঠামো store_entry {
	nsurl *url; /**< The URL for this entry */
	int64_t last_used; /**< UNIX time the entry was last used */
	uint16_t use_গুনতি; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	কাঠামো store_entry_element elem[ENTRY_ELEM_গুনতি];
};

/**
 * Small block file.
 */
কাঠামো block_file {
	/** file descriptor of the block file */
	পূর্ণসংখ্যা fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[BLOCK_USE_MAP_আকার];
};

/**
 * log2 of block size.
 */
স্থির ধ্রুবক unsigned পূর্ণসংখ্যা log2_block_আকার[ENTRY_ELEM_গুনতি] = {
	BLOCK_তথ্য_আকার, /**< Data block size */
	BLOCK_META_আকার  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
কাঠামো store_state {
	/* store config */
	অক্ষর *path; /**< The path to the backing store */
	size_t limit; /**< The backing store upper bound target size */
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
	কাঠামো block_file blocks[ENTRY_ELEM_গুনতি][BLOCK_FILE_গুনতি];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool blocks_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool blocks_opened;


	/* stats */
	uint64_t যোগ_alloc; /**< total size of all allocated storage. */

	size_t hit_গুনতি; /**< number of cache hits */
	uint64_t hit_আকার; /**< size of storage served */
	size_t miss_গুনতি; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
কাঠামো store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

স্থির bool
entries_hashmap_চাবি_eq(শূন্য *key1, শূন্য *key2)
{
	ফেরত nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

স্থির শূন্য *
entries_hashmap_মান_alloc(শূন্য *চাবি)
{
	কাঠামো store_entry *ent = calloc(1, আকার(কাঠামো store_entry));
	যদি (ent != ফাঁকা) {
		ent->url = nsurl_ref(চাবি);
	}
	ফেরত ent;
}

স্থির শূন্য
entries_hashmap_মান_destroy(শূন্য *মান)
{
	কাঠামো store_entry *ent = মান;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	মুক্ত(ent);
}

স্থির hashmap_parameters_t entries_hashmap_parameters = {
	.চাবি_clone = (hashmap_চাবি_clone_t)nsurl_ref,
	.চাবি_destroy = (hashmap_চাবি_destroy_t)nsurl_unref,
	.চাবি_hash = (hashmap_চাবি_hash_t)nsurl_hash,
	.চাবি_eq = entries_hashmap_চাবি_eq,
	.মান_alloc = entries_hashmap_মান_alloc,
	.মান_destroy = entries_hashmap_মান_destroy,
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
স্থির অক্ষর *
store_fname(কাঠামো store_state *state,
	    entry_ident_t ident,
	    পূর্ণসংখ্যা elem_idx)
{
	অক্ষর *fname = ফাঁকা;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	ধ্রুবক uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	ধ্রুবক অক্ষর *base_dir_table[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	ধ্রুবক uint8_t encoding_table[64][3] = {
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
	b32u_i[0] = encoding_table[(ident      ) & 0x1f][0];
	b32u_i[1] = encoding_table[(ident >>  5) & 0x1f][0];
	b32u_i[2] = encoding_table[(ident >> 10) & 0x1f][0];
	b32u_i[3] = encoding_table[(ident >> 15) & 0x1f][0];
	b32u_i[4] = encoding_table[(ident >> 20) & 0x1f][0];
	b32u_i[5] = encoding_table[(ident >> 25) & 0x1f][0];
	b32u_i[6] = encoding_table[(ident >> 30) & 0x1f][0];
	b32u_i[7] = 0; /* null terminate ident string */

	/* base32 encode directory separators */
	b32u_d[0] = (uint8_t*)base_dir_table[elem_idx];
	b32u_d[1] = &encoding_table[(ident      ) & 0x3f][0];
	b32u_d[2] = &encoding_table[(ident >>  6) & 0x3f][0];
	b32u_d[3] = &encoding_table[(ident >> 12) & 0x3f][0];
	b32u_d[4] = &encoding_table[(ident >> 18) & 0x3f][0];
	b32u_d[5] = &encoding_table[(ident >> 24) & 0x3f][0];

	নির্বাচন (elem_idx) {
	ক্ষেত্র ENTRY_ELEM_তথ্য:
	ক্ষেত্র ENTRY_ELEM_META:
		netsurf_mkpath(&fname, ফাঁকা, 8,
			       state->path, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		বিরতি;

	ক্ষেত্র (ENTRY_ELEM_গুনতি + ENTRY_ELEM_META):
	ক্ষেত্র (ENTRY_ELEM_গুনতি + ENTRY_ELEM_তথ্য):
		netsurf_mkpath(&fname, ফাঁকা, 3,
			       state->path, b32u_d[0], b32u_d[1]);
		বিরতি;

	পূর্বনির্ধারিত:
		প্রতিজ্ঞা("bad element index" == ফাঁকা);
		বিরতি;
	}

	ফেরত fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
invalidate_element(কাঠামো store_state *state,
		   কাঠামো store_entry *bse,
		   পূর্ণসংখ্যা elem_idx)
{
	যদি (bse->elem[elem_idx].block != 0) {
		block_সূচক_t bf;
		block_সূচক_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_গুনতি) &
			((1 << BLOCK_FILE_গুনতি) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].block & ((1U << BLOCK_ENTRY_গুনতি) -1);

		/* clear bit in use map */
		state->blocks[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} অন্যথা {
		অক্ষর *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		যদি (fname == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}
		unlink(fname);
		মুক্ত(fname);
	}

	state->যোগ_alloc -= bse->elem[elem_idx].আকার_2;

	ফেরত NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
invalidate_entry(কাঠামো store_state *state, কাঠামো store_entry *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= ENTRY_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	যদি (((bse->elem[ENTRY_ELEM_তথ্য].flags &
	     (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[ENTRY_ELEM_META].flags &
	      (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		ফেরত NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, ENTRY_ELEM_META);
	যদি (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, ENTRY_ELEM_তথ্য);
	যদি (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	ফেরত NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
স্থির পূর্ণসংখ্যা compar(ধ্রুবক শূন্য *va, ধ্রুবক শূন্য *vb)
{
	ধ্রুবক কাঠামো store_entry *a = *(ধ্রুবক কাঠামো store_entry **)va;
	ধ্রুবক কাঠামো store_entry *b = *(ধ্রুবক কাঠামো store_entry **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	যদি ((a->elem[ENTRY_ELEM_তথ্য].flags == ENTRY_ELEM_FLAG_NONE) &&
	    (b->elem[ENTRY_ELEM_তথ্য].flags != ENTRY_ELEM_FLAG_NONE)) {
		ফেরত -1;
	} অন্যথা যদি ((a->elem[ENTRY_ELEM_তথ্য].flags != ENTRY_ELEM_FLAG_NONE) &&
		   (b->elem[ENTRY_ELEM_তথ্য].flags == ENTRY_ELEM_FLAG_NONE)) {
		ফেরত 1;
	}

	যদি ((a->elem[ENTRY_ELEM_META].flags == ENTRY_ELEM_FLAG_NONE) &&
	    (b->elem[ENTRY_ELEM_META].flags != ENTRY_ELEM_FLAG_NONE)) {
		ফেরত -1;
	} অন্যথা যদি ((a->elem[ENTRY_ELEM_META].flags != ENTRY_ELEM_FLAG_NONE) &&
		   (b->elem[ENTRY_ELEM_META].flags == ENTRY_ELEM_FLAG_NONE)) {
		ফেরত 1;
	}

	যদি (a->use_গুনতি < b->use_গুনতি) {
		ফেরত -1;
	} অন্যথা যদি (a->use_গুনতি > b->use_গুনতি) {
		ফেরত 1;
	}
	/* use count is the same - now consider last use time */

	যদি (a->last_used < b->last_used) {
		ফেরত -1;
	} অন্যথা যদি (a->last_used > b->last_used) {
		ফেরত 1;
	}

	/* they are the same */
	ফেরত 0;
}

প্রকারলেখ কাঠামো {
	কাঠামো store_entry **elist;
	size_t ent_গুনতি;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
স্থির bool
entry_eviction_iterator_cb(শূন্য *চাবি, শূন্য *মান, শূন্য *ctx)
{
	eviction_state_t *estate = ctx;
	কাঠামো store_entry *ent = মান;
	estate->elist[estate->ent_গুনতি++] = ent;
	ফেরত false;
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
স্থির nserror store_evict(কাঠামো store_state *state)
{
	size_t ent = 0;
	size_t removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	size_t old_গুনতি;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	যদি (state->যোগ_alloc < state->limit) {
		/* cache within limits */
		ফেরত NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->যোগ_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_গুনতি = hashmap_গুনতি(state->entries);
	estate.ent_গুনতি = 0;
	estate.elist = malloc(আকার(কাঠামো state_entry*) * old_গুনতি);
	যদি (estate.elist == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	যদি (hashmap_iterate(state->entries, entry_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		মুক্ত(estate.elist);
		ফেরত NSERROR_UNKNOWN;
	}

	যদি (old_গুনতি != estate.ent_গুনতি) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		মুক্ত(estate.elist);
		ফেরত NSERROR_UNKNOWN;
	}

	qsort(estate.elist, estate.ent_গুনতি, আকার(কাঠামো state_entry*), compar);

	/* evict entries in listed order */
	removed = 0;
	জন্য (ent = 0; ent < estate.ent_গুনতি; ent++) {
		কাঠামো store_entry *bse = estate.elist[ent];

		removed += bse->elem[ENTRY_ELEM_তথ্য].আকার_2;
		removed += bse->elem[ENTRY_ELEM_META].আকার_2;

		ret = invalidate_entry(state, bse);
		যদি (ret != NSERROR_OK) {
			বিরতি;
		}

		যদি (removed > state->hysteresis) {
			বিরতি;
		}
	}

	মুক্ত(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->যোগ_alloc, old_গুনতি - ent);

	ফেরত ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
স্থির nserror
write_entry(কাঠামো store_entry *ent, পূর্ণসংখ্যা fd)
{
	uint32_t len = strlen(nsurl_access(ent->url));
	যদি (লিখ(fd, &len, আকার(len)) != আকার(len))
		ফেরত NSERROR_SAVE_FAILED;
	যদি (লিখ(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		ফেরত NSERROR_SAVE_FAILED;
	যদি (লিখ(fd, ent, আকার(*ent)) != আকার(*ent))
		ফেরত NSERROR_SAVE_FAILED;

	ফেরত NSERROR_OK;
}

প্রকারলেখ কাঠামো {
	পূর্ণসংখ্যা fd;
	size_t written;
} write_entry_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
স্থির bool
write_entry_iterator(শূন্য *চাবি, শূন্য *মান, শূন্য *ctx)
{
	/* We ignore the key */
	কাঠামো store_entry *ent = মান;
	write_entry_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	ফেরত write_entry(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror write_entries(কাঠামো store_state *state)
{
	অক্ষর *tname = ফাঁকা; /* temporary file name for atomic replace */
	অক্ষর *fname = ফাঁকা; /* target filename */
	write_entry_iteration_state weistate;
	nserror ret;

	memset(&weistate, 0, আকার(weistate));

	যদি (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		ফেরত NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ফাঁকা, 2, state->path, "t"ENTRIES_FNAME);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	weistate.fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	যদি (weistate.fd == -1) {
		মুক্ত(tname);
		ফেরত NSERROR_SAVE_FAILED;
	}

	যদি (hashmap_iterate(state->entries, write_entry_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		close(weistate.fd);
		unlink(tname);
		মুক্ত(tname);
		ফেরত NSERROR_SAVE_FAILED;
	}

	close(weistate.fd);

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, ENTRIES_FNAME);
	যদি (ret != NSERROR_OK) {
		unlink(tname);
		মুক্ত(tname);
		ফেরত ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(শূন্য)remove(fname);
	যদি (rename(tname, fname) != 0) {
		unlink(tname);
		মুক্ত(tname);
		মুক্ত(fname);
		ফেরত NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	ফেরত NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
স্থির nserror write_blocks(কাঠামো store_state *state)
{
	পূর্ণসংখ্যা fd;
	অক্ষর *tname = ফাঁকা; /* temporary file name for atomic replace */
	অক্ষর *fname = ফাঁকা; /* target filename */
	size_t blocks_আকার;
	size_t written = 0;
	size_t wr;
	nserror ret;
	পূর্ণসংখ্যা bfidx; /* block file index */
	পূর্ণসংখ্যা elem_idx;

	যদি (state->blocks_dirty == false) {
		/* blocks use maps have not been updated since last write */
		ফেরত NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ফাঁকা, 2, state->path, "t"BLOCKS_FNAME);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	যদি (fd == -1) {
		মুক্ত(tname);
		ফেরত NSERROR_SAVE_FAILED;
	}

	blocks_আকার = (BLOCK_FILE_গুনতি * ENTRY_ELEM_গুনতি) * BLOCK_USE_MAP_আকার;

	জন্য (elem_idx = 0; elem_idx < ENTRY_ELEM_গুনতি; elem_idx++) {
		জন্য (bfidx = 0; bfidx < BLOCK_FILE_গুনতি; bfidx++) {
			wr = লিখ(fd,
				   &state->blocks[elem_idx][bfidx].use_map[0],
				   BLOCK_USE_MAP_আকার);
			যদি (wr != BLOCK_USE_MAP_আকার) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				যাও wr_err;
			}
			written += wr;
		}
	}
wr_err:
	close(fd);

	/* check all data was written */
	যদি (written != blocks_আকার) {
		unlink(tname);
		মুক্ত(tname);
		ফেরত NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, BLOCKS_FNAME);
	যদি (ret != NSERROR_OK) {
		unlink(tname);
		মুক্ত(tname);
		ফেরত ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(শূন্য)remove(fname);
	যদি (rename(tname, fname) != 0) {
		unlink(tname);
		মুক্ত(tname);
		মুক্ত(fname);
		ফেরত NSERROR_SAVE_FAILED;
	}

	ফেরত NSERROR_OK;
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
স্থির nserror set_block_extents(কাঠামো store_state *state)
{
	পূর্ণসংখ্যা bfidx; /* block file index */
	পূর্ণসংখ্যা elem_idx;
	পূর্ণসংখ্যা ftr;

	যদি (state->blocks_opened == false) {
		/* no blocks have been opened since last write */
		ফেরত NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	জন্য (elem_idx = 0; elem_idx < ENTRY_ELEM_গুনতি; elem_idx++) {
		জন্য (bfidx = 0; bfidx < BLOCK_FILE_গুনতি; bfidx++) {
			যদি (state->blocks[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->blocks[elem_idx][bfidx].fd, 1U << (log2_block_আকার[elem_idx] + BLOCK_ENTRY_গুনতি));
				যদি (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      errno);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->blocks_opened = false;

	ফেরত NSERROR_OK;
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
স্থির শূন্য control_maintenance(শূন্য *s)
{
	কাঠামো store_state *state = s;

	write_entries(state);
	write_blocks(state);
	set_block_extents(state);
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
স্থির nserror
get_store_entry(কাঠামো store_state *state, nsurl *url, কাঠামো store_entry **bse)
{
	কাঠামো store_entry *ent;

	ent = hashmap_lookup(state->entries, url);

	যদি (ent == ফাঁকা) {
		ফেরত NSERROR_NOT_FOUND;
	}

	*bse = ent;

	ent->last_used = time(ফাঁকা);
	ent->use_গুনতি++;

	state->entries_dirty = true;

	guit->misc->schedule(CONTROL_MAINT_TIME, control_maintenance, state);

	ফেরত NSERROR_OK;
}

/**
 * Find next available small block.
 */
স্থির block_সূচক_t alloc_block(কাঠামো store_state *state, পূর্ণসংখ্যা elem_idx)
{
	পূর্ণসংখ্যা bf;
	পূর্ণসংখ্যা idx;
	পূর্ণসংখ্যা bit;
	uint8_t *map;

	জন্য (bf = 0; bf < BLOCK_FILE_গুনতি; bf++) {
		map = &state->blocks[elem_idx][bf].use_map[0];

		জন্য (idx = 0; idx < BLOCK_USE_MAP_আকার; idx++) {
			যদি (*(map + idx) != 0xff) {
				/* located an unused block */
				জন্য (bit = 0; bit < 8;bit++) {
					যদি (((*(map + idx)) & (1U << bit)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << bit;
						state->blocks_dirty = true;
						ফেরত (((bf * BLOCK_USE_MAP_আকার) + idx) * 8) + bit;
					}
				}
			}
		}
	}

	ফেরত 0;
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
স্থির nserror
set_store_entry(কাঠামো store_state *state,
		nsurl *url,
		পূর্ণসংখ্যা elem_idx,
		uint8_t *তথ্য,
		ধ্রুবক size_t datalen,
		কাঠামো store_entry **bse)
{
	কাঠামো store_entry *se;
	nserror ret;
	কাঠামো store_entry_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	se = hashmap_lookup(state->entries, url);
	যদি (se == ফাঁকা) {
		se = hashmap_ঢোকাও(state->entries, url);
	}
	যদি (se == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	যদি ((elem->flags & (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		ফেরত NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_গুনতি = 1;
	se->last_used = time(ফাঁকা);

	/* store the data in the element */
	elem->flags |= ENTRY_ELEM_FLAG_HEAP;
	elem->তথ্য = তথ্য;
	elem->ref = 1;

	/* account for size of entry element */
	state->যোগ_alloc -= elem->আকার_2;
	elem->আকার_2 = datalen;
	state->যোগ_alloc += elem->আকার_2;

	/* if the element will fit in a small block attempt to allocate one */
	যদি (elem->আকার_2 <= (1U << log2_block_আকার[elem_idx])) {
		elem->block = alloc_block(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->schedule(CONTROL_MAINT_TIME, control_maintenance, state);

	*bse = se;

	ফেরত NSERROR_OK;
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
স্থির পূর্ণসংখ্যা
store_open(কাঠামো store_state *state,
	   entry_ident_t ident,
	   পূর্ণসংখ্যা elem_idx,
	   পূর্ণসংখ্যা openflags)
{
	অক্ষর *fname;
	nserror ret;
	পূর্ণসংখ্যা fd;

	fname = store_fname(state, ident, elem_idx);
	যদি (fname == ফাঁকা) {
		NSLOG(netsurf, ERROR, "filename error");
		ফেরত -1;
	}

	/* ensure all path elements to file exist if creating file */
	যদি (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		যদি (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			মুক্ত(fname);
			ফেরত -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = open(fname, openflags, S_IRUSR | S_IWUSR);

	মুক্ত(fname);

	ফেরত fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
unlink_entries(কাঠামো store_state *state)
{
	অক্ষর *fname = ফাঁকা;
	nserror ret;

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, ENTRIES_FNAME);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	unlink(fname);

	মুক্ত(fname);
	ফেরত NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
স্থির nserror
read_entries(কাঠামো store_state *state)
{
	অক্ষর *fname = ফাঁকা;
	অক্ষর *url;
	nsurl *nsurl;
	nserror ret;
	size_t read_entries = 0;
	কাঠামো store_entry *ent;
	পূর্ণসংখ্যা fd;

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, ENTRIES_FNAME);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	যদি (state->entries == ফাঁকা) {
		মুক্ত(fname);
		ফেরত NSERROR_NOMEM;
	}

	fd = open(fname, O_RDWR);
	যদি (fd != -1) {
		uint32_t urllen;
		যখন (পঠ(fd, &urllen, আকার(urllen)) == আকার(urllen)) {
			url = calloc(1, urllen+1);
			যদি (url == ফাঁকা) {
				close(fd);
				মুক্ত(fname);
				ফেরত NSERROR_NOMEM;
			}
			যদি (পঠ(fd, url, urllen) != (ssize_t)urllen) {
				মুক্ত(url);
				close(fd);
				মুক্ত(fname);
				ফেরত NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			যদি (ret != NSERROR_OK) {
				মুক্ত(url);
				close(fd);
				মুক্ত(fname);
				ফেরত ret;
			}
			মুক্ত(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_ঢোকাও(state->entries, nsurl);
			যদি (ent == ফাঁকা) {
				nsurl_unref(nsurl);
				close(fd);
				মুক্ত(fname);
				ফেরত NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			যদি (পঠ(fd, ent, আকার(*ent)) != আকার(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				close(fd);
				মুক্ত(fname);
				ফেরত NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->যোগ_alloc += ent->elem[ENTRY_ELEM_তথ্য].আকার_2;
			state->যোগ_alloc += ent->elem[ENTRY_ELEM_META].আকার_2;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[ENTRY_ELEM_তথ্য].flags &= ~(ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP);
			ent->elem[ENTRY_ELEM_META].flags &= ~(ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP);

		}
		close(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	মুক্ত(fname);
	ফেরত NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
read_blocks(কাঠামো store_state *state)
{
	পূর্ণসংখ্যা bfidx; /* block file index */
	পূর্ণসংখ্যা elem_idx;
	পূর্ণসংখ্যা fd;
	ssize_t rd;
	অক্ষর *fname = ফাঁকা;
	nserror ret;

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, BLOCKS_FNAME);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = open(fname, O_RDWR);
	মুক্ত(fname);
	যদি (fd != -1) {
		/* initialise block file use array */
		জন্য (elem_idx = 0; elem_idx < ENTRY_ELEM_গুনতি; elem_idx++) {
			জন্য (bfidx = 0; bfidx < BLOCK_FILE_গুনতি; bfidx++) {
				rd = পঠ(fd,
					  &state->blocks[elem_idx][bfidx].use_map[0],
					  BLOCK_USE_MAP_আকার);
				যদি (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					যাও rd_err;
				}
			}
		}
	rd_err:
		close(fd);

	} অন্যথা {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->blocks[ENTRY_ELEM_তথ্য][0].use_map[0] = 1;
		state->blocks[ENTRY_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	জন্য (bfidx = 0; bfidx < BLOCK_FILE_গুনতি; bfidx++) {
		state->blocks[ENTRY_ELEM_তথ্য][bfidx].fd = -1;
		state->blocks[ENTRY_ELEM_META][bfidx].fd = -1;
	}

	ফেরত NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
write_cache_tag(কাঠামো store_state *state)
{
	FILE *fcachetag;
	nserror ret;
	অক্ষর *fname = ফাঁকা;

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, "CACHEDIR.TAG");
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	fcachetag = fopen(fname, "wb");

	মুক্ত(fname);

	যদি (fcachetag == ফাঁকা) {
		ফেরত NSERROR_NOT_FOUND;
	}

	fprintf(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	fclose(fcachetag);

	ফেরত NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
write_control(কাঠামো store_state *state)
{
	FILE *fcontrol;
	nserror ret;
	অক্ষর *fname = ফাঁকা;

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, "control");
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	যদি (ret != NSERROR_OK) {
		মুক্ত(fname);
		ফেরত ret;
	}

	fcontrol = fopen(fname, "wb");

	মুক্ত(fname);

	যদি (fcontrol == ফাঁকা) {
		ফেরত NSERROR_NOT_FOUND;
	}

	fprintf(fcontrol, "%u%c", CONTROL_VERSION, 0);

	fclose(fcontrol);

	ফেরত NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
read_control(কাঠামো store_state *state)
{
	nserror ret;
	FILE *fcontrol;
	unsigned পূর্ণসংখ্যা ctrlversion;
	অক্ষর *fname = ফাঁকা;

	ret = netsurf_mkpath(&fname, ফাঁকা, 2, state->path, "control");
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = fopen(fname, "rb");

	মুক্ত(fname);

	যদি (fcontrol == ফাঁকা) {
		/* unable to open control file */
		যদি (errno == ENOENT) {
			ফেরত NSERROR_NOT_FOUND;
		} অন্যথা {
			ফেরত NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	যদি (fscanf(fcontrol, "%u", &ctrlversion) != 1) {
		যাও control_error;
	}

	যদি (ctrlversion != CONTROL_VERSION) {
		যাও control_error;
	}

	যদি (fgetc(fcontrol) != 0) {
		যাও control_error;
	}

	fclose(fcontrol);

	ফেরত NSERROR_OK;

control_error: /* problem with the control file */

	fclose(fcontrol);

	ফেরত NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror
initialise(ধ্রুবক কাঠামো llcache_store_parameters *parameters)
{
	কাঠামো store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	যদি (storestate != ফাঁকা) {
		ফেরত NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	যদি (parameters->limit == 0) {
		ফেরত NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	যদি (parameters->path == ফাঁকা) {
		ফেরত NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = calloc(1, আকার(কাঠামো store_state));
	যদি (newstate == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	newstate->path = strdup(parameters->path);
	newstate->limit = parameters->limit;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	যদি (ret != NSERROR_OK) {
		যদি (ret == NSERROR_NOT_FOUND) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} অন্যথা {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->path);
			যদি (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->path);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} অন্যথা {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->path);
			}
		}
		ret = write_control(newstate);
		যদি (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_tag(newstate);
		}
	}
	যদি (ret != NSERROR_OK) {
		/* that went well obviously */
		মুক্ত(newstate->path);
		মুক্ত(newstate);
		ফেরত ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	যদি (ret != NSERROR_OK) {
		/* that went well obviously */
		মুক্ত(newstate->path);
		মুক্ত(newstate);
		ফেরত ret;
	}

	/* read blocks */
	ret = read_blocks(newstate);
	যদি (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		মুক্ত(newstate->path);
		মুক্ত(newstate);
		ফেরত ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->path,
	      newstate->limit,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->যোগ_alloc, newstate->limit);

	ফেরত NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
স্থির nserror
finalise(শূন্য)
{
	পূর্ণসংখ্যা bf; /* block file index */
	unsigned পূর্ণসংখ্যা op_গুনতি;

	যদি (storestate != ফাঁকা) {
		guit->misc->schedule(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_blocks(storestate);

		/* ensure all block files are closed */
		জন্য (bf = 0; bf < BLOCK_FILE_গুনতি; bf++) {
			যদি (storestate->blocks[ENTRY_ELEM_তথ্য][bf].fd != -1) {
				close(storestate->blocks[ENTRY_ELEM_তথ্য][bf].fd);
			}
			যদি (storestate->blocks[ENTRY_ELEM_META][bf].fd != -1) {
				close(storestate->blocks[ENTRY_ELEM_META][bf].fd);
			}
		}

		op_গুনতি = storestate->hit_গুনতি + storestate->miss_গুনতি;

		/* avoid division by zero */
		যদি (op_গুনতি > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_গুনতি,
			      storestate->hit_গুনতি,
			      storestate->miss_গুনতি,
			      0,
			      (storestate->hit_গুনতি * 100) / op_গুনতি,
			      (storestate->miss_গুনতি * 100) / op_গুনতি,
			      0);
		}

		hashmap_destroy(storestate->entries);
		মুক্ত(storestate->path);
		মুক্ত(storestate);
		storestate = ফাঁকা;
	}
	ফেরত NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
স্থির nserror store_write_block(কাঠামো store_state *state,
			 কাঠামো store_entry *bse,
			 পূর্ণসংখ্যা elem_idx)
{
	block_সূচক_t bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_গুনতি) &
		((1 << BLOCK_FILE_গুনতি) - 1); /* block file block resides in */
	block_সূচক_t bi = bse->elem[elem_idx].block & ((1U << BLOCK_ENTRY_গুনতি) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	যদি (state->blocks[elem_idx][bf].fd == -1) {
		state->blocks[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ENTRY_ELEM_গুনতি, O_CREAT | O_RDWR);
		যদি (state->blocks[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			ফেরত NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->blocks_opened = true;
	}

	offst = (unsigned পূর্ণসংখ্যা)bi << log2_block_আকার[elem_idx];

	wr = nsu_pwrite(state->blocks[elem_idx][bf].fd,
			bse->elem[elem_idx].তথ্য,
			bse->elem[elem_idx].আকার_2,
			offst);
	যদি (wr != (ssize_t)bse->elem[elem_idx].আকার_2) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].আকার_2,
		      bse->elem[elem_idx].তথ্য,
		      (size_t)offst,
		      bse->elem[elem_idx].block,
		      errno);
		ফেরত NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].তথ্য, (size_t)offst,
	      bse->elem[elem_idx].block);

	ফেরত NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
স্থির nserror store_write_file(কাঠামো store_state *state,
			 কাঠামো store_entry *bse,
			 পূর্ণসংখ্যা elem_idx)
{
	ssize_t wr;
	পূর্ণসংখ্যা fd;
	পূর্ণসংখ্যা err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	যদি (fd < 0) {
		perror("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		ফেরত NSERROR_SAVE_FAILED;
	}

	wr = লিখ(fd, bse->elem[elem_idx].তথ্য, bse->elem[elem_idx].আকার_2);
	err = errno; /* close can change errno */

	close(fd);
	যদি (wr != (ssize_t)bse->elem[elem_idx].আকার_2) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].আকার_2,
		      bse->elem[elem_idx].তথ্য,
		      err);

		/** @todo Delete the file? */
		ফেরত NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].তথ্য);

	ফেরত NSERROR_OK;
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
স্থির nserror
store(nsurl *url,
      গণনা backing_store_flags bsflags,
      uint8_t *তথ্য,
      ধ্রুবক size_t datalen)
{
	nserror ret;
	কাঠামো store_entry *bse;
	পূর্ণসংখ্যা elem_idx;

	/* check backing store is initialised */
	যদি (storestate == ফাঁকা) {
		ফেরত NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	যদি ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ENTRY_ELEM_META;
	} অন্যথা {
		elem_idx = ENTRY_ELEM_তথ্য;
	}

	/* set the store entry up */
	ret = set_store_entry(storestate, url, elem_idx, তথ্য, datalen, &bse);
	যদি (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		ফেরত ret;
	}

	যদি (bse->elem[elem_idx].block != 0) {
		/* small block storage */
		ret = store_write_block(storestate, bse, elem_idx);
	} অন্যথা {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	ফেরত ret;
}

/**
 * release any allocation for an entry
 */
স্থির nserror entry_release_alloc(কাঠামো store_entry_element *elem)
{
	যদি ((elem->flags & ENTRY_ELEM_FLAG_HEAP) != 0) {
		elem->ref--;
		যদি (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->তথ্য);
			মুক্ত(elem->তথ্য);
			elem->flags &= ~ENTRY_ELEM_FLAG_HEAP;
		}
	}
	ফেরত NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
স্থির nserror store_read_block(কাঠামো store_state *state,
			 কাঠামো store_entry *bse,
			 পূর্ণসংখ্যা elem_idx)
{
	block_সূচক_t bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_গুনতি) &
		((1 << BLOCK_FILE_গুনতি) - 1); /* block file block resides in */
	block_সূচক_t bi = bse->elem[elem_idx].block & ((1 << BLOCK_ENTRY_গুনতি) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	যদি (state->blocks[elem_idx][bf].fd == -1) {
		state->blocks[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ENTRY_ELEM_গুনতি, O_CREAT | O_RDWR);
		যদি (state->blocks[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			ফেরত NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->blocks_opened = true;
	}

	offst = (unsigned পূর্ণসংখ্যা)bi << log2_block_আকার[elem_idx];

	rd = nsu_pread(state->blocks[elem_idx][bf].fd,
		       bse->elem[elem_idx].তথ্য,
		       bse->elem[elem_idx].আকার_2,
		       offst);
	যদি (rd != (ssize_t)bse->elem[elem_idx].আকার_2) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].আকার_2,
		      bse->elem[elem_idx].তথ্য,
		      (size_t)offst,
		      bse->elem[elem_idx].block,
		      errno);
		ফেরত NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].তথ্য, (size_t)offst,
	      bse->elem[elem_idx].block);

	ফেরত NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
স্থির nserror store_read_file(কাঠামো store_state *state,
			 কাঠামো store_entry *bse,
			 পূর্ণসংখ্যা elem_idx)
{
	পূর্ণসংখ্যা fd;
	ssize_t rd; /* return from read */
	পূর্ণসংখ্যা ret = NSERROR_OK;
	size_t tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	যদি (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		/** @todo should this invalidate the entry? */
		ফেরত NSERROR_NOT_FOUND;
	}

	যখন (tot < bse->elem[elem_idx].আকার_2) {
		rd = পঠ(fd,
			  bse->elem[elem_idx].তথ্য + tot,
			  bse->elem[elem_idx].আকার_2 - tot);
		যদি (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      errno);
			ret = NSERROR_NOT_FOUND;
			বিরতি;
		}
		tot += rd;
	}

	close(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].তথ্য);

	ফেরত ret;
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
স্থির nserror
fetch(nsurl *url,
      গণনা backing_store_flags bsflags,
      uint8_t **তথ্য_out,
      size_t *datalen_out)
{
	nserror ret;
	কাঠামো store_entry *bse;
	কাঠামো store_entry_element *elem;
	পূর্ণসংখ্যা elem_idx;

	/* check backing store is initialised */
	যদি (storestate == ফাঁকা) {
		ফেরত NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_entry(storestate, url, &bse);
	যদি (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->miss_গুনতি++;
		ফেরত ret;
	}
	storestate->hit_গুনতি++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	যদি ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ENTRY_ELEM_META;
	} অন্যথা {
		elem_idx = ENTRY_ELEM_তথ্য;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	যদি ((elem->flags & ENTRY_ELEM_FLAG_HEAP) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->তথ্য, elem->ref);

	} অন্যথা {
		/* allocate from the heap */
		elem->তথ্য = malloc(elem->আকার_2);
		যদি (elem->তথ্য == ফাঁকা) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			ফেরত NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->তথ্য);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= ENTRY_ELEM_FLAG_HEAP;
		elem->ref = 1;

		/* fill the new block */
		যদি (elem->block != 0) {
			ret = store_read_block(storestate, bse, elem_idx);
		} অন্যথা {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	যদি (ret != NSERROR_OK) {
		entry_release_alloc(elem);
	} অন্যথা {
		/* update stats and setup return pointers */
		storestate->hit_আকার += elem->আকার_2;

		*তথ্য_out = elem->তথ্য;
		*datalen_out = elem->আকার_2;
	}

	ফেরত ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
স্থির nserror release(nsurl *url, গণনা backing_store_flags bsflags)
{
	nserror ret;
	কাঠামো store_entry *bse;
	কাঠামো store_entry_element *elem;

	/* check backing store is initialised */
	যদি (storestate == ফাঁকা) {
		ফেরত NSERROR_INIT_FAILED;
	}

	ret = get_store_entry(storestate, url, &bse);
	যদি (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		ফেরত ret;
	}

	/* the entry element */
	যদি ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[ENTRY_ELEM_META];
	} অন্যথা {
		elem = &bse->elem[ENTRY_ELEM_তথ্য];
	}

	ret = entry_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	যদি ((ret == NSERROR_OK) &&
	    ((bse->flags & ENTRY_FLAGS_INVALID) != 0)) {
		ret = invalidate_entry(storestate, bse);
	}

	ফেরত ret;
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
স্থির nserror
invalidate(nsurl *url)
{
	nserror ret;
	কাঠামো store_entry *bse;

	/* check backing store is initialised */
	যদি (storestate == ফাঁকা) {
		ফেরত NSERROR_INIT_FAILED;
	}

	ret = get_store_entry(storestate, url, &bse);
	যদি (ret != NSERROR_OK) {
		ফেরত ret;
	}

	ফেরত invalidate_entry(storestate, bse);
}


স্থির কাঠামো gui_llcache_table llcache_table = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

কাঠামো gui_llcache_table *filesystem_llcache_table = &llcache_table;
