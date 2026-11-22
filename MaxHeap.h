#ifndef MAX_HEAP_H
#define MAX_HEAP_H

#include "DynamicArray.h"
#include "Models.h"

class MaxHeap {
public:
    void push(const HeapItem& item);
    bool pop(HeapItem& removedItem);
    bool isEmpty() const;
    int size() const;
    void clear();

private:
    DynamicArray<HeapItem> items;

    void swapItems(int firstIndex, int secondIndex);
    void moveItemUp(int index);
    void moveItemDown(int index);
};

#endif
