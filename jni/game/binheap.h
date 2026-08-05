/************************************/
/* Warcraft Tower Defense - by Noda */
/* Binary heap for AI      31/12/06 */
/************************************/

// Based on implementation by Steven S. Skiena

#ifndef __BINHEAP__
#define __BINHEAP__

#define MinData (point){0, 0, 0}

// types definition
typedef struct _point {
    u8 x;
    u8 y;
    u16 w;
} point;

typedef point ElementType;

struct HeapStruct {
    int Capacity;
    int Size;
    ElementType* Elements;
};

struct HeapStruct;
typedef struct HeapStruct* PriorityQueue;

// Initialize a new queue
inline PriorityQueue Initialize(int MaxElements);

// Insert a element in the right position
inline void Insert(ElementType e, PriorityQueue pq);

// Return & delete the minimum
inline ElementType DeleteMin(PriorityQueue pq);

// Return the minimum
inline ElementType FindMin(PriorityQueue pq);

// Return true is the queue is empty
inline bool IsEmpty(PriorityQueue pq);

// Return true is the queue is full
inline bool IsFull(PriorityQueue pq);

// Destroy the queue
inline void Destroy(PriorityQueue pq);

#endif
