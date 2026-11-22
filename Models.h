#ifndef MODELS_H
#define MODELS_H

#include "String.h"

enum class AccountType { Savings, Current };
enum class AccountStatus { Pending, Active, Frozen, Locked, Closed };
enum class AccountLevel { Standard = 1, Silver = 2, Gold = 3, Platinum = 4 };
enum class UserRole { Administrator, Manager, Teller };
enum class TransactionType {
    Deposit,
    Withdrawal,
    TransferOut,
    TransferIn,
    Interest,
    BillPayment,
    LoanDisbursement,
    LoanRepayment,
    ReversalCredit,
    ReversalDebit
};
enum class LoanStatus { Pending, Approved, Rejected, Repaid };

const int MAXIMUM_PIN_ATTEMPTS = 3;
const int PRIORITY_LEVEL_NORMAL = 0;
const int TRANSACTION_TYPE_COUNT = 10;

class Transaction {
public:
    int transactionId;
    int accountNumber;
    int relatedAccountNumber;
    TransactionType type;
    double amount;
    long long timestamp;
};

class Loan {
public:
    int loanId = 0;
    int accountNumber = 0;
    double amount = 0;
    int durationInMonths = 0;
    LoanStatus status = LoanStatus::Pending;
    int remainingInstalments = 0;
    double instalmentAmount = 0;
};

class User {
public:
    String userId;
    String passwordHash;
    UserRole role = UserRole::Teller;
    int failedLoginAttempts = 0;
    bool isLocked = false;
};

class ServiceToken {
public:
    int tokenNumber = 0;
    String customerName;
    int priorityLevel = PRIORITY_LEVEL_NORMAL;
};

class BankRules {
public:
    double interestRatePercent = 5.0;
    double minimumBalance = 500.0;
    double dailyWithdrawalLimit = 50000.0;
};

class AuditEntry {
public:
    long long timestamp;
    String actorName;
    String actionDescription;
};

class HeapItem {
public:
    long long priorityKey;
    int itemId;
};

class TransferLink {
public:
    int fromAccount;
    int toAccount;
    double totalAmount;
    int transferCount;
};

String accountTypeToString(AccountType type);
String accountStatusToString(AccountStatus status);
String accountLevelToString(AccountLevel level);
double dailyLimitForLevel(AccountLevel level, double baseLimit);
double requiredBalanceForLevel(AccountLevel targetLevel);
int requiredTransactionsForLevel(AccountLevel targetLevel);
String userRoleToString(UserRole role);
String transactionTypeToString(TransactionType type);
String loanStatusToString(LoanStatus status);
bool isCreditTransaction(TransactionType type);

double calculateOutstandingLoanAmount(const Loan& loan);
double calculateNextInstalment(const Loan& loan);

long long computePriorityTokenKey(const ServiceToken& token);
long long computeLoanPriorityKey(double loanAmount);
long long computeBalanceKey(double balance);

#endif
