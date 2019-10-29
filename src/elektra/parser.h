//
// Created by dardan on 10/29/19.
//

#ifndef KCONFIG_PARSER_H
#define KCONFIG_PARSER_H


#include "fileinputiterator.h"
#include <elektra/kdb.hpp>

using kdb::Key;
using kdb::KeySet;

bool skip_if_closing_bracket(FileInputIterator &iterator);

Key parseGroupNameFromFileInputIterator(FileInputIterator &iterator, const Key &parent);

Key parseEntryFromFileInputIterator(FileInputIterator &iterator, const Key &parent);

void add_meta_if_any(KeySet &keySet, const Key &key);

KeySet get_key(FileInputIterator &iterator, const Key &parent);


#endif //KCONFIG_PARSER_H
