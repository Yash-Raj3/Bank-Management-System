#ifndef INT_QUEUE_H
#define INT_QUEUE_H

#include "DynamicArray.h"

class IntQueue {
public:
    IntQueue();
    ~IntQueue();
    IntQueue(const IntQueue&) = delete;
    IntQueue& operator=(const IntQueue&) = delete;

    void enqueue(int value);
    bool dequeue(int& removedValue);
    bool peek(int& frontValue) const;
    bool isEmpty() const;
    int size() const;
    DynamicArray<int> toArray() const;
    void clear();

private:
    class Node {
    public:
        int value;
        Node* next;
    };

    Node* front;
    Node* rear;
    int itemCount;
};

#endif
