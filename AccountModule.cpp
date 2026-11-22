#include "AccountModule.h"

#include <iostream>

#include "Utility.h"

namespace {

bool comesBeforeByName(const Account* firstAccount, const Account* secondAccount) {
    return firstAccount->holderName.toLowerCase() < secondAccount->holderName.toLowerCase();
}

bool comesBeforeByBalance(const Account* firstAccount, const Account* secondAccount) {
    return firstAccount->balance > secondAccount->balance;
}

void sortAccounts(DynamicArray<Account*>& accounts, bool (*comesBefore)(const Account*, const Account*)) {
    for (int current = 1; current < accounts.size(); ++current) {
        Account* movingAccount = accounts[current];
        int position = current - 1;
        while (position >= 0 && comesBefore(movingAccount, accounts[position])) {
            accounts[position + 1] = accounts[position];
            --position;
        }
        accounts[position + 1] = movingAccount;
    }
}

String describeNominee(const Account& account) {
    if (account.nomineeName.isEmpty()) {
        return "Not set";
    }
    return account.nomineeName + " (" + account.nomineeRelation + ")";
}

}

AccountModule::AccountModule(BankData& bankData, TransactionModule& transactionModule)
    : bank(bankData), transactions(transactionModule) {}

Account* AccountModule::promptForAccount() {
    int accountNumber = Utility::readIntInRange("Enter account number: ", 1, 999999999);
    Account* foundAccount = bank.findAccount(accountNumber);
    if (foundAccount == nullptr) {
        std::cout << "No account was found with that number.\n";
    }
    return foundAccount;
}

void AccountModule::showAccountDetails(const Account& account) const {
    Utility::printHeader("ACCOUNT " + String::fromInteger(account.accountNumber));
    std::cout << "Holder name   : " << account.holderName << "\n";
    std::cout << "CNIC          : " << account.cnic << "\n";
    std::cout << "Account type  : " << accountTypeToString(account.accountType) << "\n";
    std::cout << "Tier / Level  : " << accountLevelToString(account.level) << "\n";
    if (account.requestedLevel > account.level) {
        std::cout << "Pending Tier  : " << accountLevelToString(account.requestedLevel) << " (Upgrade Requested)\n";
    }
    std::cout << "Daily limit   : " << Utility::formatMoney(account.getDailyLimit(bank.rules)) << "\n";
    std::cout << "Status        : " << accountStatusToString(account.status) << "\n";
    std::cout << "Balance       : " << Utility::formatMoney(account.balance) << "\n";
    std::cout << "Nominee       : " << describeNominee(account) << "\n";
    std::cout << "Last activity : " << Utility::formatTimestamp(account.lastActivityTimestamp) << "\n";
}

void AccountModule::printAccountTable(const DynamicArray<Account*>& accounts) const {
    if (accounts.size() == 0) {
        std::cout << "No accounts found.\n";
        return;
    }
    std::cout << Utility::padRight("Number", 8) << Utility::padRight("Holder name", 20) << Utility::padRight("Type", 9)
              << Utility::padRight("Tier", 10) << Utility::padRight("Status", 9) << Utility::padLeft("Balance", 12) << "\n";
    Utility::printSeparator();
    for (int index = 0; index < accounts.size(); ++index) {
        const Account* account = accounts[index];
        std::cout << Utility::padRight(String::fromInteger(account->accountNumber), 8)
                  << Utility::padRight(account->holderName.substring(0, 18), 20)
                  << Utility::padRight(accountTypeToString(account->accountType), 9)
                  << Utility::padRight(accountLevelToString(account->level), 10)
                  << Utility::padRight(accountStatusToString(account->status), 9)
                  << Utility::padLeft(Utility::formatMoney(account->balance), 12) << "\n";
    }
}

void AccountModule::createAccountRequest() {
    Utility::printHeader("CREATE NEW ACCOUNT");
    String holderName = Utility::readNonEmptyLine("Holder name: ");
    String cnic = Utility::readDigits("CNIC (13 digits, no dashes): ", 13);
    std::cout << "1. Savings\n2. Current\n";
    AccountType accountType = Utility::readIntInRange("Account type: ", 1, 2) == 1 ? AccountType::Savings : AccountType::Current;

    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        Account* existingAccount = allAccounts[index];
        if (existingAccount->cnic == cnic && existingAccount->accountType == accountType &&
            existingAccount->status != AccountStatus::Closed) {
            std::cout << "This customer already has a " << accountTypeToString(accountType) << " account ("
                      << existingAccount->accountNumber << ").\n";
            return;
        }
    }

    double openingDeposit = 0;
    while (true) {
        openingDeposit = Utility::readPositiveAmount("Opening deposit: ");
        if (openingDeposit >= bank.rules.minimumBalance) {
            break;
        }
        std::cout << "The opening deposit must be at least " << Utility::formatMoney(bank.rules.minimumBalance) << ".\n";
    }

    String chosenPin;
    while (true) {
        chosenPin = Utility::readDigits("Set a 4-digit PIN: ", 4);
        if (chosenPin == Utility::readDigits("Confirm PIN: ", 4)) {
            break;
        }
        std::cout << "The PINs did not match. Try again.\n";
    }

    String nomineeName = Utility::readLine("Nominee name (press Enter to skip): ");
    String nomineeRelation;
    if (!nomineeName.isEmpty()) {
        nomineeRelation = Utility::readNonEmptyLine("Nominee relation: ");
    }

    Account* newAccount = new Account();
    newAccount->accountNumber = bank.nextAccountNumber++;
    newAccount->holderName = holderName;
    newAccount->cnic = cnic;
    newAccount->accountType = accountType;
    newAccount->balance = openingDeposit;
    newAccount->pinHash = Utility::hashSecret(chosenPin);
    newAccount->status = AccountStatus::Pending;
    newAccount->nomineeName = nomineeName;
    newAccount->nomineeRelation = nomineeRelation;
    newAccount->lastActivityTimestamp = Utility::currentTimestamp();
    bank.addAccount(newAccount);
    bank.approvalQueue.enqueue(newAccount->accountNumber);
    bank.logAction("Requested new " + accountTypeToString(accountType) + " account " +
                   String::fromInteger(newAccount->accountNumber) + " for " + holderName);
    std::cout << "Account request created. Account number: " << newAccount->accountNumber
              << ". It will be active after administrator approval.\n";
}

void AccountModule::searchAccount() {
    Utility::printHeader("SEARCH ACCOUNT");
    std::cout << "1. By account number\n2. By holder name\n3. By CNIC\n";
    int searchChoice = Utility::readIntInRange("Search by: ", 1, 3);
    DynamicArray<Account*> matchingAccounts;
    if (searchChoice == 1) {
        int accountNumber = Utility::readIntInRange("Account number: ", 1, 999999999);
        Account* foundAccount = bank.findAccount(accountNumber);
        if (foundAccount != nullptr) {
            matchingAccounts.add(foundAccount);
        }
    } else if (searchChoice == 2) {
        String searchedName = Utility::readNonEmptyLine("Name (or part of it): ").toLowerCase();
        DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
        for (int index = 0; index < allAccounts.size(); ++index) {
            if (allAccounts[index]->holderName.toLowerCase().contains(searchedName)) {
                matchingAccounts.add(allAccounts[index]);
            }
        }
    } else {
        String searchedCnic = Utility::readDigits("CNIC (13 digits): ", 13);
        DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
        for (int index = 0; index < allAccounts.size(); ++index) {
            if (allAccounts[index]->cnic == searchedCnic) {
                matchingAccounts.add(allAccounts[index]);
            }
        }
    }
    printAccountTable(matchingAccounts);
}

void AccountModule::displayAllAccounts() {
    Utility::printHeader("ALL ACCOUNTS");
    std::cout << "1. Sort by account number\n2. Sort by holder name\n3. Sort by balance (highest first)\n";
    int sortChoice = Utility::readIntInRange("Sort order: ", 1, 3);
    DynamicArray<Account*> sortedAccounts = bank.accountTree.collectInOrder();
    if (sortChoice == 2) {
        sortAccounts(sortedAccounts, comesBeforeByName);
    } else if (sortChoice == 3) {
        sortAccounts(sortedAccounts, comesBeforeByBalance);
    }
    printAccountTable(sortedAccounts);
}

void AccountModule::updateAccountDetails(Account& account) {
    if (account.status == AccountStatus::Closed) {
        std::cout << "A closed account cannot be updated.\n";
        return;
    }
    showAccountDetails(account);
    std::cout << "1. Change holder name\n2. Change CNIC\n";
    int updateChoice = Utility::readIntInRange("What do you want to change: ", 1, 2);
    if (updateChoice == 1) {
        account.holderName = Utility::readNonEmptyLine("New holder name: ");
    } else {
        account.cnic = Utility::readDigits("New CNIC (13 digits): ", 13);
    }
    bank.logAction("Updated details of account " + String::fromInteger(account.accountNumber));
    std::cout << "Account details updated.\n";
}

void AccountModule::closeAccount(Account& account) {
    if (account.status != AccountStatus::Active) {
        std::cout << "Only an active account can be closed. Current status: " << accountStatusToString(account.status) << ".\n";
        return;
    }
    for (int index = 0; index < bank.loans.size(); ++index) {
        const Loan& loan = bank.loans[index];
        if (loan.accountNumber == account.accountNumber &&
            (loan.status == LoanStatus::Approved || loan.status == LoanStatus::Pending)) {
            std::cout << "This account has an unfinished loan and cannot be closed.\n";
            return;
        }
    }
    std::cout << "Remaining balance to be paid out: " << Utility::formatMoney(account.balance) << "\n";
    if (!Utility::readYesNo("Close account " + String::fromInteger(account.accountNumber) + "?")) {
        std::cout << "Closing cancelled.\n";
        return;
    }
    transactions.payOutFullBalance(account);
    account.status = AccountStatus::Closed;
    bank.logAction("Closed account " + String::fromInteger(account.accountNumber));
    std::cout << "Account closed.\n";
}

void AccountModule::checkBalance(const Account& account) const {
    showAccountDetails(account);
    std::cout << "Remaining allowance today: "
              << Utility::formatMoney(Utility::maximumOf(0.0, account.getDailyLimit(bank.rules) - account.withdrawnToday)) << "\n";
}

void AccountModule::changePin(Account& account) {
    String oldPin = Utility::readDigits("Current PIN: ", 4);
    if (Utility::hashSecret(oldPin) != account.pinHash) {
        std::cout << "Incorrect current PIN.\n";
        return;
    }
    String newPin = Utility::readDigits("New 4-digit PIN: ", 4);
    if (newPin != Utility::readDigits("Confirm new PIN: ", 4)) {
        std::cout << "The PINs did not match. PIN was not changed.\n";
        return;
    }
    account.pinHash = Utility::hashSecret(newPin);
    bank.logAction("PIN changed for account " + String::fromInteger(account.accountNumber));
    std::cout << "PIN changed successfully.\n";
}

void AccountModule::updateNominee(Account& account) {
    std::cout << "Current nominee: " << describeNominee(account) << "\n";
    account.nomineeName = Utility::readNonEmptyLine("Nominee name: ");
    account.nomineeRelation = Utility::readNonEmptyLine("Nominee relation: ");
    bank.logAction("Updated nominee of account " + String::fromInteger(account.accountNumber));
    std::cout << "Nominee saved.\n";
}

void AccountModule::changeAccountLevel(Account& account) {
    Utility::printHeader("CHANGE ACCOUNT TIER / DAILY LIMIT");
    std::cout << "Account Number: " << account.accountNumber << "\n";
    std::cout << "Holder Name   : " << account.holderName << "\n";
    std::cout << "Current Tier  : " << accountLevelToString(account.level) << "\n";
    std::cout << "Current Limit : " << Utility::formatMoney(account.getDailyLimit(bank.rules)) << "\n\n";

    std::cout << "Select new tier:\n";
    std::cout << "1. Standard (" << Utility::formatMoney(dailyLimitForLevel(AccountLevel::Standard, bank.rules.dailyWithdrawalLimit)) << " / day)\n";
    std::cout << "2. Silver   (" << Utility::formatMoney(dailyLimitForLevel(AccountLevel::Silver, bank.rules.dailyWithdrawalLimit)) << " / day)\n";
    std::cout << "3. Gold     (" << Utility::formatMoney(dailyLimitForLevel(AccountLevel::Gold, bank.rules.dailyWithdrawalLimit)) << " / day)\n";
    std::cout << "4. Platinum (" << Utility::formatMoney(dailyLimitForLevel(AccountLevel::Platinum, bank.rules.dailyWithdrawalLimit)) << " / day)\n";
    std::cout << "0. Cancel\n";

    int choice = Utility::readIntInRange("Choose tier (0-4): ", 0, 4);
    if (choice == 0) {
        std::cout << "Operation cancelled.\n";
        return;
    }

    AccountLevel newLevel = static_cast<AccountLevel>(choice);
    if (newLevel == account.level) {
        std::cout << "The account is already at " << accountLevelToString(newLevel) << " tier.\n";
        return;
    }

    account.level = newLevel;
    bank.logAction("Changed level of account " + String::fromInteger(account.accountNumber) + " to " + accountLevelToString(newLevel));
    std::cout << "Account tier updated successfully to " << accountLevelToString(newLevel) << "!\n";
    std::cout << "New daily withdrawal limit: " << Utility::formatMoney(account.getDailyLimit(bank.rules)) << "\n";
}

void AccountModule::requestAccountUpgrade(Account& account) {
    Utility::printHeader("REQUEST ACCOUNT UPGRADE");
    std::cout << "Account Number : " << account.accountNumber << "\n";
    std::cout << "Current Tier   : " << accountLevelToString(account.level) << "\n";
    std::cout << "Daily Limit    : " << Utility::formatMoney(account.getDailyLimit(bank.rules)) << "\n";
    std::cout << "Current Balance: " << Utility::formatMoney(account.balance) << "\n";
    std::cout << "Transactions   : " << account.history.size() << "\n\n";

    if (!account.isActive()) {
        std::cout << "Only active accounts can request an upgrade.\n";
        return;
    }

    if (account.level == AccountLevel::Platinum) {
        std::cout << "Your account is already at the highest tier (Platinum).\n";
        return;
    }

    if (account.requestedLevel > account.level) {
        std::cout << "You already have a pending upgrade request to "
                  << accountLevelToString(account.requestedLevel) << " tier. Please wait for staff approval.\n";
        return;
    }

    std::cout << "Available Tiers & Qualification Requirements:\n";
    Utility::printSeparator();
    if (account.level < AccountLevel::Silver) {
        std::cout << "1. Silver   - Daily Limit: "
                  << Utility::formatMoney(dailyLimitForLevel(AccountLevel::Silver, bank.rules.dailyWithdrawalLimit))
                  << "\n   Requirements: Balance >= "
                  << Utility::formatMoney(requiredBalanceForLevel(AccountLevel::Silver))
                  << " and >= " << requiredTransactionsForLevel(AccountLevel::Silver) << " completed transactions.\n";
    }
    if (account.level < AccountLevel::Gold) {
        std::cout << "2. Gold     - Daily Limit: "
                  << Utility::formatMoney(dailyLimitForLevel(AccountLevel::Gold, bank.rules.dailyWithdrawalLimit))
                  << "\n   Requirements: Balance >= "
                  << Utility::formatMoney(requiredBalanceForLevel(AccountLevel::Gold))
                  << " and >= " << requiredTransactionsForLevel(AccountLevel::Gold) << " completed transactions.\n";
    }
    if (account.level < AccountLevel::Platinum) {
        std::cout << "3. Platinum - Daily Limit: "
                  << Utility::formatMoney(dailyLimitForLevel(AccountLevel::Platinum, bank.rules.dailyWithdrawalLimit))
                  << "\n   Requirements: Balance >= "
                  << Utility::formatMoney(requiredBalanceForLevel(AccountLevel::Platinum))
                  << " and >= " << requiredTransactionsForLevel(AccountLevel::Platinum) << " completed transactions.\n";
    }
    std::cout << "0. Cancel\n";
    Utility::printSeparator();

    int choice = Utility::readIntInRange("Choose tier to apply for: ", 0, 3);
    if (choice == 0) {
        std::cout << "Upgrade request cancelled.\n";
        return;
    }

    AccountLevel targetLevel = AccountLevel::Standard;
    if (choice == 1) {
        targetLevel = AccountLevel::Silver;
    } else if (choice == 2) {
        targetLevel = AccountLevel::Gold;
    } else if (choice == 3) {
        targetLevel = AccountLevel::Platinum;
    }

    if (targetLevel <= account.level) {
        std::cout << "You cannot apply for a tier equal to or lower than your current level.\n";
        return;
    }

    double reqBalance = requiredBalanceForLevel(targetLevel);
    int reqTransactions = requiredTransactionsForLevel(targetLevel);
    int currentTransactions = account.history.size();

    bool balanceOk = account.balance >= reqBalance - 0.001;
    bool transOk = currentTransactions >= reqTransactions;

    if (!balanceOk || !transOk) {
        std::cout << "\nCannot submit request: Requirements not completed for "
                  << accountLevelToString(targetLevel) << " tier.\n";
        if (!balanceOk) {
            std::cout << "- Balance requirement: " << Utility::formatMoney(reqBalance)
                      << " (Current: " << Utility::formatMoney(account.balance)
                      << ", Short by " << Utility::formatMoney(reqBalance - account.balance) << ")\n";
        }
        if (!transOk) {
            std::cout << "- Transaction count requirement: " << reqTransactions
                      << " transactions (Current: " << currentTransactions
                      << ", Short by " << (reqTransactions - currentTransactions) << ")\n";
        }
        return;
    }

    account.requestedLevel = targetLevel;
    bank.logAction("Upgrade request to " + accountLevelToString(targetLevel) + " submitted for account " +
                   String::fromInteger(account.accountNumber));
    std::cout << "\nRequirements verified! Your upgrade request to "
              << accountLevelToString(targetLevel) << " tier has been submitted successfully.\n";
    std::cout << "A bank manager or administrator will review and activate your upgraded limit.\n";
}

void AccountModule::processUpgradeRequests() {
    Utility::printHeader("PROCESS ACCOUNT UPGRADE REQUESTS");
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    DynamicArray<Account*> pendingUpgrades;
    for (int index = 0; index < allAccounts.size(); ++index) {
        if (allAccounts[index]->requestedLevel > allAccounts[index]->level) {
            pendingUpgrades.add(allAccounts[index]);
        }
    }

    if (pendingUpgrades.size() == 0) {
        std::cout << "There are no pending account upgrade requests.\n";
        return;
    }

    for (int index = 0; index < pendingUpgrades.size(); ++index) {
        Account* account = pendingUpgrades[index];
        Utility::printSeparator();
        std::cout << "Request " << (index + 1) << " of " << pendingUpgrades.size() << "\n";
        std::cout << "Account Number: " << account->accountNumber << "\n";
        std::cout << "Holder Name   : " << account->holderName << "\n";
        std::cout << "Current Balance: " << Utility::formatMoney(account->balance) << "\n";
        std::cout << "Current Tier  : " << accountLevelToString(account->level)
                  << " (" << Utility::formatMoney(account->getDailyLimit(bank.rules)) << " / day)\n";
        std::cout << "Requested Tier: " << accountLevelToString(account->requestedLevel)
                  << " (" << Utility::formatMoney(dailyLimitForLevel(account->requestedLevel, bank.rules.dailyWithdrawalLimit)) << " / day)\n";
        std::cout << "Total Transactions: " << account->history.size() << "\n";

        std::cout << "1. Approve upgrade\n2. Reject request\n3. Skip for now\n0. Stop reviewing\n";
        int decision = Utility::readIntInRange("Decision: ", 0, 3);
        if (decision == 1) {
            account->level = account->requestedLevel;
            account->requestedLevel = AccountLevel::Standard;
            bank.logAction("Approved upgrade of account " + String::fromInteger(account->accountNumber) +
                           " to " + accountLevelToString(account->level));
            std::cout << "Account " << account->accountNumber << " upgraded to "
                      << accountLevelToString(account->level) << " tier successfully!\n";
        } else if (decision == 2) {
            account->requestedLevel = AccountLevel::Standard;
            bank.logAction("Rejected upgrade of account " + String::fromInteger(account->accountNumber));
            std::cout << "Upgrade request rejected.\n";
        } else if (decision == 0) {
            break;
        }
    }
}
