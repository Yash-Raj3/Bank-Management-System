#include "UserHashTable.h"

namespace {
const int INITIAL_BUCKET_COUNT = 31;
}

UserHashTable::UserHashTable() : buckets(new Node*[INITIAL_BUCKET_COUNT]), bucketCount(INITIAL_BUCKET_COUNT), itemCount(0) {
    for (int index = 0; index < bucketCount; ++index) {
        buckets[index] = nullptr;
    }
}

UserHashTable::~UserHashTable() {
    clear();
    delete[] buckets;
}

int UserHashTable::bucketIndexFor(const String& userId, int bucketCount) {
    unsigned long long hashValue = 0;
    for (int index = 0; index < userId.length(); ++index) {
        hashValue = hashValue * 31 + static_cast<unsigned char>(userId.charAt(index));
    }
    return static_cast<int>(hashValue % static_cast<unsigned long long>(bucketCount));
}

bool UserHashTable::insert(User* user) {
    if (find(user->userId) != nullptr) {
        return false;
    }
    if (itemCount + 1 > bucketCount * 3 / 4) {
        growTable();
    }
    int bucketIndex = bucketIndexFor(user->userId, bucketCount);
    buckets[bucketIndex] = new Node{user, buckets[bucketIndex]};
    ++itemCount;
    return true;
}

User* UserHashTable::find(const String& userId) const {
    for (Node* current = buckets[bucketIndexFor(userId, bucketCount)]; current != nullptr; current = current->next) {
        if (current->user->userId == userId) {
            return current->user;
        }
    }
    return nullptr;
}

bool UserHashTable::remove(const String& userId) {
    int bucketIndex = bucketIndexFor(userId, bucketCount);
    Node* previous = nullptr;
    for (Node* current = buckets[bucketIndex]; current != nullptr; previous = current, current = current->next) {
        if (current->user->userId == userId) {
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

DynamicArray<User*> UserHashTable::collectAll() const {
    DynamicArray<User*> sortedUsers;
    for (int bucketIndex = 0; bucketIndex < bucketCount; ++bucketIndex) {
        for (Node* current = buckets[bucketIndex]; current != nullptr; current = current->next) {
            sortedUsers.add(current->user);
            int position = sortedUsers.size() - 1;
            while (position > 0 && sortedUsers[position]->userId < sortedUsers[position - 1]->userId) {
                User* temporaryUser = sortedUsers[position];
                sortedUsers[position] = sortedUsers[position - 1];
                sortedUsers[position - 1] = temporaryUser;
                --position;
            }
        }
    }
    return sortedUsers;
}

int UserHashTable::size() const {
    return itemCount;
}

void UserHashTable::growTable() {
    int newBucketCount = bucketCount * 2 + 1;
    Node** newBuckets = new Node*[newBucketCount];
    for (int index = 0; index < newBucketCount; ++index) {
        newBuckets[index] = nullptr;
    }
    for (int index = 0; index < bucketCount; ++index) {
        Node* current = buckets[index];
        while (current != nullptr) {
            Node* nextNode = current->next;
            int newIndex = bucketIndexFor(current->user->userId, newBucketCount);
            current->next = newBuckets[newIndex];
            newBuckets[newIndex] = current;
            current = nextNode;
        }
    }
    delete[] buckets;
    buckets = newBuckets;
    bucketCount = newBucketCount;
}

void UserHashTable::clear() {
    for (int index = 0; index < bucketCount; ++index) {
        while (buckets[index] != nullptr) {
            Node* nodeToDelete = buckets[index];
            buckets[index] = nodeToDelete->next;
            delete nodeToDelete;
        }
    }
    itemCount = 0;
}
