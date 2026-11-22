#include "AdminModule.h"

#include <iostream>

#include "FileManager.h"
#include "Utility.h"

AdminModule::AdminModule(BankData& bankData, AccountModule& accountModule, TransactionModule& transactionModule)
    : bank(bankData), accounts(accountModule), transactions(transactionModule) {}

User* AdminModule::promptForStaffUser() {
    String userId = Utility::readNonEmptyLine("Staff user ID: ");
    User* user = bank.userTable.find(userId);
    if (user == nullptr) {
        std::cout << "No staff user was found with that ID.\n";
    }
    return user;
}

UserRole AdminModule::promptForRole() {
    std::cout << "1. Administrator\n2. Manager\n3. Teller\n";
    int roleChoice = Utility::readIntInRange("Role: ", 1, 3);
    return static_cast<UserRole>(roleChoice - 1);
}

String AdminModule::promptForNewPassword() {
    while (true) {
        String password = Utility::readNonEmptyLine("Password (at least 4 characters): ");
        if (password.length() >= 4) {
            return password;
        }
        std::cout << "The password is too short.\n";
    }
}

int AdminModule::countAdministrators() const {
    int administratorCount = 0;
    DynamicArray<User*> allUsers = bank.userTable.collectAll();
    for (int index = 0; index < allUsers.size(); ++index) {
        if (allUsers[index]->role == UserRole::Administrator) {
            ++administratorCount;
        }
    }
    return administratorCount;
}

void AdminModule::addStaff() {
    Utility::printHeader("ADD STAFF");
    String userId = Utility::readNonEmptyLine("New user ID (no spaces): ");
    if (userId.indexOfChar(' ') != -1) {
        std::cout << "The user ID cannot contain spaces.\n";
        return;
    }
    if (bank.userTable.find(userId) != nullptr) {
        std::cout << "This user ID already exists.\n";
        return;
    }
    User* newUser = new User();
    newUser->userId = userId;
    newUser->passwordHash = Utility::hashSecret(promptForNewPassword());
    newUser->role = promptForRole();
    bank.userTable.insert(newUser);
    bank.logAction("Added staff " + userId + " as " + userRoleToString(newUser->role));
    std::cout << "Staff user created.\n";
}

void AdminModule::removeStaff() {
    Utility::printHeader("REMOVE STAFF");
    User* user = promptForStaffUser();
    if (user == nullptr) {
        return;
    }
    if (user->userId == bank.currentActorName) {
        std::cout << "You cannot remove the user you are logged in as.\n";
        return;
    }
    if (user->role == UserRole::Administrator && countAdministrators() <= 1) {
        std::cout << "The last administrator cannot be removed.\n";
        return;
    }
    if (!Utility::readYesNo("Remove " + user->userId + "?")) {
        return;
    }
    String removedUserId = user->userId;
    bank.userTable.remove(removedUserId);
    delete user;
    bank.logAction("Removed staff " + removedUserId);
    std::cout << "Staff user removed.\n";
}

void AdminModule::editStaffRole() {
    Utility::printHeader("EDIT STAFF ROLE");
    User* user = promptForStaffUser();
    if (user == nullptr) {
        return;
    }
    UserRole newRole = promptForRole();
    if (user->role == UserRole::Administrator && newRole != UserRole::Administrator && countAdministrators() <= 1) {
        std::cout << "The last administrator cannot lose the administrator role.\n";
        return;
    }
    user->role = newRole;
    bank.logAction("Changed role of " + user->userId + " to " + userRoleToString(newRole));
    std::cout << "Role updated.\n";
}

void AdminModule::resetStaffPassword() {
    Utility::printHeader("RESET STAFF PASSWORD");
    User* user = promptForStaffUser();
    if (user == nullptr) {
        return;
    }
    user->passwordHash = Utility::hashSecret(promptForNewPassword());
    user->failedLoginAttempts = 0;
    user->isLocked = false;
    bank.logAction("Reset password of staff " + user->userId);
    std::cout << "Password reset and user unlocked.\n";
}

void AdminModule::changeOwnPassword() {
    User* user = bank.userTable.find(bank.currentActorName);
    if (user == nullptr) {
        std::cout << "No staff user is logged in.\n";
        return;
    }
    String currentPassword = Utility::readLine("Current password: ");
    if (user->passwordHash != Utility::hashSecret(currentPassword)) {
        std::cout << "Incorrect current password.\n";
        return;
    }
    user->passwordHash = Utility::hashSecret(promptForNewPassword());
    bank.logAction("Changed own password");
    std::cout << "Password changed successfully.\n";
}

void AdminModule::listStaff() const {
    Utility::printHeader("STAFF LIST");
    DynamicArray<User*> allUsers = bank.userTable.collectAll();
    for (int index = 0; index < allUsers.size(); ++index) {
        std::cout << Utility::padRight(allUsers[index]->userId, 20) << Utility::padRight(userRoleToString(allUsers[index]->role), 16)
                  << (allUsers[index]->isLocked ? "Locked" : "Active") << "\n";
    }
}

void AdminModule::freezeAccount(Account& account) {
    if (account.status != AccountStatus::Active) {
        std::cout << "Only an active account can be frozen. Current status: " << accountStatusToString(account.status) << ".\n";
        return;
    }
    account.status = AccountStatus::Frozen;
    bank.logAction("Froze account " + String::fromInteger(account.accountNumber));
    std::cout << "Account frozen.\n";
}

void AdminModule::unfreezeAccount(Account& account) {
    if (account.status != AccountStatus::Frozen) {
        std::cout << "This account is not frozen.\n";
        return;
    }
    account.status = AccountStatus::Active;
    bank.logAction("Unfroze account " + String::fromInteger(account.accountNumber));
    std::cout << "Account is active again.\n";
}

void AdminModule::resetCustomerPin(Account& account) {
    String newPin = Utility::readDigits("New 4-digit PIN for the customer: ", 4);
    account.pinHash = Utility::hashSecret(newPin);
    account.failedPinAttempts = 0;
    bank.logAction("Reset PIN of account " + String::fromInteger(account.accountNumber));
    std::cout << "PIN has been reset.\n";
}

void AdminModule::unlockAccount(Account& account) {
    if (account.status != AccountStatus::Locked) {
        std::cout << "This account is not locked.\n";
        return;
    }
    account.status = AccountStatus::Active;
    account.failedPinAttempts = 0;
    bank.logAction("Unlocked account " + String::fromInteger(account.accountNumber));
    std::cout << "Account unlocked.\n";
}

void AdminModule::approveNewAccounts() {
    Utility::printHeader("APPROVE NEW ACCOUNTS");
    if (bank.approvalQueue.isEmpty()) {
        std::cout << "There are no pending account requests.\n";
        return;
    }
    DynamicArray<int> skippedAccountNumbers;
    bool stopReviewing = false;
    int accountNumber = 0;
    while (!stopReviewing && bank.approvalQueue.dequeue(accountNumber)) {
        Account* pendingAccount = bank.findAccount(accountNumber);
        if (pendingAccount == nullptr || pendingAccount->status != AccountStatus::Pending) {
            continue;
        }
        accounts.showAccountDetails(*pendingAccount);
        std::cout << "1. Approve\n2. Reject\n3. Skip for now\n0. Stop reviewing\n";
        int decision = Utility::readIntInRange("Decision: ", 0, 3);
        if (decision == 1) {
            double openingDeposit = pendingAccount->balance;
            pendingAccount->balance = 0;
            pendingAccount->status = AccountStatus::Active;
            transactions.creditForService(*pendingAccount, openingDeposit, TransactionType::Deposit);
            bank.logAction("Approved account " + String::fromInteger(accountNumber));
            std::cout << "Account " << accountNumber << " approved.\n";
        } else if (decision == 2) {
            bank.logAction("Rejected account request " + String::fromInteger(accountNumber));
            bank.removeAndDeleteAccount(accountNumber);
            std::cout << "Account request rejected and removed.\n";
        } else {
            skippedAccountNumbers.add(accountNumber);
            stopReviewing = decision == 0;
        }
    }
    for (int index = 0; index < skippedAccountNumbers.size(); ++index) {
        bank.approvalQueue.enqueue(skippedAccountNumbers[index]);
    }
}

void AdminModule::setBankRules() {
    Utility::printHeader("BANK RULES");
    std::cout << "1. Interest rate         : " << Utility::formatMoney(bank.rules.interestRatePercent) << " %\n";
    std::cout << "2. Minimum balance       : " << Utility::formatMoney(bank.rules.minimumBalance) << "\n";
    std::cout << "3. Base daily limit      : " << Utility::formatMoney(bank.rules.dailyWithdrawalLimit) << " (Silver: 2x, Gold: 5x, Platinum: 10x)\n";
    std::cout << "0. Back\n";
    int ruleChoice = Utility::readIntInRange("Which rule do you want to change: ", 0, 3);
    if (ruleChoice == 1) {
        bank.rules.interestRatePercent = Utility::readPositiveAmount("New interest rate (%): ");
    } else if (ruleChoice == 2) {
        bank.rules.minimumBalance = Utility::readPositiveAmount("New minimum balance: ");
    } else if (ruleChoice == 3) {
        bank.rules.dailyWithdrawalLimit = Utility::readPositiveAmount("New daily withdrawal limit: ");
    } else {
        return;
    }
    bank.logAction("Changed bank rule " + String::fromInteger(ruleChoice));
    std::cout << "Rule updated.\n";
}

void AdminModule::backupData() {
    bank.logAction("Created data backup");
    FileManager::saveAll(bank);
    if (FileManager::backupDataFiles()) {
        std::cout << "Backup completed (files starting with backup_).\n";
    } else {
        std::cout << "Backup failed.\n";
    }
}

bool AdminModule::restoreData() {
    if (!Utility::readYesNo("This replaces all current data with the backup. Continue?")) {
        return false;
    }
    if (!FileManager::restoreDataFiles()) {
        std::cout << "No usable backup was found.\n";
        return false;
    }
    bank.clearAllData();
    FileManager::loadAll(bank);
    bank.logAction("Restored data from backup");
    std::cout << "Data restored. You will be logged out so the restored data can be used.\n";
    return true;
}
