#ifndef KCONFIG_PARSER_H
#define KCONFIG_PARSER_H

#include "fileinputiterator.h"
#include <elektra/kdb.hpp>

kdb::Key parseGroupNameFromFileInputIterator(FileInputIterator &iterator, const kdb::Key &parent);

kdb::Key parseEntryFromFileInputIterator(FileInputIterator &iterator, const kdb::Key &parent);

void addMetaIfAny(kdb::KeySet &keySet, const kdb::Key &key);

kdb::KeySet parseFileToKeySet(FileInputIterator &iterator, const kdb::Key &parent);

#endif //KCONFIG_PARSER_H
