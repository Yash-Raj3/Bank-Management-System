#ifndef TRANSACTION_STACK_H
#define TRANSACTION_STACK_H

#include "Models.h"

class TransactionStack {
public:
    TransactionStack();
    ~TransactionStack();
    TransactionStack(const TransactionStack&) = delete;
    TransactionStack& operator=(const TransactionStack&) = delete;

    void push(const Transaction& transaction);
    bool pop(Transaction& removedTransaction);
    bool peek(Transaction& topTransaction) const;
    bool isEmpty() const;
    void clear();

private:
    class Node {
    public:
        Transaction data;
        Node* below;
    };

    Node* top;
};

#endif
