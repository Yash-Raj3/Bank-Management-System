#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "Models.h"
#include "String.h"
#include "TransactionList.h"

class Account {
public:
    int accountNumber = 0;
    String holderName;
    String cnic;
    AccountType accountType = AccountType::Savings;
    AccountLevel level = AccountLevel::Standard;
    AccountLevel requestedLevel = AccountLevel::Standard;
    double balance = 0;
    String pinHash;
    AccountStatus status = AccountStatus::Pending;
    String nomineeName;
    String nomineeRelation;
    int failedPinAttempts = 0;
    long long lastActivityTimestamp = 0;
    double withdrawnToday = 0;
    int withdrawalDateKey = 0;
    TransactionList history;

    bool isActive() const;
    void resetDailyCounterIfNewDay(int todayDateKey);
    double getDailyLimit(const BankRules& rules) const;
};

#endif 
