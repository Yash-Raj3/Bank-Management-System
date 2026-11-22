#include "TransferGraph.h"

#include "IntQueue.h"

namespace {

const double MINIMUM_VISIBLE_AMOUNT = 0.005;

bool arrayContains(const DynamicArray<int>& numbers, int searchedNumber) {
    for (int index = 0; index < numbers.size(); ++index) {
        if (numbers[index] == searchedNumber) {
            return true;
        }
    }
    return false;
}

}

TransferGraph::TransferGraph() : firstVertex(nullptr) {}

TransferGraph::~TransferGraph() {
    clear();
}

TransferGraph::Vertex* TransferGraph::findVertex(int accountNumber) const {
    for (Vertex* current = firstVertex; current != nullptr; current = current->next) {
        if (current->accountNumber == accountNumber) {
            return current;
        }
    }
    return nullptr;
}

TransferGraph::Vertex* TransferGraph::findOrCreateVertex(int accountNumber) {
    Vertex* existingVertex = findVertex(accountNumber);
    if (existingVertex != nullptr) {
        return existingVertex;
    }
    firstVertex = new Vertex{accountNumber, nullptr, firstVertex};
    return firstVertex;
}

void TransferGraph::addTransfer(int fromAccount, int toAccount, double amount) {
    Vertex* sourceVertex = findOrCreateVertex(fromAccount);
    findOrCreateVertex(toAccount);
    for (Edge* current = sourceVertex->firstEdge; current != nullptr; current = current->next) {
        if (current->destinationAccount == toAccount) {
            current->totalAmount += amount;
            current->transferCount += amount >= 0 ? 1 : -1;
            return;
        }
    }
    sourceVertex->firstEdge = new Edge{toAccount, amount, amount >= 0 ? 1 : -1, sourceVertex->firstEdge};
}

DynamicArray<TransferLink> TransferGraph::getOutgoingLinks(int accountNumber) const {
    DynamicArray<TransferLink> outgoingLinks;
    Vertex* sourceVertex = findVertex(accountNumber);
    if (sourceVertex == nullptr) {
        return outgoingLinks;
    }
    for (Edge* current = sourceVertex->firstEdge; current != nullptr; current = current->next) {
        if (current->totalAmount > MINIMUM_VISIBLE_AMOUNT) {
            TransferLink link{accountNumber, current->destinationAccount, current->totalAmount, current->transferCount};
            outgoingLinks.add(link);
        }
    }
    return outgoingLinks;
}

DynamicArray<TransferLink> TransferGraph::getIncomingLinks(int accountNumber) const {
    DynamicArray<TransferLink> incomingLinks;
    for (Vertex* vertex = firstVertex; vertex != nullptr; vertex = vertex->next) {
        for (Edge* current = vertex->firstEdge; current != nullptr; current = current->next) {
            if (current->destinationAccount == accountNumber && current->totalAmount > MINIMUM_VISIBLE_AMOUNT) {
                TransferLink link{vertex->accountNumber, accountNumber, current->totalAmount, current->transferCount};
                incomingLinks.add(link);
            }
        }
    }
    return incomingLinks;
}

DynamicArray<int> TransferGraph::getReachableAccounts(int startAccount) const {
    DynamicArray<int> reachableAccounts;
    DynamicArray<int> visitedAccounts;
    visitedAccounts.add(startAccount);
    IntQueue accountsToExplore;
    accountsToExplore.enqueue(startAccount);
    int currentAccount = 0;
    while (accountsToExplore.dequeue(currentAccount)) {
        Vertex* currentVertex = findVertex(currentAccount);
        if (currentVertex == nullptr) {
            continue;
        }
        for (Edge* edge = currentVertex->firstEdge; edge != nullptr; edge = edge->next) {
            if (edge->totalAmount > MINIMUM_VISIBLE_AMOUNT && !arrayContains(visitedAccounts, edge->destinationAccount)) {
                visitedAccounts.add(edge->destinationAccount);
                reachableAccounts.add(edge->destinationAccount);
                accountsToExplore.enqueue(edge->destinationAccount);
            }
        }
    }
    return reachableAccounts;
}

void TransferGraph::clear() {
    while (firstVertex != nullptr) {
        Vertex* vertexToDelete = firstVertex;
        firstVertex = firstVertex->next;
        while (vertexToDelete->firstEdge != nullptr) {
            Edge* edgeToDelete = vertexToDelete->firstEdge;
            vertexToDelete->firstEdge = edgeToDelete->next;
            delete edgeToDelete;
        }
        delete vertexToDelete;
    }
}
