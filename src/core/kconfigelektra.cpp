//
// Created by felix on 28.10.19.
//

#include "kconfigelektra.h"
#include "kconfigdata.h"

#include <utility>
#include <iostream>

using namespace kdb;

KConfigElektra::KConfigElektra(std::string appName, uint majorVersion, std::string profile) : app_name(std::move(
        appName)), major_version(majorVersion), profile(std::move(profile)) {}

KConfigElektra::KConfigElektra(std::string appName, uint majorVersion) :
        KConfigElektra::KConfigElektra(std::move(appName), majorVersion, "current") {}

KConfigElektra::~KConfigElektra() = default;

/**
 * Move both name iterators to the point, where they differ.
 *
 * @param parent the iterator of the parent key
 * @param child the iterator of the child key
 */
inline void traverseIterators(Key::iterator* parent, Key::iterator* child) {
    /*
     * First item of iterator seems to be namespace, as namespaces can differ with cascading keys, we skip these.
     * (Loop initializer)
     *
     * Loop as long as the paths are the same and iterate through the path segments.
     */
    for ((*parent)++, (*child)++; parent->get() == child->get(); (*parent)++, (*child)++);
}

struct KConfigKey {
    std::string group;
    std::string key;
};

inline KConfigKey elektraKeyToKConfigKey(Key::iterator* key, Key::iterator* end) {
    std::ostringstream group;
    std::string key_name;
    bool first = true;

    for(; (*key) != (*end); (*key)++) {
        if (!key_name.empty()) {
            if (!first) {
                group << "\x1d";
            } else {
                first = false;
            }
            group << key_name;
        }
        key_name = key->get();
    }

    return KConfigKey {
        group.str(),
        key_name,
    };
}

KConfigBackend::ParseInfo
KConfigElektra::parseConfig(const QByteArray& /*locale*/, KEntryMap &entryMap, KConfigBackend::ParseOptions options) {
    //TODO error handling
    //TODO properly handle parse options (or figure out how they actually work)

    KDB kdb = KConfigElektra::open_kdb();
    KeySet keySet;

    bool oGlobal = options & ParseGlobal;

    Key parent_key = Key(app_key());

    kdb.get(keySet, parent_key);
    keySet = keySet.cut(parent_key);

    Key key;


    while((key = keySet.pop()) != nullptr) {

        if (key.isDirectBelow(parent_key)) {

            KEntryMap::EntryOptions entryOptions = nullptr;

            if (oGlobal) {
                entryOptions |= KEntryMap::EntryGlobal;
            }

            entryMap.setEntry(
                    QByteArray::fromStdString("<default>"),
                    QByteArray::fromStdString(key.getBaseName()),
                    QByteArray::fromStdString(key.getString()),
                    entryOptions
            );
        } else {

            KEntryMap::EntryOptions entryOptions = nullptr;

            if (oGlobal) {
                entryOptions |= KEntryMap::EntryGlobal;
            }

            auto parentIter = parent_key->begin();
            auto childIter = key.begin();
            traverseIterators(&parentIter, &childIter);

            auto childEnd = key.end();

            auto configKey = elektraKeyToKConfigKey(&childIter, &childEnd);

            entryMap.setEntry(
                    QByteArray::fromStdString(configKey.group),
                    QByteArray::fromStdString(configKey.key),
                    QByteArray::fromStdString(key.getString()),
                    entryOptions
            );
        }
    }

    kdb.close();

    return ParseOk;
}

inline std::string kConfigGroupToElektraKey(std::string group, const std::string& keyname) {
    std::replace(group.begin(), group.end(), '\x1d', '/');

    return group.append("/")
                .append(keyname);
}

bool KConfigElektra::writeConfig(const QByteArray& /*locale*/, KEntryMap &entryMap, KConfigBackend::WriteOptions options) {

    bool onlyGlobal = options & WriteGlobal;

    const KEntryMapConstIterator end = entryMap.constEnd();
    KeySet keySet;

    Key parent_key = Key(write_key());
    KDB kdb = KConfigElektra::open_kdb();

    kdb.get(keySet, parent_key);

    for (KEntryMapConstIterator it = entryMap.constBegin(); it != end; ++it) {
        const KEntryKey &entryKey = it.key();
        KEntry entry = it.value();

        if (entryKey.mKey.isEmpty()) // ignore group attributes for the moment
            continue;

        if ((onlyGlobal && !entry.bGlobal) || (!onlyGlobal && entry.bGlobal)) { // if in global mode, only write global entries
            continue;
        }

        std::string eKeyName = parent_key.getFullName() + "/"
                + kConfigGroupToElektraKey(entryKey.mGroup.toStdString(), entryKey.mKey.toStdString());

        if (entry.bDeleted || entry.mValue.isEmpty()) {
            keySet.cut(Key(eKeyName, KEY_END));

            continue;
        }

        if (entry.bDirty) {

            Key eKey = Key(eKeyName, KEY_END);
            eKey.set(it.value().mValue.toStdString());
            keySet.append(eKey);
        }
    }

    kdb.set(keySet, parent_key);

    kdb.close();

    return true;
}

bool KConfigElektra::isWritable() const {
    return true;
}

QString KConfigElektra::nonWritableErrorMessage() const {
    return QString();
}

KConfigBase::AccessMode KConfigElektra::accessMode() const {
    return KConfigBase::ReadWrite;
}

void KConfigElektra::createEnclosing() {
    qDebug() << "createEnclosing not implemented in Elektra backend";
}

void KConfigElektra::setFilePath(const QString &) {
    qDebug() << "setFilePath(const QString) not implmeneted in Elektra backend";
}

bool KConfigElektra::lock() {
    return false;
}

void KConfigElektra::unlock() {

}

bool KConfigElektra::isLocked() const {
    return false;
}

std::string KConfigElektra::app_key() {
    return "/sw/org/kde/" + this->app_name + "/#" + std::to_string(this->major_version) + "/" + this->profile;
}

std::string KConfigElektra::write_key() {
    return "user/sw/org/kde/" + this->app_name + "/#" + std::to_string(this->major_version) + "/" + this->profile;
}

KDB KConfigElektra::open_kdb() {
    return KDB();
}
