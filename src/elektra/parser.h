#ifndef KCONFIG_PARSER_H
#define KCONFIG_PARSER_H

#include "fileinputiterator.h"
#include <elektra/kdb.hpp>

kdb::KeySet parseFileToKeySet(FileInputIterator &iterator, const kdb::Key &parent);

#endif //KCONFIG_PARSER_H
