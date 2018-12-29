#ifndef __INCLUDE_SEGMENT_TREE_HEADER___
#define __INCLUDE_SEGMENT_TREE_HEADER___
#include <algorithm>
#include <cstring>
#include <climits>

class SegmentTree {
    public:
    const int N;
    const int *from_array;
    int  *seg; 
    SegmentTree (const int *array, int N) : N(N), from_array(array) {
        using std::fill;
        int n=N*3+5;
        this->seg  = new int[n];
        fill(this->seg,  this->seg+n,  INT_MAX);
        __build(1, N, 1);
    };
    ~SegmentTree() {
        delete[] this->seg;
    };

// #define HALF_OPEN_INTERVAL
#define LEFT(i) ((i)<<1)
#define RIGHT(i) ((i)<<1|1)

#ifndef HALF_OPEN_INTERVAL
    // 節點紀錄閉區間   [, ]
#define IN_RANGE (qL<=L && qR>=R)
#define OO_RANGE (L>qR || R<qL)
#else
    // 節點紀錄半開區間 [, )
#define IN_RANGE (qL<=L && qR>=R)
#define OO_RANGE (L>=qR || R<=qL)
#endif

    void __build(int L, int R, int i) {
        if (L==R) { // leaf
            seg[i] = from_array[L-1]; // 0-base to 1-base
            return;
        }
        int M=(L+R)>>1;
        __build(L,   M, LEFT(i));
#ifndef HALF_OPEN_INTERVAL
        __build(M+1, R, RIGHT(i));
#else
        __build(M,   R, RIGHT(i));
#endif
        seg[i] = std::min(seg[LEFT(i)],seg[RIGHT(i)]); // update internal node
    }

    int real_query(int L, int R, int qL, int qR, int flag) {
        int M=(L+R)>>1;
        if (OO_RANGE) return INT_MAX; // 沒東西
        if (IN_RANGE) return seg[flag];
        int left=INT_MAX, right=INT_MAX;
        left  = real_query(L,M,qL,qR,LEFT(flag));
#ifndef HALF_OPEN_INTERVAL
        right = real_query(M+1,R,qL,qR,RIGHT(flag));
#else
        right = real_query(M,R,qL,qR,RIGHT(flag));
#endif
        return std::min(left, right);
    };

    int query(int qL, int qR) {
        return real_query(1, this->N, qL+1, qR+1, 1); // 0-base to 1-base
    };

#undef IN_RANGE
#undef OO_RANGE
#undef LEFT
#undef RIGHT
#undef HALF_OPEN_INTERVAL
};
#endif
