//
// Created by felix on 28.10.19.
//

#ifdef FEAT_ELEKTRA

#include "kconfigelektra_p.h"
#include "kconfigdata.h"

#include <QDir>
#include <utility>
#include <iostream>
#include <merging/threewaymerge.hpp>
#include <merging/onesidestrategy.hpp>
#include <merging/mergeresult.hpp>
#include <merging/mergetask.hpp>
#include <merging/automergestrategy.hpp>


using namespace kdb;
using namespace kdb::tools::merging;


KConfigElektra::KConfigElektra(std::string appName, uint majorVersion, std::string profile) : app_name(std::move(
                appName)), major_version(majorVersion), profile(std::move(profile))
{

    this->kdb = new KDB();

    Key parentKey = Key(read_key(), KEY_END);

    this->ks = new KeySet();

    this->kdb->get(*this->ks, parentKey);

#ifdef _WIN32
    setLocalFilePath(QString::fromStdString("NUL")); //NUL behaves like /dev/null on windows
#else
    setLocalFilePath(QString::fromStdString("/dev/null"));
#endif
}

KConfigElektra::KConfigElektra(std::string appName, uint majorVersion) :
    KConfigElektra::KConfigElektra(std::move(appName), majorVersion, "current")
{}

KConfigElektra::~KConfigElektra()
{
    delete this->kdb;
    delete this->ks;
}

/**
 * Move both name iterators to the point, where they differ.
 *
 * @param parent the iterator of the parent key
 * @param child the iterator of the child key
 */
inline void traverseIterators(Key::iterator &parent, Key::iterator &child)
{
    /*
     * First item of iterator seems to be namespace, as namespaces can differ with cascading keys, we skip these.
     * (Loop initializer)
     *
     * Loop as long as the paths are the same and iterate through the path segments.
     */
    for (parent++, child++; parent.get() == child.get(); parent++, child++) {}
}

struct KConfigKey {
    std::string group;
    std::string key;
};

inline KConfigKey elektraKeyToKConfigKey(Key::iterator key, Key::iterator end)
{
    std::ostringstream group;
    std::string key_name;
    bool first = true;

    for (; key.get() != end.get(); key++) {
        if (!key_name.empty()) {
            if (!first) {
                group << "\x1d";
            } else {
                first = false;
            }
            group << key_name;
        }
        key_name = key.get();
    }

    return KConfigKey{
        group.str(),
        key_name,
    };
}

KConfigBackend::ParseInfo
KConfigElektra::parseConfig(const QByteArray & /*locale*/, KEntryMap &entryMap, KConfigBackend::ParseOptions options)
{
    //TODO error handling
    //TODO properly handle parse options (or figure out how they actually work) (part of #10)
    //TODO read meta keys for proper key options

    Key parentKey = Key(read_key(), KEY_END);

    bool oGlobal = options & ParseGlobal;

    this->kdb->get(*this->ks, parentKey);

    for(const auto &key: *this->ks) {
    //for (auto iterator = this->ks->cbegin(); iterator != this->ks->cend(); iterator++) {

        //Key key = iterator.get();

        if (!key.isBelowOrSame(parentKey)) {
            continue;
        }

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
            traverseIterators(parentIter, childIter);

            auto childEnd = key.end();

            auto configKey = elektraKeyToKConfigKey(childIter, childEnd);

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

inline Key kConfigGroupToElektraKey(const Key &writeKey, const std::string group, const std::string &keyname)
{
    Key key (writeKey.dup());
    if (!(group == "<default>" || group.empty())) {
        std::stringstream strstream(group);
        std::string tmp;

        while (std::getline(strstream, tmp, '\x1d')) {
            key.addBaseName(tmp);
        }
    }

    key.addBaseName(keyname);

    return key;
}

bool
KConfigElektra::writeConfig(const QByteArray & /*locale*/, KEntryMap &entryMap, KConfigBackend::WriteOptions options)
{
<<<<<<< HEAD
    //TODO merge
//TODO write group meta (#10)
=======
    ThreeWayMerge merger;

    auto autoStrategy = AutoMergeStrategy();
    auto strategy = OneSideStrategy(ConflictResolutionSide::OURS);

    merger.addConflictStrategy(&autoStrategy);
    merger.addConflictStrategy(&strategy);


>>>>>>> added threeway merge to elektra backend
    bool onlyGlobal = options & WriteGlobal;

    const KEntryMapConstIterator end = entryMap.constEnd();

    Key writeKey = Key(this->write_key(), KEY_END);
    Key mergeRoot = Key(this->read_key(), KEY_END);

    KeySet baseCut = this->ks->cut(mergeRoot);

    /*std::cout << std::endl << "Base: " << std::endl;

    for (auto iterator = this->ks->cbegin(); iterator != this->ks->cend(); iterator++) {
        Key k = iterator.get();
        std::cout << "\t" << iterator.get().getFullName() << " (" << k.isBelow(mergeRoot) << ", " << k.isBelow(writeKey) << ")" << std::endl;
    }

    std::cout << std::endl << "Base Cut: " << std::endl;

    for (auto iterator = baseCut.cbegin(); iterator != baseCut.cend(); iterator++) {
        std::cout << "\t" << iterator.get().getFullName() << std::endl;
    }*/

    KeySet ours;
    KeySet theirs;

    ours.copy(baseCut);
    theirs.copy(baseCut);

    for (KEntryMapConstIterator it = entryMap.constBegin(); it != end; ++it) {
        const KEntryKey &entryKey = it.key();
        KEntry entry = it.value();

        if (entryKey.mKey.isEmpty()) {
            continue;
        }

        if ((onlyGlobal && !entry.bGlobal) ||
            (!onlyGlobal && entry.bGlobal)) { // if in global mode, only write global entries
            continue;
        }

<<<<<<< HEAD
        Key eKey = kConfigGroupToElektraKey(write_key, entryKey.mGroup.toStdString(), entryKey.mKey.toStdString());

        if (entry.bDeleted || entry.mValue.isEmpty()) {
            this->ks->cut(eKey);
=======
        std::string eKeyName = writeKey.getFullName() + "/"
                               + kConfigGroupToElektraKey(entryKey.mGroup.toStdString(), entryKey.mKey.toStdString());

        if (entry.bDeleted || entry.mValue.isEmpty()) {
            ours.cut(Key(eKeyName, KEY_END));
>>>>>>> added threeway merge to elektra backend

            continue;
        }

        if (entry.bDirty) {
            eKey.set(it.value().mValue.toStdString());
            ours.append(eKey);
        }
    }

    /*std::cout << std::endl << "Ours: " << std::endl;

    for (auto iterator = ours.cbegin(); iterator != ours.cend(); iterator++) {
        std::cout << "\t" << iterator.get().getFullName() << std::endl;
    }*/

    this->kdb->get(theirs, mergeRoot);

    KeySet theirsCut = theirs.cut(mergeRoot);

    /*std::cout << std::endl << "Theirs: " << std::endl;

    for (auto iterator = theirs.cbegin(); iterator != theirs.cend(); iterator++) {
        std::cout << "\t" << iterator.get().getFullName() << std::endl;
    }

    std::cout << std::endl << "Theirs Cut: " << std::endl;

    for (auto iterator = theirsCut.cbegin(); iterator != theirsCut.cend(); iterator++) {
        std::cout << "\t" << iterator.get().getFullName() << std::endl;
    }*/

    MergeTask task(BaseMergeKeys(baseCut, mergeRoot), OurMergeKeys(ours, mergeRoot), TheirMergeKeys(theirsCut, mergeRoot), mergeRoot);

    MergeResult result = merger.mergeKeySet(task);

    if (result.hasConflicts()) {
        qInfo() << "merge found conflicts";
        KeySet conflictSet = result.getConflictSet();
        for (KeySetIterator it = conflictSet.cbegin(); it < conflictSet.cend(); it++) {
            Key key = it.get();
            qInfo() << "\tconflict in key: " << QString::fromStdString(key.getFullName());
            key.rewindMeta();
            Key meta;
            while ((meta = key.nextMeta()) != nullptr) {
                qInfo() << "\t\t" << QString::fromStdString(meta.getFullName()) << ": " << QString::fromStdString(meta.getString());
            }
        }
    }

    KeySet mergedKeys = result.getMergedKeys();

    /*std::cout << std::endl << "Merged Keys: " << std::endl;

    for (auto iterator = mergedKeys.cbegin(); iterator != mergedKeys.cend(); iterator++) {
        std::cout << "\t" << iterator.get().getFullName() << std::endl;
    }*/

    KeySet* old = this->ks;

    this->ks = new KeySet(theirs.release());
    delete old;

    this->ks->append(mergedKeys);

    this->kdb->set(*this->ks, writeKey);

    setLocalFilePath(QString::fromStdString(writeKey.getString()));

    return true;
}

bool KConfigElektra::isWritable() const
{
    return true;
}

QString KConfigElektra::nonWritableErrorMessage() const
{
    return QString();
}

KConfigBase::AccessMode KConfigElektra::accessMode() const
{
    return KConfigBase::ReadWrite;
}

void KConfigElektra::createEnclosing()
{
    //ignore
}

void KConfigElektra::setFilePath(const QString &file)
{

    qDebug() << file;
    Q_ASSERT_X(!QDir::isAbsolutePath(file), "change elektra app_name", "absolute file path");
    Q_ASSERT(file.contains(QChar::fromLatin1('/')));    //???

    this->app_name = file.toStdString();

    Key parentKey = Key(this->read_key(), KEY_END);

    this->kdb->get(*this->ks, parentKey);

#ifdef _WIN32
    setLocalFilePath(QString::fromStdString("NUL")); //NUL behaves like /dev/null on windows
#else
    setLocalFilePath(QString::fromStdString("/dev/null"));
#endif
}

bool KConfigElektra::lock()
{
    return true;
}

void KConfigElektra::unlock()
{

}

bool KConfigElektra::isLocked() const
{
    return false;
}

/**
 *
 * @return key to use for reading values (utilizes cascading keys)
 */
std::string KConfigElektra::read_key()
{
    return "/sw/org/kde/" + this->app_name + "/#" + std::to_string(this->major_version) + "/" + this->profile;
}

std::string KConfigElektra::write_key()
{
    return "user/sw/org/kde/" + this->app_name + "/#" + std::to_string(this->major_version) + "/" + this->profile;
}

KDB KConfigElektra::open_kdb()
{
    return KDB();
}

/**
 * Uses the format "elektra://sw/org/kde/<app_name>/<major_version>/profile"
 * @return
 */
QString KConfigElektra::uniqueGlobalIdentifier()
{
    std::string url;

    url.reserve(this->app_name.size() + this->profile.size() + 4 /* max assumed version length */ +
                23 /* url preface and filling chars */);

    url += "elektra://sw/org/kde/" + this->app_name + "/" +
           std::to_string(this->major_version) + "/" + this->profile;

    return QString::fromStdString(url);
}

#endif //FEAT_ELEKTRA