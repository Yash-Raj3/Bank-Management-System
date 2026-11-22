#include "String.h"

void String::assignFrom(const char* text, int count) {
    characters = new char[count + 1];
    for (int index = 0; index < count; ++index) {
        characters[index] = text[index];
    }
    characters[count] = '\0';
    stringLength = count;
}

String::String() : characters(nullptr), stringLength(0) {
    assignFrom("", 0);
}

String::String(const char* text) : characters(nullptr), stringLength(0) {
    int count = 0;
    while (text != nullptr && text[count] != '\0') {
        ++count;
    }
    assignFrom(text, count);
}

String::String(const String& other) : characters(nullptr), stringLength(0) {
    assignFrom(other.characters, other.stringLength);
}

String::~String() {
    delete[] characters;
}

String& String::operator=(const String& other) {
    if (this != &other) {
        char* oldCharacters = characters;
        assignFrom(other.characters, other.stringLength);
        delete[] oldCharacters;
    }
    return *this;
}

int String::length() const {
    return stringLength;
}

bool String::isEmpty() const {
    return stringLength == 0;
}

const char* String::c_str() const {
    return characters;
}

char String::charAt(int index) const {
    return characters[index];
}

String String::operator+(const String& other) const {
    String combined;
    delete[] combined.characters;
    combined.characters = new char[stringLength + other.stringLength + 1];
    for (int index = 0; index < stringLength; ++index) {
        combined.characters[index] = characters[index];
    }
    for (int index = 0; index < other.stringLength; ++index) {
        combined.characters[stringLength + index] = other.characters[index];
    }
    combined.stringLength = stringLength + other.stringLength;
    combined.characters[combined.stringLength] = '\0';
    return combined;
}

String& String::operator+=(const String& other) {
    *this = *this + other;
    return *this;
}

String& String::operator+=(char character) {
    char singleCharacterText[2] = {character, '\0'};
    *this = *this + String(singleCharacterText);
    return *this;
}

bool String::operator==(const String& other) const {
    if (stringLength != other.stringLength) {
        return false;
    }
    for (int index = 0; index < stringLength; ++index) {
        if (characters[index] != other.characters[index]) {
            return false;
        }
    }
    return true;
}

bool String::operator!=(const String& other) const {
    return !(*this == other);
}

bool String::operator<(const String& other) const {
    int shorterLength = stringLength < other.stringLength ? stringLength : other.stringLength;
    for (int index = 0; index < shorterLength; ++index) {
        if (characters[index] != other.characters[index]) {
            return characters[index] < other.characters[index];
        }
    }
    return stringLength < other.stringLength;
}

String String::substring(int startIndex, int count) const {
    String part;
    for (int index = startIndex; index < stringLength && index < startIndex + count; ++index) {
        part += characters[index];
    }
    return part;
}

bool String::contains(const String& part) const {
    if (part.stringLength == 0) {
        return true;
    }
    for (int startIndex = 0; startIndex + part.stringLength <= stringLength; ++startIndex) {
        bool allCharactersMatch = true;
        for (int offset = 0; offset < part.stringLength && allCharactersMatch; ++offset) {
            allCharactersMatch = characters[startIndex + offset] == part.characters[offset];
        }
        if (allCharactersMatch) {
            return true;
        }
    }
    return false;
}

int String::indexOfChar(char character) const {
    for (int index = 0; index < stringLength; ++index) {
        if (characters[index] == character) {
            return index;
        }
    }
    return -1;
}

String String::toLowerCase() const {
    String lowerCaseText;
    for (int index = 0; index < stringLength; ++index) {
        char character = characters[index];
        if (character >= 'A' && character <= 'Z') {
            character = static_cast<char>(character - 'A' + 'a');
        }
        lowerCaseText += character;
    }
    return lowerCaseText;
}

String String::trimmed() const {
    int firstIndex = 0;
    int lastIndex = stringLength - 1;
    while (firstIndex <= lastIndex && (characters[firstIndex] == ' ' || characters[firstIndex] == '\t' ||
                                       characters[firstIndex] == '\r' || characters[firstIndex] == '\n')) {
        ++firstIndex;
    }
    while (lastIndex >= firstIndex && (characters[lastIndex] == ' ' || characters[lastIndex] == '\t' ||
                                       characters[lastIndex] == '\r' || characters[lastIndex] == '\n')) {
        --lastIndex;
    }
    return substring(firstIndex, lastIndex - firstIndex + 1);
}

bool String::isDigitsOnly() const {
    if (stringLength == 0) {
        return false;
    }
    for (int index = 0; index < stringLength; ++index) {
        if (characters[index] < '0' || characters[index] > '9') {
            return false;
        }
    }
    return true;
}

String String::fromInteger(long long value) {
    if (value == 0) {
        return String("0");
    }
    bool isNegative = value < 0;
    unsigned long long remainingValue = isNegative ? static_cast<unsigned long long>(-(value + 1)) + 1ULL
                                                   : static_cast<unsigned long long>(value);
    String digits = fromUnsigned(remainingValue);
    return isNegative ? String("-") + digits : digits;
}

String String::fromUnsigned(unsigned long long value) {
    if (value == 0) {
        return String("0");
    }
    char reversedDigits[24];
    int digitCount = 0;
    while (value > 0) {
        reversedDigits[digitCount++] = static_cast<char>('0' + value % 10);
        value /= 10;
    }
    String text;
    for (int index = digitCount - 1; index >= 0; --index) {
        text += reversedDigits[index];
    }
    return text;
}

String operator+(const char* left, const String& right) {
    return String(left) + right;
}

std::ostream& operator<<(std::ostream& output, const String& text) {
    output << text.c_str();
    return output;
}
