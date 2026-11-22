#include "TransactionModule.h"

#include <iostream>

#include "FileManager.h"
#include "Utility.h"

TransactionModule::TransactionModule(BankData& bankData) : bank(bankData) {}

Transaction TransactionModule::recordTransaction(Account& account, TransactionType type, double amount,
                                                 int relatedAccountNumber) {
    Transaction newTransaction{};
    newTransaction.transactionId = bank.nextTransactionId++;
    newTransaction.accountNumber = account.accountNumber;
    newTransaction.relatedAccountNumber = relatedAccountNumber;
    newTransaction.type = type;
    newTransaction.amount = amount;
    newTransaction.timestamp = Utility::currentTimestamp();
    account.history.append(newTransaction);
    account.lastActivityTimestamp = newTransaction.timestamp;
    return newTransaction;
}

bool TransactionModule::checkAccountIsUsable(const Account& account, String& errorMessage) const {
    if (account.isActive()) {
        return true;
    }
    errorMessage = "Account " + String::fromInteger(account.accountNumber) + " is " +
                   accountStatusToString(account.status) + " and cannot be used for transactions.";
    return false;
}

bool TransactionModule::debitAccount(Account& account, double amount, TransactionType type, int relatedAccountNumber,
                                     bool countsTowardDailyLimit, String& errorMessage) {
    if (!checkAccountIsUsable(account, errorMessage)) {
        return false;
    }
    if (account.balance - amount < bank.rules.minimumBalance - 0.001) {
        errorMessage = "Insufficient funds. A minimum balance of " + Utility::formatMoney(bank.rules.minimumBalance) +
                       " must remain. Available to use: " +
                       Utility::formatMoney(Utility::maximumOf(0.0, account.balance - bank.rules.minimumBalance));
        return false;
    }
    if (countsTowardDailyLimit) {
        account.resetDailyCounterIfNewDay(Utility::dateKeyFromTimestamp(Utility::currentTimestamp()));
        double effectiveLimit = account.getDailyLimit(bank.rules);
        if (account.withdrawnToday + amount > effectiveLimit + 0.001) {
            errorMessage = "Daily limit exceeded for " + accountLevelToString(account.level) +
                           " tier (Limit: " + Utility::formatMoney(effectiveLimit) +
                           "). Remaining allowance today: " +
                           Utility::formatMoney(Utility::maximumOf(0.0, effectiveLimit - account.withdrawnToday));
            return false;
        }
        account.withdrawnToday += amount;
    }
    account.balance -= amount;
    recordTransaction(account, type, amount, relatedAccountNumber);
    return true;
}

void TransactionModule::creditAccount(Account& account, double amount, TransactionType type, int relatedAccountNumber) {
    account.balance += amount;
    recordTransaction(account, type, amount, relatedAccountNumber);
}

void TransactionModule::pushUndoRecord(const Account& account, TransactionType type, int relatedAccountNumber,
                                       double amount) {
    Transaction undoRecord{};
    undoRecord.accountNumber = account.accountNumber;
    undoRecord.relatedAccountNumber = relatedAccountNumber;
    undoRecord.type = type;
    undoRecord.amount = amount;
    undoRecord.timestamp = Utility::currentTimestamp();
    bank.undoStack.push(undoRecord);
}

void TransactionModule::restoreDailyAllowance(Account& account, double amount) {
    if (account.withdrawalDateKey == Utility::dateKeyFromTimestamp(Utility::currentTimestamp())) {
        account.withdrawnToday = Utility::maximumOf(0.0, account.withdrawnToday - amount);
    }
}

bool TransactionModule::debitForService(Account& account, double amount, TransactionType type, String& errorMessage) {
    return debitAccount(account, amount, type, 0, false, errorMessage);
}

void TransactionModule::creditForService(Account& account, double amount, TransactionType type) {
    creditAccount(account, amount, type, 0);
}

void TransactionModule::payOutFullBalance(Account& account) {
    if (account.balance > 0) {
        double payoutAmount = account.balance;
        account.balance = 0;
        recordTransaction(account, TransactionType::Withdrawal, payoutAmount, 0);
    }
}

void TransactionModule::deposit(Account& account) {
    String errorMessage;
    if (!checkAccountIsUsable(account, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    double depositAmount = Utility::readPositiveAmount("Amount to deposit: ");
    creditAccount(account, depositAmount, TransactionType::Deposit, 0);
    pushUndoRecord(account, TransactionType::Deposit, 0, depositAmount);
    bank.logAction("Deposited " + Utility::formatMoney(depositAmount) + " into account " +
                   String::fromInteger(account.accountNumber));
    std::cout << "Deposit successful. New balance: " << Utility::formatMoney(account.balance) << "\n";
}

void TransactionModule::withdraw(Account& account) {
    String errorMessage;
    if (!checkAccountIsUsable(account, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    double withdrawalAmount = Utility::readPositiveAmount("Amount to withdraw: ");
    if (!debitAccount(account, withdrawalAmount, TransactionType::Withdrawal, 0, true, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    pushUndoRecord(account, TransactionType::Withdrawal, 0, withdrawalAmount);
    bank.logAction("Withdrew " + Utility::formatMoney(withdrawalAmount) + " from account " +
                   String::fromInteger(account.accountNumber));
    std::cout << "Withdrawal successful. New balance: " << Utility::formatMoney(account.balance) << "\n";
}

void TransactionModule::transfer(Account& sourceAccount) {
    String errorMessage;
    if (!checkAccountIsUsable(sourceAccount, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    int destinationNumber = Utility::readIntInRange("Destination account number: ", 1, 999999999);
    Account* destinationAccount = bank.findAccount(destinationNumber);
    if (destinationAccount == nullptr) {
        std::cout << "Destination account was not found.\n";
        return;
    }
    if (destinationAccount == &sourceAccount) {
        std::cout << "You cannot transfer money to the same account.\n";
        return;
    }
    if (!checkAccountIsUsable(*destinationAccount, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    double transferAmount = Utility::readPositiveAmount("Amount to transfer: ");
    if (!debitAccount(sourceAccount, transferAmount, TransactionType::TransferOut, destinationNumber, true, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    creditAccount(*destinationAccount, transferAmount, TransactionType::TransferIn, sourceAccount.accountNumber);
    pushUndoRecord(sourceAccount, TransactionType::TransferOut, destinationNumber, transferAmount);
    bank.transferGraph.addTransfer(sourceAccount.accountNumber, destinationNumber, transferAmount);
    bank.logAction("Transferred " + Utility::formatMoney(transferAmount) + " from account " +
                   String::fromInteger(sourceAccount.accountNumber) + " to account " + String::fromInteger(destinationNumber));
    std::cout << "Transfer successful. New balance: " << Utility::formatMoney(sourceAccount.balance) << "\n";
}

void TransactionModule::undoLastTransaction() {
    Transaction lastRecord{};
    if (!bank.undoStack.peek(lastRecord)) {
        std::cout << "There is no transaction to undo.\n";
        return;
    }
    Account* mainAccount = bank.findAccount(lastRecord.accountNumber);
    if (mainAccount == nullptr || !mainAccount->isActive()) {
        std::cout << "Cannot undo: the account is not active.\n";
        return;
    }
    if (lastRecord.type == TransactionType::Deposit) {
        if (mainAccount->balance < lastRecord.amount) {
            std::cout << "Cannot undo: the deposited money has already been used.\n";
            return;
        }
        mainAccount->balance -= lastRecord.amount;
        recordTransaction(*mainAccount, TransactionType::ReversalDebit, lastRecord.amount, 0);
    } else if (lastRecord.type == TransactionType::Withdrawal) {
        mainAccount->balance += lastRecord.amount;
        recordTransaction(*mainAccount, TransactionType::ReversalCredit, lastRecord.amount, 0);
        restoreDailyAllowance(*mainAccount, lastRecord.amount);
    } else if (lastRecord.type == TransactionType::TransferOut) {
        Account* receivingAccount = bank.findAccount(lastRecord.relatedAccountNumber);
        if (receivingAccount == nullptr || !receivingAccount->isActive() || receivingAccount->balance < lastRecord.amount) {
            std::cout << "Cannot undo: the receiving account cannot return the money.\n";
            return;
        }
        receivingAccount->balance -= lastRecord.amount;
        recordTransaction(*receivingAccount, TransactionType::ReversalDebit, lastRecord.amount, mainAccount->accountNumber);
        mainAccount->balance += lastRecord.amount;
        recordTransaction(*mainAccount, TransactionType::ReversalCredit, lastRecord.amount, receivingAccount->accountNumber);
        restoreDailyAllowance(*mainAccount, lastRecord.amount);
        bank.transferGraph.addTransfer(mainAccount->accountNumber, receivingAccount->accountNumber, -lastRecord.amount);
    } else {
        std::cout << "This transaction type cannot be undone.\n";
        return;
    }
    Transaction removedRecord{};
    bank.undoStack.pop(removedRecord);
    bank.logAction("Undid " + transactionTypeToString(lastRecord.type) + " of " + Utility::formatMoney(lastRecord.amount) +
                   " on account " + String::fromInteger(lastRecord.accountNumber));
    std::cout << "Last transaction was reversed successfully.\n";
}

void TransactionModule::printTransactionTable(const DynamicArray<Transaction>& transactions) {
    if (transactions.size() == 0) {
        std::cout << "No transactions found.\n";
        return;
    }
    std::cout << Utility::padRight("ID", 8) << Utility::padRight("Date and time", 21) << Utility::padRight("Type", 16)
              << Utility::padRight("Related", 10) << Utility::padLeft("Amount", 14) << "\n";
    Utility::printSeparator();
    for (int index = 0; index < transactions.size(); ++index) {
        const Transaction& transaction = transactions[index];
        String relatedText = transaction.relatedAccountNumber != 0 ? String::fromInteger(transaction.relatedAccountNumber)
                                                                   : String("-");
        String signedAmount = String(isCreditTransaction(transaction.type) ? "+" : "-") + Utility::formatMoney(transaction.amount);
        std::cout << Utility::padRight(String::fromInteger(transaction.transactionId), 8)
                  << Utility::padRight(Utility::formatTimestamp(transaction.timestamp), 21)
                  << Utility::padRight(transactionTypeToString(transaction.type), 16) << Utility::padRight(relatedText, 10)
                  << Utility::padLeft(signedAmount, 14) << "\n";
    }
}

void TransactionModule::showHistory(const Account& account) const {
    Utility::printHeader("TRANSACTION HISTORY - ACCOUNT " + String::fromInteger(account.accountNumber));
    printTransactionTable(account.history.toArray());
}

void TransactionModule::showMiniStatement(const Account& account) const {
    Utility::printHeader("MINI STATEMENT - ACCOUNT " + String::fromInteger(account.accountNumber));
    std::cout << "Account Level : " << accountLevelToString(account.level) << " Tier\n";
    std::cout << "Daily Limit   : " << Utility::formatMoney(account.getDailyLimit(bank.rules)) << "\n";
    std::cout << "Remaining Limit Today: "
              << Utility::formatMoney(Utility::maximumOf(0.0, account.getDailyLimit(bank.rules) - account.withdrawnToday)) << "\n";
    Utility::printSeparator();
    printTransactionTable(account.history.lastEntries(5));
    std::cout << "Current balance: " << Utility::formatMoney(account.balance) << "\n";
}

void TransactionModule::payBill(Account& account) {
    String errorMessage;
    if (!checkAccountIsUsable(account, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    const char* const billTypes[] = {"Electricity", "Gas", "Water", "Internet", "Telephone"};
    const int billTypeCount = 5;
    for (int index = 0; index < billTypeCount; ++index) {
        std::cout << index + 1 << ". " << billTypes[index] << "\n";
    }
    int billChoice = Utility::readIntInRange("Select bill type: ", 1, billTypeCount);
    String consumerNumber = Utility::readNonEmptyLine("Consumer number: ");
    double billAmount = Utility::readPositiveAmount("Bill amount: ");
    if (!debitAccount(account, billAmount, TransactionType::BillPayment, 0, false, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    String billName = billTypes[billChoice - 1];
    String receiptText = "RECEIPT #" + String::fromInteger(bank.nextTransactionId - 1) + " | " +
                         Utility::formatTimestamp(Utility::currentTimestamp()) + " | Account " +
                         String::fromInteger(account.accountNumber) + " | " + billName + " bill | Consumer " +
                         consumerNumber + " | Paid " + Utility::formatMoney(billAmount);
    FileManager::appendReceipt(receiptText);
    bank.logAction("Paid " + billName + " bill of " + Utility::formatMoney(billAmount) + " from account " +
                   String::fromInteger(account.accountNumber));
    std::cout << "Bill paid successfully.\n" << receiptText << "\n";
}

void TransactionModule::applyInterestToSavingsAccounts() {
    int accountsUpdated = 0;
    double totalInterestPaid = 0;
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        Account* account = allAccounts[index];
        if (account->accountType != AccountType::Savings || !account->isActive()) {
            continue;
        }
        double interestAmount = Utility::roundToTwoDecimals(account->balance * bank.rules.interestRatePercent / 100.0);
        if (interestAmount <= 0) {
            continue;
        }
        creditAccount(*account, interestAmount, TransactionType::Interest, 0);
        totalInterestPaid += interestAmount;
        ++accountsUpdated;
    }
    bank.logAction("Applied interest of " + Utility::formatMoney(bank.rules.interestRatePercent) + "% to " +
                   String::fromInteger(accountsUpdated) + " savings accounts");
    std::cout << "Interest added to " << accountsUpdated << " savings accounts. Total paid: "
              << Utility::formatMoney(totalInterestPaid) << "\n";
}
