#include "ServiceQueueModule.h"

#include <iostream>

#include "Utility.h"

ServiceQueueModule::ServiceQueueModule(BankData& bankData) : bank(bankData) {}

String ServiceQueueModule::describePriority(int priorityLevel) const {
    switch (priorityLevel) {
        case 1: return "Priority: Senior citizen";
        case 2: return "Priority: Special assistance";
        case 3: return "Priority: Urgent";
        default: return "Normal";
    }
}

void ServiceQueueModule::removeWaitingToken(int tokenNumber) {
    for (int index = 0; index < bank.waitingTokens.size(); ++index) {
        if (bank.waitingTokens[index].tokenNumber == tokenNumber) {
            bank.waitingTokens.removeAt(index);
            return;
        }
    }
}

void ServiceQueueModule::issueToken() {
    ServiceToken newToken;
    newToken.tokenNumber = bank.nextTokenNumber++;
    newToken.customerName = Utility::readNonEmptyLine("Customer name: ");
    std::cout << "0. Normal\n1. Senior citizen\n2. Special assistance\n3. Urgent case\n";
    newToken.priorityLevel = Utility::readIntInRange("Token type: ", 0, 3);
    bank.waitingTokens.add(newToken);
    if (newToken.priorityLevel == PRIORITY_LEVEL_NORMAL) {
        bank.normalTokenQueue.enqueue(newToken.tokenNumber);
    } else {
        HeapItem tokenItem{computePriorityTokenKey(newToken), newToken.tokenNumber};
        bank.priorityTokenHeap.push(tokenItem);
    }
    bank.logAction("Issued service token " + String::fromInteger(newToken.tokenNumber));
    std::cout << "Token number " << newToken.tokenNumber << " issued to " << newToken.customerName << " ("
              << describePriority(newToken.priorityLevel) << ").\n";
}

void ServiceQueueModule::serveNextToken() {
    int servedTokenNumber = 0;
    HeapItem priorityItem{};
    if (bank.priorityTokenHeap.pop(priorityItem)) {
        servedTokenNumber = priorityItem.itemId;
    } else if (!bank.normalTokenQueue.dequeue(servedTokenNumber)) {
        std::cout << "No customers are waiting.\n";
        return;
    }
    ServiceToken* servedToken = bank.findToken(servedTokenNumber);
    if (servedToken != nullptr) {
        std::cout << "Now serving token " << servedToken->tokenNumber << ": " << servedToken->customerName << " ("
                  << describePriority(servedToken->priorityLevel) << ")\n";
        bank.logAction("Served service token " + String::fromInteger(servedToken->tokenNumber));
    }
    removeWaitingToken(servedTokenNumber);
}

void ServiceQueueModule::showWaitingTokens() {
    Utility::printHeader("WAITING CUSTOMERS");
    int position = 1;
    MaxHeap heapCopy = bank.priorityTokenHeap;
    HeapItem priorityItem{};
    while (heapCopy.pop(priorityItem)) {
        ServiceToken* token = bank.findToken(priorityItem.itemId);
        if (token != nullptr) {
            std::cout << position++ << ". Token " << token->tokenNumber << " - " << token->customerName << " ("
                      << describePriority(token->priorityLevel) << ")\n";
        }
    }
    DynamicArray<int> normalTokenNumbers = bank.normalTokenQueue.toArray();
    for (int index = 0; index < normalTokenNumbers.size(); ++index) {
        ServiceToken* token = bank.findToken(normalTokenNumbers[index]);
        if (token != nullptr) {
            std::cout << position++ << ". Token " << token->tokenNumber << " - " << token->customerName << " (Normal)\n";
        }
    }
    if (position == 1) {
        std::cout << "No customers are waiting.\n";
    }
}
