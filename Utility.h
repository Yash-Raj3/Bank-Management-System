#ifndef UTILITY_H
#define UTILITY_H

#include "DynamicArray.h"
#include "String.h"

namespace Utility {

class InputClosed {};

void pauseForEnter();
String readLine(const String& promptText);
String readNonEmptyLine(const String& promptText);
int readIntInRange(const String& promptText, int minimumValue, int maximumValue);
double readPositiveAmount(const String& promptText);
String readDigits(const String& promptText, int requiredLength);
bool readYesNo(const String& promptText);

long long currentTimestamp();
String formatTimestamp(long long timestamp);
int dateKeyFromTimestamp(long long timestamp);

bool parseInteger(const String& text, int& parsedValue);
bool parseLongInteger(const String& text, long long& parsedValue);
bool parseDecimal(const String& text, double& parsedValue);

String formatMoney(double amount);
double roundToTwoDecimals(double amount);
double maximumOf(double firstValue, double secondValue);
String hashSecret(const String& plainText);
DynamicArray<String> splitString(const String& text, char delimiter);

String padRight(const String& text, int width);
String padLeft(const String& text, int width);
void printHeader(const String& title);
void printSeparator();

}

#endif
