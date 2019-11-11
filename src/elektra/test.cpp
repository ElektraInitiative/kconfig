#include "parser.h"
#include "serializer.h"

#include <iostream>

using kdb::KeySet;
using kdb::Key;

#include "elektra_plugin.h"

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
    if (elektraPluginSymbol() == nullptr) {
        return -1;
    }

    KeySet keySet = parseFileToKeySet(config_file, parent);

    saveKeySetToFile(std::cout, keySet, parent);

    return 0;
}
