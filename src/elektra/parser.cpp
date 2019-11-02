#include "parser.h"

#include "fileinputiterator.cpp"

#define KCONFIG_METADATA_KEY "kconfig"

using kdb::Key;
using kdb::KeySet;

static const char character_equals = '=';
static const char character_newline = '\n';
static const char character_dollar_sign = '\n';
static const char character_open_bracket = '[';
static const char character_close_bracket = ']';

Key parseGroupNameFromFileInputIterator(FileInputIterator &iterator, const Key &parent) {
    Key key{parent.getName()};

    while (iterator.nextValType() == KCIniToken::OPEN_BRACKET) {
        iterator.skipChar();
        if (iterator.peekNextChar() == character_dollar_sign) {
            iterator.skipChar();

            std::string meta_candidate{iterator.getUntilChar(character_close_bracket)};

            if (!iterator.skipCharIfToken(KCIniToken::CLOSE_BRACKET)) {
                // Handle not ending with ]
                return parent;
            }

            // Check if it is in the last bracket
            if (iterator.isNextCharNewlineOrEOF()) {
                key.setMeta(KCONFIG_METADATA_KEY, meta_candidate);
            } else {
                // If it's not the last value, treat it as a normal key name
                key.addBaseName(meta_candidate);
            }
        } else {
            // Otherwise append the key name
            key.addBaseName(iterator.getUntilChar(character_close_bracket));

            if (!iterator.skipCharIfToken(KCIniToken::CLOSE_BRACKET)) {
                // Handle not ending with ]
                return parent;
            }
        }
    }

    if (!iterator.isNextCharNewlineOrEOF()) {
        // Next char is neither newline nor end of file
        // Handle the error if any
        return parent;
    }

    return key;
}

Key parseEntryFromFileInputIterator(FileInputIterator &iterator, const Key &parent) {
    iterator.skipCharsIfBlank();
    if (iterator.nextValType() != KCIniToken::OTHER) {
        // ???
        return parent;
    }

    std::string keyname = iterator.getUntilChar(character_equals, character_open_bracket);
    Key key{parent.dup()};

    if (iterator.isNextCharNewlineOrEOF()) {
        // Key only.
        key.addBaseName(keyname);
        return key;
    }

    std::string meta{""};
    bool has_locale = false;
    while (iterator.nextValType() == KCIniToken::OPEN_BRACKET) {
        iterator.skipChar();
        if (iterator.peekNextChar() == character_dollar_sign) {
            iterator.skipChar();
            meta += iterator.getUntilChar(character_close_bracket);
        } else {
            if (has_locale) {
                // Only one locale is allowed
                return parent;
            } else {
                has_locale = true;
            }
            keyname +=
                    character_open_bracket + iterator.getUntilChar(character_close_bracket) + character_close_bracket;
        }

        if (iterator.nextValType() != KCIniToken::CLOSE_BRACKET) {
            iterator.skipLineIfNotEndOfLine();
            return parent;
        }
        iterator.skipChar();
    }


    key.addBaseName(keyname);
    // Skip empty spaces if any
    iterator.skipCharsIfBlank();
    if (iterator.nextValType() == KCIniToken::EQUALS_SIGN) {
        iterator.skipChar();
        iterator.skipCharsIfBlank();
        std::stringstream value;
        iterator.readUntilChar(value, character_newline);
        key.setString(value.str());
    } else if (!iterator.isNextCharNewlineOrEOF()) {
        // There must've been an error in the previous while loop
        return parent;
    }
    iterator.skipChar();

    if (meta != "") {
        key.setMeta(KCONFIG_METADATA_KEY, meta);
    }

    return key;
}

void addMetaIfAny(KeySet &keySet, const Key &key) {
    if (key.hasMeta(KCONFIG_METADATA_KEY)) {
        keySet.append(key);
    }
}

KeySet parseFileToKeySet(FileInputIterator &iterator, const Key &parent) {
    KeySet ret{};

    Key current_group{parent};
    Key current_key{parent};
    KCIniToken current_token;


    while (true) {
        iterator.skipLineIfEmptyOrComment();
        current_token = iterator.nextValType();
        if (current_token == KCIniToken::END_FILE) {
            break;
        } else if (current_token == KCIniToken::OPEN_BRACKET) {
            // Handle group definition
            current_group = parseGroupNameFromFileInputIterator(iterator, parent);
            addMetaIfAny(ret, current_group);
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
#include <iostream>

int main(int argc, char **argv) {
    std::string filename;
    if (argc == 2) {
        filename = std::string{argv[1]};
    } else {
        filename = "testrc";
    }
    FileInputIterator config_file{filename};
    Key parent{"/sw/MyApp"};
    if (config_file.nextValType() == KCIniToken::END_FILE) {
        std::cout << "Problems parsing the file or it is empty" << std::endl;
        return 1;
    }

    KeySet keySet = parseFileToKeySet(config_file, parent);

    for (const Key &k : keySet) {
        std::cout << k.getName() << std::endl;
        std::cout << "\tValue: " << k.getString() << std::endl;
        std::string metadata = k.getMeta<std::string>(KCONFIG_METADATA_KEY);
        std::cout << "\tMeta: " << metadata << std::endl;
    }


    return 0;
}
