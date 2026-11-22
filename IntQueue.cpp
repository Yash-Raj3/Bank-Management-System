#include "IntQueue.h"

IntQueue::IntQueue() : front(nullptr), rear(nullptr), itemCount(0) {}

IntQueue::~IntQueue() {
    clear();
}

void IntQueue::enqueue(int value) {
    Node* newNode = new Node{value, nullptr};
    if (rear != nullptr) {
        rear->next = newNode;
    } else {
        front = newNode;
    }
    rear = newNode;
    ++itemCount;
}

bool IntQueue::dequeue(int& removedValue) {
    if (front == nullptr) {
        return false;
    }
    Node* nodeToDelete = front;
    removedValue = nodeToDelete->value;
    front = front->next;
    if (front == nullptr) {
        rear = nullptr;
    }
    delete nodeToDelete;
    --itemCount;
    return true;
}

bool IntQueue::peek(int& frontValue) const {
    if (front == nullptr) {
        return false;
    }
    frontValue = front->value;
    return true;
}

bool IntQueue::isEmpty() const {
    return front == nullptr;
}

int IntQueue::size() const {
    return itemCount;
}

DynamicArray<int> IntQueue::toArray() const {
    DynamicArray<int> allValues;
    for (Node* current = front; current != nullptr; current = current->next) {
        allValues.add(current->value);
    }
    return allValues;
}

void IntQueue::clear() {
    int ignoredValue;
    while (dequeue(ignoredValue)) {
    }
}
