#include "AccountHashTable.h"

namespace {
const int INITIAL_BUCKET_COUNT = 101;
}

AccountHashTable::AccountHashTable() : buckets(new Node*[INITIAL_BUCKET_COUNT]), bucketCount(INITIAL_BUCKET_COUNT), itemCount(0) {
    for (int index = 0; index < bucketCount; ++index) {
        buckets[index] = nullptr;
    }
}

AccountHashTable::~AccountHashTable() {
    clear();
    delete[] buckets;
}

bool AccountHashTable::insert(Account* account) {
    if (find(account->accountNumber) != nullptr) {
        return false;
    }
    if (itemCount + 1 > bucketCount * 3 / 4) {
        growTable();
    }
    int bucketIndex = account->accountNumber % bucketCount;
    buckets[bucketIndex] = new Node{account, buckets[bucketIndex]};
    ++itemCount;
    return true;
}

Account* AccountHashTable::find(int accountNumber) const {
    for (Node* current = buckets[accountNumber % bucketCount]; current != nullptr; current = current->next) {
        if (current->account->accountNumber == accountNumber) {
            return current->account;
        }
    }
    return nullptr;
}

bool AccountHashTable::remove(int accountNumber) {
    int bucketIndex = accountNumber % bucketCount;
    Node* previous = nullptr;
    for (Node* current = buckets[bucketIndex]; current != nullptr; previous = current, current = current->next) {
        if (current->account->accountNumber == accountNumber) {
            if (previous != nullptr) {
                previous->next = current->next;
            } else {
                buckets[bucketIndex] = current->next;
            }
            delete current;
            --itemCount;
            return true;
        }
    }
    return false;
}

int AccountHashTable::size() const {
    return itemCount;
}

void AccountHashTable::growTable() {
    int newBucketCount = bucketCount * 2 + 1;
    Node** newBuckets = new Node*[newBucketCount];
    for (int index = 0; index < newBucketCount; ++index) {
        newBuckets[index] = nullptr;
    }
    for (int index = 0; index < bucketCount; ++index) {
        Node* current = buckets[index];
        while (current != nullptr) {
            Node* nextNode = current->next;
            int newIndex = current->account->accountNumber % newBucketCount;
            current->next = newBuckets[newIndex];
            newBuckets[newIndex] = current;
            current = nextNode;
        }
    }
    delete[] buckets;
    buckets = newBuckets;
    bucketCount = newBucketCount;
}

void AccountHashTable::clear() {
    for (int index = 0; index < bucketCount; ++index) {
        while (buckets[index] != nullptr) {
            Node* nodeToDelete = buckets[index];
            buckets[index] = nodeToDelete->next;
            delete nodeToDelete;
        }
    }
    itemCount = 0;
}
