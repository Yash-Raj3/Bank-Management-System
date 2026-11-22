#include "Utility.h"

#include <cmath>
#include <ctime>
#include <iostream>

namespace Utility {

void pauseForEnter() {
    std::cout << "\nPress Enter to continue...";
    char character;
    while (std::cin.get(character)) {
        if (character == '\n') {
            return;
        }
    }
    throw InputClosed();
}

String readLine(const String& promptText) {
    std::cout << promptText;
    String enteredLine;
    char character;
    bool sawAnyCharacter = false;
    while (std::cin.get(character)) {
        sawAnyCharacter = true;
        if (character == '\n') {
            break;
        }
        if (character != '\r') {
            enteredLine += character;
        }
    }
    if (!sawAnyCharacter) {
        throw InputClosed();
    }
    enteredLine = enteredLine.trimmed();
    String cleanedLine;
    for (int index = 0; index < enteredLine.length(); ++index) {
        cleanedLine += enteredLine.charAt(index) == '|' ? ' ' : enteredLine.charAt(index);
    }
    return cleanedLine;
}

String readNonEmptyLine(const String& promptText) {
    while (true) {
        String enteredLine = readLine(promptText);
        if (!enteredLine.isEmpty()) {
            return enteredLine;
        }
        std::cout << "This field cannot be empty.\n";
    }
}

int readIntInRange(const String& promptText, int minimumValue, int maximumValue) {
    while (true) {
        String enteredText = readLine(promptText);
        int enteredNumber = 0;
        if (enteredText.length() <= 9 && parseInteger(enteredText, enteredNumber) && enteredNumber >= minimumValue &&
            enteredNumber <= maximumValue) {
            return enteredNumber;
        }
        std::cout << "Please enter a number between " << minimumValue << " and " << maximumValue << ".\n";
    }
}

double readPositiveAmount(const String& promptText) {
    while (true) {
        String enteredText = readLine(promptText);
        double enteredAmount = 0;
        if (parseDecimal(enteredText, enteredAmount) && enteredAmount > 0 && enteredAmount <= 1000000000.0) {
            return roundToTwoDecimals(enteredAmount);
        }
        std::cout << "Please enter a valid positive amount.\n";
    }
}

String readDigits(const String& promptText, int requiredLength) {
    while (true) {
        String enteredText = readLine(promptText);
        if (enteredText.isDigitsOnly() && enteredText.length() == requiredLength) {
            return enteredText;
        }
        std::cout << "Please enter exactly " << requiredLength << " digits.\n";
    }
}

bool readYesNo(const String& promptText) {
    while (true) {
        String answer = readLine(promptText + " (y/n): ").toLowerCase();
        if (answer == "y" || answer == "yes") {
            return true;
        }
        if (answer == "n" || answer == "no") {
            return false;
        }
        std::cout << "Please answer y or n.\n";
    }
}

long long currentTimestamp() {
    return static_cast<long long>(std::time(nullptr));
}

String formatTimestamp(long long timestamp) {
    std::time_t timeValue = static_cast<std::time_t>(timestamp);
    std::tm localTime = *std::localtime(&timeValue);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return String(buffer);
}

int dateKeyFromTimestamp(long long timestamp) {
    std::time_t timeValue = static_cast<std::time_t>(timestamp);
    std::tm localTime = *std::localtime(&timeValue);
    return (localTime.tm_year + 1900) * 10000 + (localTime.tm_mon + 1) * 100 + localTime.tm_mday;
}

bool parseLongInteger(const String& text, long long& parsedValue) {
    int startIndex = (text.length() > 0 && text.charAt(0) == '-') ? 1 : 0;
    if (text.length() - startIndex < 1 || text.length() - startIndex > 18) {
        return false;
    }
    long long result = 0;
    for (int index = startIndex; index < text.length(); ++index) {
        if (text.charAt(index) < '0' || text.charAt(index) > '9') {
            return false;
        }
        result = result * 10 + (text.charAt(index) - '0');
    }
    parsedValue = startIndex == 1 ? -result : result;
    return true;
}

bool parseInteger(const String& text, int& parsedValue) {
    long long longValue = 0;
    if (!parseLongInteger(text, longValue) || longValue > 2147483647LL || longValue < -2147483647LL) {
        return false;
    }
    parsedValue = static_cast<int>(longValue);
    return true;
}

bool parseDecimal(const String& text, double& parsedValue) {
    int startIndex = (text.length() > 0 && text.charAt(0) == '-') ? 1 : 0;
    long long mantissa = 0;
    int digitCount = 0;
    int fractionDigitCount = 0;
    bool seenDecimalPoint = false;
    for (int index = startIndex; index < text.length(); ++index) {
        char character = text.charAt(index);
        if (character == '.' && !seenDecimalPoint) {
            seenDecimalPoint = true;
        } else if (character >= '0' && character <= '9') {
            if (++digitCount > 17) {
                return false;
            }
            mantissa = mantissa * 10 + (character - '0');
            if (seenDecimalPoint) {
                ++fractionDigitCount;
            }
        } else {
            return false;
        }
    }
    if (digitCount == 0) {
        return false;
    }
    double result = static_cast<double>(mantissa);
    for (int step = 0; step < fractionDigitCount; ++step) {
        result /= 10.0;
    }
    parsedValue = startIndex == 1 ? -result : result;
    return true;
}

String formatMoney(double amount) {
    long long totalPaisa = std::llround(amount * 100.0);
    bool isNegative = totalPaisa < 0;
    if (isNegative) {
        totalPaisa = -totalPaisa;
    }
    long long wholePart = totalPaisa / 100;
    long long fractionPart = totalPaisa % 100;
    String text = isNegative ? String("-") : String("");
    text += String::fromInteger(wholePart);
    text += ".";
    text += fractionPart < 10 ? String("0") : String("");
    text += String::fromInteger(fractionPart);
    return text;
}

double roundToTwoDecimals(double amount) {
    return std::round(amount * 100.0) / 100.0;
}

double maximumOf(double firstValue, double secondValue) {
    return firstValue > secondValue ? firstValue : secondValue;
}

String hashSecret(const String& plainText) {
    String saltedText = plainText + "bank-system-salt";
    unsigned long long hashValue = 1469598103934665603ULL;
    for (int index = 0; index < saltedText.length(); ++index) {
        hashValue ^= static_cast<unsigned char>(saltedText.charAt(index));
        hashValue *= 1099511628211ULL;
    }
    return String::fromUnsigned(hashValue);
}

DynamicArray<String> splitString(const String& text, char delimiter) {
    DynamicArray<String> fields;
    String currentField;
    for (int index = 0; index < text.length(); ++index) {
        if (text.charAt(index) == delimiter) {
            fields.add(currentField);
            currentField = "";
        } else {
            currentField += text.charAt(index);
        }
    }
    fields.add(currentField);
    return fields;
}

String padRight(const String& text, int width) {
    String paddedText = text;
    while (paddedText.length() < width) {
        paddedText += ' ';
    }
    return paddedText;
}

String padLeft(const String& text, int width) {
    String paddedText;
    for (int count = text.length(); count < width; ++count) {
        paddedText += ' ';
    }
    paddedText += text;
    return paddedText;
}

void printHeader(const String& title) {
    std::cout << "\n==================================================\n";
    std::cout << " " << title << "\n";
    std::cout << "==================================================\n";
}

void printSeparator() {
    std::cout << "--------------------------------------------------\n";
}

}
