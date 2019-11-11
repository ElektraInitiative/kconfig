//
// Created by dardan on 10/29/19.
//

#ifndef KCONFIG_FILEINPUTITERATOR_H
#define KCONFIG_FILEINPUTITERATOR_H


#include <string>
#include <fstream>
#include <ostream>

enum KCIniToken {
    END_FILE = 0,
    NEWLINE = 1,
    OPEN_BRACKET = 2,
    CLOSE_BRACKET = 3,
    EQUALS_SIGN = 4,
    OTHER = 5
};


class FileInputIterator {
private:
    std::ifstream file;
    KCIniToken next_val;

    void skipLineNoUpdate();

public:
    FileInputIterator(const std::string &filename);

    KCIniToken nextValType() const;

    char peekNextChar();

    bool isNextCharNewlineOrEOF() const;

    void skipChar();

    bool skipCharIfToken(KCIniToken token);

    void skipCharsIfBlank();

    void skipLine();

    void skipLineIfNotEndOfLine();

    void skipLineIfEmptyOrComment();

    void updateNextVal();

    void readUntilChar(std::ostream &str, const char &delimiter);

    void readUntilChar(std::ostream &str, const char &delimiterA, const char &delimiterB);

    std::string getUntilChar(const char &delimiter);

    std::string getUntilChar(const char &delimiterA, const char &delimiterB);
};

#endif //KCONFIG_FILEINPUTITERATOR_H
