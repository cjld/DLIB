#define __CL_KERNEL__
#define FOR(i,l,r) for (int i=(l);i<=(r);i++)
#define globalSync(x) globalSync2(x)

int globalIdDim1() {return get_global_id(0);}
int globalSizeDim1() {return get_global_size(0);}
int globalIdDim2() {return get_global_id(0) + get_global_id(1)*get_global_size(0);}
int globalSizeDim2() {return get_global_size(0)*get_global_size(1);}
int globalIdDim3() {return get_global_id(0) + get_global_id(1)*get_global_size(0) + get_global_id(2)*globalSizeDim2();}
int globalSizeDim3() {return get_global_size(0)*get_global_size(1)*get_global_size(2);}

int localIdDim1() {return get_local_id(0);}
int localSizeDim1() {return get_local_size(0);}
int localIdDim2() {return get_local_id(0) + get_local_id(1)*get_local_size(0);}
int localSizeDim2() {return get_local_size(0)*get_local_size(1);}
int localIdDim3() {return get_local_id(0) + get_local_id(1)*get_local_size(0) + get_local_id(2)*localSizeDim2();}
int localSizeDim3() {return get_local_size(0)*get_local_size(1)*get_local_size(2);}

int groupIdDim1() {return get_group_id(0);}
int groupSizeDim1() {return get_num_groups(0);}
int groupIdDim2() {return get_group_id(0) + get_group_id(1)*get_num_groups(0);}
int groupSizeDim2() {return get_num_groups(0)*get_num_groups(1);}
int groupIdDim3() {return get_group_id(0) + get_group_id(1)*get_num_groups(0) + get_group_id(2)*groupSizeDim2();}
int groupSizeDim3() {return get_num_groups(0)*get_num_groups(1)*get_num_groups(2);}

#define getWorkSectionDim1(size,len,l,r) \
    int len = (size-1)/(int)get_global_size(0)+1;\
    int l = (int)get_global_id(0)*len;\
    int r = l+len;\
    if (r>(size)) r=(size); \
    r--;

#define getWorkSectionDim2(size,len,l,r) \
    int len = (size-1)/globalSizeDim2()+1;\
    int l = (int)globalIdDim2()*len;\
    int r = l+len;\
    if (r>(size)) r=(size); \
    r--;

#define getWorkSectionDim3(size,len,l,r) \
    int len = (size-1)/globalSizeDim3()+1;\
    int l = (int)globalIdDim3()*len;\
    int r = l+len;\
    if (r>(size)) r=(size); \
    r--;

#define getWorkSection2Dim2(xs,ys,xl,xr,yl,yr) \
    int lenx = (xs-1)/(int)get_global_size(0)+1;\
    int leny = (ys-1)/(int)get_global_size(1)+1;\
    int xl = lenx * get_global_id(0), xr = xl + lenx;\
    int yl = leny * get_global_id(1), yr = yl + leny;\
    if (xr>xs) xr=xs;\
    if (yr>ys) yr=ys;\
    xr--, yr--;

#define swap(type,a,b) {type temp=a;a=b;b=temp;}

 float __OVERLOADABLE__ cross(float ax, float ay, float bx, float by) {return ax*by-ay*bx;}

void globalSync1(volatile global int *s) {
    atomic_inc(s);
    while (*s!=get_global_size(0));
    if (get_global_id(0)==0) atomic_sub(s,get_global_size(0));
}

void globalSync2(volatile global int *s) {
    if (get_local_id(0)==0) {
        atomic_inc(s);
        while (*s!=get_num_groups(0));
    };
    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    if (get_global_id(0)==0) atomic_sub(s,get_num_groups(0));
}

void globalSyncDim1(volatile global int *s) {
    if (get_local_id(0)==0) {
        atomic_inc(s);
        while (*s!=get_num_groups(0));
    };
    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    if (globalIdDim1()==0) atomic_sub(s,get_num_groups(0));
}

void globalSyncDim2(volatile global int *s) {
    if (localSizeDim2()==0) {
        atomic_inc(s);
        while (*s!=groupSizeDim2());
    };
    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    if (globalIdDim2()==0) atomic_sub(s,get_num_groups(0));
}

void globalSyncDim3(volatile global int *s) {
    if (localSizeDim3()==0) {
        atomic_inc(s);
        while (*s!=groupSizeDim3());
    };
    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    if (globalIdDim3()==0) atomic_sub(s,get_num_groups(0));
}


void globalSync3(volatile global int *s) {
    const int lid=get_local_id(0);
    const int gid=get_group_id(0);
    if (!lid) s[gid]=1;
    if (!gid) {
        if (lid<get_num_groups(0))
            while (!s[lid]);
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
        s[lid]=0;
    }
    while (!lid && s[gid]);
    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}
