#ifndef ACCOUNT_HASH_TABLE_H
#define ACCOUNT_HASH_TABLE_H

#include "Account.h"

class AccountHashTable {
public:
    AccountHashTable();
    ~AccountHashTable();
    AccountHashTable(const AccountHashTable&) = delete;
    AccountHashTable& operator=(const AccountHashTable&) = delete;

    bool insert(Account* account);
    Account* find(int accountNumber) const;
    bool remove(int accountNumber);
    int size() const;
    void clear();

private:
    class Node {
    public:
        Account* account;
        Node* next;
    };

    Node** buckets;
    int bucketCount;
    int itemCount;

    void growTable();
};

#endif
