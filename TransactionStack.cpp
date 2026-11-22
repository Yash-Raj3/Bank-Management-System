#include "TransactionStack.h"

TransactionStack::TransactionStack() : top(nullptr) {}

TransactionStack::~TransactionStack() {
    clear();
}

void TransactionStack::push(const Transaction& transaction) {
    top = new Node{transaction, top};
}

bool TransactionStack::pop(Transaction& removedTransaction) {
    if (top == nullptr) {
        return false;
    }
    Node* nodeToDelete = top;
    removedTransaction = nodeToDelete->data;
    top = top->below;
    delete nodeToDelete;
    return true;
}

bool TransactionStack::peek(Transaction& topTransaction) const {
    if (top == nullptr) {
        return false;
    }
    topTransaction = top->data;
    return true;
}

bool TransactionStack::isEmpty() const {
    return top == nullptr;
}

void TransactionStack::clear() {
    Transaction ignoredTransaction;
    while (pop(ignoredTransaction)) {
    }
}
