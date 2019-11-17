//
// Created by felix on 28.10.19.
//

#ifdef FEAT_ELEKTRA
#include "kconfigelektra.h"
#include "kconfigdata.h"

#include <QDir>
#include <utility>
#include <iostream>

using namespace kdb;

KConfigElektra::KConfigElektra(std::string appName, uint majorVersion, std::string profile) : app_name(std::move(
        appName)), major_version(majorVersion), profile(std::move(profile)) {

    this->kdb = new KDB();

    Key parentKey = Key(app_key(), KEY_END);

    this->ks = new KeySet();

    this->kdb->get(*this->ks, parentKey);

    setLocalFilePath(QString::fromStdString(parentKey.getString()));
}

KConfigElektra::KConfigElektra(std::string appName, uint majorVersion) :
        KConfigElektra::KConfigElektra(std::move(appName), majorVersion, "current") {}

KConfigElektra::~KConfigElektra() {
    delete this->kdb;
    delete this->ks;
}

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

    Key parentKey = Key(app_key(), KEY_END);

    bool oGlobal = options & ParseGlobal;

    this->kdb->get(*this->ks, parentKey);

    for(auto iterator = this->ks->cbegin(); iterator != this->ks->cend(); iterator++) {

        Key key = iterator.get();

        if(!key.isBelowOrSame(parentKey))
            continue;

        if (key.isDirectBelow(parentKey)) {

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

            auto parentIter = parentKey.begin();
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

    return ParseOk;
}

inline std::string kConfigGroupToElektraKey(std::string group, const std::string& keyname) {
    if (group == "<default>" || group.empty()) {
        return keyname;
    } else {
        std::replace(group.begin(), group.end(), '\x1d', '/');

        return group.append("/")
                .append(keyname);
    }
}

bool KConfigElektra::writeConfig(const QByteArray& /*locale*/, KEntryMap &entryMap, KConfigBackend::WriteOptions options) {
    //TODO merge
    bool onlyGlobal = options & WriteGlobal;

    const KEntryMapConstIterator end = entryMap.constEnd();

    Key write_key = Key(this->write_key(), KEY_END);

    this->kdb->get(*this->ks, write_key);

    setLocalFilePath(QString::fromStdString(write_key.getString()));

    for (KEntryMapConstIterator it = entryMap.constBegin(); it != end; ++it) {
        const KEntryKey &entryKey = it.key();
        KEntry entry = it.value();

        if (entryKey.mKey.isEmpty()) // ignore group attributes for the moment
            continue;

        if ((onlyGlobal && !entry.bGlobal) || (!onlyGlobal && entry.bGlobal)) { // if in global mode, only write global entries
            continue;
        }

        std::string eKeyName = write_key.getFullName() + "/"
                               + kConfigGroupToElektraKey(entryKey.mGroup.toStdString(), entryKey.mKey.toStdString());

        if (entry.bDeleted || entry.mValue.isEmpty()) {
            this->ks->cut(Key(eKeyName, KEY_END));

            continue;
        }

        if (entry.bDirty) {

            Key eKey = Key(eKeyName, KEY_END);
            eKey.set(it.value().mValue.toStdString());
            this->ks->append(eKey);
        }
    }

    this->kdb->set(*this->ks, write_key);

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

void KConfigElektra::createEnclosing() {    //ignore
    //qDebug() << "createEnclosing not implemented in Elektra backend";
}

void KConfigElektra::setFilePath(const QString &file) {

    qDebug() << file;
    Q_ASSERT_X(!QDir::isAbsolutePath(file), "change elektra app_name", "absolute file path");
    Q_ASSERT(file.contains(QChar::fromLatin1('/')));

    this->app_name = file.toStdString();

    Key parentKey = Key(this->app_key(), KEY_END);

    this->kdb->get(*this->ks, parentKey);

    setLocalFilePath(QString::fromStdString(parentKey.getString()));
}

bool KConfigElektra::lock() {
    return true;
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

QString KConfigElektra::uniqueGlobalIdentifier() {
    std::string url;

    url.reserve(this->app_name.size() + this->profile.size() + 4 /* max assumed version length */ + 0 /* url preface and filling chars */);

    url += "elektra:/" + this->app_name + "/" +
           std::to_string(this->major_version) + "/" + this->profile;

    return QString::fromStdString(url);
}

#endif //FEAT_ELEKTRA