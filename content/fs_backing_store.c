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

#ضمن <unistd.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <fcntl.h>
#ضمن <errno.h>
#ضمن <time.h>
#ضمن <stdlib.h>
#ضمن <nsutils/unistd.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/filepath.h"
#ضمن "utils/file.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/hashmap.h"
#ضمن "desktop/gui_internal.h"
#ضمن "netsurf/misc.h"

#ضمن "content/backing_store.h"

/** Backing store file format version */
#تعریف_کرو CONTROL_نسخہ 202

/**
 * Number of milliseconds after a update before control data
 * maintenance is performed
 */
#تعریف_کرو CONTROL_MAINT_ویلا 10000

/** Filename of serialised entries */
#تعریف_کرو ENTRIES_FNAME "entries"

/** Filename of block file index */
#تعریف_کرو بلاکاں_FNAME "blocks"

/** log2 block data address length (64k) */
#تعریف_کرو بلاک_ADDR_LEN 16

/** log2 number of entries per block file(1024) */
#تعریف_کرو بلاک_اندراج_گنتی 10

/** log2 number of data block files */
#تعریف_کرو بلاک_FILE_گنتی (بلاک_ADDR_LEN - بلاک_اندراج_گنتی)

/** log2 size of data blocks (8k) */
#تعریف_کرو بلاک_انکڑے_ناپ 13

/** log2 size of metadata blocks (8k) */
#تعریف_کرو بلاک_META_ناپ 13

/** length in bytes of a block files use map */
#تعریف_کرو بلاک_USE_MAP_ناپ (1 << (بلاک_اندراج_گنتی - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
تعريفنوع uint32_t اندراج_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
تعريفنوع uint16_t بلاک_اشاریہ_t;


/**
 * Entry element index values.
 */
تعداد store_اندراج_elem_idx {
	اندراج_ELEM_انکڑے = 0, /**< entry element is data */
	اندراج_ELEM_META = 1,  /**< entry element is metadata */
	اندراج_ELEM_گنتی = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
تعداد store_اندراج_elem_flags {
	/** store not managing any allocation on entry */
	اندراج_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	اندراج_ELEM_FLAG_ڈھیر = 0x1,
	/** entry data allocation is mmaped */
	اندراج_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	اندراج_ELEM_FLAG_SMALL = 0x4,
};


تعداد store_اندراج_flags {
	/** entry is normal */
	اندراج_FLAGS_NONE = 0,
	/** entry has been invalidated but something still holding a reference */
	اندراج_FLAGS_INVALID = 1,
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
بنية store_اندراج_element {
	uint8_t* انکڑے; /**< data allocated */
	uint32_t ناپ; /**< size of entry element on disc */
	بلاک_اشاریہ_t بلاک; /**< small object data block */
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
بنية store_اندراج {
	nsurl *url; /**< The URL for this entry */
	int64_t آخری_ورتیا; /**< UNIX time the entry was last used */
	uint16_t use_گنتی; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	بنية store_اندراج_element elem[اندراج_ELEM_گنتی];
};

/**
 * Small block file.
 */
بنية بلاک_file {
	/** file descriptor of the block file */
	صحيح fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[بلاک_USE_MAP_ناپ];
};

/**
 * log2 of block size.
 */
ساكن ثابت بنانشانی صحيح log2_بلاک_ناپ[اندراج_ELEM_گنتی] = {
	بلاک_انکڑے_ناپ, /**< Data block size */
	بلاک_META_ناپ  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
بنية store_state {
	/* store config */
	حرف *رستہ; /**< The path to the backing store */
	size_t حد_بندی; /**< The backing store upper bound target size */
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
	بنية بلاک_file بلاکاں[اندراج_ELEM_گنتی][بلاک_FILE_گنتی];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool بلاکاں_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool بلاکاں_opened;


	/* stats */
	uint64_t کل_alloc; /**< total size of all allocated storage. */

	size_t کامیابی_گنتی; /**< number of cache hits */
	uint64_t کامیابی_ناپ; /**< size of storage served */
	size_t کھنج_گنتی; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
بنية store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

ساكن bool
entries_hashmap_کلید_eq(فراغ *key1, فراغ *key2)
{
	أرجع nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

ساكن فراغ *
entries_hashmap_قدر_alloc(فراغ *کلید)
{
	بنية store_اندراج *ent = صفر_حافظہ_دیو(1, حجم(بنية store_اندراج));
	إذا (ent != عدم) {
		ent->url = nsurl_ref(کلید);
	}
	أرجع ent;
}

ساكن فراغ
entries_hashmap_قدر_destroy(فراغ *قدر)
{
	بنية store_اندراج *ent = قدر;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	حرر(ent);
}

ساكن hashmap_parameters_t entries_hashmap_parameters = {
	.کلید_clone = (hashmap_کلید_clone_t)nsurl_ref,
	.کلید_destroy = (hashmap_کلید_destroy_t)nsurl_unref,
	.کلید_hash = (hashmap_کلید_hash_t)nsurl_hash,
	.کلید_eq = entries_hashmap_کلید_eq,
	.قدر_alloc = entries_hashmap_قدر_alloc,
	.قدر_destroy = entries_hashmap_قدر_destroy,
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
ساكن حرف *
store_fname(بنية store_state *state,
	    اندراج_ident_t ident,
	    صحيح elem_idx)
{
	حرف *fname = عدم;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	ثابت uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	ثابت حرف *ادھار_dir_جدول[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	ثابت uint8_t encoding_جدول[64][3] = {
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
	b32u_i[0] = encoding_جدول[(ident      ) & 0x1f][0];
	b32u_i[1] = encoding_جدول[(ident >>  5) & 0x1f][0];
	b32u_i[2] = encoding_جدول[(ident >> 10) & 0x1f][0];
	b32u_i[3] = encoding_جدول[(ident >> 15) & 0x1f][0];
	b32u_i[4] = encoding_جدول[(ident >> 20) & 0x1f][0];
	b32u_i[5] = encoding_جدول[(ident >> 25) & 0x1f][0];
	b32u_i[6] = encoding_جدول[(ident >> 30) & 0x1f][0];
	b32u_i[7] = 0; /* null terminate ident string */

	/* base32 encode directory separators */
	b32u_d[0] = (uint8_t*)ادھار_dir_جدول[elem_idx];
	b32u_d[1] = &encoding_جدول[(ident      ) & 0x3f][0];
	b32u_d[2] = &encoding_جدول[(ident >>  6) & 0x3f][0];
	b32u_d[3] = &encoding_جدول[(ident >> 12) & 0x3f][0];
	b32u_d[4] = &encoding_جدول[(ident >> 18) & 0x3f][0];
	b32u_d[5] = &encoding_جدول[(ident >> 24) & 0x3f][0];

	بدل (elem_idx) {
	حالة اندراج_ELEM_انکڑے:
	حالة اندراج_ELEM_META:
		netsurf_mkpath(&fname, عدم, 8,
			       state->رستہ, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		اكسر;

	حالة (اندراج_ELEM_گنتی + اندراج_ELEM_META):
	حالة (اندراج_ELEM_گنتی + اندراج_ELEM_انکڑے):
		netsurf_mkpath(&fname, عدم, 3,
			       state->رستہ, b32u_d[0], b32u_d[1]);
		اكسر;

	افتراضي:
		أكد("bad element index" == عدم);
		اكسر;
	}

	أرجع fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
invalidate_element(بنية store_state *state,
		   بنية store_اندراج *bse,
		   صحيح elem_idx)
{
	إذا (bse->elem[elem_idx].بلاک != 0) {
		بلاک_اشاریہ_t bf;
		بلاک_اشاریہ_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].بلاک >> بلاک_اندراج_گنتی) &
			((1 << بلاک_FILE_گنتی) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].بلاک & ((1U << بلاک_اندراج_گنتی) -1);

		/* clear bit in use map */
		state->بلاکاں[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} وإلا {
		حرف *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		إذا (fname == عدم) {
			أرجع NSERROR_NOMEM;
		}
		unlink(fname);
		حرر(fname);
	}

	state->کل_alloc -= bse->elem[elem_idx].ناپ;

	أرجع NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
invalidate_اندراج(بنية store_state *state, بنية store_اندراج *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= اندراج_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	إذا (((bse->elem[اندراج_ELEM_انکڑے].flags &
	     (اندراج_ELEM_FLAG_ڈھیر | اندراج_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[اندراج_ELEM_META].flags &
	      (اندراج_ELEM_FLAG_ڈھیر | اندراج_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		أرجع NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, اندراج_ELEM_META);
	إذا (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, اندراج_ELEM_انکڑے);
	إذا (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	أرجع NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
ساكن صحيح compar(ثابت فراغ *va, ثابت فراغ *vb)
{
	ثابت بنية store_اندراج *a = *(ثابت بنية store_اندراج **)va;
	ثابت بنية store_اندراج *b = *(ثابت بنية store_اندراج **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	إذا ((a->elem[اندراج_ELEM_انکڑے].flags == اندراج_ELEM_FLAG_NONE) &&
	    (b->elem[اندراج_ELEM_انکڑے].flags != اندراج_ELEM_FLAG_NONE)) {
		أرجع -1;
	} وإلا إذا ((a->elem[اندراج_ELEM_انکڑے].flags != اندراج_ELEM_FLAG_NONE) &&
		   (b->elem[اندراج_ELEM_انکڑے].flags == اندراج_ELEM_FLAG_NONE)) {
		أرجع 1;
	}

	إذا ((a->elem[اندراج_ELEM_META].flags == اندراج_ELEM_FLAG_NONE) &&
	    (b->elem[اندراج_ELEM_META].flags != اندراج_ELEM_FLAG_NONE)) {
		أرجع -1;
	} وإلا إذا ((a->elem[اندراج_ELEM_META].flags != اندراج_ELEM_FLAG_NONE) &&
		   (b->elem[اندراج_ELEM_META].flags == اندراج_ELEM_FLAG_NONE)) {
		أرجع 1;
	}

	إذا (a->use_گنتی < b->use_گنتی) {
		أرجع -1;
	} وإلا إذا (a->use_گنتی > b->use_گنتی) {
		أرجع 1;
	}
	/* use count is the same - now consider last use time */

	إذا (a->آخری_ورتیا < b->آخری_ورتیا) {
		أرجع -1;
	} وإلا إذا (a->آخری_ورتیا > b->آخری_ورتیا) {
		أرجع 1;
	}

	/* they are the same */
	أرجع 0;
}

تعريفنوع بنية {
	بنية store_اندراج **elist;
	size_t ent_گنتی;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
ساكن bool
اندراج_eviction_iterator_cb(فراغ *کلید, فراغ *قدر, فراغ *ctx)
{
	eviction_state_t *estate = ctx;
	بنية store_اندراج *ent = قدر;
	estate->elist[estate->ent_گنتی++] = ent;
	أرجع false;
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
ساكن nserror store_evict(بنية store_state *state)
{
	size_t ent = 0;
	size_t removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	size_t old_گنتی;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	إذا (state->کل_alloc < state->حد_بندی) {
		/* cache within limits */
		أرجع NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->کل_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_گنتی = hashmap_گنتی(state->entries);
	estate.ent_گنتی = 0;
	estate.elist = حافظہ_دیو(حجم(بنية state_اندراج*) * old_گنتی);
	إذا (estate.elist == عدم) {
		أرجع NSERROR_NOMEM;
	}

	إذا (hashmap_iterate(state->entries, اندراج_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		حرر(estate.elist);
		أرجع NSERROR_UNKNOWN;
	}

	إذا (old_گنتی != estate.ent_گنتی) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		حرر(estate.elist);
		أرجع NSERROR_UNKNOWN;
	}

	qsort(estate.elist, estate.ent_گنتی, حجم(بنية state_اندراج*), compar);

	/* evict entries in listed order */
	removed = 0;
	لكل (ent = 0; ent < estate.ent_گنتی; ent++) {
		بنية store_اندراج *bse = estate.elist[ent];

		removed += bse->elem[اندراج_ELEM_انکڑے].ناپ;
		removed += bse->elem[اندراج_ELEM_META].ناپ;

		ret = invalidate_اندراج(state, bse);
		إذا (ret != NSERROR_OK) {
			اكسر;
		}

		إذا (removed > state->hysteresis) {
			اكسر;
		}
	}

	حرر(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->کل_alloc, old_گنتی - ent);

	أرجع ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
ساكن nserror
write_اندراج(بنية store_اندراج *ent, صحيح fd)
{
	uint32_t len = لڑی_لمبائی(nsurl_access(ent->url));
	إذا (اكتب(fd, &len, حجم(len)) != حجم(len))
		أرجع NSERROR_SAVE_FAILED;
	إذا (اكتب(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		أرجع NSERROR_SAVE_FAILED;
	إذا (اكتب(fd, ent, حجم(*ent)) != حجم(*ent))
		أرجع NSERROR_SAVE_FAILED;

	أرجع NSERROR_OK;
}

تعريفنوع بنية {
	صحيح fd;
	size_t written;
} write_اندراج_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
ساكن bool
write_اندراج_iterator(فراغ *کلید, فراغ *قدر, فراغ *ctx)
{
	/* We ignore the key */
	بنية store_اندراج *ent = قدر;
	write_اندراج_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	أرجع write_اندراج(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror write_entries(بنية store_state *state)
{
	حرف *tname = عدم; /* temporary file name for atomic replace */
	حرف *fname = عدم; /* target filename */
	write_اندراج_iteration_state weistate;
	nserror ret;

	memset(&weistate, 0, حجم(weistate));

	إذا (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		أرجع NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, عدم, 2, state->رستہ, "t"ENTRIES_FNAME);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	weistate.fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	إذا (weistate.fd == -1) {
		حرر(tname);
		أرجع NSERROR_SAVE_FAILED;
	}

	إذا (hashmap_iterate(state->entries, write_اندراج_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		close(weistate.fd);
		unlink(tname);
		حرر(tname);
		أرجع NSERROR_SAVE_FAILED;
	}

	close(weistate.fd);

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, ENTRIES_FNAME);
	إذا (ret != NSERROR_OK) {
		unlink(tname);
		حرر(tname);
		أرجع ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(فراغ)remove(fname);
	إذا (rename(tname, fname) != 0) {
		unlink(tname);
		حرر(tname);
		حرر(fname);
		أرجع NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	أرجع NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
ساكن nserror write_بلاکاں(بنية store_state *state)
{
	صحيح fd;
	حرف *tname = عدم; /* temporary file name for atomic replace */
	حرف *fname = عدم; /* target filename */
	size_t بلاکاں_ناپ;
	size_t written = 0;
	size_t wr;
	nserror ret;
	صحيح bfidx; /* block file index */
	صحيح elem_idx;

	إذا (state->بلاکاں_dirty == false) {
		/* blocks use maps have not been updated since last write */
		أرجع NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, عدم, 2, state->رستہ, "t"بلاکاں_FNAME);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	إذا (fd == -1) {
		حرر(tname);
		أرجع NSERROR_SAVE_FAILED;
	}

	بلاکاں_ناپ = (بلاک_FILE_گنتی * اندراج_ELEM_گنتی) * بلاک_USE_MAP_ناپ;

	لكل (elem_idx = 0; elem_idx < اندراج_ELEM_گنتی; elem_idx++) {
		لكل (bfidx = 0; bfidx < بلاک_FILE_گنتی; bfidx++) {
			wr = اكتب(fd,
				   &state->بلاکاں[elem_idx][bfidx].use_map[0],
				   بلاک_USE_MAP_ناپ);
			إذا (wr != بلاک_USE_MAP_ناپ) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				اذهب wr_err;
			}
			written += wr;
		}
	}
wr_err:
	close(fd);

	/* check all data was written */
	إذا (written != بلاکاں_ناپ) {
		unlink(tname);
		حرر(tname);
		أرجع NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, بلاکاں_FNAME);
	إذا (ret != NSERROR_OK) {
		unlink(tname);
		حرر(tname);
		أرجع ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(فراغ)remove(fname);
	إذا (rename(tname, fname) != 0) {
		unlink(tname);
		حرر(tname);
		حرر(fname);
		أرجع NSERROR_SAVE_FAILED;
	}

	أرجع NSERROR_OK;
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
ساكن nserror سیٹ_بلاک_extents(بنية store_state *state)
{
	صحيح bfidx; /* block file index */
	صحيح elem_idx;
	صحيح ftr;

	إذا (state->بلاکاں_opened == false) {
		/* no blocks have been opened since last write */
		أرجع NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	لكل (elem_idx = 0; elem_idx < اندراج_ELEM_گنتی; elem_idx++) {
		لكل (bfidx = 0; bfidx < بلاک_FILE_گنتی; bfidx++) {
			إذا (state->بلاکاں[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->بلاکاں[elem_idx][bfidx].fd, 1U << (log2_بلاک_ناپ[elem_idx] + بلاک_اندراج_گنتی));
				إذا (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      errno);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->بلاکاں_opened = false;

	أرجع NSERROR_OK;
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
ساكن فراغ control_maintenance(فراغ *s)
{
	بنية store_state *state = s;

	write_entries(state);
	write_بلاکاں(state);
	سیٹ_بلاک_extents(state);
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
ساكن nserror
get_store_اندراج(بنية store_state *state, nsurl *url, بنية store_اندراج **bse)
{
	بنية store_اندراج *ent;

	ent = hashmap_lookup(state->entries, url);

	إذا (ent == عدم) {
		أرجع NSERROR_NOT_لبھیا;
	}

	*bse = ent;

	ent->آخری_ورتیا = time(عدم);
	ent->use_گنتی++;

	state->entries_dirty = true;

	guit->misc->ویلا_سارنی(CONTROL_MAINT_ویلا, control_maintenance, state);

	أرجع NSERROR_OK;
}

/**
 * Find next available small block.
 */
ساكن بلاک_اشاریہ_t alloc_بلاک(بنية store_state *state, صحيح elem_idx)
{
	صحيح bf;
	صحيح idx;
	صحيح ثنائی_ہندسہ;
	uint8_t *map;

	لكل (bf = 0; bf < بلاک_FILE_گنتی; bf++) {
		map = &state->بلاکاں[elem_idx][bf].use_map[0];

		لكل (idx = 0; idx < بلاک_USE_MAP_ناپ; idx++) {
			إذا (*(map + idx) != 0xff) {
				/* located an unused block */
				لكل (ثنائی_ہندسہ = 0; ثنائی_ہندسہ < 8;ثنائی_ہندسہ++) {
					إذا (((*(map + idx)) & (1U << ثنائی_ہندسہ)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << ثنائی_ہندسہ;
						state->بلاکاں_dirty = true;
						أرجع (((bf * بلاک_USE_MAP_ناپ) + idx) * 8) + ثنائی_ہندسہ;
					}
				}
			}
		}
	}

	أرجع 0;
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
ساكن nserror
سیٹ_store_اندراج(بنية store_state *state,
		nsurl *url,
		صحيح elem_idx,
		uint8_t *انکڑے,
		ثابت size_t datalen,
		بنية store_اندراج **bse)
{
	بنية store_اندراج *se;
	nserror ret;
	بنية store_اندراج_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	se = hashmap_lookup(state->entries, url);
	إذا (se == عدم) {
		se = hashmap_داخل_کرو(state->entries, url);
	}
	إذا (se == عدم) {
		أرجع NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	إذا ((elem->flags & (اندراج_ELEM_FLAG_ڈھیر | اندراج_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		أرجع NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_گنتی = 1;
	se->آخری_ورتیا = time(عدم);

	/* store the data in the element */
	elem->flags |= اندراج_ELEM_FLAG_ڈھیر;
	elem->انکڑے = انکڑے;
	elem->ref = 1;

	/* account for size of entry element */
	state->کل_alloc -= elem->ناپ;
	elem->ناپ = datalen;
	state->کل_alloc += elem->ناپ;

	/* if the element will fit in a small block attempt to allocate one */
	إذا (elem->ناپ <= (1U << log2_بلاک_ناپ[elem_idx])) {
		elem->بلاک = alloc_بلاک(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->ویلا_سارنی(CONTROL_MAINT_ویلا, control_maintenance, state);

	*bse = se;

	أرجع NSERROR_OK;
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
ساكن صحيح
store_open(بنية store_state *state,
	   اندراج_ident_t ident,
	   صحيح elem_idx,
	   صحيح openflags)
{
	حرف *fname;
	nserror ret;
	صحيح fd;

	fname = store_fname(state, ident, elem_idx);
	إذا (fname == عدم) {
		NSLOG(netsurf, ERROR, "filename error");
		أرجع -1;
	}

	/* ensure all path elements to file exist if creating file */
	إذا (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		إذا (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			حرر(fname);
			أرجع -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = open(fname, openflags, S_IRUSR | S_IWUSR);

	حرر(fname);

	أرجع fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
unlink_entries(بنية store_state *state)
{
	حرف *fname = عدم;
	nserror ret;

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, ENTRIES_FNAME);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	unlink(fname);

	حرر(fname);
	أرجع NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
ساكن nserror
read_entries(بنية store_state *state)
{
	حرف *fname = عدم;
	حرف *url;
	nsurl *nsurl;
	nserror ret;
	size_t read_entries = 0;
	بنية store_اندراج *ent;
	صحيح fd;

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, ENTRIES_FNAME);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	إذا (state->entries == عدم) {
		حرر(fname);
		أرجع NSERROR_NOMEM;
	}

	fd = open(fname, O_RDWR);
	إذا (fd != -1) {
		uint32_t urllen;
		طالما (اقرأ(fd, &urllen, حجم(urllen)) == حجم(urllen)) {
			url = صفر_حافظہ_دیو(1, urllen+1);
			إذا (url == عدم) {
				close(fd);
				حرر(fname);
				أرجع NSERROR_NOMEM;
			}
			إذا (اقرأ(fd, url, urllen) != (ssize_t)urllen) {
				حرر(url);
				close(fd);
				حرر(fname);
				أرجع NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			إذا (ret != NSERROR_OK) {
				حرر(url);
				close(fd);
				حرر(fname);
				أرجع ret;
			}
			حرر(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_داخل_کرو(state->entries, nsurl);
			إذا (ent == عدم) {
				nsurl_unref(nsurl);
				close(fd);
				حرر(fname);
				أرجع NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			إذا (اقرأ(fd, ent, حجم(*ent)) != حجم(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				close(fd);
				حرر(fname);
				أرجع NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->کل_alloc += ent->elem[اندراج_ELEM_انکڑے].ناپ;
			state->کل_alloc += ent->elem[اندراج_ELEM_META].ناپ;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[اندراج_ELEM_انکڑے].flags &= ~(اندراج_ELEM_FLAG_ڈھیر | اندراج_ELEM_FLAG_MMAP);
			ent->elem[اندراج_ELEM_META].flags &= ~(اندراج_ELEM_FLAG_ڈھیر | اندراج_ELEM_FLAG_MMAP);

		}
		close(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	حرر(fname);
	أرجع NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
read_بلاکاں(بنية store_state *state)
{
	صحيح bfidx; /* block file index */
	صحيح elem_idx;
	صحيح fd;
	ssize_t rd;
	حرف *fname = عدم;
	nserror ret;

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, بلاکاں_FNAME);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = open(fname, O_RDWR);
	حرر(fname);
	إذا (fd != -1) {
		/* initialise block file use array */
		لكل (elem_idx = 0; elem_idx < اندراج_ELEM_گنتی; elem_idx++) {
			لكل (bfidx = 0; bfidx < بلاک_FILE_گنتی; bfidx++) {
				rd = اقرأ(fd,
					  &state->بلاکاں[elem_idx][bfidx].use_map[0],
					  بلاک_USE_MAP_ناپ);
				إذا (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					اذهب rd_err;
				}
			}
		}
	rd_err:
		close(fd);

	} وإلا {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->بلاکاں[اندراج_ELEM_انکڑے][0].use_map[0] = 1;
		state->بلاکاں[اندراج_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	لكل (bfidx = 0; bfidx < بلاک_FILE_گنتی; bfidx++) {
		state->بلاکاں[اندراج_ELEM_انکڑے][bfidx].fd = -1;
		state->بلاکاں[اندراج_ELEM_META][bfidx].fd = -1;
	}

	أرجع NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
write_cache_ٹیگ(بنية store_state *state)
{
	FILE *fcachetag;
	nserror ret;
	حرف *fname = عدم;

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, "CACHEDIR.TAG");
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	fcachetag = fopen(fname, "wb");

	حرر(fname);

	إذا (fcachetag == عدم) {
		أرجع NSERROR_NOT_لبھیا;
	}

	fprintf(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	fclose(fcachetag);

	أرجع NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
write_control(بنية store_state *state)
{
	FILE *fcontrol;
	nserror ret;
	حرف *fname = عدم;

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, "control");
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	إذا (ret != NSERROR_OK) {
		حرر(fname);
		أرجع ret;
	}

	fcontrol = fopen(fname, "wb");

	حرر(fname);

	إذا (fcontrol == عدم) {
		أرجع NSERROR_NOT_لبھیا;
	}

	fprintf(fcontrol, "%u%c", CONTROL_نسخہ, 0);

	fclose(fcontrol);

	أرجع NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
read_control(بنية store_state *state)
{
	nserror ret;
	FILE *fcontrol;
	بنانشانی صحيح ctrlversion;
	حرف *fname = عدم;

	ret = netsurf_mkpath(&fname, عدم, 2, state->رستہ, "control");
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = fopen(fname, "rb");

	حرر(fname);

	إذا (fcontrol == عدم) {
		/* unable to open control file */
		إذا (errno == ENOENT) {
			أرجع NSERROR_NOT_لبھیا;
		} وإلا {
			أرجع NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	إذا (fscanf(fcontrol, "%u", &ctrlversion) != 1) {
		اذهب control_error;
	}

	إذا (ctrlversion != CONTROL_نسخہ) {
		اذهب control_error;
	}

	إذا (fgetc(fcontrol) != 0) {
		اذهب control_error;
	}

	fclose(fcontrol);

	أرجع NSERROR_OK;

control_error: /* problem with the control file */

	fclose(fcontrol);

	أرجع NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror
initialise(ثابت بنية llcache_store_parameters *parameters)
{
	بنية store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	إذا (storestate != عدم) {
		أرجع NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	إذا (parameters->حد_بندی == 0) {
		أرجع NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	إذا (parameters->رستہ == عدم) {
		أرجع NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = صفر_حافظہ_دیو(1, حجم(بنية store_state));
	إذا (newstate == عدم) {
		أرجع NSERROR_NOMEM;
	}

	newstate->رستہ = strdup(parameters->رستہ);
	newstate->حد_بندی = parameters->حد_بندی;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	إذا (ret != NSERROR_OK) {
		إذا (ret == NSERROR_NOT_لبھیا) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} وإلا {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->رستہ);
			إذا (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->رستہ);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} وإلا {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->رستہ);
			}
		}
		ret = write_control(newstate);
		إذا (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_ٹیگ(newstate);
		}
	}
	إذا (ret != NSERROR_OK) {
		/* that went well obviously */
		حرر(newstate->رستہ);
		حرر(newstate);
		أرجع ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	إذا (ret != NSERROR_OK) {
		/* that went well obviously */
		حرر(newstate->رستہ);
		حرر(newstate);
		أرجع ret;
	}

	/* read blocks */
	ret = read_بلاکاں(newstate);
	إذا (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		حرر(newstate->رستہ);
		حرر(newstate);
		أرجع ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->رستہ,
	      newstate->حد_بندی,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->کل_alloc, newstate->حد_بندی);

	أرجع NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
ساكن nserror
finalise(فراغ)
{
	صحيح bf; /* block file index */
	بنانشانی صحيح op_گنتی;

	إذا (storestate != عدم) {
		guit->misc->ویلا_سارنی(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_بلاکاں(storestate);

		/* ensure all block files are closed */
		لكل (bf = 0; bf < بلاک_FILE_گنتی; bf++) {
			إذا (storestate->بلاکاں[اندراج_ELEM_انکڑے][bf].fd != -1) {
				close(storestate->بلاکاں[اندراج_ELEM_انکڑے][bf].fd);
			}
			إذا (storestate->بلاکاں[اندراج_ELEM_META][bf].fd != -1) {
				close(storestate->بلاکاں[اندراج_ELEM_META][bf].fd);
			}
		}

		op_گنتی = storestate->کامیابی_گنتی + storestate->کھنج_گنتی;

		/* avoid division by zero */
		إذا (op_گنتی > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_گنتی,
			      storestate->کامیابی_گنتی,
			      storestate->کھنج_گنتی,
			      0,
			      (storestate->کامیابی_گنتی * 100) / op_گنتی,
			      (storestate->کھنج_گنتی * 100) / op_گنتی,
			      0);
		}

		hashmap_destroy(storestate->entries);
		حرر(storestate->رستہ);
		حرر(storestate);
		storestate = عدم;
	}
	أرجع NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ساكن nserror store_write_بلاک(بنية store_state *state,
			 بنية store_اندراج *bse,
			 صحيح elem_idx)
{
	بلاک_اشاریہ_t bf = (bse->elem[elem_idx].بلاک >> بلاک_اندراج_گنتی) &
		((1 << بلاک_FILE_گنتی) - 1); /* block file block resides in */
	بلاک_اشاریہ_t bi = bse->elem[elem_idx].بلاک & ((1U << بلاک_اندراج_گنتی) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	إذا (state->بلاکاں[elem_idx][bf].fd == -1) {
		state->بلاکاں[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + اندراج_ELEM_گنتی, O_CREAT | O_RDWR);
		إذا (state->بلاکاں[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			أرجع NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->بلاکاں_opened = true;
	}

	offst = (بنانشانی صحيح)bi << log2_بلاک_ناپ[elem_idx];

	wr = nsu_pwrite(state->بلاکاں[elem_idx][bf].fd,
			bse->elem[elem_idx].انکڑے,
			bse->elem[elem_idx].ناپ,
			offst);
	إذا (wr != (ssize_t)bse->elem[elem_idx].ناپ) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].ناپ,
		      bse->elem[elem_idx].انکڑے,
		      (size_t)offst,
		      bse->elem[elem_idx].بلاک,
		      errno);
		أرجع NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].انکڑے, (size_t)offst,
	      bse->elem[elem_idx].بلاک);

	أرجع NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ساكن nserror store_write_file(بنية store_state *state,
			 بنية store_اندراج *bse,
			 صحيح elem_idx)
{
	ssize_t wr;
	صحيح fd;
	صحيح err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	إذا (fd < 0) {
		perror("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		أرجع NSERROR_SAVE_FAILED;
	}

	wr = اكتب(fd, bse->elem[elem_idx].انکڑے, bse->elem[elem_idx].ناپ);
	err = errno; /* close can change errno */

	close(fd);
	إذا (wr != (ssize_t)bse->elem[elem_idx].ناپ) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].ناپ,
		      bse->elem[elem_idx].انکڑے,
		      err);

		/** @todo Delete the file? */
		أرجع NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].انکڑے);

	أرجع NSERROR_OK;
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
ساكن nserror
store(nsurl *url,
      تعداد backing_store_flags bsflags,
      uint8_t *انکڑے,
      ثابت size_t datalen)
{
	nserror ret;
	بنية store_اندراج *bse;
	صحيح elem_idx;

	/* check backing store is initialised */
	إذا (storestate == عدم) {
		أرجع NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	إذا ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = اندراج_ELEM_META;
	} وإلا {
		elem_idx = اندراج_ELEM_انکڑے;
	}

	/* set the store entry up */
	ret = سیٹ_store_اندراج(storestate, url, elem_idx, انکڑے, datalen, &bse);
	إذا (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		أرجع ret;
	}

	إذا (bse->elem[elem_idx].بلاک != 0) {
		/* small block storage */
		ret = store_write_بلاک(storestate, bse, elem_idx);
	} وإلا {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	أرجع ret;
}

/**
 * release any allocation for an entry
 */
ساكن nserror اندراج_release_alloc(بنية store_اندراج_element *elem)
{
	إذا ((elem->flags & اندراج_ELEM_FLAG_ڈھیر) != 0) {
		elem->ref--;
		إذا (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->انکڑے);
			حرر(elem->انکڑے);
			elem->flags &= ~اندراج_ELEM_FLAG_ڈھیر;
		}
	}
	أرجع NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ساكن nserror store_read_بلاک(بنية store_state *state,
			 بنية store_اندراج *bse,
			 صحيح elem_idx)
{
	بلاک_اشاریہ_t bf = (bse->elem[elem_idx].بلاک >> بلاک_اندراج_گنتی) &
		((1 << بلاک_FILE_گنتی) - 1); /* block file block resides in */
	بلاک_اشاریہ_t bi = bse->elem[elem_idx].بلاک & ((1 << بلاک_اندراج_گنتی) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	إذا (state->بلاکاں[elem_idx][bf].fd == -1) {
		state->بلاکاں[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + اندراج_ELEM_گنتی, O_CREAT | O_RDWR);
		إذا (state->بلاکاں[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			أرجع NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->بلاکاں_opened = true;
	}

	offst = (بنانشانی صحيح)bi << log2_بلاک_ناپ[elem_idx];

	rd = nsu_pread(state->بلاکاں[elem_idx][bf].fd,
		       bse->elem[elem_idx].انکڑے,
		       bse->elem[elem_idx].ناپ,
		       offst);
	إذا (rd != (ssize_t)bse->elem[elem_idx].ناپ) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].ناپ,
		      bse->elem[elem_idx].انکڑے,
		      (size_t)offst,
		      bse->elem[elem_idx].بلاک,
		      errno);
		أرجع NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].انکڑے, (size_t)offst,
	      bse->elem[elem_idx].بلاک);

	أرجع NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
ساكن nserror store_read_file(بنية store_state *state,
			 بنية store_اندراج *bse,
			 صحيح elem_idx)
{
	صحيح fd;
	ssize_t rd; /* return from read */
	صحيح ret = NSERROR_OK;
	size_t tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	إذا (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		/** @todo should this invalidate the entry? */
		أرجع NSERROR_NOT_لبھیا;
	}

	طالما (tot < bse->elem[elem_idx].ناپ) {
		rd = اقرأ(fd,
			  bse->elem[elem_idx].انکڑے + tot,
			  bse->elem[elem_idx].ناپ - tot);
		إذا (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      errno);
			ret = NSERROR_NOT_لبھیا;
			اكسر;
		}
		tot += rd;
	}

	close(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].انکڑے);

	أرجع ret;
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
ساكن nserror
fetch(nsurl *url,
      تعداد backing_store_flags bsflags,
      uint8_t **انکڑے_باہرلا,
      size_t *datalen_باہرلا)
{
	nserror ret;
	بنية store_اندراج *bse;
	بنية store_اندراج_element *elem;
	صحيح elem_idx;

	/* check backing store is initialised */
	إذا (storestate == عدم) {
		أرجع NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_اندراج(storestate, url, &bse);
	إذا (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->کھنج_گنتی++;
		أرجع ret;
	}
	storestate->کامیابی_گنتی++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	إذا ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = اندراج_ELEM_META;
	} وإلا {
		elem_idx = اندراج_ELEM_انکڑے;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	إذا ((elem->flags & اندراج_ELEM_FLAG_ڈھیر) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->انکڑے, elem->ref);

	} وإلا {
		/* allocate from the heap */
		elem->انکڑے = حافظہ_دیو(elem->ناپ);
		إذا (elem->انکڑے == عدم) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			أرجع NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->انکڑے);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= اندراج_ELEM_FLAG_ڈھیر;
		elem->ref = 1;

		/* fill the new block */
		إذا (elem->بلاک != 0) {
			ret = store_read_بلاک(storestate, bse, elem_idx);
		} وإلا {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	إذا (ret != NSERROR_OK) {
		اندراج_release_alloc(elem);
	} وإلا {
		/* update stats and setup return pointers */
		storestate->کامیابی_ناپ += elem->ناپ;

		*انکڑے_باہرلا = elem->انکڑے;
		*datalen_باہرلا = elem->ناپ;
	}

	أرجع ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
ساكن nserror release(nsurl *url, تعداد backing_store_flags bsflags)
{
	nserror ret;
	بنية store_اندراج *bse;
	بنية store_اندراج_element *elem;

	/* check backing store is initialised */
	إذا (storestate == عدم) {
		أرجع NSERROR_INIT_FAILED;
	}

	ret = get_store_اندراج(storestate, url, &bse);
	إذا (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		أرجع ret;
	}

	/* the entry element */
	إذا ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[اندراج_ELEM_META];
	} وإلا {
		elem = &bse->elem[اندراج_ELEM_انکڑے];
	}

	ret = اندراج_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	إذا ((ret == NSERROR_OK) &&
	    ((bse->flags & اندراج_FLAGS_INVALID) != 0)) {
		ret = invalidate_اندراج(storestate, bse);
	}

	أرجع ret;
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
ساكن nserror
invalidate(nsurl *url)
{
	nserror ret;
	بنية store_اندراج *bse;

	/* check backing store is initialised */
	إذا (storestate == عدم) {
		أرجع NSERROR_INIT_FAILED;
	}

	ret = get_store_اندراج(storestate, url, &bse);
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	أرجع invalidate_اندراج(storestate, bse);
}


ساكن بنية gui_llcache_جدول llcache_جدول = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

بنية gui_llcache_جدول *filesystem_llcache_جدول = &llcache_جدول;
