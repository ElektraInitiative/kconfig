#include "parser.h"

#include "fileinputiterator.cpp"

#define KCONFIG_METADATA_KEY "kconfig"

// WIP: The code below was used for testing purposes
#include <iostream>

bool skip_if_closing_bracket(FileInputIterator &iterator) {
    if (iterator.next_val_type() != KCIniToken::CLOSE_BRACKET) {
        return false;
    }
    iterator.skip_char();
    return true;
}

Key parseGroupNameFromFileInputIterator(FileInputIterator &iterator, const Key &parent) {
    Key key{parent.getName()};

    while (iterator.next_val_type() == KCIniToken::OPEN_BRACKET) {
        iterator.skip_char();
        if (iterator.peek_next() == '$') {
            iterator.skip_char();

            std::string meta_candidate{iterator.get_in_line_until_char(']')};

            if (!iterator.skip_if_token(KCIniToken::CLOSE_BRACKET)) {
                // Handle not ending with ]
                return parent;
            }

            // Check if it is in the last bracket
            if (iterator.is_end_or_newline_next()) {
                key.setMeta(KCONFIG_METADATA_KEY, meta_candidate);
            } else {
                // If it's not the last value, treat it as a normal key name
                key.addBaseName(meta_candidate);
            }
        } else {
            // Otherwise append the key name
            key.addBaseName(iterator.get_in_line_until_char(']'));

            if (!iterator.skip_if_token(KCIniToken::CLOSE_BRACKET)) {
                // Handle not ending with ]
                return parent;
            }
        }
    }

    if (!iterator.is_end_or_newline_next()) {
        // Next char is neither newline nor end of file
        // Handle the error if any
        return parent;
    }

    return key;
}

Key parseEntryFromFileInputIterator(FileInputIterator &iterator, const Key &parent) {
    iterator.skip_blank_chars();
    if (iterator.next_val_type() != KCIniToken::OTHER) {
        // ???
        return parent;
    }

    std::string keyname = iterator.get_in_line_until_char('=', '[');

    Key key{parent.getName()};
    key.addBaseName(keyname);

    if (iterator.is_end_or_newline_next()) {
        // Key only.
        return key;
    }

    std::string meta{""};
    bool has_locale = false;
    while (iterator.next_val_type() == KCIniToken::OPEN_BRACKET) {
        iterator.skip_char();
        if (iterator.peek_next() == '$') {
            iterator.skip_char();
            meta += iterator.get_in_line_until_char(']');
        } else {
            if (has_locale) {
                // Only one locale is allowed
                return parent;
            } else {
                has_locale = true;
            }
//            maybe change the following to:
//            keyname += '[' + iterator.read_in_line_until_char (key, ']') + ']';
            key.addBaseName('[' + iterator.get_in_line_until_char(']') + ']');
        }

        if (iterator.next_val_type() != KCIniToken::CLOSE_BRACKET) {
            iterator.skip_line_if_not_end();
            return parent;
        }
        iterator.skip_char();
    }

    // Skip empty spaces if any
    iterator.skip_blank_chars();
    if (iterator.next_val_type() == KCIniToken::EQUALS_SIGN) {
        iterator.skip_char();
        iterator.skip_blank_chars();
        std::stringstream value;
        iterator.read_in_line_until_char(value, '\n');
        key.setString(value.str());
    } else if (!iterator.is_end_or_newline_next()) {
        // There must've been an error in the previous while loop
        return parent;
    }
    iterator.skip_char();

    return key;
}

void add_meta_if_any(KeySet &keySet, const Key &key) {
    if (key.hasMeta(KCONFIG_METADATA_KEY)) {
        keySet.append(key);
    }
}

KeySet get_key(FileInputIterator &iterator, const Key &parent) {
    KeySet ret{};

    Key current_group{parent};
    Key current_key{parent};
    KCIniToken current_token;


    while (true) {
        iterator.skip_empty_and_comments();
        current_token = iterator.next_val_type();
        if (current_token == KCIniToken::END_FILE) {
            break;
        } else if (current_token == KCIniToken::OPEN_BRACKET) {
            // Handle group definition
            current_group = parseGroupNameFromFileInputIterator(iterator, parent);
            add_meta_if_any(ret, current_group);
        } else {
            current_key = parseEntryFromFileInputIterator(iterator, current_group);
            if (current_key != current_group) {
                ret.append(current_key);
            }
        }
    }

    return ret;
}

// WIP: The code below was used for testing purposes
int main(int argc, char **argv) {
    std::string filename;
    if (argc == 2) {
        filename = std::string{argv[1]};
    } else {
        filename = "testrc";
    }
    FileInputIterator config_file{filename};
    Key parent{"/sw/MyApp"};
    if (!config_file.next_val_type() != KCIniToken::END_FILE) {
        std::cout << "Problems parsing the file or it is empty" << std::endl;
        return 1;
    }

    KeySet keySet = get_key(config_file, parent);

    for (const Key &k : keySet) {
        std::cout << k << std::endl;
        std::cout << "\tValue: " << k.getString() << std::endl;
        std::string metadata = k.getMeta<std::string>(KCONFIG_METADATA_KEY);
        std::cout << "\tMeta: " << metadata << std::endl;
    }

    return 0;
}
