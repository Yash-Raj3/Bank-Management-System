#ifndef LOAN_MODULE_H
#define LOAN_MODULE_H

#include "BankData.h"
#include "TransactionModule.h"

class LoanModule {
public:
    LoanModule(BankData& bankData, TransactionModule& transactionModule);

    void applyForLoan(Account& account);
    void processPendingLoans();
    void payInstalment(Account& account);
    void showLoansOfAccount(const Account& account) const;
    void showAllLoans() const;

private:
    BankData& bank;
    TransactionModule& transactions;

    void printLoanRow(const Loan& loan) const;
    void printLoanTableHeading() const;
};

#endif
