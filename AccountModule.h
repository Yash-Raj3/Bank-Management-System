#ifndef ACCOUNT_MODULE_H
#define ACCOUNT_MODULE_H

#include "BankData.h"
#include "DynamicArray.h"
#include "TransactionModule.h"

class AccountModule {
public:
    AccountModule(BankData& bankData, TransactionModule& transactionModule);

    Account* promptForAccount();
    void showAccountDetails(const Account& account) const;
    void printAccountTable(const DynamicArray<Account*>& accounts) const;

    void createAccountRequest();
    void searchAccount();
    void displayAllAccounts();
    void updateAccountDetails(Account& account);
    void closeAccount(Account& account);
    void checkBalance(const Account& account) const;
    void changePin(Account& account);
    void updateNominee(Account& account);
    void changeAccountLevel(Account& account);
    void requestAccountUpgrade(Account& account);
    void processUpgradeRequests();

private:
    BankData& bank;
    TransactionModule& transactions;
};

#endif
