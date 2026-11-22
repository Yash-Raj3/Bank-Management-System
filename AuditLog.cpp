#include "AuditLog.h"

#include "Utility.h"

AuditLog::AuditLog() : head(nullptr), tail(nullptr), entryCount(0) {}

AuditLog::~AuditLog() {
    clear();
}

void AuditLog::record(const String& actorName, const String& actionDescription) {
    AuditEntry newEntry{Utility::currentTimestamp(), actorName, actionDescription};
    addEntry(newEntry);
}

void AuditLog::addEntry(const AuditEntry& entry) {
    Node* newNode = new Node{entry, tail, nullptr};
    if (tail != nullptr) {
        tail->next = newNode;
    } else {
        head = newNode;
    }
    tail = newNode;
    ++entryCount;
}

DynamicArray<AuditEntry> AuditLog::toArray() const {
    DynamicArray<AuditEntry> allEntries;
    for (Node* current = head; current != nullptr; current = current->next) {
        allEntries.add(current->data);
    }
    return allEntries;
}

int AuditLog::size() const {
    return entryCount;
}

void AuditLog::clear() {
    while (head != nullptr) {
        Node* nodeToDelete = head;
        head = head->next;
        delete nodeToDelete;
    }
    tail = nullptr;
    entryCount = 0;
}
