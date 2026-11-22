#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "BankData.h"
#include "String.h"

class FileManager {
public:
    static bool saveAll(BankData& bank);
    static void loadAll(BankData& bank);
    static bool backupDataFiles();
    static bool restoreDataFiles();
    static void appendReceipt(const String& receiptText);
};

#endif
