#include "LoanModule.h"

#include <iostream>

#include "Utility.h"

LoanModule::LoanModule(BankData& bankData, TransactionModule& transactionModule)
    : bank(bankData), transactions(transactionModule) {}

void LoanModule::printLoanTableHeading() const {
    std::cout << Utility::padRight("Loan", 7) << Utility::padRight("Account", 10) << Utility::padLeft("Amount", 14)
              << Utility::padLeft("Months", 8) << "  " << Utility::padRight("Status", 10) << Utility::padLeft("Left", 10)
              << Utility::padLeft("Instalment", 14) << "\n";
    Utility::printSeparator();
}

void LoanModule::printLoanRow(const Loan& loan) const {
    std::cout << Utility::padRight(String::fromInteger(loan.loanId), 7)
              << Utility::padRight(String::fromInteger(loan.accountNumber), 10)
              << Utility::padLeft(Utility::formatMoney(loan.amount), 14)
              << Utility::padLeft(String::fromInteger(loan.durationInMonths), 8) << "  "
              << Utility::padRight(loanStatusToString(loan.status), 10)
              << Utility::padLeft(String::fromInteger(loan.remainingInstalments), 10)
              << Utility::padLeft(Utility::formatMoney(loan.instalmentAmount), 14) << "\n";
}

void LoanModule::applyForLoan(Account& account) {
    if (!account.isActive()) {
        std::cout << "Only active accounts can apply for a loan.\n";
        return;
    }
    for (int index = 0; index < bank.loans.size(); ++index) {
        const Loan& existingLoan = bank.loans[index];
        if (existingLoan.accountNumber == account.accountNumber &&
            (existingLoan.status == LoanStatus::Pending || existingLoan.status == LoanStatus::Approved)) {
            std::cout << "This account already has an unfinished loan (ID " << existingLoan.loanId << ").\n";
            return;
        }
    }
    Loan newLoan;
    newLoan.loanId = bank.nextLoanId++;
    newLoan.accountNumber = account.accountNumber;
    newLoan.amount = Utility::readPositiveAmount("Loan amount: ");
    newLoan.durationInMonths = Utility::readIntInRange("Duration in months (1-60): ", 1, 60);
    newLoan.status = LoanStatus::Pending;
    newLoan.remainingInstalments = newLoan.durationInMonths;
    newLoan.instalmentAmount = Utility::roundToTwoDecimals(newLoan.amount / newLoan.durationInMonths);
    bank.loans.add(newLoan);
    HeapItem requestItem{computeLoanPriorityKey(newLoan.amount), newLoan.loanId};
    bank.loanRequestHeap.push(requestItem);
    bank.logAction("Loan request " + String::fromInteger(newLoan.loanId) + " of " + Utility::formatMoney(newLoan.amount) +
                   " for account " + String::fromInteger(account.accountNumber));
    std::cout << "Loan request submitted. Loan ID: " << newLoan.loanId << ". Monthly instalment would be "
              << Utility::formatMoney(newLoan.instalmentAmount) << ".\n";
}

void LoanModule::processPendingLoans() {
    Utility::printHeader("PENDING LOAN REQUESTS");
    if (bank.loanRequestHeap.isEmpty()) {
        std::cout << "There are no pending loan requests.\n";
        return;
    }
    DynamicArray<HeapItem> skippedRequests;
    bool stopProcessing = false;
    while (!bank.loanRequestHeap.isEmpty() && !stopProcessing) {
        HeapItem nextRequest{};
        bank.loanRequestHeap.pop(nextRequest);
        Loan* loan = bank.findLoan(nextRequest.itemId);
        if (loan == nullptr || loan->status != LoanStatus::Pending) {
            continue;
        }
        Account* applicant = bank.findAccount(loan->accountNumber);
        printLoanTableHeading();
        printLoanRow(*loan);
        std::cout << "Applicant: " << (applicant != nullptr ? applicant->holderName : String("Unknown")) << "\n";
        std::cout << "1. Approve\n2. Reject\n3. Skip for now\n0. Stop reviewing\n";
        int decision = Utility::readIntInRange("Decision: ", 0, 3);
        if (decision == 1) {
            if (applicant == nullptr || !applicant->isActive()) {
                std::cout << "The applicant account is not active. Request kept pending.\n";
                skippedRequests.add(nextRequest);
                continue;
            }
            loan->status = LoanStatus::Approved;
            transactions.creditForService(*applicant, loan->amount, TransactionType::LoanDisbursement);
            bank.logAction("Approved loan " + String::fromInteger(loan->loanId));
            std::cout << "Loan approved and " << Utility::formatMoney(loan->amount) << " added to the account.\n";
        } else if (decision == 2) {
            loan->status = LoanStatus::Rejected;
            bank.logAction("Rejected loan " + String::fromInteger(loan->loanId));
            std::cout << "Loan rejected.\n";
        } else {
            skippedRequests.add(nextRequest);
            stopProcessing = decision == 0;
        }
    }
    for (int index = 0; index < skippedRequests.size(); ++index) {
        bank.loanRequestHeap.push(skippedRequests[index]);
    }
}

void LoanModule::payInstalment(Account& account) {
    Loan* activeLoan = nullptr;
    for (int index = 0; index < bank.loans.size(); ++index) {
        if (bank.loans[index].accountNumber == account.accountNumber && bank.loans[index].status == LoanStatus::Approved) {
            activeLoan = &bank.loans[index];
        }
    }
    if (activeLoan == nullptr) {
        std::cout << "This account has no approved loan to repay.\n";
        return;
    }
    double instalmentDue = calculateNextInstalment(*activeLoan);
    std::cout << "Loan " << activeLoan->loanId << ": " << activeLoan->remainingInstalments
              << " instalments left, next payment " << Utility::formatMoney(instalmentDue) << ".\n";
    if (!Utility::readYesNo("Pay one instalment now?")) {
        return;
    }
    String errorMessage;
    if (!transactions.debitForService(account, instalmentDue, TransactionType::LoanRepayment, errorMessage)) {
        std::cout << errorMessage << "\n";
        return;
    }
    --activeLoan->remainingInstalments;
    if (activeLoan->remainingInstalments == 0) {
        activeLoan->status = LoanStatus::Repaid;
        std::cout << "Final instalment paid. The loan is fully repaid.\n";
    } else {
        std::cout << "Instalment paid. " << activeLoan->remainingInstalments << " instalments remaining.\n";
    }
    bank.logAction("Instalment paid for loan " + String::fromInteger(activeLoan->loanId));
}

void LoanModule::showLoansOfAccount(const Account& account) const {
    Utility::printHeader("LOANS OF ACCOUNT " + String::fromInteger(account.accountNumber));
    bool foundAnyLoan = false;
    for (int index = 0; index < bank.loans.size(); ++index) {
        if (bank.loans[index].accountNumber == account.accountNumber) {
            if (!foundAnyLoan) {
                printLoanTableHeading();
            }
            printLoanRow(bank.loans[index]);
            foundAnyLoan = true;
        }
    }
    if (!foundAnyLoan) {
        std::cout << "No loans found.\n";
    }
}

void LoanModule::showAllLoans() const {
    Utility::printHeader("ALL LOANS");
    if (bank.loans.size() == 0) {
        std::cout << "No loans found.\n";
        return;
    }
    printLoanTableHeading();
    for (int index = 0; index < bank.loans.size(); ++index) {
        printLoanRow(bank.loans[index]);
    }
}
