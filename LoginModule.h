#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include "BankData.h"

class LoginModule {
public:
    explicit LoginModule(BankData& bankData);

    User* loginStaff();
    Account* loginCustomer();

private:
    BankData& bank;
};

#endif
