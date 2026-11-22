#include "ReportModule.h"

#include <iostream>

#include "TransactionModule.h"
#include "Utility.h"

ReportModule::ReportModule(BankData& bankData) : bank(bankData) {}

void ReportModule::printAccountStatement(const Account& account) const {
    Utility::printHeader("ACCOUNT STATEMENT");
    std::cout << "Account number : " << account.accountNumber << "\n";
    std::cout << "Holder name    : " << account.holderName << "\n";
    std::cout << "Account type   : " << accountTypeToString(account.accountType) << "\n";
    std::cout << "Account level  : " << accountLevelToString(account.level) << " Tier\n";
    std::cout << "Printed on     : " << Utility::formatTimestamp(Utility::currentTimestamp()) << "\n\n";
    TransactionModule::printTransactionTable(account.history.toArray());
    Utility::printSeparator();
    std::cout << "Closing balance: " << Utility::formatMoney(account.balance) << "\n";
}

void ReportModule::showDailySummary() const {
    Utility::printHeader("DAILY SUMMARY");
    int todayDateKey = Utility::dateKeyFromTimestamp(Utility::currentTimestamp());
    int countsPerType[TRANSACTION_TYPE_COUNT];
    double totalsPerType[TRANSACTION_TYPE_COUNT];
    for (int typeIndex = 0; typeIndex < TRANSACTION_TYPE_COUNT; ++typeIndex) {
        countsPerType[typeIndex] = 0;
        totalsPerType[typeIndex] = 0;
    }
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int accountIndex = 0; accountIndex < allAccounts.size(); ++accountIndex) {
        DynamicArray<Transaction> accountTransactions = allAccounts[accountIndex]->history.toArray();
        for (int index = 0; index < accountTransactions.size(); ++index) {
            if (Utility::dateKeyFromTimestamp(accountTransactions[index].timestamp) == todayDateKey) {
                int typeIndex = static_cast<int>(accountTransactions[index].type);
                ++countsPerType[typeIndex];
                totalsPerType[typeIndex] += accountTransactions[index].amount;
            }
        }
    }
    bool foundAnyTransaction = false;
    for (int typeIndex = 0; typeIndex < TRANSACTION_TYPE_COUNT; ++typeIndex) {
        if (countsPerType[typeIndex] > 0) {
            foundAnyTransaction = true;
            std::cout << Utility::padRight(transactionTypeToString(static_cast<TransactionType>(typeIndex)), 18)
                      << Utility::padRight(String::fromInteger(countsPerType[typeIndex]), 8)
                      << Utility::padLeft(Utility::formatMoney(totalsPerType[typeIndex]), 16) << "\n";
        }
    }
    if (!foundAnyTransaction) {
        std::cout << "No transactions were made today.\n";
    }
}

void ReportModule::showTopCustomers() const {
    Utility::printHeader("TOP CUSTOMERS");
    MaxHeap balanceHeap;
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        if (allAccounts[index]->isActive()) {
            HeapItem balanceItem{computeBalanceKey(allAccounts[index]->balance), allAccounts[index]->accountNumber};
            balanceHeap.push(balanceItem);
        }
    }
    if (balanceHeap.isEmpty()) {
        std::cout << "There are no active accounts.\n";
        return;
    }
    int requestedCount = Utility::readIntInRange("How many top accounts to show (1-50): ", 1, 50);
    HeapItem topItem{};
    for (int rank = 1; rank <= requestedCount && balanceHeap.pop(topItem); ++rank) {
        Account* account = bank.findAccount(topItem.itemId);
        std::cout << Utility::padRight(String::fromInteger(rank), 5)
                  << Utility::padRight(String::fromInteger(account->accountNumber), 10)
                  << Utility::padRight(account->holderName.substring(0, 22), 24)
                  << Utility::padLeft(Utility::formatMoney(account->balance), 14) << "\n";
    }
}

void ReportModule::showDormantAccounts() const {
    Utility::printHeader("DORMANT ACCOUNTS");
    int inactiveDays = Utility::readIntInRange("Inactive for at least how many days: ", 1, 3650);
    long long cutoffTimestamp = Utility::currentTimestamp() - static_cast<long long>(inactiveDays) * 86400LL;
    int dormantCount = 0;
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        Account* account = allAccounts[index];
        bool canBeDormant = account->status == AccountStatus::Active || account->status == AccountStatus::Frozen;
        if (canBeDormant && account->lastActivityTimestamp < cutoffTimestamp) {
            std::cout << account->accountNumber << " - " << account->holderName << " - last activity "
                      << Utility::formatTimestamp(account->lastActivityTimestamp) << "\n";
            ++dormantCount;
        }
    }
    if (dormantCount == 0) {
        std::cout << "No dormant accounts found.\n";
    }
}

void ReportModule::showTransferTracking() const {
    Utility::printHeader("TRANSFER TRACKING");
    int accountNumber = Utility::readIntInRange("Account number to track: ", 1, 999999999);
    if (bank.findAccount(accountNumber) == nullptr) {
        std::cout << "No account was found with that number.\n";
        return;
    }
    DynamicArray<TransferLink> outgoingLinks = bank.transferGraph.getOutgoingLinks(accountNumber);
    DynamicArray<TransferLink> incomingLinks = bank.transferGraph.getIncomingLinks(accountNumber);
    std::cout << "Money sent to:\n";
    for (int index = 0; index < outgoingLinks.size(); ++index) {
        std::cout << "  -> " << outgoingLinks[index].toAccount << " : " << Utility::formatMoney(outgoingLinks[index].totalAmount)
                  << " in " << outgoingLinks[index].transferCount << " transfer(s)\n";
    }
    if (outgoingLinks.size() == 0) {
        std::cout << "  (none)\n";
    }
    std::cout << "Money received from:\n";
    for (int index = 0; index < incomingLinks.size(); ++index) {
        std::cout << "  <- " << incomingLinks[index].fromAccount << " : " << Utility::formatMoney(incomingLinks[index].totalAmount)
                  << " in " << incomingLinks[index].transferCount << " transfer(s)\n";
    }
    if (incomingLinks.size() == 0) {
        std::cout << "  (none)\n";
    }
    std::cout << "Accounts reached by money from this account (directly or through others):";
    DynamicArray<int> reachableAccounts = bank.transferGraph.getReachableAccounts(accountNumber);
    for (int index = 0; index < reachableAccounts.size(); ++index) {
        std::cout << " " << reachableAccounts[index];
    }
    std::cout << (reachableAccounts.size() == 0 ? " (none)" : "") << "\n";
}

void ReportModule::showBankSummary() const {
    Utility::printHeader("BANK SUMMARY");
    int totalCustomers = 0;
    int pendingAccounts = 0;
    double totalDeposits = 0;
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        Account* account = allAccounts[index];
        if (account->status == AccountStatus::Pending) {
            ++pendingAccounts;
        } else if (account->status != AccountStatus::Closed) {
            ++totalCustomers;
            totalDeposits += account->balance;
        }
    }
    double totalLoansOutstanding = 0;
    for (int index = 0; index < bank.loans.size(); ++index) {
        if (bank.loans[index].status == LoanStatus::Approved) {
            totalLoansOutstanding += calculateOutstandingLoanAmount(bank.loans[index]);
        }
    }
    int activeStaff = 0;
    DynamicArray<User*> allUsers = bank.userTable.collectAll();
    for (int index = 0; index < allUsers.size(); ++index) {
        if (!allUsers[index]->isLocked) {
            ++activeStaff;
        }
    }
    std::cout << "Total customer accounts   : " << totalCustomers << "\n";
    std::cout << "Accounts awaiting approval: " << pendingAccounts << "\n";
    std::cout << "Total deposits            : " << Utility::formatMoney(totalDeposits) << "\n";
    std::cout << "Total loans outstanding   : " << Utility::formatMoney(totalLoansOutstanding) << "\n";
    std::cout << "Active staff              : " << activeStaff << "\n";
    std::cout << "Customers waiting in line : " << bank.waitingTokens.size() << "\n";
}

void ReportModule::showAuditLog() const {
    Utility::printHeader("AUDIT LOG");
    DynamicArray<AuditEntry> entries = bank.auditLog.toArray();
    if (entries.size() == 0) {
        std::cout << "The audit log is empty.\n";
        return;
    }
    for (int index = 0; index < entries.size(); ++index) {
        std::cout << Utility::formatTimestamp(entries[index].timestamp) << " | "
                  << Utility::padRight(entries[index].actorName, 16) << " | " << entries[index].actionDescription << "\n";
    }
}
