#include <algorithm>
#include <iostream>

using namespace std;

struct VebTree {
    int size, max, min;
    VebTree *summary;
    VebTree **cluster;

    VebTree() : size(0), max(-1), min(-1), summary(0), cluster(0) {}
    ~VebTree();
    VebTree(int);
    void build(int);
    void setMaxMin(int x) {max=min=x;}
    int low() {return (size+1)>>1;}
    int high(const int &x) {return x>>((size+1)>>1);}
    int low(const int &x) {return x&((1<<((size+1)>>1))-1);}
    int index(const int &x, const int &y) {return (x<<low())+y;}

    void ins(int);
    int insGetPrev(int);
    void del(int);
    int prev(int);
    int next(int);

};

VebTree::VebTree(int s) : size(0), max(-1), min(-1), summary(0), cluster(0) {
    int i=1;
    while ((1<<i) < s) i++;
    build(i);
}

VebTree::~VebTree() {
    if (size==1) return;
    delete summary;
    int csize=1<<(size>>1);
    for (int i=0; i<csize; i++)
        delete cluster[i];
    delete[] cluster;
}

void VebTree::build(int s) {
    size=s;
    if (s==1) return;
    summary = new VebTree;
    summary->build(s>>1);
    int csize=1<<(s>>1);
    cluster = new VebTree*[csize];
    for (int i=0; i<csize; i++) {
        cluster[i] = new VebTree;
        cluster[i]->build((s+1)>>1);
    }
}

#ifdef PROFILE
int instime=0;
int deltime=0;
int prevtime=0;
#endif

void VebTree::ins(int x) {
    #ifdef PROFILE
    instime++;
    #endif
    if (min==-1) {
        setMaxMin(x); 
        return;
    }
    if (x<min) swap(x,min);
    int hx=high(x), lx=low(x);
    if (size>1) {
        if (cluster[hx]->min == -1) {
            summary->ins(hx);
            cluster[hx]->setMaxMin(lx);
        } else
            cluster[hx]->ins(lx);
    }
    max=std::max(max,x);
}

void VebTree::del(int x) {
    #ifdef PROFILE
    deltime++;
    #endif
    if (max==min) {
        setMaxMin(-1);
        return;
    }
    if (size==1) {
        setMaxMin(!x);
        return;
    }
    int hx,lx;
    if (x==min) {
        hx = summary->min;
        lx = cluster[hx]->min;
        x = min = index(hx,lx);
    } else
        hx=high(x), lx=low(x);
    if (cluster[hx]->max == cluster[hx]->min) {
        cluster[hx]->setMaxMin(-1);
        summary->del(hx);
    } else
        cluster[hx]->del(lx);
    if (x==max) {
        hx = summary->max;
        if (hx==-1) max=min;
        else max = index(hx, cluster[hx]->max);
    }
}

int VebTree::prev(int x) {
    #ifdef PROFILE
    prevtime++;
    #endif
    if (min==-1 || x<min) return -1;
    if (x>=max) return max;
    if (size==1) return min;
    int hx=high(x), lx=low(x);
    if (cluster[hx]->min==-1 || lx < cluster[hx]->min) {
        hx = summary->prev(hx-1);
        if (hx==-1) return min;
        return index(hx, cluster[hx]->max);
    }
    return index(hx, cluster[hx]->prev(lx));
}

int VebTree::insGetPrev(int x) {
    #ifdef PROFILE
    instime++;
    #endif
    if (min==-1) {
        setMaxMin(x); 
        return -1;
    }
    if (x<min) {ins(x); return -1;}
    int hx=high(x), lx=low(x), prev = min;
    if (size>1) {
        if (cluster[hx]->min == -1) {
            int cp = summary->insGetPrev(hx);
            cluster[hx]->setMaxMin(lx);
            if (cp!=-1) 
                prev = index(cp, cluster[cp]->max);
        } else {
            int cp = cluster[hx]->insGetPrev(lx);
            if (cp==-1) {
                cp = summary->prev(hx-1);
                if (cp!=-1) 
                    prev = index(cp, cluster[cp]->max);
            } else 
                prev = index(hx, cp);
        }
    }
    max=std::max(max,x);
    return prev;
}

int VebTree::next(int x) {
    if (min==-1 || x>max) return -1;
    if (x<=min) return min;
    if (size==1) return max;
    int hx=high(x), lx=low(x);
    if (cluster[hx]->max==-1 || lx > cluster[hx]->max) {
        hx = summary->next(hx+1);
        return index(hx, cluster[hx]->min);
    }
    return index(hx, cluster[hx]->next(lx));
}

#ifndef VEBTREE_H

int main() {
    VebTree root(10);
    root.ins(1);
    root.ins(8);
    cout << root.prev(0) <<endl;
    cout << root.prev(2) <<endl;
    cout << root.prev(3) <<endl;
    cout << root.next(1) <<endl;
    cout << root.next(3) <<endl;
    cout << root.next(7) <<endl;
}

#endif