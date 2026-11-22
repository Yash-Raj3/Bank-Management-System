#ifndef AUDIT_LOG_H
#define AUDIT_LOG_H

#include "DynamicArray.h"
#include "Models.h"
#include "String.h"

class AuditLog {
public:
    AuditLog();
    ~AuditLog();
    AuditLog(const AuditLog&) = delete;
    AuditLog& operator=(const AuditLog&) = delete;

    void record(const String& actorName, const String& actionDescription);
    void addEntry(const AuditEntry& entry);
    DynamicArray<AuditEntry> toArray() const;
    int size() const;
    void clear();

private:
    class Node {
    public:
        AuditEntry data;
        Node* previous;
        Node* next;
    };

    Node* head;
    Node* tail;
    int entryCount;
};

#endif
