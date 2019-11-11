#ifndef KCONFIG_ELEKTRA_PLUGIN_H
#define KCONFIG_ELEKTRA_PLUGIN_H

#include <elektra/kdbplugin.hpp>

extern "C" {

int elektraKconfigGet(ckdb::Plugin *handle, ckdb::KeySet *ks, ckdb::Key *parentKey);
int elektraKconfigSet(ckdb::Plugin *handle, ckdb::KeySet *ks, ckdb::Key *parentKey);

__attribute__ ((visibility ("default"))) ckdb::Plugin *elektraPluginSymbol(void);

} // end extern "C"


#endif //KCONFIG_ELEKTRA_PLUGIN_H
