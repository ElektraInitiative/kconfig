//
// Created by felix on 28.10.19.
//

#include <QtTest/QTest>
#include <kconfigdata.h>
#include <kconfigelektra.h>
#include "kconfigelektratest.h"

QTEST_MAIN(KConfigElektraTest)



void KConfigElektraTest::testBackend() {

    KDB kdb_local;

    KeySet ks;

    kdb_local.get(ks, "user/sw/org/kde/elektratest/#0/current/");

    ks.append(Key("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/DELETE", KEY_VALUE, "delete me", KEY_END));

    kdb_local.set(ks, "user/sw/org/kde/elektratest/#0/current/");

    kdb_local.close();
    ks.clear();

    KEntryMap map;

    std::string elektratest = "elektratest";

    KConfigElektra elektraBackend(elektratest, 0);
    KConfigElektra elektraGlobalBackend("elektratestglobals", 0);

    elektraBackend.parseConfig(nullptr, map, nullptr);

    QCOMPARE(map.getEntry("<default>", "hello"), "hello");
    QCOMPARE(map.getEntry("Text Editor", "preferred"), "kate");
    QCOMPARE(map.getEntry("Text Editor\x1d""Font", "Name"), "Arial");

    map.setEntry(QByteArray::fromStdString("Text Editor\x1d""Font"),
            QByteArray::fromStdString("Color"),
            QString::fromStdString("green"),
            KEntryMap::EntryDirty);

    map.setEntry(QByteArray::fromStdString("Text Editor\x1d""Global"),
            QByteArray::fromStdString("The Earth is"),
            QString::fromStdString("Globular ... err GLOBAL!"),
            KEntryMap::EntryDirty | KEntryMap::EntryGlobal);

    map.setEntry(QByteArray::fromStdString("Text Editor\x1d""Font"),
                 QByteArray::fromStdString("DELETE"),
                 QByteArray(),
                 KEntryMap::EntryDeleted);

    elektraBackend.writeConfig(nullptr, map, nullptr);
    elektraGlobalBackend.writeConfig(nullptr, map, KConfigBackend::WriteGlobal);

    KDB kdb_local_check;
    ks = KeySet();

    kdb_local_check.get(ks, "user/sw/org/kde/elektratest/#0/current");
    kdb_local_check.get(ks, "user/sw/org/kde/elektratestglobals/#0/current");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/hello/"), "hello");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Text Editor/preferred"), "kate");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/Name"), "Arial");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/Color"), "green");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratestglobals/#0/current/Text Editor/Global/The Earth is"), "Globular ... err GLOBAL!");

    QCOMPARE(ks.lookup(Key("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/DELETE")), nullptr);

    kdb_local_check.close();
}

void KConfigElektraTest::initTestCase() {
    KDB kdb_local;

    KeySet ks;

    kdb_local.get(ks, "user/sw/org/kde/elektratest/#0/current/");

    ks.append(Key("user/sw/org/kde/elektratest/#0/current/hello", KEY_VALUE, "hello", KEY_END));
    ks.append(Key("user/sw/org/kde/elektratest/#0/current/Text Editor/preferred", KEY_VALUE, "kate", KEY_END));
    ks.append(Key("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/Name", KEY_VALUE, "Arial", KEY_END));

    kdb_local.set(ks, "user/sw/org/kde/elektratest/#0/current/");

    kdb_local.close();
}

void KConfigElektraTest::cleanupTestCase() {
    KDB kdb_local;

    KeySet ks;

    kdb_local.get(ks, "user/sw/org/kde/elektratest/#0/current/");

    ks.cut(Key("user/sw/org/kde/elektratest/#0/current/", KEY_END));

    kdb_local.set(ks, "user/sw/org/kde/elektratest/#0/current/");

    ks.clear();

    kdb_local.get(ks, "user/sw/org/kde/elektratestglobals/#0/current/");

    ks.cut(Key("user/sw/org/kde/elektratestglobals/#0/current/", KEY_END));

    kdb_local.set(ks, "user/sw/org/kde/elektratestglobals/#0/current/");

    kdb_local.close();
}

