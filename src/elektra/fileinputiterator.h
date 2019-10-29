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
    COMMENT = 5,
    OTHER = 6
};


struct FileInputIterator {
private:
    std::ifstream file;
    KCIniToken next_val;

public:
    FileInputIterator(const std::string &filename);

    void update_next_val();

    KCIniToken next_val_type() const;

    void skip_char();

    char peek_next();

    void skip_line_no_update();

    void skip_line();

    bool is_end_or_newline_next() const;

    void skip_line_if_not_end();

    void skip_empty_and_comments();

    void read_in_line_until_char(std::ostream &str, const char &delimiter);

    void read_in_line_until_char(std::ostream &str, const char &delimiterA, const char &delimiterB);

    void skip_blank_chars();

    std::string get_in_line_until_char(const char &delimiter);

    std::string get_in_line_until_char(const char &delimiterA, const char &delimiterB);

    bool skip_if_token(KCIniToken token);
};

#endif //KCONFIG_FILEINPUTITERATOR_H
