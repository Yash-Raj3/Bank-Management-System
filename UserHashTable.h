#ifndef USER_HASH_TABLE_H
#define USER_HASH_TABLE_H

#include "DynamicArray.h"
#include "Models.h"
#include "String.h"

class UserHashTable {
public:
    UserHashTable();
    ~UserHashTable();
    UserHashTable(const UserHashTable&) = delete;
    UserHashTable& operator=(const UserHashTable&) = delete;

    bool insert(User* user);
    User* find(const String& userId) const;
    bool remove(const String& userId);
    DynamicArray<User*> collectAll() const;
    int size() const;
    void clear();

private:
    class Node {
    public:
        User* user;
        Node* next;
    };

    Node** buckets;
    int bucketCount;
    int itemCount;

    static int bucketIndexFor(const String& userId, int bucketCount);
    void growTable();
};

#endif
