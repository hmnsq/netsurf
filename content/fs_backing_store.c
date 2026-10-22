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

#समावेश <unistd.h>
#समावेश <string.h>
#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <fcntl.h>
#समावेश <errno.h>
#समावेश <time.h>
#समावेश <stdlib.h>
#समावेश <nsutils/unistd.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/filepath.h"
#समावेश "utils/file.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/hashmap.h"
#समावेश "desktop/gui_internal.h"
#समावेश "netsurf/misc.h"

#समावेश "content/backing_store.h"

/** Backing store file format version */
#घोषणा CONTROL_VERSION 202

/**
 * Number of milliseconds after a update before control data
 * maintenance is performed
 */
#घोषणा CONTROL_MAINT_TIME 10000

/** Filename of serialised entries */
#घोषणा ENTRIES_FNAME "entries"

/** Filename of block file index */
#घोषणा BLOCKS_FNAME "blocks"

/** log2 block data address length (64k) */
#घोषणा BLOCK_ADDR_LEN 16

/** log2 number of entries per block file(1024) */
#घोषणा BLOCK_ENTRY_गिनती 10

/** log2 number of data block files */
#घोषणा BLOCK_FILE_गिनती (BLOCK_ADDR_LEN - BLOCK_ENTRY_गिनती)

/** log2 size of data blocks (8k) */
#घोषणा BLOCK_तथ्य_आकार 13

/** log2 size of metadata blocks (8k) */
#घोषणा BLOCK_META_आकार 13

/** length in bytes of a block files use map */
#घोषणा BLOCK_USE_MAP_आकार (1 << (BLOCK_ENTRY_गिनती - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
प्रकार uint32_t entry_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
प्रकार uint16_t block_सूचकांक_t;


/**
 * Entry element index values.
 */
क्रमागत store_entry_elem_idx {
	ENTRY_ELEM_तथ्य = 0, /**< entry element is data */
	ENTRY_ELEM_META = 1,  /**< entry element is metadata */
	ENTRY_ELEM_गिनती = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
क्रमागत store_entry_elem_flags {
	/** store not managing any allocation on entry */
	ENTRY_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	ENTRY_ELEM_FLAG_HEAP = 0x1,
	/** entry data allocation is mmaped */
	ENTRY_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	ENTRY_ELEM_FLAG_SMALL = 0x4,
};


क्रमागत store_entry_flags {
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
काष्ठा store_entry_element {
	uint8_t* तथ्य; /**< data allocated */
	uint32_t आकार; /**< size of entry element on disc */
	block_सूचकांक_t block; /**< small object data block */
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
काष्ठा store_entry {
	nsurl *url; /**< The URL for this entry */
	int64_t last_used; /**< UNIX time the entry was last used */
	uint16_t use_गिनती; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	काष्ठा store_entry_element elem[ENTRY_ELEM_गिनती];
};

/**
 * Small block file.
 */
काष्ठा block_file {
	/** file descriptor of the block file */
	पूर्णांक fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[BLOCK_USE_MAP_आकार];
};

/**
 * log2 of block size.
 */
अटल स्थिर अचिन्हित पूर्णांक log2_block_आकार[ENTRY_ELEM_गिनती] = {
	BLOCK_तथ्य_आकार, /**< Data block size */
	BLOCK_META_आकार  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
काष्ठा store_state {
	/* store config */
	अक्षर *path; /**< The path to the backing store */
	माप_प्रकार limit; /**< The backing store upper bound target size */
	माप_प्रकार hysteresis; /**< The hysteresis around the target size */

	/**
	 * The cache object hash
	 */
	hashmap_t *entries;

	/** flag indicating if the entries have been made persistent
	 * since they were last changed.
	 */
	bool entries_dirty;

	/** small block indexes */
	काष्ठा block_file blocks[ENTRY_ELEM_गिनती][BLOCK_FILE_गिनती];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool blocks_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool blocks_opened;


	/* stats */
	uint64_t योग_alloc; /**< total size of all allocated storage. */

	माप_प्रकार hit_गिनती; /**< number of cache hits */
	uint64_t hit_आकार; /**< size of storage served */
	माप_प्रकार miss_गिनती; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
काष्ठा store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

अटल bool
entries_hashmap_कुंजी_eq(व्योम *key1, व्योम *key2)
{
	वापस nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

अटल व्योम *
entries_hashmap_मूल्य_alloc(व्योम *कुंजी)
{
	काष्ठा store_entry *ent = सुस्मृति(1, माप(काष्ठा store_entry));
	अगर (ent != शून्य) {
		ent->url = nsurl_ref(कुंजी);
	}
	वापस ent;
}

अटल व्योम
entries_hashmap_मूल्य_destroy(व्योम *मूल्य)
{
	काष्ठा store_entry *ent = मूल्य;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	मुक्त(ent);
}

अटल hashmap_parameters_t entries_hashmap_parameters = {
	.कुंजी_clone = (hashmap_कुंजी_clone_t)nsurl_ref,
	.कुंजी_destroy = (hashmap_कुंजी_destroy_t)nsurl_unref,
	.कुंजी_hash = (hashmap_कुंजी_hash_t)nsurl_hash,
	.कुंजी_eq = entries_hashmap_कुंजी_eq,
	.मूल्य_alloc = entries_hashmap_मूल्य_alloc,
	.मूल्य_destroy = entries_hashmap_मूल्य_destroy,
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
अटल अक्षर *
store_fname(काष्ठा store_state *state,
	    entry_ident_t ident,
	    पूर्णांक elem_idx)
{
	अक्षर *fname = शून्य;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	स्थिर uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	स्थिर अक्षर *base_dir_table[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	स्थिर uint8_t encoding_table[64][3] = {
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

	चयन (elem_idx) {
	हाल ENTRY_ELEM_तथ्य:
	हाल ENTRY_ELEM_META:
		netsurf_mkpath(&fname, शून्य, 8,
			       state->path, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		अवरोध;

	हाल (ENTRY_ELEM_गिनती + ENTRY_ELEM_META):
	हाल (ENTRY_ELEM_गिनती + ENTRY_ELEM_तथ्य):
		netsurf_mkpath(&fname, शून्य, 3,
			       state->path, b32u_d[0], b32u_d[1]);
		अवरोध;

	शेष:
		निश्चित("bad element index" == शून्य);
		अवरोध;
	}

	वापस fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
invalidate_element(काष्ठा store_state *state,
		   काष्ठा store_entry *bse,
		   पूर्णांक elem_idx)
{
	अगर (bse->elem[elem_idx].block != 0) {
		block_सूचकांक_t bf;
		block_सूचकांक_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_गिनती) &
			((1 << BLOCK_FILE_गिनती) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].block & ((1U << BLOCK_ENTRY_गिनती) -1);

		/* clear bit in use map */
		state->blocks[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} अन्यथा {
		अक्षर *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		अगर (fname == शून्य) {
			वापस NSERROR_NOMEM;
		}
		unlink(fname);
		मुक्त(fname);
	}

	state->योग_alloc -= bse->elem[elem_idx].आकार;

	वापस NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
invalidate_entry(काष्ठा store_state *state, काष्ठा store_entry *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= ENTRY_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	अगर (((bse->elem[ENTRY_ELEM_तथ्य].flags &
	     (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[ENTRY_ELEM_META].flags &
	      (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		वापस NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, ENTRY_ELEM_META);
	अगर (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, ENTRY_ELEM_तथ्य);
	अगर (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	वापस NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
अटल पूर्णांक compar(स्थिर व्योम *va, स्थिर व्योम *vb)
{
	स्थिर काष्ठा store_entry *a = *(स्थिर काष्ठा store_entry **)va;
	स्थिर काष्ठा store_entry *b = *(स्थिर काष्ठा store_entry **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	अगर ((a->elem[ENTRY_ELEM_तथ्य].flags == ENTRY_ELEM_FLAG_NONE) &&
	    (b->elem[ENTRY_ELEM_तथ्य].flags != ENTRY_ELEM_FLAG_NONE)) {
		वापस -1;
	} अन्यथा अगर ((a->elem[ENTRY_ELEM_तथ्य].flags != ENTRY_ELEM_FLAG_NONE) &&
		   (b->elem[ENTRY_ELEM_तथ्य].flags == ENTRY_ELEM_FLAG_NONE)) {
		वापस 1;
	}

	अगर ((a->elem[ENTRY_ELEM_META].flags == ENTRY_ELEM_FLAG_NONE) &&
	    (b->elem[ENTRY_ELEM_META].flags != ENTRY_ELEM_FLAG_NONE)) {
		वापस -1;
	} अन्यथा अगर ((a->elem[ENTRY_ELEM_META].flags != ENTRY_ELEM_FLAG_NONE) &&
		   (b->elem[ENTRY_ELEM_META].flags == ENTRY_ELEM_FLAG_NONE)) {
		वापस 1;
	}

	अगर (a->use_गिनती < b->use_गिनती) {
		वापस -1;
	} अन्यथा अगर (a->use_गिनती > b->use_गिनती) {
		वापस 1;
	}
	/* use count is the same - now consider last use time */

	अगर (a->last_used < b->last_used) {
		वापस -1;
	} अन्यथा अगर (a->last_used > b->last_used) {
		वापस 1;
	}

	/* they are the same */
	वापस 0;
}

प्रकार काष्ठा {
	काष्ठा store_entry **elist;
	माप_प्रकार ent_गिनती;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
अटल bool
entry_eviction_iterator_cb(व्योम *कुंजी, व्योम *मूल्य, व्योम *ctx)
{
	eviction_state_t *estate = ctx;
	काष्ठा store_entry *ent = मूल्य;
	estate->elist[estate->ent_गिनती++] = ent;
	वापस false;
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
अटल nserror store_evict(काष्ठा store_state *state)
{
	माप_प्रकार ent = 0;
	माप_प्रकार removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	माप_प्रकार old_गिनती;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	अगर (state->योग_alloc < state->limit) {
		/* cache within limits */
		वापस NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->योग_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_गिनती = hashmap_गिनती(state->entries);
	estate.ent_गिनती = 0;
	estate.elist = दो_स्मृति(माप(काष्ठा state_entry*) * old_गिनती);
	अगर (estate.elist == शून्य) {
		वापस NSERROR_NOMEM;
	}

	अगर (hashmap_iterate(state->entries, entry_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		मुक्त(estate.elist);
		वापस NSERROR_UNKNOWN;
	}

	अगर (old_गिनती != estate.ent_गिनती) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		मुक्त(estate.elist);
		वापस NSERROR_UNKNOWN;
	}

	क्विक(estate.elist, estate.ent_गिनती, माप(काष्ठा state_entry*), compar);

	/* evict entries in listed order */
	removed = 0;
	क्रम (ent = 0; ent < estate.ent_गिनती; ent++) {
		काष्ठा store_entry *bse = estate.elist[ent];

		removed += bse->elem[ENTRY_ELEM_तथ्य].आकार;
		removed += bse->elem[ENTRY_ELEM_META].आकार;

		ret = invalidate_entry(state, bse);
		अगर (ret != NSERROR_OK) {
			अवरोध;
		}

		अगर (removed > state->hysteresis) {
			अवरोध;
		}
	}

	मुक्त(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->योग_alloc, old_गिनती - ent);

	वापस ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
अटल nserror
write_entry(काष्ठा store_entry *ent, पूर्णांक fd)
{
	uint32_t len = म_माप(nsurl_access(ent->url));
	अगर (ग_लिखो(fd, &len, माप(len)) != माप(len))
		वापस NSERROR_SAVE_FAILED;
	अगर (ग_लिखो(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		वापस NSERROR_SAVE_FAILED;
	अगर (ग_लिखो(fd, ent, माप(*ent)) != माप(*ent))
		वापस NSERROR_SAVE_FAILED;

	वापस NSERROR_OK;
}

प्रकार काष्ठा {
	पूर्णांक fd;
	माप_प्रकार written;
} write_entry_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
अटल bool
write_entry_iterator(व्योम *कुंजी, व्योम *मूल्य, व्योम *ctx)
{
	/* We ignore the key */
	काष्ठा store_entry *ent = मूल्य;
	write_entry_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	वापस write_entry(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror write_entries(काष्ठा store_state *state)
{
	अक्षर *tname = शून्य; /* temporary file name for atomic replace */
	अक्षर *fname = शून्य; /* target filename */
	write_entry_iteration_state weistate;
	nserror ret;

	स_रखो(&weistate, 0, माप(weistate));

	अगर (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		वापस NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, शून्य, 2, state->path, "t"ENTRIES_FNAME);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	weistate.fd = खोलो(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	अगर (weistate.fd == -1) {
		मुक्त(tname);
		वापस NSERROR_SAVE_FAILED;
	}

	अगर (hashmap_iterate(state->entries, write_entry_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		बंद(weistate.fd);
		unlink(tname);
		मुक्त(tname);
		वापस NSERROR_SAVE_FAILED;
	}

	बंद(weistate.fd);

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, ENTRIES_FNAME);
	अगर (ret != NSERROR_OK) {
		unlink(tname);
		मुक्त(tname);
		वापस ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(व्योम)हटाओ(fname);
	अगर (नाम(tname, fname) != 0) {
		unlink(tname);
		मुक्त(tname);
		मुक्त(fname);
		वापस NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	वापस NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
अटल nserror write_blocks(काष्ठा store_state *state)
{
	पूर्णांक fd;
	अक्षर *tname = शून्य; /* temporary file name for atomic replace */
	अक्षर *fname = शून्य; /* target filename */
	माप_प्रकार blocks_आकार;
	माप_प्रकार written = 0;
	माप_प्रकार wr;
	nserror ret;
	पूर्णांक bfidx; /* block file index */
	पूर्णांक elem_idx;

	अगर (state->blocks_dirty == false) {
		/* blocks use maps have not been updated since last write */
		वापस NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, शून्य, 2, state->path, "t"BLOCKS_FNAME);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	fd = खोलो(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	अगर (fd == -1) {
		मुक्त(tname);
		वापस NSERROR_SAVE_FAILED;
	}

	blocks_आकार = (BLOCK_FILE_गिनती * ENTRY_ELEM_गिनती) * BLOCK_USE_MAP_आकार;

	क्रम (elem_idx = 0; elem_idx < ENTRY_ELEM_गिनती; elem_idx++) {
		क्रम (bfidx = 0; bfidx < BLOCK_FILE_गिनती; bfidx++) {
			wr = ग_लिखो(fd,
				   &state->blocks[elem_idx][bfidx].use_map[0],
				   BLOCK_USE_MAP_आकार);
			अगर (wr != BLOCK_USE_MAP_आकार) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				जाओ wr_err;
			}
			written += wr;
		}
	}
wr_err:
	बंद(fd);

	/* check all data was written */
	अगर (written != blocks_आकार) {
		unlink(tname);
		मुक्त(tname);
		वापस NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, BLOCKS_FNAME);
	अगर (ret != NSERROR_OK) {
		unlink(tname);
		मुक्त(tname);
		वापस ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(व्योम)हटाओ(fname);
	अगर (नाम(tname, fname) != 0) {
		unlink(tname);
		मुक्त(tname);
		मुक्त(fname);
		वापस NSERROR_SAVE_FAILED;
	}

	वापस NSERROR_OK;
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
अटल nserror set_block_extents(काष्ठा store_state *state)
{
	पूर्णांक bfidx; /* block file index */
	पूर्णांक elem_idx;
	पूर्णांक ftr;

	अगर (state->blocks_opened == false) {
		/* no blocks have been opened since last write */
		वापस NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	क्रम (elem_idx = 0; elem_idx < ENTRY_ELEM_गिनती; elem_idx++) {
		क्रम (bfidx = 0; bfidx < BLOCK_FILE_गिनती; bfidx++) {
			अगर (state->blocks[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->blocks[elem_idx][bfidx].fd, 1U << (log2_block_आकार[elem_idx] + BLOCK_ENTRY_गिनती));
				अगर (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      त्रुटि_सं);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->blocks_opened = false;

	वापस NSERROR_OK;
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
अटल व्योम control_maintenance(व्योम *s)
{
	काष्ठा store_state *state = s;

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
अटल nserror
get_store_entry(काष्ठा store_state *state, nsurl *url, काष्ठा store_entry **bse)
{
	काष्ठा store_entry *ent;

	ent = hashmap_lookup(state->entries, url);

	अगर (ent == शून्य) {
		वापस NSERROR_NOT_FOUND;
	}

	*bse = ent;

	ent->last_used = समय(शून्य);
	ent->use_गिनती++;

	state->entries_dirty = true;

	guit->misc->schedule(CONTROL_MAINT_TIME, control_maintenance, state);

	वापस NSERROR_OK;
}

/**
 * Find next available small block.
 */
अटल block_सूचकांक_t alloc_block(काष्ठा store_state *state, पूर्णांक elem_idx)
{
	पूर्णांक bf;
	पूर्णांक idx;
	पूर्णांक bit;
	uint8_t *map;

	क्रम (bf = 0; bf < BLOCK_FILE_गिनती; bf++) {
		map = &state->blocks[elem_idx][bf].use_map[0];

		क्रम (idx = 0; idx < BLOCK_USE_MAP_आकार; idx++) {
			अगर (*(map + idx) != 0xff) {
				/* located an unused block */
				क्रम (bit = 0; bit < 8;bit++) {
					अगर (((*(map + idx)) & (1U << bit)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << bit;
						state->blocks_dirty = true;
						वापस (((bf * BLOCK_USE_MAP_आकार) + idx) * 8) + bit;
					}
				}
			}
		}
	}

	वापस 0;
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
अटल nserror
set_store_entry(काष्ठा store_state *state,
		nsurl *url,
		पूर्णांक elem_idx,
		uint8_t *तथ्य,
		स्थिर माप_प्रकार datalen,
		काष्ठा store_entry **bse)
{
	काष्ठा store_entry *se;
	nserror ret;
	काष्ठा store_entry_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	se = hashmap_lookup(state->entries, url);
	अगर (se == शून्य) {
		se = hashmap_जोड़ो(state->entries, url);
	}
	अगर (se == शून्य) {
		वापस NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	अगर ((elem->flags & (ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		वापस NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_गिनती = 1;
	se->last_used = समय(शून्य);

	/* store the data in the element */
	elem->flags |= ENTRY_ELEM_FLAG_HEAP;
	elem->तथ्य = तथ्य;
	elem->ref = 1;

	/* account for size of entry element */
	state->योग_alloc -= elem->आकार;
	elem->आकार = datalen;
	state->योग_alloc += elem->आकार;

	/* if the element will fit in a small block attempt to allocate one */
	अगर (elem->आकार <= (1U << log2_block_आकार[elem_idx])) {
		elem->block = alloc_block(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->schedule(CONTROL_MAINT_TIME, control_maintenance, state);

	*bse = se;

	वापस NSERROR_OK;
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
अटल पूर्णांक
store_open(काष्ठा store_state *state,
	   entry_ident_t ident,
	   पूर्णांक elem_idx,
	   पूर्णांक openflags)
{
	अक्षर *fname;
	nserror ret;
	पूर्णांक fd;

	fname = store_fname(state, ident, elem_idx);
	अगर (fname == शून्य) {
		NSLOG(netsurf, ERROR, "filename error");
		वापस -1;
	}

	/* ensure all path elements to file exist if creating file */
	अगर (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		अगर (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			मुक्त(fname);
			वापस -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = खोलो(fname, openflags, S_IRUSR | S_IWUSR);

	मुक्त(fname);

	वापस fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
unlink_entries(काष्ठा store_state *state)
{
	अक्षर *fname = शून्य;
	nserror ret;

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, ENTRIES_FNAME);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	unlink(fname);

	मुक्त(fname);
	वापस NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
अटल nserror
read_entries(काष्ठा store_state *state)
{
	अक्षर *fname = शून्य;
	अक्षर *url;
	nsurl *nsurl;
	nserror ret;
	माप_प्रकार read_entries = 0;
	काष्ठा store_entry *ent;
	पूर्णांक fd;

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, ENTRIES_FNAME);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	अगर (state->entries == शून्य) {
		मुक्त(fname);
		वापस NSERROR_NOMEM;
	}

	fd = खोलो(fname, O_RDWR);
	अगर (fd != -1) {
		uint32_t urllen;
		जबतक (पढ़ो(fd, &urllen, माप(urllen)) == माप(urllen)) {
			url = सुस्मृति(1, urllen+1);
			अगर (url == शून्य) {
				बंद(fd);
				मुक्त(fname);
				वापस NSERROR_NOMEM;
			}
			अगर (पढ़ो(fd, url, urllen) != (ssize_t)urllen) {
				मुक्त(url);
				बंद(fd);
				मुक्त(fname);
				वापस NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			अगर (ret != NSERROR_OK) {
				मुक्त(url);
				बंद(fd);
				मुक्त(fname);
				वापस ret;
			}
			मुक्त(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_जोड़ो(state->entries, nsurl);
			अगर (ent == शून्य) {
				nsurl_unref(nsurl);
				बंद(fd);
				मुक्त(fname);
				वापस NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			अगर (पढ़ो(fd, ent, माप(*ent)) != माप(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				बंद(fd);
				मुक्त(fname);
				वापस NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->योग_alloc += ent->elem[ENTRY_ELEM_तथ्य].आकार;
			state->योग_alloc += ent->elem[ENTRY_ELEM_META].आकार;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[ENTRY_ELEM_तथ्य].flags &= ~(ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP);
			ent->elem[ENTRY_ELEM_META].flags &= ~(ENTRY_ELEM_FLAG_HEAP | ENTRY_ELEM_FLAG_MMAP);

		}
		बंद(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	मुक्त(fname);
	वापस NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
read_blocks(काष्ठा store_state *state)
{
	पूर्णांक bfidx; /* block file index */
	पूर्णांक elem_idx;
	पूर्णांक fd;
	ssize_t rd;
	अक्षर *fname = शून्य;
	nserror ret;

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, BLOCKS_FNAME);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = खोलो(fname, O_RDWR);
	मुक्त(fname);
	अगर (fd != -1) {
		/* initialise block file use array */
		क्रम (elem_idx = 0; elem_idx < ENTRY_ELEM_गिनती; elem_idx++) {
			क्रम (bfidx = 0; bfidx < BLOCK_FILE_गिनती; bfidx++) {
				rd = पढ़ो(fd,
					  &state->blocks[elem_idx][bfidx].use_map[0],
					  BLOCK_USE_MAP_आकार);
				अगर (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					जाओ rd_err;
				}
			}
		}
	rd_err:
		बंद(fd);

	} अन्यथा {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->blocks[ENTRY_ELEM_तथ्य][0].use_map[0] = 1;
		state->blocks[ENTRY_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	क्रम (bfidx = 0; bfidx < BLOCK_FILE_गिनती; bfidx++) {
		state->blocks[ENTRY_ELEM_तथ्य][bfidx].fd = -1;
		state->blocks[ENTRY_ELEM_META][bfidx].fd = -1;
	}

	वापस NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
write_cache_tag(काष्ठा store_state *state)
{
	खाता *fcachetag;
	nserror ret;
	अक्षर *fname = शून्य;

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, "CACHEDIR.TAG");
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	fcachetag = ख_खोलो(fname, "wb");

	मुक्त(fname);

	अगर (fcachetag == शून्य) {
		वापस NSERROR_NOT_FOUND;
	}

	ख_लिखो(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	ख_बंद(fcachetag);

	वापस NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
write_control(काष्ठा store_state *state)
{
	खाता *fcontrol;
	nserror ret;
	अक्षर *fname = शून्य;

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, "control");
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	अगर (ret != NSERROR_OK) {
		मुक्त(fname);
		वापस ret;
	}

	fcontrol = ख_खोलो(fname, "wb");

	मुक्त(fname);

	अगर (fcontrol == शून्य) {
		वापस NSERROR_NOT_FOUND;
	}

	ख_लिखो(fcontrol, "%u%c", CONTROL_VERSION, 0);

	ख_बंद(fcontrol);

	वापस NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
read_control(काष्ठा store_state *state)
{
	nserror ret;
	खाता *fcontrol;
	अचिन्हित पूर्णांक ctrlversion;
	अक्षर *fname = शून्य;

	ret = netsurf_mkpath(&fname, शून्य, 2, state->path, "control");
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = ख_खोलो(fname, "rb");

	मुक्त(fname);

	अगर (fcontrol == शून्य) {
		/* unable to open control file */
		अगर (त्रुटि_सं == ENOENT) {
			वापस NSERROR_NOT_FOUND;
		} अन्यथा {
			वापस NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	अगर (ख_पूछो(fcontrol, "%u", &ctrlversion) != 1) {
		जाओ control_error;
	}

	अगर (ctrlversion != CONTROL_VERSION) {
		जाओ control_error;
	}

	अगर (ख_अक्षर_लो(fcontrol) != 0) {
		जाओ control_error;
	}

	ख_बंद(fcontrol);

	वापस NSERROR_OK;

control_error: /* problem with the control file */

	ख_बंद(fcontrol);

	वापस NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror
initialise(स्थिर काष्ठा llcache_store_parameters *parameters)
{
	काष्ठा store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	अगर (storestate != शून्य) {
		वापस NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	अगर (parameters->limit == 0) {
		वापस NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	अगर (parameters->path == शून्य) {
		वापस NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = सुस्मृति(1, माप(काष्ठा store_state));
	अगर (newstate == शून्य) {
		वापस NSERROR_NOMEM;
	}

	newstate->path = strdup(parameters->path);
	newstate->limit = parameters->limit;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	अगर (ret != NSERROR_OK) {
		अगर (ret == NSERROR_NOT_FOUND) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} अन्यथा {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->path);
			अगर (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->path);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} अन्यथा {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->path);
			}
		}
		ret = write_control(newstate);
		अगर (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_tag(newstate);
		}
	}
	अगर (ret != NSERROR_OK) {
		/* that went well obviously */
		मुक्त(newstate->path);
		मुक्त(newstate);
		वापस ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	अगर (ret != NSERROR_OK) {
		/* that went well obviously */
		मुक्त(newstate->path);
		मुक्त(newstate);
		वापस ret;
	}

	/* read blocks */
	ret = read_blocks(newstate);
	अगर (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		मुक्त(newstate->path);
		मुक्त(newstate);
		वापस ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->path,
	      newstate->limit,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->योग_alloc, newstate->limit);

	वापस NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
अटल nserror
finalise(व्योम)
{
	पूर्णांक bf; /* block file index */
	अचिन्हित पूर्णांक op_गिनती;

	अगर (storestate != शून्य) {
		guit->misc->schedule(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_blocks(storestate);

		/* ensure all block files are closed */
		क्रम (bf = 0; bf < BLOCK_FILE_गिनती; bf++) {
			अगर (storestate->blocks[ENTRY_ELEM_तथ्य][bf].fd != -1) {
				बंद(storestate->blocks[ENTRY_ELEM_तथ्य][bf].fd);
			}
			अगर (storestate->blocks[ENTRY_ELEM_META][bf].fd != -1) {
				बंद(storestate->blocks[ENTRY_ELEM_META][bf].fd);
			}
		}

		op_गिनती = storestate->hit_गिनती + storestate->miss_गिनती;

		/* avoid division by zero */
		अगर (op_गिनती > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_गिनती,
			      storestate->hit_गिनती,
			      storestate->miss_गिनती,
			      0,
			      (storestate->hit_गिनती * 100) / op_गिनती,
			      (storestate->miss_गिनती * 100) / op_गिनती,
			      0);
		}

		hashmap_destroy(storestate->entries);
		मुक्त(storestate->path);
		मुक्त(storestate);
		storestate = शून्य;
	}
	वापस NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
अटल nserror store_write_block(काष्ठा store_state *state,
			 काष्ठा store_entry *bse,
			 पूर्णांक elem_idx)
{
	block_सूचकांक_t bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_गिनती) &
		((1 << BLOCK_FILE_गिनती) - 1); /* block file block resides in */
	block_सूचकांक_t bi = bse->elem[elem_idx].block & ((1U << BLOCK_ENTRY_गिनती) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	अगर (state->blocks[elem_idx][bf].fd == -1) {
		state->blocks[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ENTRY_ELEM_गिनती, O_CREAT | O_RDWR);
		अगर (state->blocks[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", त्रुटि_सं);
			वापस NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->blocks_opened = true;
	}

	offst = (अचिन्हित पूर्णांक)bi << log2_block_आकार[elem_idx];

	wr = nsu_pwrite(state->blocks[elem_idx][bf].fd,
			bse->elem[elem_idx].तथ्य,
			bse->elem[elem_idx].आकार,
			offst);
	अगर (wr != (ssize_t)bse->elem[elem_idx].आकार) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].आकार,
		      bse->elem[elem_idx].तथ्य,
		      (माप_प्रकार)offst,
		      bse->elem[elem_idx].block,
		      त्रुटि_सं);
		वापस NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].तथ्य, (माप_प्रकार)offst,
	      bse->elem[elem_idx].block);

	वापस NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
अटल nserror store_write_file(काष्ठा store_state *state,
			 काष्ठा store_entry *bse,
			 पूर्णांक elem_idx)
{
	ssize_t wr;
	पूर्णांक fd;
	पूर्णांक err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	अगर (fd < 0) {
		लिखो_त्रुटि("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, त्रुटि_सं);
		वापस NSERROR_SAVE_FAILED;
	}

	wr = ग_लिखो(fd, bse->elem[elem_idx].तथ्य, bse->elem[elem_idx].आकार);
	err = त्रुटि_सं; /* close can change errno */

	बंद(fd);
	अगर (wr != (ssize_t)bse->elem[elem_idx].आकार) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].आकार,
		      bse->elem[elem_idx].तथ्य,
		      err);

		/** @todo Delete the file? */
		वापस NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].तथ्य);

	वापस NSERROR_OK;
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
अटल nserror
store(nsurl *url,
      क्रमागत backing_store_flags bsflags,
      uint8_t *तथ्य,
      स्थिर माप_प्रकार datalen)
{
	nserror ret;
	काष्ठा store_entry *bse;
	पूर्णांक elem_idx;

	/* check backing store is initialised */
	अगर (storestate == शून्य) {
		वापस NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	अगर ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ENTRY_ELEM_META;
	} अन्यथा {
		elem_idx = ENTRY_ELEM_तथ्य;
	}

	/* set the store entry up */
	ret = set_store_entry(storestate, url, elem_idx, तथ्य, datalen, &bse);
	अगर (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		वापस ret;
	}

	अगर (bse->elem[elem_idx].block != 0) {
		/* small block storage */
		ret = store_write_block(storestate, bse, elem_idx);
	} अन्यथा {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	वापस ret;
}

/**
 * release any allocation for an entry
 */
अटल nserror entry_release_alloc(काष्ठा store_entry_element *elem)
{
	अगर ((elem->flags & ENTRY_ELEM_FLAG_HEAP) != 0) {
		elem->ref--;
		अगर (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->तथ्य);
			मुक्त(elem->तथ्य);
			elem->flags &= ~ENTRY_ELEM_FLAG_HEAP;
		}
	}
	वापस NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
अटल nserror store_read_block(काष्ठा store_state *state,
			 काष्ठा store_entry *bse,
			 पूर्णांक elem_idx)
{
	block_सूचकांक_t bf = (bse->elem[elem_idx].block >> BLOCK_ENTRY_गिनती) &
		((1 << BLOCK_FILE_गिनती) - 1); /* block file block resides in */
	block_सूचकांक_t bi = bse->elem[elem_idx].block & ((1 << BLOCK_ENTRY_गिनती) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	अगर (state->blocks[elem_idx][bf].fd == -1) {
		state->blocks[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + ENTRY_ELEM_गिनती, O_CREAT | O_RDWR);
		अगर (state->blocks[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", त्रुटि_सं);
			वापस NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->blocks_opened = true;
	}

	offst = (अचिन्हित पूर्णांक)bi << log2_block_आकार[elem_idx];

	rd = nsu_pread(state->blocks[elem_idx][bf].fd,
		       bse->elem[elem_idx].तथ्य,
		       bse->elem[elem_idx].आकार,
		       offst);
	अगर (rd != (ssize_t)bse->elem[elem_idx].आकार) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].आकार,
		      bse->elem[elem_idx].तथ्य,
		      (माप_प्रकार)offst,
		      bse->elem[elem_idx].block,
		      त्रुटि_सं);
		वापस NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].तथ्य, (माप_प्रकार)offst,
	      bse->elem[elem_idx].block);

	वापस NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
अटल nserror store_read_file(काष्ठा store_state *state,
			 काष्ठा store_entry *bse,
			 पूर्णांक elem_idx)
{
	पूर्णांक fd;
	ssize_t rd; /* return from read */
	पूर्णांक ret = NSERROR_OK;
	माप_प्रकार tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	अगर (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, त्रुटि_सं);
		/** @todo should this invalidate the entry? */
		वापस NSERROR_NOT_FOUND;
	}

	जबतक (tot < bse->elem[elem_idx].आकार) {
		rd = पढ़ो(fd,
			  bse->elem[elem_idx].तथ्य + tot,
			  bse->elem[elem_idx].आकार - tot);
		अगर (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      त्रुटि_सं);
			ret = NSERROR_NOT_FOUND;
			अवरोध;
		}
		tot += rd;
	}

	बंद(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].तथ्य);

	वापस ret;
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
अटल nserror
fetch(nsurl *url,
      क्रमागत backing_store_flags bsflags,
      uint8_t **तथ्य_out,
      माप_प्रकार *datalen_out)
{
	nserror ret;
	काष्ठा store_entry *bse;
	काष्ठा store_entry_element *elem;
	पूर्णांक elem_idx;

	/* check backing store is initialised */
	अगर (storestate == शून्य) {
		वापस NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_entry(storestate, url, &bse);
	अगर (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->miss_गिनती++;
		वापस ret;
	}
	storestate->hit_गिनती++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	अगर ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = ENTRY_ELEM_META;
	} अन्यथा {
		elem_idx = ENTRY_ELEM_तथ्य;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	अगर ((elem->flags & ENTRY_ELEM_FLAG_HEAP) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->तथ्य, elem->ref);

	} अन्यथा {
		/* allocate from the heap */
		elem->तथ्य = दो_स्मृति(elem->आकार);
		अगर (elem->तथ्य == शून्य) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			वापस NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->तथ्य);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= ENTRY_ELEM_FLAG_HEAP;
		elem->ref = 1;

		/* fill the new block */
		अगर (elem->block != 0) {
			ret = store_read_block(storestate, bse, elem_idx);
		} अन्यथा {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	अगर (ret != NSERROR_OK) {
		entry_release_alloc(elem);
	} अन्यथा {
		/* update stats and setup return pointers */
		storestate->hit_आकार += elem->आकार;

		*तथ्य_out = elem->तथ्य;
		*datalen_out = elem->आकार;
	}

	वापस ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
अटल nserror release(nsurl *url, क्रमागत backing_store_flags bsflags)
{
	nserror ret;
	काष्ठा store_entry *bse;
	काष्ठा store_entry_element *elem;

	/* check backing store is initialised */
	अगर (storestate == शून्य) {
		वापस NSERROR_INIT_FAILED;
	}

	ret = get_store_entry(storestate, url, &bse);
	अगर (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		वापस ret;
	}

	/* the entry element */
	अगर ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[ENTRY_ELEM_META];
	} अन्यथा {
		elem = &bse->elem[ENTRY_ELEM_तथ्य];
	}

	ret = entry_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	अगर ((ret == NSERROR_OK) &&
	    ((bse->flags & ENTRY_FLAGS_INVALID) != 0)) {
		ret = invalidate_entry(storestate, bse);
	}

	वापस ret;
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
अटल nserror
invalidate(nsurl *url)
{
	nserror ret;
	काष्ठा store_entry *bse;

	/* check backing store is initialised */
	अगर (storestate == शून्य) {
		वापस NSERROR_INIT_FAILED;
	}

	ret = get_store_entry(storestate, url, &bse);
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	वापस invalidate_entry(storestate, bse);
}


अटल काष्ठा gui_llcache_table llcache_table = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

काष्ठा gui_llcache_table *filesystem_llcache_table = &llcache_table;
