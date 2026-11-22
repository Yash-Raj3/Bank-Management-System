#include "BankData.h"

#include "Utility.h"

BankData::~BankData() {
    clearAllData();
}

Account* BankData::findAccount(int accountNumber) const {
    return accountTable.find(accountNumber);
}

Loan* BankData::findLoan(int loanId) {
    for (int index = 0; index < loans.size(); ++index) {
        if (loans[index].loanId == loanId) {
            return &loans[index];
        }
    }
    return nullptr;
}

ServiceToken* BankData::findToken(int tokenNumber) {
    for (int index = 0; index < waitingTokens.size(); ++index) {
        if (waitingTokens[index].tokenNumber == tokenNumber) {
            return &waitingTokens[index];
        }
    }
    return nullptr;
}

bool BankData::addAccount(Account* account) {
    if (!accountTable.insert(account)) {
        return false;
    }
    accountTree.insert(account);
    return true;
}

void BankData::removeAndDeleteAccount(int accountNumber) {
    Account* accountToDelete = accountTable.find(accountNumber);
    accountTable.remove(accountNumber);
    accountTree.remove(accountNumber);
    delete accountToDelete;
}

void BankData::logAction(const String& actionDescription) {
    auditLog.record(currentActorName, actionDescription);
}

void BankData::ensureDefaultAdministratorExists() {
    DynamicArray<User*> allUsers = userTable.collectAll();
    for (int index = 0; index < allUsers.size(); ++index) {
        if (allUsers[index]->role == UserRole::Administrator) {
            return;
        }
    }
    User* defaultAdministrator = new User();
    defaultAdministrator->userId = "admin";
    defaultAdministrator->passwordHash = Utility::hashSecret("admin123");
    defaultAdministrator->role = UserRole::Administrator;
    if (!userTable.insert(defaultAdministrator)) {
        delete defaultAdministrator;
    }
}

void BankData::clearAllData() {
    DynamicArray<Account*> allAccounts = accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        delete allAccounts[index];
    }
    accountTree.clear();
    accountTable.clear();
    DynamicArray<User*> allUsers = userTable.collectAll();
    for (int index = 0; index < allUsers.size(); ++index) {
        delete allUsers[index];
    }
    userTable.clear();
    auditLog.clear();
    undoStack.clear();
    approvalQueue.clear();
    normalTokenQueue.clear();
    priorityTokenHeap.clear();
    loanRequestHeap.clear();
    transferGraph.clear();
    loans.clear();
    waitingTokens.clear();
    rules = BankRules();
    nextAccountNumber = 1001;
    nextTransactionId = 1;
    nextLoanId = 1;
    nextTokenNumber = 1;
    skippedRecordCount = 0;
}
