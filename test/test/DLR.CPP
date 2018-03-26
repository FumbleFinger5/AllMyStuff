#include "xlib.h"
#include "dlr.h"

short Dlr::add(void *data)
{
DLRid	*i=h;
short	id=1;
if (h)
	do	{
		i=h;
		do {if (i->id==id) break;} while ((i=i->nxt) != h);
		} while (i->id==id && ++id);
i=(DLRid*)memgive(sz+10);
i->id=id;
memmove(((char*)i)+10,data,sz);
dlrlink(i);
return(id);
}

DLRid* Dlr::scan(int *again)		// returns 0 if it's been thru the whole list
{
static DLRid *i;
if (*again)
	{
	if ((i=i->nxt)==h)
		i=0;
	}
else
	{
	i=h;
	*again=YES;
	}
return(i);
}

void Dlr::top(DLRid *item)		// 'item' MUST already be in the list!
{
if	(item!=h) {dlrsnap(item); dlrlink(item);}	// make it the new ring head
}

void* Dlr::get(short id)
{					// Find item 'id', or first item if id==0
DLRid	*i=h;		// Return pointer to userdata within selected item
if (i)
	do {
	   if (i->id==id || !id) {top(i); return(&i[1]);}	// return 'userdata' address
	   } while ((i=i->nxt) != h);
return(0);
}

void Dlr::del(short id)			// Delete item 'id' if it exists
{										// (something's gone wrong if it doesn't exist)
if (!get(id)) throw SE_MEMDLR;
DLRid *a=h;
dlrsnap(h);
memtake(a);
}

void Dlr::dlrsnap(DLRid *p)
{			// Remove 'p' from ring, setting 'ph' to next one, or 0 if now empty
if (h==p && (h=p->nxt)==p) h=0; // p is the only elm in ring
p->nxt->prv=p->prv; p->prv->nxt=p->nxt; p->nxt=p->prv=p;
}

void Dlr::dlrlink(DLRid *p)
{			// Link new element 'p' into the ring and make it the head
p->nxt=p->prv=p;	// (in case there's no existing ring)
if	(h) {(h->prv->nxt=p)->nxt=(DLRid*)h; p->prv=h->prv; h->prv=p;}
h=p;
}

