#include "kconfig_elektra_base.h"
#include "fileinputiterator.h"

#include <sstream>

FileInputIterator::FileInputIterator(const std::string &filename) : file{filename} {
    if (!(this->file).is_open()) {
        this->next_val = KCIniToken::END_FILE;
        return;
    }
    updateNextVal();
}

void FileInputIterator::updateNextVal() {
    switch ((this->file).peek()) {
        case EOF:
            this->next_val = KCIniToken::END_FILE;
            break;
        case character_newline:
            this->next_val = KCIniToken::NEWLINE;
            break;
        case character_open_bracket:
            this->next_val = KCIniToken::OPEN_BRACKET;
            break;
        case character_close_bracket:
            this->next_val = KCIniToken::CLOSE_BRACKET;
            break;
        case character_equals_sign:
            this->next_val = KCIniToken::EQUALS_SIGN;
            break;
        default:
            this->next_val = KCIniToken::OTHER;
            break;
    }
}

KCIniToken FileInputIterator::nextValType() const {
    return this->next_val;
}

void FileInputIterator::skipChar() {
    (this->file).get();
    updateNextVal();
}

char FileInputIterator::peekNextChar() {
    return (this->file).peek();
}

void FileInputIterator::skipLineNoUpdate() {
    while (true) {
        switch ((this->file).get()) {
            case character_newline:
                return;
            case EOF:
                (this->file).putback(EOF);
                return;
        }
    }
}

void FileInputIterator::skipLine() {
    skipLineNoUpdate();
    updateNextVal();
}

bool FileInputIterator::isNextCharNewlineOrEOF() const {
    return static_cast<int> (nextValType()) < 2;
}

void FileInputIterator::skipLineIfNotEndOfLine() {
    if (!isNextCharNewlineOrEOF()) {
        skipLine();
    }
}


void FileInputIterator::skipLineIfEmptyOrComment() {
    while (true) {
        switch ((this->file).peek()) {
            case character_newline:
                skipChar();
                break;
            case character_hash_sign:
                skipLineNoUpdate();
                break;
            default:
                updateNextVal();
                return;
        }
    }
}

void FileInputIterator::skipCharsIfBlank() {
    while (isblank(peekNextChar())) {
        skipChar();
    }
}

void FileInputIterator::readUntilChar(std::ostream &str, const char &delimiter) {
    char c;
    while (true) {
        c = this->file.get();
        if (c == EOF || c == character_newline || c == delimiter) {
            this->file.putback(c);
            break;
        }
        str << c;
    }
    updateNextVal();
}

void FileInputIterator::readUntilChar(std::ostream &str, const char &delimiterA, const char &delimiterB) {
    char c;
    while (true) {
        c = this->file.get();
        if (c == EOF || c == character_newline || c == delimiterA || c == delimiterB) {
            this->file.putback(c);
            break;
        }
        str << c;
    }
    updateNextVal();
}

std::string FileInputIterator::getUntilChar(const char &delimiter) {
    std::stringstream ret;
    readUntilChar(ret, delimiter);
    return ret.str();
}

std::string FileInputIterator::getUntilChar(const char &delimiterA, const char &delimiterB) {
    std::stringstream ret;
    readUntilChar(ret, delimiterA, delimiterB);
    return ret.str();
}

bool FileInputIterator::skipCharIfToken(KCIniToken token) {
    if (nextValType() != token) {
        return false;
    }
    skipChar();
    updateNextVal();
    return true;
}