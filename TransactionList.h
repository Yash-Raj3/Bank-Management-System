#ifndef TRANSACTION_LIST_H
#define TRANSACTION_LIST_H

#include "DynamicArray.h"
#include "Models.h"

class TransactionList {
public:
    TransactionList();
    ~TransactionList();
    TransactionList(const TransactionList&) = delete;
    TransactionList& operator=(const TransactionList&) = delete;

    void append(const Transaction& transaction);
    int size() const;
    DynamicArray<Transaction> toArray() const;
    DynamicArray<Transaction> lastEntries(int maximumCount) const;
    void clear();

private:
    class Node {
    public:
        Transaction data;
        Node* previous;
        Node* next;
    };

    Node* head;
    Node* tail;
    int entryCount;
};

#endif
