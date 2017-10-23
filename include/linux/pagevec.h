/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/linux/pagevec.h
 *
 * In many places it is efficient to batch an operation up against multiple
 * pages.  A pagevec is a multipage container which is used for that.
 */

#ifndef _LINUX_PAGEVEC_H
#define _LINUX_PAGEVEC_H

/* 15 pointers + header align the pagevec structure to a power of two */
#define PAGEVEC_SIZE	15

struct page;
struct address_space;

struct pagevec {
	unsigned char nr;
	bool percpu_pvec_drained;
	struct page *pages[PAGEVEC_SIZE];
};

struct pagevec_large_header {
	unsigned long nr;
	spinlock_t lock;
} ____cacheline_aligned;

#define PAGEVEC_LARGE_SIZE_BYTES (PAGE_SIZE - sizeof(struct pagevec_large_header))
#define PAGEVEC_LARGE_SIZE (PAGEVEC_LARGE_SIZE_BYTES / sizeof(struct page *))

struct pagevec_large {
	struct pagevec_large_header header;
	struct page *pages[PAGEVEC_LARGE_SIZE];
};

void __pagevec_release(struct pagevec *pvec);
void __pagevec_lru_add(struct pagevec *pvec);
unsigned pagevec_lookup_entries(struct pagevec *pvec,
				struct address_space *mapping,
				pgoff_t start, unsigned nr_entries,
				pgoff_t *indices);
void pagevec_remove_exceptionals(struct pagevec *pvec);
unsigned pagevec_lookup_range(struct pagevec *pvec,
			      struct address_space *mapping,
			      pgoff_t *start, pgoff_t end);
static inline unsigned pagevec_lookup(struct pagevec *pvec,
				      struct address_space *mapping,
				      pgoff_t *start)
{
	return pagevec_lookup_range(pvec, mapping, start, (pgoff_t)-1);
}

unsigned pagevec_lookup_range_tag(struct pagevec *pvec,
		struct address_space *mapping, pgoff_t *index, pgoff_t end,
		int tag);
unsigned pagevec_lookup_range_nr_tag(struct pagevec *pvec,
		struct address_space *mapping, pgoff_t *index, pgoff_t end,
		int tag, unsigned max_pages);
static inline unsigned pagevec_lookup_tag(struct pagevec *pvec,
		struct address_space *mapping, pgoff_t *index, int tag)
{
	return pagevec_lookup_range_tag(pvec, mapping, index, (pgoff_t)-1, tag);
}

static inline void pagevec_init(struct pagevec *pvec)
{
	pvec->nr = 0;
	pvec->percpu_pvec_drained = false;
}

static inline void pagevec_reinit(struct pagevec *pvec)
{
	pvec->nr = 0;
}

static inline unsigned pagevec_count(struct pagevec *pvec)
{
	return pvec->nr;
}

static inline unsigned pagevec_space(struct pagevec *pvec)
{
	return PAGEVEC_SIZE - pvec->nr;
}

/*
 * Add a page to a pagevec.  Returns the number of slots still available.
 */
static inline unsigned pagevec_add(struct pagevec *pvec, struct page *page)
{
	pvec->pages[pvec->nr++] = page;
	return pagevec_space(pvec);
}

static inline void pagevec_release(struct pagevec *pvec)
{
	if (pagevec_count(pvec))
		__pagevec_release(pvec);
}

static inline void pagevec_large_init(struct pagevec_large *pvec)
{
	pvec->header.nr = 0;
	spin_lock_init(&pvec->header.lock);
}

static inline unsigned pagevec_large_space(struct pagevec_large *pvec)
{
	return PAGEVEC_LARGE_SIZE - pvec->header.nr;
}

static inline unsigned pagevec_large_add(struct pagevec_large *pvec,
					 struct page *page)
{
	pvec->pages[pvec->header.nr++] = page;
	return pagevec_large_space(pvec);
}

static inline struct page *pagevec_large_sub(struct pagevec_large *pvec)
{
	if (!pvec->header.nr)
		return NULL;

	return pvec->pages[--pvec->header.nr];
}
#endif /* _LINUX_PAGEVEC_H */
