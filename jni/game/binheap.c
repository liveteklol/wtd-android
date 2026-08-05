/************************************/
/* Warcraft Tower Defense - by Noda */
/* Binary heap for AI      31/12/06 */
/************************************/

// Based on implementation by Steven S. Skiena

// Includes
#include <PA9.h>       // Include for PA_Lib
#include "binheap.h"
#include "nds.h"

// Initialize a new queue
PriorityQueue Initialize(int MaxElements) {

    PriorityQueue pq;

    pq = malloc(sizeof(struct HeapStruct));

    // Allocate the array plus one extra for sentinel
    pq->Elements = malloc((MaxElements+1) * sizeof(ElementType));
    pq->Capacity = MaxElements;
    pq->Size = 0;
    pq->Elements[0] = MinData;  // the sentinel
    return pq;
}

// Insert a element in the right position
void Insert(ElementType e, PriorityQueue pq)
{
    int i;

//    if(IsFull(pq))
//        return;

    for(i=++pq->Size; pq->Elements[i/2].w > e.w; i/=2)
        pq->Elements[i] = pq->Elements[i / 2];
        
    pq->Elements[i] = e;
}

// Return & delete the minimum
ElementType DeleteMin(PriorityQueue pq)
{
    int i, Child;
    ElementType MinElement, LastElement;

//    if(IsEmpty(pq))
//        return pq->Elements[0];

    MinElement = pq->Elements[1];
    LastElement = pq->Elements[pq->Size--];

    for(i=1; i*2 <= pq->Size; i=Child)
    {
        /* Find smaller child */
        Child = i * 2;
        if(Child != pq->Size && pq->Elements[Child+1].w < pq->Elements[Child].w)
            Child++;

        /* Percolate one level */
        if(LastElement.w > pq->Elements[Child].w)
            pq->Elements[i] = pq->Elements[Child];
        else
            break;
    }
    pq->Elements[i] = LastElement;

    return MinElement;
}

// Return the minimum
ElementType FindMin(PriorityQueue pq)
{
//    if(!IsEmpty(pq))
        return pq->Elements[1];

//    return pq->Elements[0];
}

// Return true is the queue is empty
bool IsEmpty(PriorityQueue pq)
{
    return pq->Size == 0;
}

// Return true is the queue is full
bool IsFull(PriorityQueue pq)
{
    return pq->Size == pq->Capacity;
}

// Destroy the queue
void Destroy(PriorityQueue pq)
{
    free(pq->Elements);
    free(pq);
}

