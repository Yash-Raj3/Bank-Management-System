#ifndef MENU_MODULE_H
#define MENU_MODULE_H

#include "AccountModule.h"
#include "AdminModule.h"
#include "BankData.h"
#include "LoanModule.h"
#include "LoginModule.h"
#include "ReportModule.h"
#include "ServiceQueueModule.h"
#include "String.h"
#include "TransactionModule.h"

enum class MenuAction {
    CreateAccount,
    SearchAccount,
    DisplayAccounts,
    Deposit,
    Withdraw,
    Transfer,
    CheckBalance,
    UpdateDetails,
    CloseAccount,
    History,
    MiniStatement,
    Statement,
    UndoLast,
    BillPayment,
    Nominee,
    ApplyLoan,
    PayInstalment,
    ViewAccountLoans,
    ChangePin,
    ChangePassword,
    IssueToken,
    ServeToken,
    ShowTokens,
    ProcessLoans,
    ViewAllLoans,
    FreezeAccount,
    UnfreezeAccount,
    UnlockAccount,
    ResetCustomerPin,
    ApplyInterest,
    DormantCheck,
    TopCustomers,
    TransferTracking,
    DailySummary,
    BankSummary,
    AddStaff,
    RemoveStaff,
    ChangeRole,
    ResetStaffPassword,
    ListStaff,
    ApproveAccounts,
    SetRules,
    AuditLogView,
    Backup,
    Restore,
    ChangeAccountLevel,
    RequestUpgrade,
    ProcessUpgrades
};

class MenuEntry {
public:
    const char* label;
    MenuAction action;
};

class MenuModule {
public:
    explicit MenuModule(BankData& bankData);

    void run();

private:
    BankData& bank;
    LoginModule loginModule;
    TransactionModule transactionModule;
    AccountModule accountModule;
    LoanModule loanModule;
    ServiceQueueModule serviceQueueModule;
    ReportModule reportModule;
    AdminModule adminModule;
    bool forceLogout;

    void runMenu(const String& title, const MenuEntry* firstEntries, int firstCount, const MenuEntry* secondEntries,
                 int secondCount, Account* customerAccount);
    void performAction(MenuAction action, Account* customerAccount);
    Account* resolveAccount(Account* customerAccount);

    void handleStaffLogin();
    void handleCustomerLogin();
    void handleNewAccountRequest();
};

#endif
