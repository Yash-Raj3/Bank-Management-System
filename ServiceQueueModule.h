#ifndef SERVICE_QUEUE_MODULE_H
#define SERVICE_QUEUE_MODULE_H

#include "BankData.h"
#include "String.h"

class ServiceQueueModule {
public:
    explicit ServiceQueueModule(BankData& bankData);

    void issueToken();
    void serveNextToken();
    void showWaitingTokens();

private:
    BankData& bank;

    void removeWaitingToken(int tokenNumber);
    String describePriority(int priorityLevel) const;
};

#endif
