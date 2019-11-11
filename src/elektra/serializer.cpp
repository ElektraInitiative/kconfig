#include "kconfig_elektra_base.h"
#include "serializer.h"

#include <utility>
#include <vector>
#include <algorithm>

using kdb::Key;
using kdb::KeySet;

CustomSerializerInfo::CustomSerializerInfo(std::ostream &o,
                                           std::size_t parent_keyname_size,
                                           std::string last_printed_group)
        : o{o},
          parent_keyname_size{parent_keyname_size},
          last_printed_group{std::move(last_printed_group)},
          is_first_key{true} {}


std::size_t countKeyBreaks(const std::string &s) {
    std::size_t count{0};

    for (std::size_t i{0}; i < s.size(); i++) {
        switch (s[i]) {
            case '/':
                ++count;
                break;
            case '\\':
                ++i;
        }
    }

    return count;
}

std::size_t findLastSlash(const std::string &s) {
    std::size_t count{0};

    for (std::size_t i{0}; i < s.size(); i++) {
        switch (s[i]) {
            case '/':
                count = i;
                break;
            case '\\':
                ++i;
        }
    }

    return count;
}

std::string groupNameFromLeaf(const std::string &leafKeyName) {
    return leafKeyName.substr(0, findLastSlash(leafKeyName));
}

bool keyABeforeKeyB(const Key &a, const Key &b) {
    std::size_t breaks_in_a{countKeyBreaks(a.getName())};
    std::size_t breaks_in_b{countKeyBreaks(b.getName())};

    if (breaks_in_a != breaks_in_b) {
        return breaks_in_a < breaks_in_b;
    }

    return a.getName() < b.getName();
}

void saveLeafKeyOut(CustomSerializerInfo &serializer_info, const kdb::Key &key) {
    serializer_info.is_first_key = false;
    std::ostream &o = serializer_info.o;
    o << key.getBaseName();

    std::string meta = key.getMeta<std::string>(KCONFIG_METADATA_KEY);
    if (!meta.empty()) {
        o << character_open_bracket;
        o << character_dollar_sign;
        o << meta;
        o << character_close_bracket;
    }

    o << character_equals_sign;
    o << key.getString();
    o << character_newline;
}


void saveGroupKeyOut(CustomSerializerInfo &serializer_info, const std::string &group) {
    std::ostream &o = serializer_info.o;
    std::size_t skip_chars = serializer_info.parent_keyname_size;

    if (group.size() > skip_chars) {
        std::string outstr{group.substr(skip_chars)};

        if (!serializer_info.is_first_key) {
            o << character_newline;
        } else {
            serializer_info.is_first_key = false;
        }

        o << character_open_bracket;
        for (std::size_t i{0}; i < outstr.size(); ++i) {
            char current_char{outstr[i]};
            if (current_char == '/') {
                o << "][";
            } else if (current_char == '\\') {
                o << current_char << outstr[++i];
            } else {
                o << current_char;
            }
        }
        o << character_close_bracket << character_newline;
    }
}

void saveLeafKeyWithGroupCandidate(CustomSerializerInfo &serializer_info, const kdb::Key &k) {
    std::string &last_printed_group = serializer_info.last_printed_group;

    std::string current_group_name{groupNameFromLeaf(k.getName())};
    if (last_printed_group != current_group_name) {
        saveGroupKeyOut(serializer_info, current_group_name);
        last_printed_group = current_group_name;
    }
    saveLeafKeyOut(serializer_info, k);
}

#include <iostream>

bool keysWithGroupCompare(const std::string &a, const std::string &b) {
    std::size_t size_min{std::min(findLastSlash(a), findLastSlash(b))};

    for (std::size_t i{0}; i < size_min; ++i) {
        if (a[i] != b[i]) {
            return a[i] < b[i];
        }
    }

    std::size_t keybreaks_a{countKeyBreaks(a)};
    std::size_t keybreaks_b{countKeyBreaks(b)};

    if (keybreaks_a != keybreaks_b) {
        return keybreaks_a < keybreaks_b;
    }

    return a < b;
}

bool saveKeySetToFile(std::ostream &o, const kdb::KeySet &keySet, const kdb::Key &parent) {
    std::vector<kdb::Key> keys{keySet.begin(), keySet.end()};

    std::size_t parent_keybreaks{countKeyBreaks(parent.getName()) + 1};

    auto key_comparator = [parent_keybreaks](const Key &a, const Key &b) {
        bool is_groupless_a{countKeyBreaks(a.getName()) == parent_keybreaks};
        bool is_groupless_b{countKeyBreaks(b.getName()) == parent_keybreaks};

        if (is_groupless_a != is_groupless_b) {
            return is_groupless_a && !is_groupless_b;
        }

        // Groupless keys before keys
        // Compare groups first
        // Compare keynames at the end

        return keysWithGroupCompare(a.getName(), b.getName());
    };

    std::sort(keys.begin(), keys.end(), key_comparator);


    CustomSerializerInfo serializer_info{o, parent.getName().size() + 1, {""}};

    const kdb::Key *group_candidate{nullptr};

    for (const auto &k: keys) {
        if (group_candidate != nullptr) {
            if (k.getName().rfind(group_candidate->getName(), 0) == 0) {
                saveGroupKeyOut(serializer_info, group_candidate->getName());
                serializer_info.last_printed_group = group_candidate->getName();
            } else {
                saveLeafKeyWithGroupCandidate(serializer_info, *group_candidate);
            }
            group_candidate = nullptr;
        }

        if (!k.getString().empty()) {
            saveLeafKeyWithGroupCandidate(serializer_info, k);
        } else {
            group_candidate = &k;
        }
    }
    return false;
}
