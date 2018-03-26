#include "xlib.h"
#include "dlrcache.h"

// Remove 'p' from the ring, setting 'ph' to the next one, or 0 if now empty
void  DLR_CACHE::dlrsnap(DLR *p)
{
if	(ph)
  	{
  	if (ph==(CTL*)p && (ph=(CTL*)(p->nxt))==(CTL*)p) ph=0; // p is the only elm in ring
  	p->nxt->prv=p->prv;
	p->prv->nxt=p->nxt;
	p->nxt=p->prv=p;
   }
}

// Link new element 'p' into the ring and make it the head
void  DLR_CACHE::dlrlink(DLR *p)
{
p->nxt=p->prv=p;	// in case there is no existing 'ring'
if	(ph)
	{
	(ph->dlr.prv->nxt=p)->nxt=(DLR*)ph;
	p->prv=ph->dlr.prv;
	ph->dlr.prv=p;
	}
ph=(CTL*)p;
}

// Create an instance of a DLR cacheing ring
DLR_CACHE::DLR_CACHE(PFI_v_v _cmp, long _max_sz)
{cmp=_cmp; ph=0; tot_sz=0; user_rls=0; max_sz=_max_sz;};

DLR_CACHE::~DLR_CACHE()
{while (ph) rls_oldest();};

// Return ptr to the 'user data' part of an item in the cache if present,
// or NULLPTR if it's not found (according to comparator function 'cmp'
// specified to the cache constructor). If the item IS found, it becomes
// the new DLR 'Head' - making a pretty spiffy LRU alogorithm.
void* DLR_CACHE::get(void *key)
{
CTL	*pi=ph;
if (pi)
	do {
	   if (!(cmp)(key,&pi[1]))
			{
			if	(pi!=ph) {dlrsnap((DLR*)pi); dlrlink((DLR*)pi);}
			return(&pi[1]);
			}
	   } while ((pi=(CTL*)pi->dlr.nxt) != ph);
return(0);
}

// Set pointer to caller's function to release a user data area if we want to drop it from the cache
void  DLR_CACHE::set_user_rls(PFV_v user_rls_function)
{user_rls=user_rls_function;}

// Discard the oldest item in the cache
void  DLR_CACHE::rls_oldest(void)
{
CTL *pi=(CTL*)(ph->dlr.prv);
dlrsnap((DLR*)pi);
tot_sz-=pi->sz;
if (user_rls) (user_rls)(&pi[1]);
memtake(pi);
}

// Put a new item into the cache at the Head
// Return a pointer to the 'user data' area of the cached item
void* DLR_CACHE::add_item(ushort sz)
{
while (tot_sz && tot_sz+sz>max_sz) rls_oldest();
CTL *pi=(CTL*)memgive(sizeof(CTL)+sz);
tot_sz+=(pi->sz=sz);
dlrlink((DLR*)pi);
return(&pi[1]);
}
