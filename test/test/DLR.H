// DLRid - Double Linked Ring Structure with integral ID's
// These DLRs always have a 2-byte 'id' after the 2 DLRid pointers, followed
// by a user data record (fixed size as specified to the constructor).

struct DLRid;
struct DLRid {DLRid *nxt,*prv; short id;};

// The Dlr class creates, deletes, and searches by 'id' for entries.
// The constructor is told the size of the userdata area to be passed to 'add()',
// which returns an internally-assigned uniq number from 1 up. Free values
// are re-used if they become available again after 'del()'

// The purpose of the 'Dlr' class is to implement an LRU (Least Recently Used)
// caching algorithm for a table. If there are a large number of elements, or
// it's expensive to examine an element and establish if it's the one we want,
// this is an efficient way to store and retrieve them.

// NOTE: Dlr objects are only useful if the calling app logic is amenable to
// LRU optimisation (i.e. - within any given series of retrievals, the same
// element(s) frequently re-occur). When the app just wants to search the table
// without disturbing the LRU internal state, or to perform some action on ALL
// entries in the table, it should use the scan() function.

// Normally the app won't need to explicitly move an item to the top of the DLR,
// because get() and add() will put it there anyway. So consider the situation
// carefully before calling top() to 'promote' an item to the top of the list,
// in case this suggests a problem with the app logic structure.

class Dlr {
friend class FileSet;
public:
Dlr(int _sz) {h=0; sz=(short)_sz;};
~Dlr() {while (h) del(h->id);};
short	add(void *data);
void	*get(short id);
void	del(short id);
DLRid	*scan(int *again);		// returns 0 if its been thru the whole list
void	top(DLRid *item);		// 'item' MUST already be in the list!
protected:
DLRid	*h;
private:
void	dlrsnap(DLRid *p);
void	dlrlink(DLRid *p);
short	sz;
};


