#include "FileManager.h"

#include <fstream>

#include "Utility.h"

namespace {

const char* const ACCOUNTS_FILE = "accounts.dat";
const char* const TRANSACTIONS_FILE = "transactions.dat";
const char* const LOANS_FILE = "loans.dat";
const char* const USERS_FILE = "users.dat";
const char* const REQUESTS_FILE = "requests.dat";
const char* const RULES_FILE = "rules.dat";
const char* const LOG_FILE = "log.txt";
const char* const RECEIPTS_FILE = "receipts.txt";
const char* const BACKUP_PREFIX = "backup_";

const char* const ALL_DATA_FILES[] = {ACCOUNTS_FILE, TRANSACTIONS_FILE, LOANS_FILE, USERS_FILE,
                                      REQUESTS_FILE, RULES_FILE,        LOG_FILE,   RECEIPTS_FILE};
const int ALL_DATA_FILE_COUNT = 8;
const int LINE_BUFFER_SIZE = 4096;

bool copyFile(const String& sourceName, const String& targetName) {
    std::ifstream sourceFile(sourceName.c_str(), std::ios::binary);
    if (!sourceFile) {
        return false;
    }
    std::ofstream targetFile(targetName.c_str(), std::ios::binary | std::ios::trunc);
    char copyBuffer[4096];
    while (true) {
        sourceFile.read(copyBuffer, sizeof(copyBuffer));
        int bytesRead = static_cast<int>(sourceFile.gcount());
        if (bytesRead <= 0) {
            break;
        }
        targetFile.write(copyBuffer, bytesRead);
    }
    return true;
}

void writeTransaction(std::ofstream& file, const Transaction& transaction) {
    int typeCode = static_cast<int>(transaction.type);
    file.write(reinterpret_cast<const char*>(&transaction.transactionId), sizeof(int));
    file.write(reinterpret_cast<const char*>(&transaction.accountNumber), sizeof(int));
    file.write(reinterpret_cast<const char*>(&transaction.relatedAccountNumber), sizeof(int));
    file.write(reinterpret_cast<const char*>(&typeCode), sizeof(int));
    file.write(reinterpret_cast<const char*>(&transaction.amount), sizeof(double));
    file.write(reinterpret_cast<const char*>(&transaction.timestamp), sizeof(long long));
}

bool readTransaction(std::ifstream& file, Transaction& transaction) {
    int typeCode = 0;
    file.read(reinterpret_cast<char*>(&transaction.transactionId), sizeof(int));
    file.read(reinterpret_cast<char*>(&transaction.accountNumber), sizeof(int));
    file.read(reinterpret_cast<char*>(&transaction.relatedAccountNumber), sizeof(int));
    file.read(reinterpret_cast<char*>(&typeCode), sizeof(int));
    file.read(reinterpret_cast<char*>(&transaction.amount), sizeof(double));
    file.read(reinterpret_cast<char*>(&transaction.timestamp), sizeof(long long));
    if (!file || typeCode < 0 || typeCode >= TRANSACTION_TYPE_COUNT) {
        return false;
    }
    transaction.type = static_cast<TransactionType>(typeCode);
    return true;
}

bool saveRules(const BankData& bank) {
    std::ofstream file(RULES_FILE, std::ios::trunc);
    file << Utility::formatMoney(bank.rules.interestRatePercent) << "|" << Utility::formatMoney(bank.rules.minimumBalance)
         << "|" << Utility::formatMoney(bank.rules.dailyWithdrawalLimit) << "\n";
    return file.good();
}

bool saveUsers(const BankData& bank) {
    std::ofstream file(USERS_FILE, std::ios::trunc);
    DynamicArray<User*> allUsers = bank.userTable.collectAll();
    for (int index = 0; index < allUsers.size(); ++index) {
        User* user = allUsers[index];
        file << user->userId << "|" << user->passwordHash << "|" << static_cast<int>(user->role) << "|"
             << user->failedLoginAttempts << "|" << (user->isLocked ? 1 : 0) << "\n";
    }
    return file.good();
}

bool saveAccounts(const BankData& bank) {
    std::ofstream file(ACCOUNTS_FILE, std::ios::trunc);
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        Account* account = allAccounts[index];
        file << account->accountNumber << "|" << account->holderName << "|" << account->cnic << "|"
             << static_cast<int>(account->accountType) << "|" << Utility::formatMoney(account->balance) << "|"
             << account->pinHash << "|" << static_cast<int>(account->status) << "|" << account->nomineeName << "|"
             << account->nomineeRelation << "|" << account->failedPinAttempts << "|" << account->lastActivityTimestamp
             << "|" << Utility::formatMoney(account->withdrawnToday) << "|" << account->withdrawalDateKey
             << "|" << static_cast<int>(account->level)
             << "|" << static_cast<int>(account->requestedLevel) << "\n";
    }
    return file.good();
}

bool saveTransactions(const BankData& bank) {
    std::ofstream file(TRANSACTIONS_FILE, std::ios::binary | std::ios::trunc);
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int accountIndex = 0; accountIndex < allAccounts.size(); ++accountIndex) {
        DynamicArray<Transaction> accountTransactions = allAccounts[accountIndex]->history.toArray();
        for (int index = 0; index < accountTransactions.size(); ++index) {
            writeTransaction(file, accountTransactions[index]);
        }
    }
    return file.good();
}

bool saveLoans(const BankData& bank) {
    std::ofstream file(LOANS_FILE, std::ios::trunc);
    for (int index = 0; index < bank.loans.size(); ++index) {
        const Loan& loan = bank.loans[index];
        file << loan.loanId << "|" << loan.accountNumber << "|" << Utility::formatMoney(loan.amount) << "|"
             << loan.durationInMonths << "|" << static_cast<int>(loan.status) << "|" << loan.remainingInstalments << "|"
             << Utility::formatMoney(loan.instalmentAmount) << "\n";
    }
    return file.good();
}

bool saveRequests(const BankData& bank) {
    std::ofstream file(REQUESTS_FILE, std::ios::trunc);
    DynamicArray<int> pendingAccountNumbers = bank.approvalQueue.toArray();
    for (int index = 0; index < pendingAccountNumbers.size(); ++index) {
        file << "A|" << pendingAccountNumbers[index] << "\n";
    }
    for (int index = 0; index < bank.waitingTokens.size(); ++index) {
        const ServiceToken& token = bank.waitingTokens[index];
        file << "T|" << token.tokenNumber << "|" << token.customerName << "|" << token.priorityLevel << "\n";
    }
    return file.good();
}

bool saveAuditLog(const BankData& bank) {
    std::ofstream file(LOG_FILE, std::ios::trunc);
    DynamicArray<AuditEntry> allEntries = bank.auditLog.toArray();
    for (int index = 0; index < allEntries.size(); ++index) {
        file << allEntries[index].timestamp << "|" << allEntries[index].actorName << "|"
             << allEntries[index].actionDescription << "\n";
    }
    return file.good();
}

void loadRules(BankData& bank) {
    std::ifstream file(RULES_FILE);
    char lineBuffer[LINE_BUFFER_SIZE];
    if (!file.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        return;
    }
    DynamicArray<String> fields = Utility::splitString(String(lineBuffer).trimmed(), '|');
    double interestRate = 0;
    double minimumBalance = 0;
    double dailyLimit = 0;
    if (fields.size() >= 3 && Utility::parseDecimal(fields[0], interestRate) &&
        Utility::parseDecimal(fields[1], minimumBalance) && Utility::parseDecimal(fields[2], dailyLimit)) {
        bank.rules.interestRatePercent = interestRate;
        bank.rules.minimumBalance = minimumBalance;
        bank.rules.dailyWithdrawalLimit = dailyLimit;
    }
}

void loadUsers(BankData& bank) {
    std::ifstream file(USERS_FILE);
    char lineBuffer[LINE_BUFFER_SIZE];
    while (file.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        DynamicArray<String> fields = Utility::splitString(String(lineBuffer).trimmed(), '|');
        int roleCode = 0;
        int failedAttempts = 0;
        if (fields.size() < 5 || !Utility::parseInteger(fields[2], roleCode) ||
            !Utility::parseInteger(fields[3], failedAttempts)) {
            ++bank.skippedRecordCount;
            continue;
        }
        User* user = new User();
        user->userId = fields[0];
        user->passwordHash = fields[1];
        user->role = static_cast<UserRole>(roleCode);
        user->failedLoginAttempts = failedAttempts;
        user->isLocked = fields[4] == "1";
        if (!bank.userTable.insert(user)) {
            delete user;
        }
    }
}

void loadAccounts(BankData& bank) {
    std::ifstream file(ACCOUNTS_FILE);
    char lineBuffer[LINE_BUFFER_SIZE];
    while (file.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        DynamicArray<String> fields = Utility::splitString(String(lineBuffer).trimmed(), '|');
        int accountNumber = 0, typeCode = 0, statusCode = 0, failedAttempts = 0, dateKey = 0;
        double balance = 0, withdrawnToday = 0;
        long long lastActivity = 0;
        bool parsedSuccessfully = fields.size() >= 13 && Utility::parseInteger(fields[0], accountNumber) &&
                                  Utility::parseInteger(fields[3], typeCode) && Utility::parseDecimal(fields[4], balance) &&
                                  Utility::parseInteger(fields[6], statusCode) &&
                                  Utility::parseInteger(fields[9], failedAttempts) &&
                                  Utility::parseLongInteger(fields[10], lastActivity) &&
                                  Utility::parseDecimal(fields[11], withdrawnToday) &&
                                  Utility::parseInteger(fields[12], dateKey);
        if (!parsedSuccessfully) {
            ++bank.skippedRecordCount;
            continue;
        }
        Account* account = new Account();
        account->accountNumber = accountNumber;
        account->holderName = fields[1];
        account->cnic = fields[2];
        account->accountType = static_cast<AccountType>(typeCode);
        account->balance = balance;
        account->pinHash = fields[5];
        account->status = static_cast<AccountStatus>(statusCode);
        account->nomineeName = fields[7];
        account->nomineeRelation = fields[8];
        account->failedPinAttempts = failedAttempts;
        account->lastActivityTimestamp = lastActivity;
        account->withdrawnToday = withdrawnToday;
        account->withdrawalDateKey = dateKey;
        if (fields.size() >= 14) {
            int levelCode = 1;
            if (Utility::parseInteger(fields[13], levelCode) && levelCode >= 1 && levelCode <= 4) {
                account->level = static_cast<AccountLevel>(levelCode);
            }
        }
        if (fields.size() >= 15) {
            int reqCode = 1;
            if (Utility::parseInteger(fields[14], reqCode) && reqCode >= 1 && reqCode <= 4) {
                account->requestedLevel = static_cast<AccountLevel>(reqCode);
            }
        }
        if (!bank.addAccount(account)) {
            delete account;
            continue;
        }
        if (accountNumber >= bank.nextAccountNumber) {
            bank.nextAccountNumber = accountNumber + 1;
        }
    }
}

void loadTransactions(BankData& bank) {
    std::ifstream file(TRANSACTIONS_FILE, std::ios::binary);
    Transaction transaction{};
    while (readTransaction(file, transaction)) {
        Account* owner = bank.findAccount(transaction.accountNumber);
        if (owner == nullptr) {
            continue;
        }
        owner->history.append(transaction);
        if (transaction.transactionId >= bank.nextTransactionId) {
            bank.nextTransactionId = transaction.transactionId + 1;
        }
        if (transaction.type == TransactionType::TransferOut) {
            bank.transferGraph.addTransfer(transaction.accountNumber, transaction.relatedAccountNumber, transaction.amount);
        } else if (transaction.type == TransactionType::ReversalCredit && transaction.relatedAccountNumber != 0) {
            bank.transferGraph.addTransfer(transaction.accountNumber, transaction.relatedAccountNumber, -transaction.amount);
        }
    }
}

void loadLoans(BankData& bank) {
    std::ifstream file(LOANS_FILE);
    char lineBuffer[LINE_BUFFER_SIZE];
    while (file.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        DynamicArray<String> fields = Utility::splitString(String(lineBuffer).trimmed(), '|');
        Loan loan;
        int statusCode = 0;
        bool parsedSuccessfully = fields.size() >= 7 && Utility::parseInteger(fields[0], loan.loanId) &&
                                  Utility::parseInteger(fields[1], loan.accountNumber) &&
                                  Utility::parseDecimal(fields[2], loan.amount) &&
                                  Utility::parseInteger(fields[3], loan.durationInMonths) &&
                                  Utility::parseInteger(fields[4], statusCode) &&
                                  Utility::parseInteger(fields[5], loan.remainingInstalments) &&
                                  Utility::parseDecimal(fields[6], loan.instalmentAmount);
        if (!parsedSuccessfully) {
            ++bank.skippedRecordCount;
            continue;
        }
        loan.status = static_cast<LoanStatus>(statusCode);
        bank.loans.add(loan);
        if (loan.loanId >= bank.nextLoanId) {
            bank.nextLoanId = loan.loanId + 1;
        }
        if (loan.status == LoanStatus::Pending) {
            HeapItem loanItem{computeLoanPriorityKey(loan.amount), loan.loanId};
            bank.loanRequestHeap.push(loanItem);
        }
    }
}

void loadRequests(BankData& bank) {
    std::ifstream file(REQUESTS_FILE);
    char lineBuffer[LINE_BUFFER_SIZE];
    while (file.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        DynamicArray<String> fields = Utility::splitString(String(lineBuffer).trimmed(), '|');
        int accountNumber = 0;
        ServiceToken token;
        if (fields[0] == "A" && fields.size() >= 2 && Utility::parseInteger(fields[1], accountNumber)) {
            Account* pendingAccount = bank.findAccount(accountNumber);
            if (pendingAccount != nullptr && pendingAccount->status == AccountStatus::Pending) {
                bank.approvalQueue.enqueue(accountNumber);
            }
        } else if (fields[0] == "T" && fields.size() >= 4 && Utility::parseInteger(fields[1], token.tokenNumber) &&
                   Utility::parseInteger(fields[3], token.priorityLevel)) {
            token.customerName = fields[2];
            bank.waitingTokens.add(token);
            if (token.priorityLevel == PRIORITY_LEVEL_NORMAL) {
                bank.normalTokenQueue.enqueue(token.tokenNumber);
            } else {
                HeapItem tokenItem{computePriorityTokenKey(token), token.tokenNumber};
                bank.priorityTokenHeap.push(tokenItem);
            }
            if (token.tokenNumber >= bank.nextTokenNumber) {
                bank.nextTokenNumber = token.tokenNumber + 1;
            }
        }
    }
    DynamicArray<int> queuedAccountNumbers = bank.approvalQueue.toArray();
    DynamicArray<Account*> allAccounts = bank.accountTree.collectInOrder();
    for (int index = 0; index < allAccounts.size(); ++index) {
        bool isAlreadyQueued = false;
        for (int queueIndex = 0; queueIndex < queuedAccountNumbers.size(); ++queueIndex) {
            if (queuedAccountNumbers[queueIndex] == allAccounts[index]->accountNumber) {
                isAlreadyQueued = true;
            }
        }
        if (allAccounts[index]->status == AccountStatus::Pending && !isAlreadyQueued) {
            bank.approvalQueue.enqueue(allAccounts[index]->accountNumber);
        }
    }
}

void loadAuditLog(BankData& bank) {
    std::ifstream file(LOG_FILE);
    char lineBuffer[LINE_BUFFER_SIZE];
    while (file.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        DynamicArray<String> fields = Utility::splitString(String(lineBuffer).trimmed(), '|');
        long long timestamp = 0;
        if (fields.size() >= 3 && Utility::parseLongInteger(fields[0], timestamp)) {
            AuditEntry entry{timestamp, fields[1], fields[2]};
            bank.auditLog.addEntry(entry);
        }
    }
}

}

bool FileManager::saveAll(BankData& bank) {
    bool allFilesSaved = true;
    allFilesSaved = saveRules(bank) && allFilesSaved;
    allFilesSaved = saveUsers(bank) && allFilesSaved;
    allFilesSaved = saveAccounts(bank) && allFilesSaved;
    allFilesSaved = saveTransactions(bank) && allFilesSaved;
    allFilesSaved = saveLoans(bank) && allFilesSaved;
    allFilesSaved = saveRequests(bank) && allFilesSaved;
    allFilesSaved = saveAuditLog(bank) && allFilesSaved;
    return allFilesSaved;
}

void FileManager::loadAll(BankData& bank) {
    loadRules(bank);
    loadUsers(bank);
    loadAccounts(bank);
    loadTransactions(bank);
    loadLoans(bank);
    loadRequests(bank);
    loadAuditLog(bank);
    bank.ensureDefaultAdministratorExists();
}

bool FileManager::backupDataFiles() {
    bool accountsWereCopied = false;
    for (int index = 0; index < ALL_DATA_FILE_COUNT; ++index) {
        bool copied = copyFile(ALL_DATA_FILES[index], String(BACKUP_PREFIX) + ALL_DATA_FILES[index]);
        if (copied && String(ALL_DATA_FILES[index]) == ACCOUNTS_FILE) {
            accountsWereCopied = true;
        }
    }
    return accountsWereCopied;
}

bool FileManager::restoreDataFiles() {
    std::ifstream backupMarker((String(BACKUP_PREFIX) + ACCOUNTS_FILE).c_str());
    if (!backupMarker) {
        return false;
    }
    for (int index = 0; index < ALL_DATA_FILE_COUNT; ++index) {
        copyFile(String(BACKUP_PREFIX) + ALL_DATA_FILES[index], ALL_DATA_FILES[index]);
    }
    return true;
}

void FileManager::appendReceipt(const String& receiptText) {
    std::ofstream file(RECEIPTS_FILE, std::ios::app);
    file << receiptText << "\n";
}
