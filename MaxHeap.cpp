#include "MaxHeap.h"

void MaxHeap::swapItems(int firstIndex, int secondIndex) {
    HeapItem temporaryItem = items[firstIndex];
    items[firstIndex] = items[secondIndex];
    items[secondIndex] = temporaryItem;
}

void MaxHeap::push(const HeapItem& item) {
    items.add(item);
    moveItemUp(items.size() - 1);
}

bool MaxHeap::pop(HeapItem& removedItem) {
    if (items.size() == 0) {
        return false;
    }
    removedItem = items[0];
    items[0] = items[items.size() - 1];
    items.removeAt(items.size() - 1);
    if (items.size() > 0) {
        moveItemDown(0);
    }
    return true;
}

bool MaxHeap::isEmpty() const {
    return items.size() == 0;
}

int MaxHeap::size() const {
    return items.size();
}

void MaxHeap::clear() {
    items.clear();
}

void MaxHeap::moveItemUp(int index) {
    while (index > 0) {
        int parentIndex = (index - 1) / 2;
        if (items[parentIndex].priorityKey >= items[index].priorityKey) {
            break;
        }
        swapItems(parentIndex, index);
        index = parentIndex;
    }
}

void MaxHeap::moveItemDown(int index) {
    int itemCount = items.size();
    while (true) {
        int leftChildIndex = 2 * index + 1;
        int rightChildIndex = 2 * index + 2;
        int largestIndex = index;
        if (leftChildIndex < itemCount && items[leftChildIndex].priorityKey > items[largestIndex].priorityKey) {
            largestIndex = leftChildIndex;
        }
        if (rightChildIndex < itemCount && items[rightChildIndex].priorityKey > items[largestIndex].priorityKey) {
            largestIndex = rightChildIndex;
        }
        if (largestIndex == index) {
            break;
        }
        swapItems(index, largestIndex);
        index = largestIndex;
    }
}
