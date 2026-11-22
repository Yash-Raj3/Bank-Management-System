#ifndef BANK_DATA_H
#define BANK_DATA_H

#include "Account.h"
#include "AccountAVLTree.h"
#include "AccountHashTable.h"
#include "AuditLog.h"
#include "DynamicArray.h"
#include "IntQueue.h"
#include "MaxHeap.h"
#include "Models.h"
#include "String.h"
#include "TransactionStack.h"
#include "TransferGraph.h"
#include "UserHashTable.h"

class BankData {
public:
    AccountHashTable accountTable;
    AccountAVLTree accountTree;
    UserHashTable userTable;
    AuditLog auditLog;
    TransactionStack undoStack;
    IntQueue approvalQueue;
    IntQueue normalTokenQueue;
    MaxHeap priorityTokenHeap;
    MaxHeap loanRequestHeap;
    TransferGraph transferGraph;

    DynamicArray<Loan> loans;
    DynamicArray<ServiceToken> waitingTokens;
    BankRules rules;

    int nextAccountNumber = 1001;
    int nextTransactionId = 1;
    int nextLoanId = 1;
    int nextTokenNumber = 1;
    int skippedRecordCount = 0;
    String currentActorName = "system";

    ~BankData();

    Account* findAccount(int accountNumber) const;
    Loan* findLoan(int loanId);
    ServiceToken* findToken(int tokenNumber);
    bool addAccount(Account* account);
    void removeAndDeleteAccount(int accountNumber);
    void logAction(const String& actionDescription);
    void ensureDefaultAdministratorExists();
    void clearAllData();
};

#endif
