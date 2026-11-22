#include "Account.h"

bool Account::isActive() const {
    return status == AccountStatus::Active;
}

void Account::resetDailyCounterIfNewDay(int todayDateKey) {
    if (withdrawalDateKey != todayDateKey) {
        withdrawalDateKey = todayDateKey;
        withdrawnToday = 0;
    }
}

double Account::getDailyLimit(const BankRules& rules) const {
    return dailyLimitForLevel(level, rules.dailyWithdrawalLimit);
}
