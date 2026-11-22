#include <iostream>

#include "BankData.h"
#include "FileManager.h"
#include "MenuModule.h"
#include "Utility.h"

int main() {
    BankData bank;
    FileManager::loadAll(bank);
    if (bank.skippedRecordCount > 0) {
        std::cout << "Warning: " << bank.skippedRecordCount << " damaged record(s) in the data files were skipped.\n";
    }
    if (!FileManager::saveAll(bank)) {
        std::cout << "Warning: data files could not be written. Check the folder permissions.\n";
    }

    MenuModule menuModule(bank);
    try {
        menuModule.run();
    } catch (const Utility::InputClosed&) {
        std::cout << "\nInput closed.\n";
    }

    FileManager::saveAll(bank);
    std::cout << "Data saved. Goodbye.\n";
    return 0;
}
