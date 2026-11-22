#include "LoginModule.h"

#include <iostream>

#include "Utility.h"

LoginModule::LoginModule(BankData& bankData) : bank(bankData) {}

User* LoginModule::loginStaff() {
    Utility::printHeader("STAFF LOGIN");
    String userId = Utility::readNonEmptyLine("User ID: ");
    String password = Utility::readLine("Password: ");
    User* user = bank.userTable.find(userId);
    if (user == nullptr) {
        std::cout << "Invalid user ID or password.\n";
        return nullptr;
    }
    if (user->isLocked) {
        std::cout << "This user is locked. Ask an administrator to reset the password.\n";
        return nullptr;
    }
    if (user->passwordHash != Utility::hashSecret(password)) {
        ++user->failedLoginAttempts;
        bank.auditLog.record(userId, "Failed login attempt");
        if (user->role != UserRole::Administrator && user->failedLoginAttempts >= MAXIMUM_PIN_ATTEMPTS) {
            user->isLocked = true;
            bank.auditLog.record(userId, "User locked after too many wrong passwords");
            std::cout << "Too many wrong attempts. This user is now locked.\n";
        } else {
            std::cout << "Invalid user ID or password.\n";
        }
        return nullptr;
    }
    user->failedLoginAttempts = 0;
    return user;
}

Account* LoginModule::loginCustomer() {
    Utility::printHeader("CUSTOMER LOGIN");
    int accountNumber = Utility::readIntInRange("Account number: ", 1, 999999999);
    String enteredPin = Utility::readLine("PIN: ");
    Account* account = bank.findAccount(accountNumber);
    if (account == nullptr) {
        std::cout << "Invalid account number or PIN.\n";
        return nullptr;
    }
    switch (account->status) {
        case AccountStatus::Pending:
            std::cout << "This account is still waiting for administrator approval.\n";
            return nullptr;
        case AccountStatus::Frozen:
            std::cout << "This account is frozen. Please contact the bank.\n";
            return nullptr;
        case AccountStatus::Locked:
            std::cout << "This account is locked after wrong PIN attempts. Please contact the bank.\n";
            return nullptr;
        case AccountStatus::Closed:
            std::cout << "This account has been closed.\n";
            return nullptr;
        case AccountStatus::Active:
            break;
    }
    String actorName = "customer:" + String::fromInteger(accountNumber);
    if (account->pinHash != Utility::hashSecret(enteredPin)) {
        ++account->failedPinAttempts;
        bank.auditLog.record(actorName, "Failed PIN attempt");
        if (account->failedPinAttempts >= MAXIMUM_PIN_ATTEMPTS) {
            account->status = AccountStatus::Locked;
            bank.auditLog.record(actorName, "Account locked after too many wrong PINs");
            std::cout << "Too many wrong attempts. The account is now locked.\n";
        } else {
            std::cout << "Invalid account number or PIN. Attempts left: "
                      << MAXIMUM_PIN_ATTEMPTS - account->failedPinAttempts << "\n";
        }
        return nullptr;
    }
    account->failedPinAttempts = 0;
    return account;
}
