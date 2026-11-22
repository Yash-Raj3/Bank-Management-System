#include "Models.h"

#include <cmath>

String accountTypeToString(AccountType type) {
    return type == AccountType::Savings ? String("Savings") : String("Current");
}

String accountStatusToString(AccountStatus status) {
    switch (status) {
        case AccountStatus::Pending: return "Pending";
        case AccountStatus::Active: return "Active";
        case AccountStatus::Frozen: return "Frozen";
        case AccountStatus::Locked: return "Locked";
        case AccountStatus::Closed: return "Closed";
    }
    return "Unknown";
}

String accountLevelToString(AccountLevel level) {
    switch (level) {
        case AccountLevel::Standard: return "Standard";
        case AccountLevel::Silver: return "Silver";
        case AccountLevel::Gold: return "Gold";
        case AccountLevel::Platinum: return "Platinum";
    }
    return "Standard";
}

double dailyLimitForLevel(AccountLevel level, double baseLimit) {
    switch (level) {
        case AccountLevel::Silver: return baseLimit * 2.0;
        case AccountLevel::Gold: return baseLimit * 5.0;
        case AccountLevel::Platinum: return baseLimit * 10.0;
        case AccountLevel::Standard:
        default: return baseLimit;
    }
}

double requiredBalanceForLevel(AccountLevel targetLevel) {
    switch (targetLevel) {
        case AccountLevel::Silver: return 25000.0;
        case AccountLevel::Gold: return 100000.0;
        case AccountLevel::Platinum: return 250000.0;
        case AccountLevel::Standard:
        default: return 0.0;
    }
}

int requiredTransactionsForLevel(AccountLevel targetLevel) {
    switch (targetLevel) {
        case AccountLevel::Silver: return 3;
        case AccountLevel::Gold: return 5;
        case AccountLevel::Platinum: return 10;
        case AccountLevel::Standard:
        default: return 0;
    }
}

String userRoleToString(UserRole role) {
    switch (role) {
        case UserRole::Administrator: return "Administrator";
        case UserRole::Manager: return "Manager";
        case UserRole::Teller: return "Teller";
    }
    return "Unknown";
}

String transactionTypeToString(TransactionType type) {
    switch (type) {
        case TransactionType::Deposit: return "Deposit";
        case TransactionType::Withdrawal: return "Withdrawal";
        case TransactionType::TransferOut: return "Transfer Out";
        case TransactionType::TransferIn: return "Transfer In";
        case TransactionType::Interest: return "Interest";
        case TransactionType::BillPayment: return "Bill Payment";
        case TransactionType::LoanDisbursement: return "Loan Disbursed";
        case TransactionType::LoanRepayment: return "Loan Repayment";
        case TransactionType::ReversalCredit: return "Reversal (+)";
        case TransactionType::ReversalDebit: return "Reversal (-)";
    }
    return "Unknown";
}

String loanStatusToString(LoanStatus status) {
    switch (status) {
        case LoanStatus::Pending: return "Pending";
        case LoanStatus::Approved: return "Approved";
        case LoanStatus::Rejected: return "Rejected";
        case LoanStatus::Repaid: return "Repaid";
    }
    return "Unknown";
}

bool isCreditTransaction(TransactionType type) {
    return type == TransactionType::Deposit || type == TransactionType::TransferIn ||
           type == TransactionType::Interest || type == TransactionType::LoanDisbursement ||
           type == TransactionType::ReversalCredit;
}

double calculateOutstandingLoanAmount(const Loan& loan) {
    if (loan.remainingInstalments <= 0) {
        return 0;
    }
    double paidSoFar = loan.instalmentAmount * (loan.durationInMonths - loan.remainingInstalments);
    double outstandingAmount = std::round((loan.amount - paidSoFar) * 100.0) / 100.0;
    return outstandingAmount > 0 ? outstandingAmount : 0;
}

double calculateNextInstalment(const Loan& loan) {
    if (loan.remainingInstalments == 1) {
        return calculateOutstandingLoanAmount(loan);
    }
    return loan.instalmentAmount;
}

long long computePriorityTokenKey(const ServiceToken& token) {
    return static_cast<long long>(token.priorityLevel) * 1000000LL - token.tokenNumber;
}

long long computeLoanPriorityKey(double loanAmount) {
    return -std::llround(loanAmount * 100.0);
}

long long computeBalanceKey(double balance) {
    return std::llround(balance * 100.0);
}
