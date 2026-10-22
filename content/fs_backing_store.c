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

#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <nsutils/unistd.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/filepath.h"
#અન્તર્ભાવ "utils/file.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/hashmap.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "netsurf/misc.h"

#અન્તર્ભાવ "content/backing_store.h"

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
#define BLOCK_ENTRY_ગણતરી 10

/** log2 number of data block files */
#define BLOCK_FILE_ગણતરી (BLOCK_ADDR_LEN - BLOCK_ENTRY_ગણતરી)

/** log2 size of data blocks (8k) */
#define BLOCK_DATA_SIZE 13

/** log2 size of metadata blocks (8k) */
#define BLOCK_META_SIZE 13

/** length in bytes of a block files use map */
#define BLOCK_USE_MAP_SIZE (1 << (BLOCK_ENTRY_ગણતરી - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
પ્રકારવ્યાખ્યા uint32_t entry_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
પ્રકારવ્યાખ્યા uint16_t block_index_t;


/**
 * Entry element index values.
 */
ગણના store_entry_elem_idx {
	ENTRY_ELEM_DATA = 0, /**< entry element is data */
	ENTRY_ELEM_META = 1,  /**< entry element is metadata */
	ENTRY_ELEM_ગણતરી = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
ગણના store_entry_elem_flags {
	/** store not managing any allocation on entry */
	ENTRY_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	ENTRY_ELEM_FLAG_HEAP = 0x1,
	/** entry data allocation is mmaped */
	ENTRY_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	ENTRY_ELEM_FLAG_SMALL = 0x4,
};


ગણના store_entry_flags {
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
સંરચના store_entry_element {
	uint8_t* data; /**< data allocated */
	uint32_t size; /**< size of entry element on disc */
	block_index_t block; /**< small object data block */
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
સંરચના store_entry {
	nsurl *url; /**< The URL for this entry */
	int64_t last_used; /**< UNIX time the entry was last used */
	uint16_t use_ગણતરી; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	સંરચના store_entry_element elem[ENTRY_ELEM_ગણતરી];
};

/**
 * Small block file.
 */
સંરચના block_file {
	/** file descriptor of the block file */
	પૂર્ણાંક fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[BLOCK_USE_MAP_SIZE];
};

/**
 * log2 of block size.
 */
સ્થિર અચલ unsigned પૂર્ણાંક log2_block_size[ENTRY_ELEM_ગણતરી] = {
	BLOCK_DATA_SIZE, /**< Data block size */
	BLOCK_META_SIZE  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
સંરચના store_state {
	/* store config */
	અક્ષર *path; /**< The path to the backing store */
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
	સંરચના block_file blocks[ENTRY_ELEM_ગણતરી][BLOCK_FILE_ગણતરી];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool blocks_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool blocks_opened;


	/* stats */
	uint64_t total_alloc; /**< total size of all allocated storage. */

	size_t hit_ગણતરી; /**< number of cache hits */
	uint64_t hit_size; /**< size of storage served */
	size_t miss_ગણતરી; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
સંરચના store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

સ્થિર bool
entries_hashmap_ચાવી_eq(શૂન્ય *key1, શૂન્ય *key2)
{
	પાછા nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

સ્થિર શૂન્ય *
entries_hashmap_કિંમત_alloc(શૂન્ય *ચાવી)
{
	સંરચના store_entry *ent = calloc(1, કદ(સંરચના store_entry));
	જો (ent != ખાલી) {
		ent->url = nsurl_ref(ચાવી);
	}
	પાછા ent;
}

સ્થિર શૂન્ય
entries_hashmap_કિંમત_destroy(શૂન્ય *કિંમત)
{
	સંરચના store_entry *ent = કિંમત;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	મુક્ત(ent);
}

સ્થિર hashmap_parameters_t entries_hashmap_parameters = {
	.ચાવી_clone = (hashmap_ચાવી_clone_t)nsurl_ref,
	.ચાવી_destroy = (hashmap_ચાવી_destroy_t)nsurl_unref,
	.ચાવી_hash = (hashmap_ચાવી_hash_t)nsurl_hash,
	.ચાવી_eq = entries_hashmap_ચાવી_eq,
	.કિંમત_alloc = entries_hashmap_કિંમત_alloc,
	.કિંમત_destroy = entries_hashmap_કિંમત_destroy,
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
સ્થિર અક્ષર *
store_fname(સંરચના store_state *state,
	    entry_ident_t ident,
	    પૂર્ણાંક elem_idx)
{
	અક્ષર *fname = ખાલી;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	અચલ uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	અચલ અક્ષર *base_dir_table[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	અચલ uint8_t encoding_table[64][3] = {
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

	પસંદગી (elem_idx) {
	સ્થિતિ ENTRY_ELEM_DATA:
	સ્થિતિ ENTRY_ELEM_META:
		netsurf_mkpath(&fname, ખાલી, 8,
			       state->path, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		તોડો;

	સ્થિતિ (ENTRY_ELEM_ગણતરી + ENTRY_ELEM_META):
	સ્થિતિ (ENTRY_ELEM_ગણતરી + ENTRY_ELEM_DATA):
		netsurf_mkpath(&fname, ખાલી, 3,
			       state->path, b32u_d[0], b32u_d[1]);
		તોડો;

	મૂળભૂત:
		પ્રતિજ્ઞા("bad element index" == ખાલી);
		તોડો;
	}

	પાછા fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
invalidate_element(સંરચના store_state *state,
		   સંરચના store_entry *bse,
		   પૂર્ણાંક elem_idx)
{
	જો (bse->elem[elem_idx].block != 0) {
		block_index_t bf;
		block_index_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_ગણતરી) &
			((1 << BLOCK_FILE_ગણતરી) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].block & ((1U << BLOCK_ENTRY_ગણતરી) -1);

		/* clear bit in use map */
		state->blocks[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} નહીંતર {
		અક્ષર *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		જો (fname == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
		unlink(fname);
		મુક્ત(fname);
	}

	state->total_alloc -= bse->elem[elem_idx].size;

	પાછા NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
invalidate_entry(સંરચના store_state *state, સંરચના store_entry *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= ENTRY_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	જો (((bse->elem[ENTRY_ELEM_DATA].flags &
	     (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[ENTRY_ELEM_META].flags &
	      (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		પાછા NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, ENTRY_ELEM_META);
	જો (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, ENTRY_ELEM_DATA);
	જો (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	પાછા NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
સ્થિર પૂર્ણાંક compar(અચલ શૂન્ય *va, અચલ શૂન્ય *vb)
{
	અચલ સંરચના store_entry *a = *(અચલ સંરચના store_entry **)va;
	અચલ સંરચના store_entry *b = *(અચલ સંરચના store_entry **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	જો ((a->elem[ENTRY_ELEM_DATA].flags == ENTRY_ELEM_FLAG_NONE) &&
	    (b->elem[ENTRY_ELEM_DATA].flags != ENTRY_ELEM_FLAG_NONE)) {
		પાછા -1;
	} નહીંતર જો ((a->elem[ENTRY_ELEM_DATA].flags != ENTRY_ELEM_FLAG_NONE) &&
		   (b->elem[ENTRY_ELEM_DATA].flags == ENTRY_ELEM_FLAG_NONE)) {
		પાછા 1;
	}

	જો ((a->elem[ENTRY_ELEM_META].flags == ENTRY_ELEM_FLAG_NONE) &&
	    (b->elem[ENTRY_ELEM_META].flags != ENTRY_ELEM_FLAG_NONE)) {
		પાછા -1;
	} નહીંતર જો ((a->elem[ENTRY_ELEM_META].flags != ENTRY_ELEM_FLAG_NONE) &&
		   (b->elem[ENTRY_ELEM_META].flags == ENTRY_ELEM_FLAG_NONE)) {
		પાછા 1;
	}

	જો (a->use_ગણતરી < b->use_ગણતરી) {
		પાછા -1;
	} નહીંતર જો (a->use_ગણતરી > b->use_ગણતરી) {
		પાછા 1;
	}
	/* use count is the same - now consider last use time */

	જો (a->last_used < b->last_used) {
		પાછા -1;
	} નહીંતર જો (a->last_used > b->last_used) {
		પાછા 1;
	}

	/* they are the same */
	પાછા 0;
}

પ્રકારવ્યાખ્યા સંરચના {
	સંરચના store_entry **elist;
	size_t ent_ગણતરી;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
સ્થિર bool
entry_eviction_iterator_cb(શૂન્ય *ચાવી, શૂન્ય *કિંમત, શૂન્ય *ctx)
{
	eviction_state_t *estate = ctx;
	સંરચના store_entry *ent = કિંમત;
	estate->elist[estate->ent_ગણતરી++] = ent;
	પાછા false;
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
સ્થિર nserror store_evict(સંરચના store_state *state)
{
	size_t ent = 0;
	size_t removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	size_t old_ગણતરી;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	જો (state->total_alloc < state->limit) {
		/* cache within limits */
		પાછા NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->total_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_ગણતરી = hashmap_ગણતરી(state->entries);
	estate.ent_ગણતરી = 0;
	estate.elist = malloc(કદ(સંરચના state_entry*) * old_ગણતરી);
	જો (estate.elist == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	જો (hashmap_iterate(state->entries, entry_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		મુક્ત(estate.elist);
		પાછા NSERROR_UNKNOWN;
	}

	જો (old_ગણતરી != estate.ent_ગણતરી) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		મુક્ત(estate.elist);
		પાછા NSERROR_UNKNOWN;
	}

	qsort(estate.elist, estate.ent_ગણતરી, કદ(સંરચના state_entry*), compar);

	/* evict entries in listed order */
	removed = 0;
	માટે (ent = 0; ent < estate.ent_ગણતરી; ent++) {
		સંરચના store_entry *bse = estate.elist[ent];

		removed += bse->elem[ENTRY_ELEM_DATA].size;
		removed += bse->elem[ENTRY_ELEM_META].size;

		ret = invalidate_entry(state, bse);
		જો (ret != NSERROR_OK) {
			તોડો;
		}

		જો (removed > state->hysteresis) {
			તોડો;
		}
	}

	મુક્ત(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->total_alloc, old_ગણતરી - ent);

	પાછા ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
સ્થિર nserror
write_entry(સંરચના store_entry *ent, પૂર્ણાંક fd)
{
	uint32_t len = strlen(nsurl_access(ent->url));
	જો (લિખ(fd, &len, કદ(len)) != કદ(len))
		પાછા NSERROR_SAVE_FAILED;
	જો (લિખ(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		પાછા NSERROR_SAVE_FAILED;
	જો (લિખ(fd, ent, કદ(*ent)) != કદ(*ent))
		પાછા NSERROR_SAVE_FAILED;

	પાછા NSERROR_OK;
}

પ્રકારવ્યાખ્યા સંરચના {
	પૂર્ણાંક fd;
	size_t written;
} write_entry_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
સ્થિર bool
write_entry_iterator(શૂન્ય *ચાવી, શૂન્ય *કિંમત, શૂન્ય *ctx)
{
	/* We ignore the key */
	સંરચના store_entry *ent = કિંમત;
	write_entry_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	પાછા write_entry(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror write_entries(સંરચના store_state *state)
{
	અક્ષર *tname = ખાલી; /* temporary file name for atomic replace */
	અક્ષર *fname = ખાલી; /* target filename */
	write_entry_iteration_state weistate;
	nserror ret;

	memset(&weistate, 0, કદ(weistate));

	જો (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		પાછા NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ખાલી, 2, state->path, "t"ENTRIES_FNAME);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	weistate.fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	જો (weistate.fd == -1) {
		મુક્ત(tname);
		પાછા NSERROR_SAVE_FAILED;
	}

	જો (hashmap_iterate(state->entries, write_entry_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		close(weistate.fd);
		unlink(tname);
		મુક્ત(tname);
		પાછા NSERROR_SAVE_FAILED;
	}

	close(weistate.fd);

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, ENTRIES_FNAME);
	જો (ret != NSERROR_OK) {
		unlink(tname);
		મુક્ત(tname);
		પાછા ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(શૂન્ય)remove(fname);
	જો (rename(tname, fname) != 0) {
		unlink(tname);
		મુક્ત(tname);
		મુક્ત(fname);
		પાછા NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	પાછા NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror write_blocks(સંરચના store_state *state)
{
	પૂર્ણાંક fd;
	અક્ષર *tname = ખાલી; /* temporary file name for atomic replace */
	અક્ષર *fname = ખાલી; /* target filename */
	size_t blocks_size;
	size_t written = 0;
	size_t wr;
	nserror ret;
	પૂર્ણાંક bfidx; /* block file index */
	પૂર્ણાંક elem_idx;

	જો (state->blocks_dirty == false) {
		/* blocks use maps have not been updated since last write */
		પાછા NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ખાલી, 2, state->path, "t"BLOCKS_FNAME);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	જો (fd == -1) {
		મુક્ત(tname);
		પાછા NSERROR_SAVE_FAILED;
	}

	blocks_size = (BLOCK_FILE_ગણતરી * ENTRY_ELEM_ગણતરી) * BLOCK_USE_MAP_SIZE;

	માટે (elem_idx = 0; elem_idx < ENTRY_ELEM_ગણતરી; elem_idx++) {
		માટે (bfidx = 0; bfidx < BLOCK_FILE_ગણતરી; bfidx++) {
			wr = લિખ(fd,
				   &state->blocks[elem_idx][bfidx].use_map[0],
				   BLOCK_USE_MAP_SIZE);
			જો (wr != BLOCK_USE_MAP_SIZE) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				જાઓ wr_err;
			}
			written += wr;
		}
	}
wr_err:
	close(fd);

	/* check all data was written */
	જો (written != blocks_size) {
		unlink(tname);
		મુક્ત(tname);
		પાછા NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, BLOCKS_FNAME);
	જો (ret != NSERROR_OK) {
		unlink(tname);
		મુક્ત(tname);
		પાછા ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(શૂન્ય)remove(fname);
	જો (rename(tname, fname) != 0) {
		unlink(tname);
		મુક્ત(tname);
		મુક્ત(fname);
		પાછા NSERROR_SAVE_FAILED;
	}

	પાછા NSERROR_OK;
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
સ્થિર nserror set_block_extents(સંરચના store_state *state)
{
	પૂર્ણાંક bfidx; /* block file index */
	પૂર્ણાંક elem_idx;
	પૂર્ણાંક ftr;

	જો (state->blocks_opened == false) {
		/* no blocks have been opened since last write */
		પાછા NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	માટે (elem_idx = 0; elem_idx < ENTRY_ELEM_ગણતરી; elem_idx++) {
		માટે (bfidx = 0; bfidx < BLOCK_FILE_ગણતરી; bfidx++) {
			જો (state->blocks[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->blocks[elem_idx][bfidx].fd, 1U << (log2_block_size[elem_idx] + BLOCK_ENTRY_ગણતરી));
				જો (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      errno);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->blocks_opened = false;

	પાછા NSERROR_OK;
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
સ્થિર શૂન્ય control_maintenance(શૂન્ય *s)
{
	સંરચના store_state *state = s;

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
સ્થિર nserror
get_store_entry(સંરચના store_state *state, nsurl *url, સંરચના store_entry **bse)
{
	સંરચના store_entry *ent;

	ent = hashmap_lookup(state->entries, url);

	જો (ent == ખાલી) {
		પાછા NSERROR_NOT_FOUND;
	}

	*bse = ent;

	ent->last_used = time(ખાલી);
	ent->use_ગણતરી++;

	state->entries_dirty = true;

	guit->misc->schedule(CONTROL_MAINT_TIME, control_maintenance, state);

	પાછા NSERROR_OK;
}

/**
 * Find next available small block.
 */
સ્થિર block_index_t alloc_block(સંરચના store_state *state, પૂર્ણાંક elem_idx)
{
	પૂર્ણાંક bf;
	પૂર્ણાંક idx;
	પૂર્ણાંક bit;
	uint8_t *map;

	માટે (bf = 0; bf < BLOCK_FILE_ગણતરી; bf++) {
		map = &state->blocks[elem_idx][bf].use_map[0];

		માટે (idx = 0; idx < BLOCK_USE_MAP_SIZE; idx++) {
			જો (*(map + idx) != 0xff) {
				/* located an unused block */
				માટે (bit = 0; bit < 8;bit++) {
					જો (((*(map + idx)) & (1U << bit)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << bit;
						state->blocks_dirty = true;
						પાછા (((bf * BLOCK_USE_MAP_SIZE) + idx) * 8) + bit;
					}
				}
			}
		}
	}

	પાછા 0;
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
સ્થિર nserror
set_store_entry(સંરચના store_state *state,
		nsurl *url,
		પૂર્ણાંક elem_idx,
		uint8_t *data,
		અચલ size_t datalen,
		સંરચના store_entry **bse)
{
	સંરચના store_entry *se;
	nserror ret;
	સંરચના store_entry_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	se = hashmap_lookup(state->entries, url);
	જો (se == ખાલી) {
		se = hashmap_ઉમેરો(state->entries, url);
	}
	જો (se == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	જો ((elem->flags & (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		પાછા NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_ગણતરી = 1;
	se->last_used = time(ખાલી);

	/* store the data in the element */
	elem->flags |= ENTRY_ELEM_FLAG_HEAP;
	elem->data = data;
	elem->ref = 1;

	/* account for size of entry element */
	state->total_alloc -= elem->size;
	elem->size = datalen;
	state->total_alloc += elem->size;

	/* if the element will fit in a small block attempt to allocate one */
	જો (elem->size <= (1U << log2_block_size[elem_idx])) {
		elem->block = alloc_block(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->schedule(CONTROL_MAINT_TIME, control_maintenance, state);

	*bse = se;

	પાછા NSERROR_OK;
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
સ્થિર પૂર્ણાંક
store_open(સંરચના store_state *state,
	   entry_ident_t ident,
	   પૂર્ણાંક elem_idx,
	   પૂર્ણાંક openflags)
{
	અક્ષર *fname;
	nserror ret;
	પૂર્ણાંક fd;

	fname = store_fname(state, ident, elem_idx);
	જો (fname == ખાલી) {
		NSLOG(netsurf, ERROR, "filename error");
		પાછા -1;
	}

	/* ensure all path elements to file exist if creating file */
	જો (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		જો (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			મુક્ત(fname);
			પાછા -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = open(fname, openflags, S_IRUSR | S_IWUSR);

	મુક્ત(fname);

	પાછા fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
unlink_entries(સંરચના store_state *state)
{
	અક્ષર *fname = ખાલી;
	nserror ret;

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, ENTRIES_FNAME);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	unlink(fname);

	મુક્ત(fname);
	પાછા NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
સ્થિર nserror
read_entries(સંરચના store_state *state)
{
	અક્ષર *fname = ખાલી;
	અક્ષર *url;
	nsurl *nsurl;
	nserror ret;
	size_t read_entries = 0;
	સંરચના store_entry *ent;
	પૂર્ણાંક fd;

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, ENTRIES_FNAME);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	જો (state->entries == ખાલી) {
		મુક્ત(fname);
		પાછા NSERROR_NOMEM;
	}

	fd = open(fname, O_RDWR);
	જો (fd != -1) {
		uint32_t urllen;
		જ્યારે (પઠ(fd, &urllen, કદ(urllen)) == કદ(urllen)) {
			url = calloc(1, urllen+1);
			જો (url == ખાલી) {
				close(fd);
				મુક્ત(fname);
				પાછા NSERROR_NOMEM;
			}
			જો (પઠ(fd, url, urllen) != (ssize_t)urllen) {
				મુક્ત(url);
				close(fd);
				મુક્ત(fname);
				પાછા NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			જો (ret != NSERROR_OK) {
				મુક્ત(url);
				close(fd);
				મુક્ત(fname);
				પાછા ret;
			}
			મુક્ત(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_ઉમેરો(state->entries, nsurl);
			જો (ent == ખાલી) {
				nsurl_unref(nsurl);
				close(fd);
				મુક્ત(fname);
				પાછા NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			જો (પઠ(fd, ent, કદ(*ent)) != કદ(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				close(fd);
				મુક્ત(fname);
				પાછા NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->total_alloc += ent->elem[ENTRY_ELEM_DATA].size;
			state->total_alloc += ent->elem[ENTRY_ELEM_META].size;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[ENTRY_ELEM_DATA].flags &= ~(ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP);
			ent->elem[ENTRY_ELEM_META].flags &= ~(ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP);

		}
		close(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	મુક્ત(fname);
	પાછા NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
read_blocks(સંરચના store_state *state)
{
	પૂર્ણાંક bfidx; /* block file index */
	પૂર્ણાંક elem_idx;
	પૂર્ણાંક fd;
	ssize_t rd;
	અક્ષર *fname = ખાલી;
	nserror ret;

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, BLOCKS_FNAME);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = open(fname, O_RDWR);
	મુક્ત(fname);
	જો (fd != -1) {
		/* initialise block file use array */
		માટે (elem_idx = 0; elem_idx < ENTRY_ELEM_ગણતરી; elem_idx++) {
			માટે (bfidx = 0; bfidx < BLOCK_FILE_ગણતરી; bfidx++) {
				rd = પઠ(fd,
					  &state->blocks[elem_idx][bfidx].use_map[0],
					  BLOCK_USE_MAP_SIZE);
				જો (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					જાઓ rd_err;
				}
			}
		}
	rd_err:
		close(fd);

	} નહીંતર {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->blocks[ENTRY_ELEM_DATA][0].use_map[0] = 1;
		state->blocks[ENTRY_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	માટે (bfidx = 0; bfidx < BLOCK_FILE_ગણતરી; bfidx++) {
		state->blocks[ENTRY_ELEM_DATA][bfidx].fd = -1;
		state->blocks[ENTRY_ELEM_META][bfidx].fd = -1;
	}

	પાછા NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
write_cache_tag(સંરચના store_state *state)
{
	FILE *fcachetag;
	nserror ret;
	અક્ષર *fname = ખાલી;

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, "CACHEDIR.TAG");
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	fcachetag = fopen(fname, "wb");

	મુક્ત(fname);

	જો (fcachetag == ખાલી) {
		પાછા NSERROR_NOT_FOUND;
	}

	fprintf(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	fclose(fcachetag);

	પાછા NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
write_control(સંરચના store_state *state)
{
	FILE *fcontrol;
	nserror ret;
	અક્ષર *fname = ખાલી;

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, "control");
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	જો (ret != NSERROR_OK) {
		મુક્ત(fname);
		પાછા ret;
	}

	fcontrol = fopen(fname, "wb");

	મુક્ત(fname);

	જો (fcontrol == ખાલી) {
		પાછા NSERROR_NOT_FOUND;
	}

	fprintf(fcontrol, "%u%c", CONTROL_VERSION, 0);

	fclose(fcontrol);

	પાછા NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
read_control(સંરચના store_state *state)
{
	nserror ret;
	FILE *fcontrol;
	unsigned પૂર્ણાંક ctrlversion;
	અક્ષર *fname = ખાલી;

	ret = netsurf_mkpath(&fname, ખાલી, 2, state->path, "control");
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = fopen(fname, "rb");

	મુક્ત(fname);

	જો (fcontrol == ખાલી) {
		/* unable to open control file */
		જો (errno == ENOENT) {
			પાછા NSERROR_NOT_FOUND;
		} નહીંતર {
			પાછા NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	જો (fscanf(fcontrol, "%u", &ctrlversion) != 1) {
		જાઓ control_error;
	}

	જો (ctrlversion != CONTROL_VERSION) {
		જાઓ control_error;
	}

	જો (fgetc(fcontrol) != 0) {
		જાઓ control_error;
	}

	fclose(fcontrol);

	પાછા NSERROR_OK;

control_error: /* problem with the control file */

	fclose(fcontrol);

	પાછા NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror
initialise(અચલ સંરચના llcache_store_parameters *parameters)
{
	સંરચના store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	જો (storestate != ખાલી) {
		પાછા NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	જો (parameters->limit == 0) {
		પાછા NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	જો (parameters->path == ખાલી) {
		પાછા NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = calloc(1, કદ(સંરચના store_state));
	જો (newstate == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	newstate->path = strdup(parameters->path);
	newstate->limit = parameters->limit;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	જો (ret != NSERROR_OK) {
		જો (ret == NSERROR_NOT_FOUND) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} નહીંતર {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->path);
			જો (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->path);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} નહીંતર {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->path);
			}
		}
		ret = write_control(newstate);
		જો (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_tag(newstate);
		}
	}
	જો (ret != NSERROR_OK) {
		/* that went well obviously */
		મુક્ત(newstate->path);
		મુક્ત(newstate);
		પાછા ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	જો (ret != NSERROR_OK) {
		/* that went well obviously */
		મુક્ત(newstate->path);
		મુક્ત(newstate);
		પાછા ret;
	}

	/* read blocks */
	ret = read_blocks(newstate);
	જો (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		મુક્ત(newstate->path);
		મુક્ત(newstate);
		પાછા ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->path,
	      newstate->limit,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->total_alloc, newstate->limit);

	પાછા NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
સ્થિર nserror
finalise(શૂન્ય)
{
	પૂર્ણાંક bf; /* block file index */
	unsigned પૂર્ણાંક op_ગણતરી;

	જો (storestate != ખાલી) {
		guit->misc->schedule(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_blocks(storestate);

		/* ensure all block files are closed */
		માટે (bf = 0; bf < BLOCK_FILE_ગણતરી; bf++) {
			જો (storestate->blocks[ENTRY_ELEM_DATA][bf].fd != -1) {
				close(storestate->blocks[ENTRY_ELEM_DATA][bf].fd);
			}
			જો (storestate->blocks[ENTRY_ELEM_META][bf].fd != -1) {
				close(storestate->blocks[ENTRY_ELEM_META][bf].fd);
			}
		}

		op_ગણતરી = storestate->hit_ગણતરી + storestate->miss_ગણતરી;

		/* avoid division by zero */
		જો (op_ગણતરી > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_ગણતરી,
			      storestate->hit_ગણતરી,
			      storestate->miss_ગણતરી,
			      0,
			      (storestate->hit_ગણતરી * 100) / op_ગણતરી,
			      (storestate->miss_ગણતરી * 100) / op_ગણતરી,
			      0);
		}

		hashmap_destroy(storestate->entries);
		મુક્ત(storestate->path);
		મુક્ત(storestate);
		storestate = ખાલી;
	}
	પાછા NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
સ્થિર nserror store_write_block(સંરચના store_state *state,
			 સંરચના store_entry *bse,
			 પૂર્ણાંક elem_idx)
{
	block_index_t bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_ગણતરી) &
		((1 << BLOCK_FILE_ગણતરી) - 1); /* block file block resides in */
	block_index_t bi = bse->elem[elem_idx].block & ((1U << BLOCK_ENTRY_ગણતરી) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	જો (state->blocks[elem_idx][bf].fd == -1) {
		state->blocks[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ENTRY_ELEM_ગણતરી, O_CREAT | O_RDWR);
		જો (state->blocks[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			પાછા NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->blocks_opened = true;
	}

	offst = (unsigned પૂર્ણાંક)bi << log2_block_size[elem_idx];

	wr = nsu_pwrite(state->blocks[elem_idx][bf].fd,
			bse->elem[elem_idx].data,
			bse->elem[elem_idx].size,
			offst);
	જો (wr != (ssize_t)bse->elem[elem_idx].size) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].size,
		      bse->elem[elem_idx].data,
		      (size_t)offst,
		      bse->elem[elem_idx].block,
		      errno);
		પાછા NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].data, (size_t)offst,
	      bse->elem[elem_idx].block);

	પાછા NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
સ્થિર nserror store_write_file(સંરચના store_state *state,
			 સંરચના store_entry *bse,
			 પૂર્ણાંક elem_idx)
{
	ssize_t wr;
	પૂર્ણાંક fd;
	પૂર્ણાંક err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	જો (fd < 0) {
		perror("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		પાછા NSERROR_SAVE_FAILED;
	}

	wr = લિખ(fd, bse->elem[elem_idx].data, bse->elem[elem_idx].size);
	err = errno; /* close can change errno */

	close(fd);
	જો (wr != (ssize_t)bse->elem[elem_idx].size) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].size,
		      bse->elem[elem_idx].data,
		      err);

		/** @todo Delete the file? */
		પાછા NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].data);

	પાછા NSERROR_OK;
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
સ્થિર nserror
store(nsurl *url,
      ગણના backing_store_flags bsflags,
      uint8_t *data,
      અચલ size_t datalen)
{
	nserror ret;
	સંરચના store_entry *bse;
	પૂર્ણાંક elem_idx;

	/* check backing store is initialised */
	જો (storestate == ખાલી) {
		પાછા NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	જો ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ENTRY_ELEM_META;
	} નહીંતર {
		elem_idx = ENTRY_ELEM_DATA;
	}

	/* set the store entry up */
	ret = set_store_entry(storestate, url, elem_idx, data, datalen, &bse);
	જો (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		પાછા ret;
	}

	જો (bse->elem[elem_idx].block != 0) {
		/* small block storage */
		ret = store_write_block(storestate, bse, elem_idx);
	} નહીંતર {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	પાછા ret;
}

/**
 * release any allocation for an entry
 */
સ્થિર nserror entry_release_alloc(સંરચના store_entry_element *elem)
{
	જો ((elem->flags & ENTRY_ELEM_FLAG_HEAP) != 0) {
		elem->ref--;
		જો (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->data);
			મુક્ત(elem->data);
			elem->flags &= ~ENTRY_ELEM_FLAG_HEAP;
		}
	}
	પાછા NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
સ્થિર nserror store_read_block(સંરચના store_state *state,
			 સંરચના store_entry *bse,
			 પૂર્ણાંક elem_idx)
{
	block_index_t bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_ગણતરી) &
		((1 << BLOCK_FILE_ગણતરી) - 1); /* block file block resides in */
	block_index_t bi = bse->elem[elem_idx].block & ((1 << BLOCK_ENTRY_ગણતરી) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	જો (state->blocks[elem_idx][bf].fd == -1) {
		state->blocks[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ENTRY_ELEM_ગણતરી, O_CREAT | O_RDWR);
		જો (state->blocks[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			પાછા NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->blocks_opened = true;
	}

	offst = (unsigned પૂર્ણાંક)bi << log2_block_size[elem_idx];

	rd = nsu_pread(state->blocks[elem_idx][bf].fd,
		       bse->elem[elem_idx].data,
		       bse->elem[elem_idx].size,
		       offst);
	જો (rd != (ssize_t)bse->elem[elem_idx].size) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].size,
		      bse->elem[elem_idx].data,
		      (size_t)offst,
		      bse->elem[elem_idx].block,
		      errno);
		પાછા NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].data, (size_t)offst,
	      bse->elem[elem_idx].block);

	પાછા NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
સ્થિર nserror store_read_file(સંરચના store_state *state,
			 સંરચના store_entry *bse,
			 પૂર્ણાંક elem_idx)
{
	પૂર્ણાંક fd;
	ssize_t rd; /* return from read */
	પૂર્ણાંક ret = NSERROR_OK;
	size_t tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	જો (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		/** @todo should this invalidate the entry? */
		પાછા NSERROR_NOT_FOUND;
	}

	જ્યારે (tot < bse->elem[elem_idx].size) {
		rd = પઠ(fd,
			  bse->elem[elem_idx].data + tot,
			  bse->elem[elem_idx].size - tot);
		જો (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      errno);
			ret = NSERROR_NOT_FOUND;
			તોડો;
		}
		tot += rd;
	}

	close(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].data);

	પાછા ret;
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
સ્થિર nserror
fetch(nsurl *url,
      ગણના backing_store_flags bsflags,
      uint8_t **data_out,
      size_t *datalen_out)
{
	nserror ret;
	સંરચના store_entry *bse;
	સંરચના store_entry_element *elem;
	પૂર્ણાંક elem_idx;

	/* check backing store is initialised */
	જો (storestate == ખાલી) {
		પાછા NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_entry(storestate, url, &bse);
	જો (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->miss_ગણતરી++;
		પાછા ret;
	}
	storestate->hit_ગણતરી++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	જો ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ENTRY_ELEM_META;
	} નહીંતર {
		elem_idx = ENTRY_ELEM_DATA;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	જો ((elem->flags & ENTRY_ELEM_FLAG_HEAP) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->data, elem->ref);

	} નહીંતર {
		/* allocate from the heap */
		elem->data = malloc(elem->size);
		જો (elem->data == ખાલી) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			પાછા NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->data);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= ENTRY_ELEM_FLAG_HEAP;
		elem->ref = 1;

		/* fill the new block */
		જો (elem->block != 0) {
			ret = store_read_block(storestate, bse, elem_idx);
		} નહીંતર {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	જો (ret != NSERROR_OK) {
		entry_release_alloc(elem);
	} નહીંતર {
		/* update stats and setup return pointers */
		storestate->hit_size += elem->size;

		*data_out = elem->data;
		*datalen_out = elem->size;
	}

	પાછા ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
સ્થિર nserror release(nsurl *url, ગણના backing_store_flags bsflags)
{
	nserror ret;
	સંરચના store_entry *bse;
	સંરચના store_entry_element *elem;

	/* check backing store is initialised */
	જો (storestate == ખાલી) {
		પાછા NSERROR_INIT_FAILED;
	}

	ret = get_store_entry(storestate, url, &bse);
	જો (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		પાછા ret;
	}

	/* the entry element */
	જો ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[ENTRY_ELEM_META];
	} નહીંતર {
		elem = &bse->elem[ENTRY_ELEM_DATA];
	}

	ret = entry_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	જો ((ret == NSERROR_OK) &&
	    ((bse->flags & ENTRY_FLAGS_INVALID) != 0)) {
		ret = invalidate_entry(storestate, bse);
	}

	પાછા ret;
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
સ્થિર nserror
invalidate(nsurl *url)
{
	nserror ret;
	સંરચના store_entry *bse;

	/* check backing store is initialised */
	જો (storestate == ખાલી) {
		પાછા NSERROR_INIT_FAILED;
	}

	ret = get_store_entry(storestate, url, &bse);
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	પાછા invalidate_entry(storestate, bse);
}


સ્થિર સંરચના gui_llcache_table llcache_table = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

સંરચના gui_llcache_table *filesystem_llcache_table = &llcache_table;
