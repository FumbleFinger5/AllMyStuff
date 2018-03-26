// This class maintains a memory block (size specified in constructor) containing as many items as will fit)




class DLR_CACHE {
public:
DLR_CACHE(PFI_v_v _cmp, long _max_sz=65000);
~DLR_CACHE();
void	*get(void *key);						// Find the item matching 'key' if it exists
void	*add_item(ushort sz);					// Allocate a new item + add to front of cache
void	set_user_rls(PFV_v user_rls_function);
private:
struct DLR
	{
	DLR		*nxt,*prv;
	};

struct CTL
	{
	DLR		dlr;
	ushort	sz;
	};

void	rls_oldest(void);
void	dlrsnap(DLR *p);
void	dlrlink(DLR *p);
PFI_v_v	cmp;					// ptr to function "int cmp(void*,void*)"
PFV_v	user_rls;	
long	tot_sz,max_sz;
CTL	*ph;
};

