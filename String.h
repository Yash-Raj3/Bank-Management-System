#ifndef STRING_H
#define STRING_H

#include <iostream>

class String {
public:
    String();
    String(const char* text);
    String(const String& other);
    ~String();
    String& operator=(const String& other);

    int length() const;
    bool isEmpty() const;
    const char* c_str() const;
    char charAt(int index) const;

    String operator+(const String& other) const;
    String& operator+=(const String& other);
    String& operator+=(char character);
    bool operator==(const String& other) const;
    bool operator!=(const String& other) const;
    bool operator<(const String& other) const;

    String substring(int startIndex, int count) const;
    bool contains(const String& part) const;
    int indexOfChar(char character) const;
    String toLowerCase() const;
    String trimmed() const;
    bool isDigitsOnly() const;

    static String fromInteger(long long value);
    static String fromUnsigned(unsigned long long value);

private:
    char* characters;
    int stringLength;

    void assignFrom(const char* text, int count);
};

String operator+(const char* left, const String& right);
std::ostream& operator<<(std::ostream& output, const String& text);

#endif
