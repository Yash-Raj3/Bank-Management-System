#ifndef TRANSFER_GRAPH_H
#define TRANSFER_GRAPH_H

#include "DynamicArray.h"
#include "Models.h"

class TransferGraph {
public:
    TransferGraph();
    ~TransferGraph();
    TransferGraph(const TransferGraph&) = delete;
    TransferGraph& operator=(const TransferGraph&) = delete;

    void addTransfer(int fromAccount, int toAccount, double amount);
    DynamicArray<TransferLink> getOutgoingLinks(int accountNumber) const;
    DynamicArray<TransferLink> getIncomingLinks(int accountNumber) const;
    DynamicArray<int> getReachableAccounts(int startAccount) const;
    void clear();

private:
    class Edge {
    public:
        int destinationAccount;
        double totalAmount;
        int transferCount;
        Edge* next;
    };

    class Vertex {
    public:
        int accountNumber;
        Edge* firstEdge;
        Vertex* next;
    };

    Vertex* firstVertex;

    Vertex* findVertex(int accountNumber) const;
    Vertex* findOrCreateVertex(int accountNumber);
};

#endif
