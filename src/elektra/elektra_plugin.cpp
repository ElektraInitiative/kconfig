#include "elektra_plugin.h"

#include <elektra/kdbplugin.h>

using ckdb::keyNew;
using kdb::KeySet;

namespace {

/**
 * @brief This function returns a key set containing the contract of this plugin.
 *
 * @return A contract describing the functionality of this plugin.
 */
    KeySet contractKconfig(void) {
        return KeySet{30,
                      keyNew("system/elektra/modules/kconfig", KEY_VALUE, "kconfig plugin waits for your orders",
                             KEY_END),
                      keyNew("system/elektra/modules/kconfig/exports", KEY_END),
                      keyNew("system/elektra/modules/kconfig/exports/get", KEY_FUNC, elektraKconfigGet, KEY_END),
                      keyNew("system/elektra/modules/kconfig/exports/set", KEY_FUNC, elektraKconfigSet, KEY_END),

#include "readme_kconfig.c"

                      keyNew("system/elektra/modules/kconfig/infos/version", KEY_VALUE, PLUGINVERSION, KEY_END),
                      keyNew("system/elektra/modules/kconfig/config/needs/binary/meta", KEY_VALUE, "true",
                             KEY_END),
                      keyNew("system/elektra/modules/kconfig/config/needs/boolean/restore", KEY_VALUE, "#1",
                             KEY_END),
                      KS_END};
    }
}

/** @see elektraDocGet */
int elektraKconfigGet(ckdb::Plugin *handle ELEKTRA_UNUSED, ckdb::KeySet *returned, ckdb::Key *parentKey) {
    kdb::Key parent = kdb::Key(parentKey);
    kdb::KeySet keys = kdb::KeySet(returned);

    if (parent.getName() == "system/elektra/modules/kconfig") {
        keys.append(contractKconfig());
        parent.release();
        keys.release();

        return ELEKTRA_PLUGIN_STATUS_SUCCESS;
    }
    return ELEKTRA_PLUGIN_STATUS_SUCCESS;
}

/** @see elektraDocSet */
int elektraKconfigSet(ckdb::Plugin *handle ELEKTRA_UNUSED, ckdb::KeySet *returned ELEKTRA_UNUSED,
                      ckdb::Key *parentKey ELEKTRA_UNUSED) {
    return 0;
}


ckdb::Plugin *elektraPluginSymbol() {
    return elektraPluginExport("kconfig",
                               ELEKTRA_PLUGIN_GET, &elektraKconfigGet,
                               ELEKTRA_PLUGIN_SET, &elektraKconfigSet,
                               ELEKTRA_PLUGIN_END);
}
