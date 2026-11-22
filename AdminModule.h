#ifndef ADMIN_MODULE_H
#define ADMIN_MODULE_H

#include "AccountModule.h"
#include "BankData.h"
#include "TransactionModule.h"

class AdminModule {
public:
    AdminModule(BankData& bankData, AccountModule& accountModule, TransactionModule& transactionModule);

    void addStaff();
    void removeStaff();
    void editStaffRole();
    void resetStaffPassword();
    void listStaff() const;
    void changeOwnPassword();

    void freezeAccount(Account& account);
    void unfreezeAccount(Account& account);
    void resetCustomerPin(Account& account);
    void unlockAccount(Account& account);
    void approveNewAccounts();

    void setBankRules();
    void backupData();
    bool restoreData();

private:
    BankData& bank;
    AccountModule& accounts;
    TransactionModule& transactions;

    User* promptForStaffUser();
    UserRole promptForRole();
    String promptForNewPassword();
    int countAdministrators() const;
};

#endif
