#ifndef KCONFIG_SERIALIZER_H
#define KCONFIG_SERIALIZER_H

#include <elektra/kdb.hpp>

struct CustomSerializerInfo {
    std::ostream &o;
    std::size_t parent_keyname_size;
    std::string last_printed_group;
    bool is_first_key;

    CustomSerializerInfo(std::ostream &o,
                         std::size_t parent_keyname_size,
                         std::string last_printed_group);
};

bool saveKeySetToFile(std::ostream &o, const kdb::KeySet &keySet, const kdb::Key &parent);

#endif //KCONFIG_SERIALIZER_H
