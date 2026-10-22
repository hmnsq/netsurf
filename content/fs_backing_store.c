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

#అన్తర్భావ <unistd.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <nsutils/unistd.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/filepath.h"
#అన్తర్భావ "utils/file.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/hashmap.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "netsurf/misc.h"

#అన్తర్భావ "content/backing_store.h"

/** Backing store file format version */
#నిర్వచించు CONTROL_సంస్కరణ 202

/**
 * Number of milliseconds after a update before control data
 * maintenance is performed
 */
#నిర్వచించు CONTROL_MAINT_సమయం 10000

/** Filename of serialised entries */
#నిర్వచించు ENTRIES_FNAME "entries"

/** Filename of block file index */
#నిర్వచించు ఖండాలు_FNAME "blocks"

/** log2 block data address length (64k) */
#నిర్వచించు ఖండం_ADDR_LEN 16

/** log2 number of entries per block file(1024) */
#నిర్వచించు ఖండం_నమోదు_లెక్క 10

/** log2 number of data block files */
#నిర్వచించు ఖండం_FILE_లెక్క (ఖండం_ADDR_LEN - ఖండం_నమోదు_లెక్క)

/** log2 size of data blocks (8k) */
#నిర్వచించు ఖండం_దత్తాంశం_కొలత 13

/** log2 size of metadata blocks (8k) */
#నిర్వచించు ఖండం_META_కొలత 13

/** length in bytes of a block files use map */
#నిర్వచించు ఖండం_USE_MAP_కొలత (1 << (ఖండం_నమోదు_లెక్క - 3))

/**
 * The type used as a binary identifier for each entry derived from
 * the URL. A larger identifier will have fewer collisions but
 * requires proportionately more storage.
 */
రకనిర్వచనం uint32_t నమోదు_ident_t;

/**
 * The type used to store block file index values. If this is changed
 * it will affect the entry storage/alignment and BLOCK_ADDR_LEN must
 * also be updated.
 */
రకనిర్వచనం uint16_t ఖండం_సూచీ_t;


/**
 * Entry element index values.
 */
గణన store_నమోదు_elem_idx {
	నమోదు_ELEM_దత్తాంశం = 0, /**< entry element is data */
	నమోదు_ELEM_META = 1,  /**< entry element is metadata */
	నమోదు_ELEM_లెక్క = 2, /**< count of elements on an entry */
};

/**
 * flags that indicate what additional information is contained within
 * an entry element.
 */
గణన store_నమోదు_elem_flags {
	/** store not managing any allocation on entry */
	నమోదు_ELEM_FLAG_NONE = 0,
	/** entry data allocation is on heap */
	నమోదు_ELEM_FLAG_కుప్ప = 0x1,
	/** entry data allocation is mmaped */
	నమోదు_ELEM_FLAG_MMAP = 0x2,
	/** entry data allocation is in small object pool */
	నమోదు_ELEM_FLAG_SMALL = 0x4,
};


గణన store_నమోదు_flags {
	/** entry is normal */
	నమోదు_FLAGS_NONE = 0,
	/** entry has been invalidated but something still holding a reference */
	నమోదు_FLAGS_INVALID = 1,
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
నిర్మాణం store_నమోదు_element {
	uint8_t* దత్తాంశం; /**< data allocated */
	uint32_t కొలత; /**< size of entry element on disc */
	ఖండం_సూచీ_t ఖండం; /**< small object data block */
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
నిర్మాణం store_నమోదు {
	nsurl *url; /**< The URL for this entry */
	int64_t చివరి_వాడిన; /**< UNIX time the entry was last used */
	uint16_t use_లెక్క; /**< number of times this entry has been accessed */
	uint8_t flags; /**< entry flags */
	/** Entry element (data or meta) specific information */
	నిర్మాణం store_నమోదు_element elem[నమోదు_ELEM_లెక్క];
};

/**
 * Small block file.
 */
నిర్మాణం ఖండం_file {
	/** file descriptor of the block file */
	పూర్ణాంకం fd;
	/** map of used and unused entries within the block file */
	uint8_t use_map[ఖండం_USE_MAP_కొలత];
};

/**
 * log2 of block size.
 */
స్థిర స్థిరరాశి గుర్తులేని పూర్ణాంకం log2_ఖండం_కొలత[నమోదు_ELEM_లెక్క] = {
	ఖండం_దత్తాంశం_కొలత, /**< Data block size */
	ఖండం_META_కొలత  /**< Metadata block size */
};

/**
 * Parameters controlling the backing store.
 */
నిర్మాణం store_state {
	/* store config */
	అక్షరం *మార్గం; /**< The path to the backing store */
	size_t హద్దు; /**< The backing store upper bound target size */
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
	నిర్మాణం ఖండం_file ఖండాలు[నమోదు_ELEM_లెక్క][ఖండం_FILE_లెక్క];

	/** flag indicating if the block file use maps have been made
	 * persistent since they were last changed.
	 */
	bool ఖండాలు_dirty;

	/** flag indicating if a block file has been opened for update
	 * since maintenance was previously done.
	 */
	bool ఖండాలు_opened;


	/* stats */
	uint64_t మొత్తం_alloc; /**< total size of all allocated storage. */

	size_t తగిలింది_లెక్క; /**< number of cache hits */
	uint64_t తగిలింది_కొలత; /**< size of storage served */
	size_t తప్పిపోవు_లెక్క; /**< number of cache misses */

};

/**
 * Global storage state.
 *
 * @todo Investigate if there is a way to have a context rather than
 * use a global.
 */
నిర్మాణం store_state *storestate;

/* Entries hashmap parameters
 *
 * Our hashmap has nsurl keys and store_entry values
 */

స్థిర bool
entries_hashmap_కీలకం_eq(శూన్య *key1, శూన్య *key2)
{
	తిరిగి nsurl_compare((nsurl *)key1, (nsurl *)key2, NSURL_COMPLETE);
}

స్థిర శూన్య *
entries_hashmap_విలువ_alloc(శూన్య *కీలకం)
{
	నిర్మాణం store_నమోదు *ent = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం store_నమోదు));
	ఒకవేళ (ent != ఖాళీ) {
		ent->url = nsurl_ref(కీలకం);
	}
	తిరిగి ent;
}

స్థిర శూన్య
entries_hashmap_విలువ_destroy(శూన్య *విలువ)
{
	నిర్మాణం store_నమోదు *ent = విలువ;
	/** \todo Do we need to do any disk cleanup here?  if so, meep! */
	nsurl_unref(ent->url);
	విడుదల(ent);
}

స్థిర hashmap_parameters_t entries_hashmap_parameters = {
	.కీలకం_clone = (hashmap_కీలకం_clone_t)nsurl_ref,
	.కీలకం_destroy = (hashmap_కీలకం_destroy_t)nsurl_unref,
	.కీలకం_hash = (hashmap_కీలకం_hash_t)nsurl_hash,
	.కీలకం_eq = entries_hashmap_కీలకం_eq,
	.విలువ_alloc = entries_hashmap_విలువ_alloc,
	.విలువ_destroy = entries_hashmap_విలువ_destroy,
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
స్థిర అక్షరం *
store_fname(నిర్మాణం store_state *state,
	    నమోదు_ident_t ident,
	    పూర్ణాంకం elem_idx)
{
	అక్షరం *fname = ఖాళీ;
	uint8_t b32u_i[8]; /* base32 encoded ident */
	స్థిరరాశి uint8_t *b32u_d[6]; /* base32 ident as separate components */

	/* directories used to separate elements */
	స్థిరరాశి అక్షరం *పునాది_dir_పట్టిక[] = {
		"d", "m", "dblk", "mblk"
	};

	/* RFC4648 base32 encoding table (six bits) */
	స్థిరరాశి uint8_t encoding_పట్టిక[64][3] = {
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
	b32u_i[0] = encoding_పట్టిక[(ident      ) & 0x1f][0];
	b32u_i[1] = encoding_పట్టిక[(ident >>  5) & 0x1f][0];
	b32u_i[2] = encoding_పట్టిక[(ident >> 10) & 0x1f][0];
	b32u_i[3] = encoding_పట్టిక[(ident >> 15) & 0x1f][0];
	b32u_i[4] = encoding_పట్టిక[(ident >> 20) & 0x1f][0];
	b32u_i[5] = encoding_పట్టిక[(ident >> 25) & 0x1f][0];
	b32u_i[6] = encoding_పట్టిక[(ident >> 30) & 0x1f][0];
	b32u_i[7] = 0; /* null terminate ident string */

	/* base32 encode directory separators */
	b32u_d[0] = (uint8_t*)పునాది_dir_పట్టిక[elem_idx];
	b32u_d[1] = &encoding_పట్టిక[(ident      ) & 0x3f][0];
	b32u_d[2] = &encoding_పట్టిక[(ident >>  6) & 0x3f][0];
	b32u_d[3] = &encoding_పట్టిక[(ident >> 12) & 0x3f][0];
	b32u_d[4] = &encoding_పట్టిక[(ident >> 18) & 0x3f][0];
	b32u_d[5] = &encoding_పట్టిక[(ident >> 24) & 0x3f][0];

	ఎంపిక (elem_idx) {
	స్థితి నమోదు_ELEM_దత్తాంశం:
	స్థితి నమోదు_ELEM_META:
		netsurf_mkpath(&fname, ఖాళీ, 8,
			       state->మార్గం, b32u_d[0], b32u_d[1], b32u_d[2],
			       b32u_d[3], b32u_d[4], b32u_d[5], b32u_i);
		ఆపు;

	స్థితి (నమోదు_ELEM_లెక్క + నమోదు_ELEM_META):
	స్థితి (నమోదు_ELEM_లెక్క + నమోదు_ELEM_దత్తాంశం):
		netsurf_mkpath(&fname, ఖాళీ, 3,
			       state->మార్గం, b32u_d[0], b32u_d[1]);
		ఆపు;

	అప్రమేయ:
		ప్రతిజ్ఞా("bad element index" == ఖాళీ);
		ఆపు;
	}

	తిరిగి fname;
}

/**
 * invalidate an element of an entry
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @param elem_idx The element index to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
invalidate_element(నిర్మాణం store_state *state,
		   నిర్మాణం store_నమోదు *bse,
		   పూర్ణాంకం elem_idx)
{
	ఒకవేళ (bse->elem[elem_idx].ఖండం != 0) {
		ఖండం_సూచీ_t bf;
		ఖండం_సూచీ_t bi;

		/* block file block resides in */
		bf = (bse->elem[elem_idx].ఖండం >> ఖండం_నమోదు_లెక్క) &
			((1 << ఖండం_FILE_లెక్క) - 1);

		/* block index in file */
		bi = bse->elem[elem_idx].ఖండం & ((1U << ఖండం_నమోదు_లెక్క) -1);

		/* clear bit in use map */
		state->ఖండాలు[elem_idx][bf].use_map[bi >> 3] &= ~(1U << (bi & 7));
	} లేకపోతే {
		అక్షరం *fname;

		/* unlink the file from disc */
		fname = store_fname(state, nsurl_hash(bse->url), elem_idx);
		ఒకవేళ (fname == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
		unlink(fname);
		విడుదల(fname);
	}

	state->మొత్తం_alloc -= bse->elem[elem_idx].కొలత;

	తిరిగి NSERROR_OK;
}

/**
 * Remove the entry and files associated with an identifier.
 *
 * @param state The store state to use.
 * @param bse The entry to invalidate.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
invalidate_నమోదు(నిర్మాణం store_state *state, నిర్మాణం store_నమోదు *bse)
{
	nserror ret;

	/* mark entry as invalid */
	bse->flags |= నమోదు_FLAGS_INVALID;

	/* check if the entry has storage already allocated */
	ఒకవేళ (((bse->elem[నమోదు_ELEM_దత్తాంశం].flags &
	     (నమోదు_ELEM_FLAG_కుప్ప | నమోదు_ELEM_FLAG_MMAP)) != 0) ||
	    ((bse->elem[నమోదు_ELEM_META].flags &
	      (నమోదు_ELEM_FLAG_కుప్ప | నమోదు_ELEM_FLAG_MMAP)) != 0)) {
		/*
		 * This entry cannot be immediately removed as it has
		 * associated allocation so wait for allocation release.
		 */
		NSLOG(netsurf, DEBUG,
		      "invalidating entry with referenced allocation");
		తిరిగి NSERROR_OK;
	}

	NSLOG(netsurf, VERBOSE, "Removing entry for %s", nsurl_access(bse->url));

	ret = invalidate_element(state, bse, నమోదు_ELEM_META);
	ఒకవేళ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating metadata element");
	}

	ret = invalidate_element(state, bse, నమోదు_ELEM_దత్తాంశం);
	ఒకవేళ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "Error invalidating data element");
	}

	/* As our final act we remove bse from the cache */
	hashmap_remove(state->entries, bse->url);
	/* From now, bse is invalid memory */

	తిరిగి NSERROR_OK;
}


/**
 * Quick sort comparison.
 */
స్థిర పూర్ణాంకం compar(స్థిరరాశి శూన్య *va, స్థిరరాశి శూన్య *vb)
{
	స్థిరరాశి నిర్మాణం store_నమోదు *a = *(స్థిరరాశి నిర్మాణం store_నమోదు **)va;
	స్థిరరాశి నిర్మాణం store_నమోదు *b = *(స్థిరరాశి నిర్మాణం store_నమోదు **)vb;

	/* consider the allocation flags - if an entry has an
	 * allocation it is considered more valuable as it cannot be
	 * freed.
	 */
	ఒకవేళ ((a->elem[నమోదు_ELEM_దత్తాంశం].flags == నమోదు_ELEM_FLAG_NONE) &&
	    (b->elem[నమోదు_ELEM_దత్తాంశం].flags != నమోదు_ELEM_FLAG_NONE)) {
		తిరిగి -1;
	} లేకపోతే ఒకవేళ ((a->elem[నమోదు_ELEM_దత్తాంశం].flags != నమోదు_ELEM_FLAG_NONE) &&
		   (b->elem[నమోదు_ELEM_దత్తాంశం].flags == నమోదు_ELEM_FLAG_NONE)) {
		తిరిగి 1;
	}

	ఒకవేళ ((a->elem[నమోదు_ELEM_META].flags == నమోదు_ELEM_FLAG_NONE) &&
	    (b->elem[నమోదు_ELEM_META].flags != నమోదు_ELEM_FLAG_NONE)) {
		తిరిగి -1;
	} లేకపోతే ఒకవేళ ((a->elem[నమోదు_ELEM_META].flags != నమోదు_ELEM_FLAG_NONE) &&
		   (b->elem[నమోదు_ELEM_META].flags == నమోదు_ELEM_FLAG_NONE)) {
		తిరిగి 1;
	}

	ఒకవేళ (a->use_లెక్క < b->use_లెక్క) {
		తిరిగి -1;
	} లేకపోతే ఒకవేళ (a->use_లెక్క > b->use_లెక్క) {
		తిరిగి 1;
	}
	/* use count is the same - now consider last use time */

	ఒకవేళ (a->చివరి_వాడిన < b->చివరి_వాడిన) {
		తిరిగి -1;
	} లేకపోతే ఒకవేళ (a->చివరి_వాడిన > b->చివరి_వాడిన) {
		తిరిగి 1;
	}

	/* they are the same */
	తిరిగి 0;
}

రకనిర్వచనం నిర్మాణం {
	నిర్మాణం store_నమోదు **elist;
	size_t ent_లెక్క;
} eviction_state_t;

/**
 * Iterator for gathering entries to compute eviction order
 */
స్థిర bool
నమోదు_eviction_iterator_cb(శూన్య *కీలకం, శూన్య *విలువ, శూన్య *ctx)
{
	eviction_state_t *estate = ctx;
	నిర్మాణం store_నమోదు *ent = విలువ;
	estate->elist[estate->ent_లెక్క++] = ent;
	తిరిగి false;
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
స్థిర nserror store_evict(నిర్మాణం store_state *state)
{
	size_t ent = 0;
	size_t removed = 0; /* size of removed entries */
	nserror ret = NSERROR_OK;
	size_t old_లెక్క;
	eviction_state_t estate;

	/* check if the cache has exceeded configured limit */
	ఒకవేళ (state->మొత్తం_alloc < state->హద్దు) {
		/* cache within limits */
		తిరిగి NSERROR_OK;
	}

	NSLOG(netsurf, INFO,
	      "Evicting entries to reduce %"PRIu64" by %"PRIsizet,
	      state->మొత్తం_alloc,
	      state->hysteresis);

	/* allocate storage for the list */
	old_లెక్క = hashmap_లెక్క(state->entries);
	estate.ent_లెక్క = 0;
	estate.elist = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం state_నమోదు*) * old_లెక్క);
	ఒకవేళ (estate.elist == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	ఒకవేళ (hashmap_iterate(state->entries, నమోదు_eviction_iterator_cb, &estate)) {
		NSLOG(netsurf, WARNING, "Unexpected termination of eviction iterator");
		విడుదల(estate.elist);
		తిరిగి NSERROR_UNKNOWN;
	}

	ఒకవేళ (old_లెక్క != estate.ent_లెక్క) {
		NSLOG(netsurf, WARNING, "Incorrect entry count after eviction iterator");
		విడుదల(estate.elist);
		తిరిగి NSERROR_UNKNOWN;
	}

	qsort(estate.elist, estate.ent_లెక్క, పరిమాణం(నిర్మాణం state_నమోదు*), compar);

	/* evict entries in listed order */
	removed = 0;
	కొరకు (ent = 0; ent < estate.ent_లెక్క; ent++) {
		నిర్మాణం store_నమోదు *bse = estate.elist[ent];

		removed += bse->elem[నమోదు_ELEM_దత్తాంశం].కొలత;
		removed += bse->elem[నమోదు_ELEM_META].కొలత;

		ret = invalidate_నమోదు(state, bse);
		ఒకవేళ (ret != NSERROR_OK) {
			ఆపు;
		}

		ఒకవేళ (removed > state->hysteresis) {
			ఆపు;
		}
	}

	విడుదల(estate.elist);

	NSLOG(netsurf, INFO,
	      "removed %"PRIsizet" in %"PRIsizet" entries, %"PRIu64" remaining in %"PRIsizet" entries",
	      removed, ent, state->మొత్తం_alloc, old_లెక్క - ent);

	తిరిగి ret;
}

/**
 * Write a single store entry to disk
 *
 * To serialise a single store entry for now we write out a 32bit int
 * which is the length of the url, then that many bytes of the url.
 * Then we write out the full store entry struct as-is, which includes
 * a useless nsurl pointer.
 */
స్థిర nserror
write_నమోదు(నిర్మాణం store_నమోదు *ent, పూర్ణాంకం fd)
{
	uint32_t len = అక్షరశ్రేణి_పొడవు(nsurl_access(ent->url));
	ఒకవేళ (లిఖ(fd, &len, పరిమాణం(len)) != పరిమాణం(len))
		తిరిగి NSERROR_SAVE_FAILED;
	ఒకవేళ (లిఖ(fd, nsurl_access(ent->url), len) != (ssize_t)len)
		తిరిగి NSERROR_SAVE_FAILED;
	ఒకవేళ (లిఖ(fd, ent, పరిమాణం(*ent)) != పరిమాణం(*ent))
		తిరిగి NSERROR_SAVE_FAILED;

	తిరిగి NSERROR_OK;
}

రకనిర్వచనం నిర్మాణం {
	పూర్ణాంకం fd;
	size_t written;
} write_నమోదు_iteration_state;

/**
 * Callback for iterating the entries hashmap
 */
స్థిర bool
write_నమోదు_iterator(శూన్య *కీలకం, శూన్య *విలువ, శూన్య *ctx)
{
	/* We ignore the key */
	నిర్మాణం store_నమోదు *ent = విలువ;
	write_నమోదు_iteration_state *state = ctx;
	state->written++;
	/* We stop early if we fail to write this entry */
	తిరిగి write_నమోదు(ent, state->fd) != NSERROR_OK;
}

/**
 * Write filesystem entries to file.
 *
 * Serialise entry index out to storage.
 *
 * @param state The backing store state to serialise.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror write_entries(నిర్మాణం store_state *state)
{
	అక్షరం *tname = ఖాళీ; /* temporary file name for atomic replace */
	అక్షరం *fname = ఖాళీ; /* target filename */
	write_నమోదు_iteration_state weistate;
	nserror ret;

	memset(&weistate, 0, పరిమాణం(weistate));

	ఒకవేళ (state->entries_dirty == false) {
		/* entries have not been updated since last write */
		తిరిగి NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ఖాళీ, 2, state->మార్గం, "t"ENTRIES_FNAME);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	weistate.fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ఒకవేళ (weistate.fd == -1) {
		విడుదల(tname);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	ఒకవేళ (hashmap_iterate(state->entries, write_నమోదు_iterator, &weistate)) {
		/* The iteration ended early, so we failed */
		close(weistate.fd);
		unlink(tname);
		విడుదల(tname);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	close(weistate.fd);

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, ENTRIES_FNAME);
	ఒకవేళ (ret != NSERROR_OK) {
		unlink(tname);
		విడుదల(tname);
		తిరిగి ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(శూన్య)remove(fname);
	ఒకవేళ (rename(tname, fname) != 0) {
		unlink(tname);
		విడుదల(tname);
		విడుదల(fname);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO, "Wrote out %"PRIsizet" entries", weistate.written);

	తిరిగి NSERROR_OK;
}

/**
 * Write block file use map to file.
 *
 * Serialise block file use map out to storage.
 *
 * \param state The backing store state to serialise.
 * \return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror write_ఖండాలు(నిర్మాణం store_state *state)
{
	పూర్ణాంకం fd;
	అక్షరం *tname = ఖాళీ; /* temporary file name for atomic replace */
	అక్షరం *fname = ఖాళీ; /* target filename */
	size_t ఖండాలు_కొలత;
	size_t written = 0;
	size_t wr;
	nserror ret;
	పూర్ణాంకం bfidx; /* block file index */
	పూర్ణాంకం elem_idx;

	ఒకవేళ (state->ఖండాలు_dirty == false) {
		/* blocks use maps have not been updated since last write */
		తిరిగి NSERROR_OK;
	}

	ret = netsurf_mkpath(&tname, ఖాళీ, 2, state->మార్గం, "t"ఖండాలు_FNAME);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ఒకవేళ (fd == -1) {
		విడుదల(tname);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	ఖండాలు_కొలత = (ఖండం_FILE_లెక్క * నమోదు_ELEM_లెక్క) * ఖండం_USE_MAP_కొలత;

	కొరకు (elem_idx = 0; elem_idx < నమోదు_ELEM_లెక్క; elem_idx++) {
		కొరకు (bfidx = 0; bfidx < ఖండం_FILE_లెక్క; bfidx++) {
			wr = లిఖ(fd,
				   &state->ఖండాలు[elem_idx][bfidx].use_map[0],
				   ఖండం_USE_MAP_కొలత);
			ఒకవేళ (wr != ఖండం_USE_MAP_కొలత) {
				NSLOG(netsurf, DEBUG,
				      "writing block file %d use index on file number %d failed",
				      elem_idx,
				      bfidx);
				వెళ్ళు wr_err;
			}
			written += wr;
		}
	}
wr_err:
	close(fd);

	/* check all data was written */
	ఒకవేళ (written != ఖండాలు_కొలత) {
		unlink(tname);
		విడుదల(tname);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, ఖండాలు_FNAME);
	ఒకవేళ (ret != NSERROR_OK) {
		unlink(tname);
		విడుదల(tname);
		తిరిగి ret;
	}

	/* remove() call is to handle non-POSIX rename() implementations */
	(శూన్య)remove(fname);
	ఒకవేళ (rename(tname, fname) != 0) {
		unlink(tname);
		విడుదల(tname);
		విడుదల(fname);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	తిరిగి NSERROR_OK;
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
స్థిర nserror సమితి_ఖండం_extents(నిర్మాణం store_state *state)
{
	పూర్ణాంకం bfidx; /* block file index */
	పూర్ణాంకం elem_idx;
	పూర్ణాంకం ftr;

	ఒకవేళ (state->ఖండాలు_opened == false) {
		/* no blocks have been opened since last write */
		తిరిగి NSERROR_OK;
	}

	NSLOG(netsurf, DEBUG, "Starting");
	కొరకు (elem_idx = 0; elem_idx < నమోదు_ELEM_లెక్క; elem_idx++) {
		కొరకు (bfidx = 0; bfidx < ఖండం_FILE_లెక్క; bfidx++) {
			ఒకవేళ (state->ఖండాలు[elem_idx][bfidx].fd != -1) {
				/* ensure block file is correct extent */
				ftr = ftruncate(state->ఖండాలు[elem_idx][bfidx].fd, 1U << (log2_ఖండం_కొలత[elem_idx] + ఖండం_నమోదు_లెక్క));
				ఒకవేళ (ftr == -1) {
					NSLOG(netsurf, ERROR,
					      "Truncate failed errno:%d",
					      errno);
				}
			}
		}
	}
	NSLOG(netsurf, DEBUG, "Complete");

	state->ఖండాలు_opened = false;

	తిరిగి NSERROR_OK;
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
స్థిర శూన్య control_maintenance(శూన్య *s)
{
	నిర్మాణం store_state *state = s;

	write_entries(state);
	write_ఖండాలు(state);
	సమితి_ఖండం_extents(state);
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
స్థిర nserror
get_store_నమోదు(నిర్మాణం store_state *state, nsurl *url, నిర్మాణం store_నమోదు **bse)
{
	నిర్మాణం store_నమోదు *ent;

	ent = hashmap_lookup(state->entries, url);

	ఒకవేళ (ent == ఖాళీ) {
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	*bse = ent;

	ent->చివరి_వాడిన = time(ఖాళీ);
	ent->use_లెక్క++;

	state->entries_dirty = true;

	guit->misc->కాలపట్టిక(CONTROL_MAINT_సమయం, control_maintenance, state);

	తిరిగి NSERROR_OK;
}

/**
 * Find next available small block.
 */
స్థిర ఖండం_సూచీ_t alloc_ఖండం(నిర్మాణం store_state *state, పూర్ణాంకం elem_idx)
{
	పూర్ణాంకం bf;
	పూర్ణాంకం idx;
	పూర్ణాంకం బిట్టు;
	uint8_t *map;

	కొరకు (bf = 0; bf < ఖండం_FILE_లెక్క; bf++) {
		map = &state->ఖండాలు[elem_idx][bf].use_map[0];

		కొరకు (idx = 0; idx < ఖండం_USE_MAP_కొలత; idx++) {
			ఒకవేళ (*(map + idx) != 0xff) {
				/* located an unused block */
				కొరకు (బిట్టు = 0; బిట్టు < 8;బిట్టు++) {
					ఒకవేళ (((*(map + idx)) & (1U << బిట్టు)) == 0) {
						/* mark block as used */
						*(map + idx) |= 1U << బిట్టు;
						state->ఖండాలు_dirty = true;
						తిరిగి (((bf * ఖండం_USE_MAP_కొలత) + idx) * 8) + బిట్టు;
					}
				}
			}
		}
	}

	తిరిగి 0;
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
స్థిర nserror
సమితి_store_నమోదు(నిర్మాణం store_state *state,
		nsurl *url,
		పూర్ణాంకం elem_idx,
		uint8_t *దత్తాంశం,
		స్థిరరాశి size_t datalen,
		నిర్మాణం store_నమోదు **bse)
{
	నిర్మాణం store_నమోదు *se;
	nserror ret;
	నిర్మాణం store_నమోదు_element *elem;

	NSLOG(netsurf, DEBUG, "url:%s", nsurl_access(url));

	/* evict entries as required and ensure there is at least one
	 * new entry available.
	 */
	ret = store_evict(state);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	se = hashmap_lookup(state->entries, url);
	ఒకవేళ (se == ఖాళీ) {
		se = hashmap_చొప్పించు(state->entries, url);
	}
	ఒకవేళ (se == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	/* the entry element */
	elem = &se->elem[elem_idx];

	/* check if the element has storage already allocated */
	ఒకవేళ ((elem->flags & (నమోదు_ELEM_FLAG_కుప్ప | నమోదు_ELEM_FLAG_MMAP)) != 0) {
		/* this entry cannot be removed as it has associated
		 * allocation.
		 */
		NSLOG(netsurf, ERROR,
		      "attempt to overwrite entry with in use data");
		తిరిగి NSERROR_PERMISSION;
	}

	/* set the common entry data */
	se->use_లెక్క = 1;
	se->చివరి_వాడిన = time(ఖాళీ);

	/* store the data in the element */
	elem->flags |= నమోదు_ELEM_FLAG_కుప్ప;
	elem->దత్తాంశం = దత్తాంశం;
	elem->ref = 1;

	/* account for size of entry element */
	state->మొత్తం_alloc -= elem->కొలత;
	elem->కొలత = datalen;
	state->మొత్తం_alloc += elem->కొలత;

	/* if the element will fit in a small block attempt to allocate one */
	ఒకవేళ (elem->కొలత <= (1U << log2_ఖండం_కొలత[elem_idx])) {
		elem->ఖండం = alloc_ఖండం(state, elem_idx);
	}

	/* ensure control maintenance scheduled. */
	state->entries_dirty = true;
	guit->misc->కాలపట్టిక(CONTROL_MAINT_సమయం, control_maintenance, state);

	*bse = se;

	తిరిగి NSERROR_OK;
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
స్థిర పూర్ణాంకం
store_open(నిర్మాణం store_state *state,
	   నమోదు_ident_t ident,
	   పూర్ణాంకం elem_idx,
	   పూర్ణాంకం openflags)
{
	అక్షరం *fname;
	nserror ret;
	పూర్ణాంకం fd;

	fname = store_fname(state, ident, elem_idx);
	ఒకవేళ (fname == ఖాళీ) {
		NSLOG(netsurf, ERROR, "filename error");
		తిరిగి -1;
	}

	/* ensure all path elements to file exist if creating file */
	ఒకవేళ (openflags & O_CREAT) {
		ret = netsurf_mkdir_all(fname);
		ఒకవేళ (ret != NSERROR_OK) {
			NSLOG(netsurf, WARNING,
			      "file path \"%s\" could not be created", fname);
			విడుదల(fname);
			తిరిగి -1;
		}
	}

	NSLOG(netsurf, DEBUG, "opening %s", fname);
	fd = open(fname, openflags, S_IRUSR | S_IWUSR);

	విడుదల(fname);

	తిరిగి fd;
}


/**
 * Unlink entries file
 *
 * @param state The backing store state.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
unlink_entries(నిర్మాణం store_state *state)
{
	అక్షరం *fname = ఖాళీ;
	nserror ret;

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, ENTRIES_FNAME);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	unlink(fname);

	విడుదల(fname);
	తిరిగి NSERROR_OK;
}

/**
 * Read description entries into memory.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on faliure.
 */
స్థిర nserror
read_entries(నిర్మాణం store_state *state)
{
	అక్షరం *fname = ఖాళీ;
	అక్షరం *url;
	nsurl *nsurl;
	nserror ret;
	size_t read_entries = 0;
	నిర్మాణం store_నమోదు *ent;
	పూర్ణాంకం fd;

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, ENTRIES_FNAME);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	state->entries = hashmap_create(&entries_hashmap_parameters);
	ఒకవేళ (state->entries == ఖాళీ) {
		విడుదల(fname);
		తిరిగి NSERROR_NOMEM;
	}

	fd = open(fname, O_RDWR);
	ఒకవేళ (fd != -1) {
		uint32_t urllen;
		అయితే (పఠ(fd, &urllen, పరిమాణం(urllen)) == పరిమాణం(urllen)) {
			url = సున్నా_జ్ఞాపకం_ఇవ్వు(1, urllen+1);
			ఒకవేళ (url == ఖాళీ) {
				close(fd);
				విడుదల(fname);
				తిరిగి NSERROR_NOMEM;
			}
			ఒకవేళ (పఠ(fd, url, urllen) != (ssize_t)urllen) {
				విడుదల(url);
				close(fd);
				విడుదల(fname);
				తిరిగి NSERROR_INIT_FAILED;
			}
			ret = nsurl_create(url, &nsurl);
			ఒకవేళ (ret != NSERROR_OK) {
				విడుదల(url);
				close(fd);
				విడుదల(fname);
				తిరిగి ret;
			}
			విడుదల(url);
			/* We have to be careful here about nsurl refs */
			ent = hashmap_చొప్పించు(state->entries, nsurl);
			ఒకవేళ (ent == ఖాళీ) {
				nsurl_unref(nsurl);
				close(fd);
				విడుదల(fname);
				తిరిగి NSERROR_NOMEM;
			}
			/* At this point, ent actually owns a ref of nsurl */
			ఒకవేళ (పఠ(fd, ent, పరిమాణం(*ent)) != పరిమాణం(*ent)) {
				/* The read failed, so reset the ptr */
				ent->url = nsurl; /* It already had a ref */
				nsurl_unref(nsurl);
				close(fd);
				విడుదల(fname);
				తిరిగి NSERROR_INIT_FAILED;
			}
			ent->url = nsurl; /* It already owns a ref */
			nsurl_unref(nsurl);
			NSLOG(netsurf, DEBUG, "Successfully read entry for %s", nsurl_access(ent->url));
			read_entries++;
			/* Note the size allocation */
			state->మొత్తం_alloc += ent->elem[నమోదు_ELEM_దత్తాంశం].కొలత;
			state->మొత్తం_alloc += ent->elem[నమోదు_ELEM_META].కొలత;
			/* And ensure we don't pretend to have this in memory yet */
			ent->elem[నమోదు_ELEM_దత్తాంశం].flags &= ~(నమోదు_ELEM_FLAG_కుప్ప | నమోదు_ELEM_FLAG_MMAP);
			ent->elem[నమోదు_ELEM_META].flags &= ~(నమోదు_ELEM_FLAG_కుప్ప | నమోదు_ELEM_FLAG_MMAP);

		}
		close(fd);
	}

	NSLOG(netsurf, INFO, "Read %"PRIsizet" entries from cache", read_entries);

	విడుదల(fname);
	తిరిగి NSERROR_OK;
}


/**
 * Read block file usage bitmaps.
 *
 * @param state The backing store state to put the loaded entries in.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
read_ఖండాలు(నిర్మాణం store_state *state)
{
	పూర్ణాంకం bfidx; /* block file index */
	పూర్ణాంకం elem_idx;
	పూర్ణాంకం fd;
	ssize_t rd;
	అక్షరం *fname = ఖాళీ;
	nserror ret;

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, ఖండాలు_FNAME);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	NSLOG(netsurf, INFO, "Initialising block use map from %s", fname);

	fd = open(fname, O_RDWR);
	విడుదల(fname);
	ఒకవేళ (fd != -1) {
		/* initialise block file use array */
		కొరకు (elem_idx = 0; elem_idx < నమోదు_ELEM_లెక్క; elem_idx++) {
			కొరకు (bfidx = 0; bfidx < ఖండం_FILE_లెక్క; bfidx++) {
				rd = పఠ(fd,
					  &state->ఖండాలు[elem_idx][bfidx].use_map[0],
					  ఖండం_USE_MAP_కొలత);
				ఒకవేళ (rd <= 0) {
					NSLOG(netsurf, ERROR,
					      "reading block file %d use index on file number %d failed",
					      elem_idx,
					      bfidx);
					వెళ్ళు rd_err;
				}
			}
		}
	rd_err:
		close(fd);

	} లేకపోతే {
		NSLOG(netsurf, INFO, "Initialising block use map to defaults");
		/* ensure block 0 (invalid sentinel) is skipped */
		state->ఖండాలు[నమోదు_ELEM_దత్తాంశం][0].use_map[0] = 1;
		state->ఖండాలు[నమోదు_ELEM_META][0].use_map[0] = 1;
	}

	/* initialise block file file descriptors */
	కొరకు (bfidx = 0; bfidx < ఖండం_FILE_లెక్క; bfidx++) {
		state->ఖండాలు[నమోదు_ELEM_దత్తాంశం][bfidx].fd = -1;
		state->ఖండాలు[నమోదు_ELEM_META][bfidx].fd = -1;
	}

	తిరిగి NSERROR_OK;
}

/**
 * Write the cache tag file.
 *
 * @param state The cache state.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
write_cache_ట్యాగ్(నిర్మాణం store_state *state)
{
	FILE *fcachetag;
	nserror ret;
	అక్షరం *fname = ఖాళీ;

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, "CACHEDIR.TAG");
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	fcachetag = fopen(fname, "wb");

	విడుదల(fname);

	ఒకవేళ (fcachetag == ఖాళీ) {
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	fprintf(fcachetag,
		"Signature: 8a477f597d28d172789f06886806bc55\n"
		"# This file is a cache directory tag created by NetSurf.\n"
		"# For information about cache directory tags, see:\n"
		"#	http://www.brynosaurus.com/cachedir/\n");

	fclose(fcachetag);

	తిరిగి NSERROR_OK;
}

/**
 * Write the control file for the current state.
 *
 * @param state The state to write to the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
write_control(నిర్మాణం store_state *state)
{
	FILE *fcontrol;
	nserror ret;
	అక్షరం *fname = ఖాళీ;

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, "control");
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	NSLOG(netsurf, INFO, "writing control file \"%s\"", fname);

	ret = netsurf_mkdir_all(fname);
	ఒకవేళ (ret != NSERROR_OK) {
		విడుదల(fname);
		తిరిగి ret;
	}

	fcontrol = fopen(fname, "wb");

	విడుదల(fname);

	ఒకవేళ (fcontrol == ఖాళీ) {
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	fprintf(fcontrol, "%u%c", CONTROL_సంస్కరణ, 0);

	fclose(fcontrol);

	తిరిగి NSERROR_OK;
}


/**
 * Read and parse the control file.
 *
 * @param state The state to read from the control file.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
read_control(నిర్మాణం store_state *state)
{
	nserror ret;
	FILE *fcontrol;
	గుర్తులేని పూర్ణాంకం ctrlversion;
	అక్షరం *fname = ఖాళీ;

	ret = netsurf_mkpath(&fname, ఖాళీ, 2, state->మార్గం, "control");
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	NSLOG(netsurf, INFO, "opening control file \"%s\"", fname);

	fcontrol = fopen(fname, "rb");

	విడుదల(fname);

	ఒకవేళ (fcontrol == ఖాళీ) {
		/* unable to open control file */
		ఒకవేళ (errno == ENOENT) {
			తిరిగి NSERROR_NOT_దొరికింది;
		} లేకపోతే {
			తిరిగి NSERROR_INIT_FAILED;
		}
	}

	/* read control and setup new state */

	/* first line is version */
	ఒకవేళ (fscanf(fcontrol, "%u", &ctrlversion) != 1) {
		వెళ్ళు control_error;
	}

	ఒకవేళ (ctrlversion != CONTROL_సంస్కరణ) {
		వెళ్ళు control_error;
	}

	ఒకవేళ (fgetc(fcontrol) != 0) {
		వెళ్ళు control_error;
	}

	fclose(fcontrol);

	తిరిగి NSERROR_OK;

control_error: /* problem with the control file */

	fclose(fcontrol);

	తిరిగి NSERROR_INIT_FAILED;
}




/* Functions exported in the backing store table */

/**
 * Initialise the backing store.
 *
 * @param parameters to configure backing store.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror
initialise(స్థిరరాశి నిర్మాణం llcache_store_parameters *parameters)
{
	నిర్మాణం store_state *newstate;
	nserror ret;

	/* check backing store is not already initialised */
	ఒకవేళ (storestate != ఖాళీ) {
		తిరిగి NSERROR_INIT_FAILED;
	}

	/* if we are not allowed any space simply give up on init */
	ఒకవేళ (parameters->హద్దు == 0) {
		తిరిగి NSERROR_OK;
	}

	/* if the path to the cache directory is not set do not init */
	ఒకవేళ (parameters->మార్గం == ఖాళీ) {
		తిరిగి NSERROR_OK;
	}

	/* allocate new store state and set defaults */
	newstate = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం store_state));
	ఒకవేళ (newstate == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	newstate->మార్గం = strdup(parameters->మార్గం);
	newstate->హద్దు = parameters->హద్దు;
	newstate->hysteresis = parameters->hysteresis;

	/* read store control and create new if required */
	ret = read_control(newstate);
	ఒకవేళ (ret != NSERROR_OK) {
		ఒకవేళ (ret == NSERROR_NOT_దొరికింది) {
			NSLOG(netsurf, INFO, "cache control file not found, making fresh");
		} లేకపోతే {
			NSLOG(netsurf, ERROR, "read control failed %s",
			      messages_get_errorcode(ret));
			ret = netsurf_recursive_rm(newstate->మార్గం);
			ఒకవేళ (ret != NSERROR_OK) {
				NSLOG(netsurf, WARNING, "Error `%s` while removing `%s`",
				      messages_get_errorcode(ret), newstate->మార్గం);
				NSLOG(netsurf, WARNING, "Unable to clean up partial cache state.");
				NSLOG(netsurf, WARNING, "Funky behaviour may ensue.");
			} లేకపోతే {
				NSLOG(netsurf, INFO, "Successfully removed old cache from `%s`",
				      newstate->మార్గం);
			}
		}
		ret = write_control(newstate);
		ఒకవేళ (ret == NSERROR_OK) {
			unlink_entries(newstate);
			write_cache_ట్యాగ్(newstate);
		}
	}
	ఒకవేళ (ret != NSERROR_OK) {
		/* that went well obviously */
		విడుదల(newstate->మార్గం);
		విడుదల(newstate);
		తిరిగి ret;
	}

	/* read filesystem entries */
	ret = read_entries(newstate);
	ఒకవేళ (ret != NSERROR_OK) {
		/* that went well obviously */
		విడుదల(newstate->మార్గం);
		విడుదల(newstate);
		తిరిగి ret;
	}

	/* read blocks */
	ret = read_ఖండాలు(newstate);
	ఒకవేళ (ret != NSERROR_OK) {
		/* oh dear */
		hashmap_destroy(newstate->entries);
		విడుదల(newstate->మార్గం);
		విడుదల(newstate);
		తిరిగి ret;
	}

	storestate = newstate;

	NSLOG(netsurf, INFO, "FS backing store init successful");

	NSLOG(netsurf, INFO,
	      "path:%s limit:%"PRIsizet" hyst:%"PRIsizet,
	      newstate->మార్గం,
	      newstate->హద్దు,
	      newstate->hysteresis);
	NSLOG(netsurf, INFO, "Using %"PRIu64"/%"PRIsizet,
	      newstate->మొత్తం_alloc, newstate->హద్దు);

	తిరిగి NSERROR_OK;
}


/**
 * Finalise the backing store.
 *
 * \todo This will cause the backing store to leak any outstanding memory
 * allocations. This will probably best be done by a global use count.
 *
 * @return NSERROR_OK on success.
 */
స్థిర nserror
finalise(శూన్య)
{
	పూర్ణాంకం bf; /* block file index */
	గుర్తులేని పూర్ణాంకం op_లెక్క;

	ఒకవేళ (storestate != ఖాళీ) {
		guit->misc->కాలపట్టిక(-1, control_maintenance, storestate);
		write_entries(storestate);
		write_ఖండాలు(storestate);

		/* ensure all block files are closed */
		కొరకు (bf = 0; bf < ఖండం_FILE_లెక్క; bf++) {
			ఒకవేళ (storestate->ఖండాలు[నమోదు_ELEM_దత్తాంశం][bf].fd != -1) {
				close(storestate->ఖండాలు[నమోదు_ELEM_దత్తాంశం][bf].fd);
			}
			ఒకవేళ (storestate->ఖండాలు[నమోదు_ELEM_META][bf].fd != -1) {
				close(storestate->ఖండాలు[నమోదు_ELEM_META][bf].fd);
			}
		}

		op_లెక్క = storestate->తగిలింది_లెక్క + storestate->తప్పిపోవు_లెక్క;

		/* avoid division by zero */
		ఒకవేళ (op_లెక్క > 0) {
			NSLOG(netsurf, INFO,
			      "Cache total/hit/miss/fail (counts) %d/%"PRIsizet"/%"PRIsizet"/%d (100%%/%"PRIsizet"%%/%"PRIsizet"%%/%d%%)",
			      op_లెక్క,
			      storestate->తగిలింది_లెక్క,
			      storestate->తప్పిపోవు_లెక్క,
			      0,
			      (storestate->తగిలింది_లెక్క * 100) / op_లెక్క,
			      (storestate->తప్పిపోవు_లెక్క * 100) / op_లెక్క,
			      0);
		}

		hashmap_destroy(storestate->entries);
		విడుదల(storestate->మార్గం);
		విడుదల(storestate);
		storestate = ఖాళీ;
	}
	తిరిగి NSERROR_OK;
}


/**
 * Write an element of an entry to backing storage in a small block file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
స్థిర nserror store_write_ఖండం(నిర్మాణం store_state *state,
			 నిర్మాణం store_నమోదు *bse,
			 పూర్ణాంకం elem_idx)
{
	ఖండం_సూచీ_t bf = (bse->elem[elem_idx].ఖండం >> ఖండం_నమోదు_లెక్క) &
		((1 << ఖండం_FILE_లెక్క) - 1); /* block file block resides in */
	ఖండం_సూచీ_t bi = bse->elem[elem_idx].ఖండం & ((1U << ఖండం_నమోదు_లెక్క) -1); /* block index in file */
	ssize_t wr;
	off_t offst;

	/* ensure the block file fd is good */
	ఒకవేళ (state->ఖండాలు[elem_idx][bf].fd == -1) {
		state->ఖండాలు[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + నమోదు_ELEM_లెక్క, O_CREAT | O_RDWR);
		ఒకవేళ (state->ఖండాలు[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			తిరిగి NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->ఖండాలు_opened = true;
	}

	offst = (గుర్తులేని పూర్ణాంకం)bi << log2_ఖండం_కొలత[elem_idx];

	wr = nsu_pwrite(state->ఖండాలు[elem_idx][bf].fd,
			bse->elem[elem_idx].దత్తాంశం,
			bse->elem[elem_idx].కొలత,
			offst);
	ఒకవేళ (wr != (ssize_t)bse->elem[elem_idx].కొలత) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p at %"PRIsizet" block %"PRIu16" errno %d",
		      wr,
		      bse->elem[elem_idx].కొలత,
		      bse->elem[elem_idx].దత్తాంశం,
		      (size_t)offst,
		      bse->elem[elem_idx].ఖండం,
		      errno);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, INFO,
	      "Wrote %"PRIssizet" bytes from %p at %"PRIsizet" block %d", wr,
	      bse->elem[elem_idx].దత్తాంశం, (size_t)offst,
	      bse->elem[elem_idx].ఖండం);

	తిరిగి NSERROR_OK;
}

/**
 * Write an element of an entry to backing storage as an individual file.
 *
 * \param state The backing store state to use.
 * \param bse The entry to store
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
స్థిర nserror store_write_file(నిర్మాణం store_state *state,
			 నిర్మాణం store_నమోదు *bse,
			 పూర్ణాంకం elem_idx)
{
	ssize_t wr;
	పూర్ణాంకం fd;
	పూర్ణాంకం err;

	fd = store_open(state, nsurl_hash(bse->url), elem_idx, O_CREAT | O_WRONLY);
	ఒకవేళ (fd < 0) {
		perror("");
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	wr = లిఖ(fd, bse->elem[elem_idx].దత్తాంశం, bse->elem[elem_idx].కొలత);
	err = errno; /* close can change errno */

	close(fd);
	ఒకవేళ (wr != (ssize_t)bse->elem[elem_idx].కొలత) {
		NSLOG(netsurf, ERROR,
		      "Write failed %"PRIssizet" of %"PRId32" bytes from %p errno %d",
		      wr,
		      bse->elem[elem_idx].కొలత,
		      bse->elem[elem_idx].దత్తాంశం,
		      err);

		/** @todo Delete the file? */
		తిరిగి NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, VERBOSE, "Wrote %"PRIssizet" bytes from %p", wr,
	      bse->elem[elem_idx].దత్తాంశం);

	తిరిగి NSERROR_OK;
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
స్థిర nserror
store(nsurl *url,
      గణన backing_store_flags bsflags,
      uint8_t *దత్తాంశం,
      స్థిరరాశి size_t datalen)
{
	nserror ret;
	నిర్మాణం store_నమోదు *bse;
	పూర్ణాంకం elem_idx;

	/* check backing store is initialised */
	ఒకవేళ (storestate == ఖాళీ) {
		తిరిగి NSERROR_INIT_FAILED;
	}

	/* calculate the entry element index */
	ఒకవేళ ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = నమోదు_ELEM_META;
	} లేకపోతే {
		elem_idx = నమోదు_ELEM_దత్తాంశం;
	}

	/* set the store entry up */
	ret = సమితి_store_నమోదు(storestate, url, elem_idx, దత్తాంశం, datalen, &bse);
	ఒకవేళ (ret != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "store entry setting failed");
		తిరిగి ret;
	}

	ఒకవేళ (bse->elem[elem_idx].ఖండం != 0) {
		/* small block storage */
		ret = store_write_ఖండం(storestate, bse, elem_idx);
	} లేకపోతే {
		/* separate file in backing store */
		ret = store_write_file(storestate, bse, elem_idx);
	}

	తిరిగి ret;
}

/**
 * release any allocation for an entry
 */
స్థిర nserror నమోదు_release_alloc(నిర్మాణం store_నమోదు_element *elem)
{
	ఒకవేళ ((elem->flags & నమోదు_ELEM_FLAG_కుప్ప) != 0) {
		elem->ref--;
		ఒకవేళ (elem->ref == 0) {
			NSLOG(netsurf, DEEPDEBUG, "freeing %p", elem->దత్తాంశం);
			విడుదల(elem->దత్తాంశం);
			elem->flags &= ~నమోదు_ELEM_FLAG_కుప్ప;
		}
	}
	తిరిగి NSERROR_OK;
}


/**
 * Read an element of an entry from a small block file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
స్థిర nserror store_read_ఖండం(నిర్మాణం store_state *state,
			 నిర్మాణం store_నమోదు *bse,
			 పూర్ణాంకం elem_idx)
{
	ఖండం_సూచీ_t bf = (bse->elem[elem_idx].ఖండం >> ఖండం_నమోదు_లెక్క) &
		((1 << ఖండం_FILE_లెక్క) - 1); /* block file block resides in */
	ఖండం_సూచీ_t bi = bse->elem[elem_idx].ఖండం & ((1 << ఖండం_నమోదు_లెక్క) -1); /* block index in file */
	ssize_t rd;
	off_t offst;

	/* ensure the block file fd is good */
	ఒకవేళ (state->ఖండాలు[elem_idx][bf].fd == -1) {
		state->ఖండాలు[elem_idx][bf].fd = store_open(state, bf,
				elem_idx + నమోదు_ELEM_లెక్క, O_CREAT | O_RDWR);
		ఒకవేళ (state->ఖండాలు[elem_idx][bf].fd == -1) {
			NSLOG(netsurf, ERROR, "Open failed errno %d", errno);
			తిరిగి NSERROR_SAVE_FAILED;
		}

		/* flag that a block file has been opened */
		state->ఖండాలు_opened = true;
	}

	offst = (గుర్తులేని పూర్ణాంకం)bi << log2_ఖండం_కొలత[elem_idx];

	rd = nsu_pread(state->ఖండాలు[elem_idx][bf].fd,
		       bse->elem[elem_idx].దత్తాంశం,
		       bse->elem[elem_idx].కొలత,
		       offst);
	ఒకవేళ (rd != (ssize_t)bse->elem[elem_idx].కొలత) {
		NSLOG(netsurf, ERROR,
		      "Failed reading %"PRIssizet" of %"PRId32" bytes into %p from %"PRIsizet" block %"PRIu16" errno %d",
		      rd,
		      bse->elem[elem_idx].కొలత,
		      bse->elem[elem_idx].దత్తాంశం,
		      (size_t)offst,
		      bse->elem[elem_idx].ఖండం,
		      errno);
		తిరిగి NSERROR_SAVE_FAILED;
	}

	NSLOG(netsurf, DEEPDEBUG,
	      "Read %"PRIssizet" bytes into %p from %"PRIsizet" block %d", rd,
	      bse->elem[elem_idx].దత్తాంశం, (size_t)offst,
	      bse->elem[elem_idx].ఖండం);

	తిరిగి NSERROR_OK;
}

/**
 * Read an element of an entry from an individual file in the backing storage.
 *
 * \param state The backing store state to use.
 * \param bse The entry to read.
 * \param elem_idx The element index within the entry.
 * \return NSERROR_OK on success or error code.
 */
స్థిర nserror store_read_file(నిర్మాణం store_state *state,
			 నిర్మాణం store_నమోదు *bse,
			 పూర్ణాంకం elem_idx)
{
	పూర్ణాంకం fd;
	ssize_t rd; /* return from read */
	పూర్ణాంకం ret = NSERROR_OK;
	size_t tot = 0; /* total size */

	/* separate file in backing store */
	fd = store_open(storestate, nsurl_hash(bse->url), elem_idx, O_RDONLY);
	ఒకవేళ (fd < 0) {
		NSLOG(netsurf, ERROR, "Open failed %d errno %d", fd, errno);
		/** @todo should this invalidate the entry? */
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	అయితే (tot < bse->elem[elem_idx].కొలత) {
		rd = పఠ(fd,
			  bse->elem[elem_idx].దత్తాంశం + tot,
			  bse->elem[elem_idx].కొలత - tot);
		ఒకవేళ (rd <= 0) {
			NSLOG(netsurf, ERROR,
			      "read error returned %"PRIssizet" errno %d",
			      rd,
			      errno);
			ret = NSERROR_NOT_దొరికింది;
			ఆపు;
		}
		tot += rd;
	}

	close(fd);

	NSLOG(netsurf, DEEPDEBUG, "Read %"PRIsizet" bytes into %p", tot,
	      bse->elem[elem_idx].దత్తాంశం);

	తిరిగి ret;
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
స్థిర nserror
fetch(nsurl *url,
      గణన backing_store_flags bsflags,
      uint8_t **దత్తాంశం_బహిర్గతం,
      size_t *datalen_బహిర్గతం)
{
	nserror ret;
	నిర్మాణం store_నమోదు *bse;
	నిర్మాణం store_నమోదు_element *elem;
	పూర్ణాంకం elem_idx;

	/* check backing store is initialised */
	ఒకవేళ (storestate == ఖాళీ) {
		తిరిగి NSERROR_INIT_FAILED;
	}

	/* fetch store entry */
	ret = get_store_నమోదు(storestate, url, &bse);
	ఒకవేళ (ret != NSERROR_OK) {
		NSLOG(netsurf, DEBUG, "Entry for %s not found", nsurl_access(url));
		storestate->తప్పిపోవు_లెక్క++;
		తిరిగి ret;
	}
	storestate->తగిలింది_లెక్క++;

	NSLOG(netsurf, DEBUG, "retrieving cache data for url:%s",
	      nsurl_access(url));

	/* calculate the entry element index */
	ఒకవేళ ((bsflags & BACKING_STORE_META) != 0) {
		elem_idx = నమోదు_ELEM_META;
	} లేకపోతే {
		elem_idx = నమోదు_ELEM_దత్తాంశం;
	}
	elem = &bse->elem[elem_idx];

	/* if an allocation already exists return it */
	ఒకవేళ ((elem->flags & నమోదు_ELEM_FLAG_కుప్ప) != 0) {
		/* use the existing allocation and bump the ref count. */
		elem->ref++;

		NSLOG(netsurf, DEEPDEBUG,
		      "Using existing entry (%p) allocation %p refs:%d", bse,
		      elem->దత్తాంశం, elem->ref);

	} లేకపోతే {
		/* allocate from the heap */
		elem->దత్తాంశం = జ్ఞాపకం_ఇవ్వు(elem->కొలత);
		ఒకవేళ (elem->దత్తాంశం == ఖాళీ) {
			NSLOG(netsurf, ERROR,
			      "Failed to create new heap allocation");
			తిరిగి NSERROR_NOMEM;
		}
		NSLOG(netsurf, DEEPDEBUG, "Created new heap allocation %p",
		      elem->దత్తాంశం);

		/* mark the entry as having a valid heap allocation */
		elem->flags |= నమోదు_ELEM_FLAG_కుప్ప;
		elem->ref = 1;

		/* fill the new block */
		ఒకవేళ (elem->ఖండం != 0) {
			ret = store_read_ఖండం(storestate, bse, elem_idx);
		} లేకపోతే {
			ret = store_read_file(storestate, bse, elem_idx);
		}
	}

	/* free the allocation if there is a read error */
	ఒకవేళ (ret != NSERROR_OK) {
		నమోదు_release_alloc(elem);
	} లేకపోతే {
		/* update stats and setup return pointers */
		storestate->తగిలింది_కొలత += elem->కొలత;

		*దత్తాంశం_బహిర్గతం = elem->దత్తాంశం;
		*datalen_బహిర్గతం = elem->కొలత;
	}

	తిరిగి ret;
}


/**
 * release a previously fetched or stored memory object.
 *
 * @param[in] url The url is used as the unique primary key to invalidate.
 * @param[in] bsflags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on failure.
 */
స్థిర nserror release(nsurl *url, గణన backing_store_flags bsflags)
{
	nserror ret;
	నిర్మాణం store_నమోదు *bse;
	నిర్మాణం store_నమోదు_element *elem;

	/* check backing store is initialised */
	ఒకవేళ (storestate == ఖాళీ) {
		తిరిగి NSERROR_INIT_FAILED;
	}

	ret = get_store_నమోదు(storestate, url, &bse);
	ఒకవేళ (ret != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "entry not found");
		తిరిగి ret;
	}

	/* the entry element */
	ఒకవేళ ((bsflags & BACKING_STORE_META) != 0) {
		elem = &bse->elem[నమోదు_ELEM_META];
	} లేకపోతే {
		elem = &bse->elem[నమోదు_ELEM_దత్తాంశం];
	}

	ret = నమోదు_release_alloc(elem);

	/* if the entry has previously been invalidated but had
	 * allocation it must be invalidated fully now the allocation
	 * has been released.
	 */
	ఒకవేళ ((ret == NSERROR_OK) &&
	    ((bse->flags & నమోదు_FLAGS_INVALID) != 0)) {
		ret = invalidate_నమోదు(storestate, bse);
	}

	తిరిగి ret;
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
స్థిర nserror
invalidate(nsurl *url)
{
	nserror ret;
	నిర్మాణం store_నమోదు *bse;

	/* check backing store is initialised */
	ఒకవేళ (storestate == ఖాళీ) {
		తిరిగి NSERROR_INIT_FAILED;
	}

	ret = get_store_నమోదు(storestate, url, &bse);
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	తిరిగి invalidate_నమోదు(storestate, bse);
}


స్థిర నిర్మాణం gui_llcache_పట్టిక llcache_పట్టిక = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

నిర్మాణం gui_llcache_పట్టిక *filesystem_llcache_పట్టిక = &llcache_పట్టిక;
