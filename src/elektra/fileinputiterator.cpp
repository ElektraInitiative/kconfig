#include "fileinputiterator.h"

#include <sstream>

FileInputIterator::FileInputIterator(const std::string &filename) : file{filename} {
    if (!(this->file).is_open()) {
        this->next_val = KCIniToken::END_FILE;
        return;
    }
    update_next_val();
}

void FileInputIterator::update_next_val() {
    switch ((this->file).peek()) {
        case EOF:
            this->next_val = KCIniToken::END_FILE;
            break;
        case '\n':
            this->next_val = KCIniToken::NEWLINE;
            break;
        case '[':
            this->next_val = KCIniToken::OPEN_BRACKET;
            break;
        case ']':
            this->next_val = KCIniToken::CLOSE_BRACKET;
            break;
        case '=':
            this->next_val = KCIniToken::EQUALS_SIGN;
            break;
        default:
            this->next_val = KCIniToken::OTHER;
            break;
    }
}

KCIniToken FileInputIterator::next_val_type() const {
    return this->next_val;
}

void FileInputIterator::skip_char() {
    (this->file).get();
    update_next_val();
}

char FileInputIterator::peek_next() {
    return (this->file).peek();
}

void FileInputIterator::skip_line_no_update() {
    while (true) {
        switch ((this->file).get()) {
            case '\n':
                return;
            case EOF:
                (this->file).putback(EOF);
                return;
        }
    }
}

void FileInputIterator::skip_line() {
    skip_line_no_update();
    update_next_val();
}

bool FileInputIterator::is_end_or_newline_next() const {
    return static_cast<int> (next_val_type()) < 2;
}

void FileInputIterator::skip_line_if_not_end() {
    if (!is_end_or_newline_next()) {
        skip_line();
    }
}


void FileInputIterator::skip_empty_and_comments() {
    while (true) {
        switch ((this->file).peek()) {
            case '\n':
                skip_char();
                break;
            case '#':
                skip_line_no_update();
                break;
            default:
                update_next_val();
                return;
        }
    }
}

void FileInputIterator::skip_blank_chars() {
    while (isblank(peek_next())) {
        skip_char();
    }
}

void FileInputIterator::read_in_line_until_char(std::ostream &str, const char &delimiter) {
    char c;
    while (true) {
        c = this->file.get();
        if (c == EOF || c == '\n' || c == delimiter) {
            this->file.putback(c);
            break;
        }
        str << c;
    }
    update_next_val();
}

void FileInputIterator::read_in_line_until_char(std::ostream &str, const char &delimiterA, const char &delimiterB) {
    char c;
    while (true) {
        c = this->file.get();
        if (c == EOF || c == '\n' || c == delimiterA || c == delimiterB) {
            this->file.putback(c);
            break;
        }
        str << c;
    }
    update_next_val();
}

std::string FileInputIterator::get_in_line_until_char(const char &delimiter) {
    std::stringstream ret;
    read_in_line_until_char(ret, delimiter);
    return ret.str();
}

std::string FileInputIterator::get_in_line_until_char(const char &delimiterA, const char &delimiterB) {
    std::stringstream ret;
    read_in_line_until_char(ret, delimiterA, delimiterB);
    return ret.str();
}

bool FileInputIterator::skip_if_token(KCIniToken token) {
    if (next_val_type() != token) {
        return false;
    }
    skip_char();
    update_next_val();
    return true;
}
