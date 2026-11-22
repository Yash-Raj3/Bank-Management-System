#ifndef TRANSACTION_MODULE_H
#define TRANSACTION_MODULE_H

#include "BankData.h"
#include "DynamicArray.h"
#include "String.h"

class TransactionModule {
public:
    explicit TransactionModule(BankData& bankData);

    void deposit(Account& account);
    void withdraw(Account& account);
    void transfer(Account& sourceAccount);
    void undoLastTransaction();
    void showHistory(const Account& account) const;
    void showMiniStatement(const Account& account) const;
    void payBill(Account& account);
    void applyInterestToSavingsAccounts();

    bool debitForService(Account& account, double amount, TransactionType type, String& errorMessage);
    void creditForService(Account& account, double amount, TransactionType type);
    void payOutFullBalance(Account& account);

    static void printTransactionTable(const DynamicArray<Transaction>& transactions);

private:
    BankData& bank;

    Transaction recordTransaction(Account& account, TransactionType type, double amount, int relatedAccountNumber);
    bool checkAccountIsUsable(const Account& account, String& errorMessage) const;
    bool debitAccount(Account& account, double amount, TransactionType type, int relatedAccountNumber,
                      bool countsTowardDailyLimit, String& errorMessage);
    void creditAccount(Account& account, double amount, TransactionType type, int relatedAccountNumber);
    void pushUndoRecord(const Account& account, TransactionType type, int relatedAccountNumber, double amount);
    void restoreDailyAllowance(Account& account, double amount);
};

#endif
