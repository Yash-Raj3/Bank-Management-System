#ifndef REPORT_MODULE_H
#define REPORT_MODULE_H

#include "BankData.h"

class ReportModule {
public:
    explicit ReportModule(BankData& bankData);

    void printAccountStatement(const Account& account) const;
    void showDailySummary() const;
    void showTopCustomers() const;
    void showDormantAccounts() const;
    void showTransferTracking() const;
    void showBankSummary() const;
    void showAuditLog() const;

private:
    BankData& bank;
};

#endif
