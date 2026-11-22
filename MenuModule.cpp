#include "MenuModule.h"

#include <iostream>

#include "FileManager.h"
#include "Utility.h"

namespace {

const MenuEntry COUNTER_MENU[] = {
    {"Create account request", MenuAction::CreateAccount},
    {"Search account", MenuAction::SearchAccount},
    {"Display all accounts", MenuAction::DisplayAccounts},
    {"Deposit", MenuAction::Deposit},
    {"Withdraw", MenuAction::Withdraw},
    {"Transfer funds", MenuAction::Transfer},
    {"Check balance", MenuAction::CheckBalance},
    {"Update account details", MenuAction::UpdateDetails},
    {"Close account", MenuAction::CloseAccount},
    {"Transaction history", MenuAction::History},
    {"Mini statement", MenuAction::MiniStatement},
    {"Account statement", MenuAction::Statement},
    {"Undo last transaction", MenuAction::UndoLast},
    {"Bill payment", MenuAction::BillPayment},
    {"Nominee details", MenuAction::Nominee},
    {"Apply for loan", MenuAction::ApplyLoan},
    {"Pay loan instalment", MenuAction::PayInstalment},
    {"Issue service token", MenuAction::IssueToken},
    {"Serve next customer", MenuAction::ServeToken},
    {"Show waiting customers", MenuAction::ShowTokens},
    {"Change my password", MenuAction::ChangePassword},
};
const int COUNTER_MENU_COUNT = sizeof(COUNTER_MENU) / sizeof(COUNTER_MENU[0]);

const MenuEntry MANAGEMENT_MENU[] = {
    {"Process loan requests", MenuAction::ProcessLoans},
    {"View all loans", MenuAction::ViewAllLoans},
    {"Freeze account", MenuAction::FreezeAccount},
    {"Unfreeze account", MenuAction::UnfreezeAccount},
    {"Unlock account", MenuAction::UnlockAccount},
    {"Reset customer PIN", MenuAction::ResetCustomerPin},
    {"Change account level / daily limit", MenuAction::ChangeAccountLevel},
    {"Process upgrade requests", MenuAction::ProcessUpgrades},
    {"Add interest to savings accounts", MenuAction::ApplyInterest},
    {"Dormant account check", MenuAction::DormantCheck},
    {"Top customers", MenuAction::TopCustomers},
    {"Transfer tracking", MenuAction::TransferTracking},
    {"Daily summary", MenuAction::DailySummary},
    {"Bank summary", MenuAction::BankSummary},
};
const int MANAGEMENT_MENU_COUNT = sizeof(MANAGEMENT_MENU) / sizeof(MANAGEMENT_MENU[0]);

const MenuEntry ADMINISTRATOR_MENU[] = {
    {"Add staff", MenuAction::AddStaff},
    {"Remove staff", MenuAction::RemoveStaff},
    {"Change staff role", MenuAction::ChangeRole},
    {"Reset staff password", MenuAction::ResetStaffPassword},
    {"List staff", MenuAction::ListStaff},
    {"Approve new accounts", MenuAction::ApproveAccounts},
    {"Set bank rules", MenuAction::SetRules},
    {"View audit log", MenuAction::AuditLogView},
    {"Search account", MenuAction::SearchAccount},
    {"Display all accounts", MenuAction::DisplayAccounts},
    {"Backup data", MenuAction::Backup},
    {"Restore data", MenuAction::Restore},
    {"Change my password", MenuAction::ChangePassword},
};
const int ADMINISTRATOR_MENU_COUNT = sizeof(ADMINISTRATOR_MENU) / sizeof(ADMINISTRATOR_MENU[0]);

const MenuEntry CUSTOMER_MENU[] = {
    {"Check balance", MenuAction::CheckBalance},
    {"Deposit", MenuAction::Deposit},
    {"Withdraw", MenuAction::Withdraw},
    {"Transfer funds", MenuAction::Transfer},
    {"Mini statement", MenuAction::MiniStatement},
    {"Transaction history", MenuAction::History},
    {"Account statement", MenuAction::Statement},
    {"Bill payment", MenuAction::BillPayment},
    {"Change PIN", MenuAction::ChangePin},
    {"Nominee details", MenuAction::Nominee},
    {"Apply for loan", MenuAction::ApplyLoan},
    {"Pay loan instalment", MenuAction::PayInstalment},
    {"View my loans", MenuAction::ViewAccountLoans},
    {"Request account upgrade", MenuAction::RequestUpgrade},
};
const int CUSTOMER_MENU_COUNT = sizeof(CUSTOMER_MENU) / sizeof(CUSTOMER_MENU[0]);

}

MenuModule::MenuModule(BankData& bankData)
    : bank(bankData),
      loginModule(bankData),
      transactionModule(bankData),
      accountModule(bankData, transactionModule),
      loanModule(bankData, transactionModule),
      serviceQueueModule(bankData),
      reportModule(bankData),
      adminModule(bankData, accountModule, transactionModule),
      forceLogout(false) {}

void MenuModule::runMenu(const String& title, const MenuEntry* firstEntries, int firstCount, const MenuEntry* secondEntries,
                         int secondCount, Account* customerAccount) {
    int totalCount = firstCount + secondCount;
    while (!forceLogout) {
        Utility::printHeader(title);
        for (int index = 0; index < totalCount; ++index) {
            const MenuEntry& entry = index < firstCount ? firstEntries[index] : secondEntries[index - firstCount];
            std::cout << index + 1 << ". " << entry.label << "\n";
        }
        std::cout << "0. Logout\n";
        int choice = Utility::readIntInRange("Choose an option: ", 0, totalCount);
        if (choice == 0) {
            return;
        }
        const MenuEntry& chosenEntry = choice <= firstCount ? firstEntries[choice - 1] : secondEntries[choice - 1 - firstCount];
        performAction(chosenEntry.action, customerAccount);
        if (!FileManager::saveAll(bank)) {
            std::cout << "Warning: data files could not be saved. Check the folder permissions.\n";
        }
        Utility::pauseForEnter();
    }
}

Account* MenuModule::resolveAccount(Account* customerAccount) {
    if (customerAccount != nullptr) {
        return customerAccount;
    }
    return accountModule.promptForAccount();
}

void MenuModule::performAction(MenuAction action, Account* customerAccount) {
    switch (action) {
        case MenuAction::CreateAccount: accountModule.createAccountRequest(); return;
        case MenuAction::SearchAccount: accountModule.searchAccount(); return;
        case MenuAction::DisplayAccounts: accountModule.displayAllAccounts(); return;
        case MenuAction::UndoLast: transactionModule.undoLastTransaction(); return;
        case MenuAction::IssueToken: serviceQueueModule.issueToken(); return;
        case MenuAction::ServeToken: serviceQueueModule.serveNextToken(); return;
        case MenuAction::ShowTokens: serviceQueueModule.showWaitingTokens(); return;
        case MenuAction::ProcessLoans: loanModule.processPendingLoans(); return;
        case MenuAction::ViewAllLoans: loanModule.showAllLoans(); return;
        case MenuAction::ApplyInterest: transactionModule.applyInterestToSavingsAccounts(); return;
        case MenuAction::DormantCheck: reportModule.showDormantAccounts(); return;
        case MenuAction::TopCustomers: reportModule.showTopCustomers(); return;
        case MenuAction::TransferTracking: reportModule.showTransferTracking(); return;
        case MenuAction::DailySummary: reportModule.showDailySummary(); return;
        case MenuAction::BankSummary: reportModule.showBankSummary(); return;
        case MenuAction::AddStaff: adminModule.addStaff(); return;
        case MenuAction::RemoveStaff: adminModule.removeStaff(); return;
        case MenuAction::ChangeRole: adminModule.editStaffRole(); return;
        case MenuAction::ResetStaffPassword: adminModule.resetStaffPassword(); return;
        case MenuAction::ListStaff: adminModule.listStaff(); return;
        case MenuAction::ChangePassword: adminModule.changeOwnPassword(); return;
        case MenuAction::ApproveAccounts: adminModule.approveNewAccounts(); return;
        case MenuAction::SetRules: adminModule.setBankRules(); return;
        case MenuAction::AuditLogView: reportModule.showAuditLog(); return;
        case MenuAction::Backup: adminModule.backupData(); return;
        case MenuAction::Restore: forceLogout = adminModule.restoreData(); return;
        case MenuAction::ProcessUpgrades: accountModule.processUpgradeRequests(); return;
        default: break;
    }

    Account* selectedAccount = resolveAccount(customerAccount);
    if (selectedAccount == nullptr) {
        return;
    }
    switch (action) {
        case MenuAction::Deposit: transactionModule.deposit(*selectedAccount); break;
        case MenuAction::Withdraw: transactionModule.withdraw(*selectedAccount); break;
        case MenuAction::Transfer: transactionModule.transfer(*selectedAccount); break;
        case MenuAction::CheckBalance: accountModule.checkBalance(*selectedAccount); break;
        case MenuAction::UpdateDetails: accountModule.updateAccountDetails(*selectedAccount); break;
        case MenuAction::CloseAccount: accountModule.closeAccount(*selectedAccount); break;
        case MenuAction::History: transactionModule.showHistory(*selectedAccount); break;
        case MenuAction::MiniStatement: transactionModule.showMiniStatement(*selectedAccount); break;
        case MenuAction::Statement: reportModule.printAccountStatement(*selectedAccount); break;
        case MenuAction::BillPayment: transactionModule.payBill(*selectedAccount); break;
        case MenuAction::Nominee: accountModule.updateNominee(*selectedAccount); break;
        case MenuAction::ApplyLoan: loanModule.applyForLoan(*selectedAccount); break;
        case MenuAction::PayInstalment: loanModule.payInstalment(*selectedAccount); break;
        case MenuAction::ViewAccountLoans: loanModule.showLoansOfAccount(*selectedAccount); break;
        case MenuAction::ChangePin: accountModule.changePin(*selectedAccount); break;
        case MenuAction::FreezeAccount: adminModule.freezeAccount(*selectedAccount); break;
        case MenuAction::UnfreezeAccount: adminModule.unfreezeAccount(*selectedAccount); break;
        case MenuAction::UnlockAccount: adminModule.unlockAccount(*selectedAccount); break;
        case MenuAction::ResetCustomerPin: adminModule.resetCustomerPin(*selectedAccount); break;
        case MenuAction::ChangeAccountLevel: accountModule.changeAccountLevel(*selectedAccount); break;
        case MenuAction::RequestUpgrade: accountModule.requestAccountUpgrade(*selectedAccount); break;
        default: break;
    }
}

void MenuModule::run() {
    while (true) {
        Utility::printHeader("         BANK MANAGEMENT SYSTEM           ");
        std::cout << "FAST NUCES PROJECT BY YASH RAJ AND KHUNAIS BAIG\n";
        Utility::printSeparator();
        std::cout << "1. Staff login\n2. Customer login\n3. Open a new account\n0. Exit\n";
        int choice = Utility::readIntInRange("Choose an option: ", 0, 3);
        if (choice == 0) {
            return;
        }
        if (choice == 1) {
            handleStaffLogin();
        } else if (choice == 2) {
            handleCustomerLogin();
        } else {
            handleNewAccountRequest();
        }
        if (!FileManager::saveAll(bank)) {
            std::cout << "Warning: data files could not be saved. Check the folder permissions.\n";
        }
    }
}

void MenuModule::handleStaffLogin() {
    User* user = loginModule.loginStaff();
    if (user == nullptr) {
        return;
    }
    String userId = user->userId;
    UserRole role = user->role;
    bank.currentActorName = userId;
    bank.logAction("Logged in as " + userRoleToString(role));
    if (role == UserRole::Administrator) {
        runMenu("ADMINISTRATOR MENU - " + userId, ADMINISTRATOR_MENU, ADMINISTRATOR_MENU_COUNT, MANAGEMENT_MENU,
                MANAGEMENT_MENU_COUNT, nullptr);
    } else if (role == UserRole::Manager) {
        runMenu("MANAGER MENU - " + userId, COUNTER_MENU, COUNTER_MENU_COUNT, MANAGEMENT_MENU, MANAGEMENT_MENU_COUNT, nullptr);
    } else {
        runMenu("TELLER MENU - " + userId, COUNTER_MENU, COUNTER_MENU_COUNT, nullptr, 0, nullptr);
    }
    bank.logAction("Logged out");
    bank.currentActorName = "system";
    forceLogout = false;
}

void MenuModule::handleNewAccountRequest() {
    bank.currentActorName = "new-customer";
    accountModule.createAccountRequest();
    bank.currentActorName = "system";
}

void MenuModule::handleCustomerLogin() {
    Account* customerAccount = loginModule.loginCustomer();
    if (customerAccount == nullptr) {
        return;
    }
    bank.currentActorName = "customer:" + String::fromInteger(customerAccount->accountNumber);
    bank.logAction("Logged in");
    runMenu("CUSTOMER MENU - ACCOUNT " + String::fromInteger(customerAccount->accountNumber), CUSTOMER_MENU,
            CUSTOMER_MENU_COUNT, nullptr, 0, customerAccount);
    bank.logAction("Logged out");
    bank.currentActorName = "system";
}
