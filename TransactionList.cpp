#include "TransactionList.h"

TransactionList::TransactionList() : head(nullptr), tail(nullptr), entryCount(0) {}

TransactionList::~TransactionList() {
    clear();
}

void TransactionList::append(const Transaction& transaction) {
    Node* newNode = new Node{transaction, tail, nullptr};
    if (tail != nullptr) {
        tail->next = newNode;
    } else {
        head = newNode;
    }
    tail = newNode;
    ++entryCount;
}

int TransactionList::size() const {
    return entryCount;
}

DynamicArray<Transaction> TransactionList::toArray() const {
    DynamicArray<Transaction> allTransactions;
    for (Node* current = head; current != nullptr; current = current->next) {
        allTransactions.add(current->data);
    }
    return allTransactions;
}

DynamicArray<Transaction> TransactionList::lastEntries(int maximumCount) const {
    DynamicArray<Transaction> newestFirst;
    for (Node* current = tail; current != nullptr && newestFirst.size() < maximumCount; current = current->previous) {
        newestFirst.add(current->data);
    }
    DynamicArray<Transaction> chronologicalOrder;
    for (int index = newestFirst.size() - 1; index >= 0; --index) {
        chronologicalOrder.add(newestFirst[index]);
    }
    return chronologicalOrder;
}

void TransactionList::clear() {
    while (head != nullptr) {
        Node* nodeToDelete = head;
        head = head->next;
        delete nodeToDelete;
    }
    tail = nullptr;
    entryCount = 0;
}
