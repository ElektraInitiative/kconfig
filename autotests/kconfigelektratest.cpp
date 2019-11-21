//
// Created by felix on 28.10.19.
//

#include <QtTest/QTest>
#include <kconfigdata.h>
#include <kconfigelektra_p.h>
#include <KConfig>
#include <KConfigGroup>
#include <iostream>
#include "kconfigelektratest.h"

#ifdef FEAT_ELEKTRA

QTEST_MAIN(KConfigElektraTest)

using namespace kdb;

void KConfigElektraTest::testBackend ()
{

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
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/hello"), "hello");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Text Editor/preferred"), "kate");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/Name"), "Arial");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/Color"), "green");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratestglobals/#0/current/Text Editor/Global/The Earth is"),
             "Globular ... err GLOBAL!");

    QCOMPARE(ks.lookup(Key("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/DELETE", KEY_END)), nullptr);

    kdb_local_check.close();
}

void KConfigElektraTest::initTestCase ()
{
    KDB kdb_local;

    KeySet ks;

    kdb_local.get(ks, "user/sw/org/kde/elektratest/#0/current/");

    ks.append(Key("user/sw/org/kde/elektratest/#0/current/hello", KEY_VALUE, "hello", KEY_END));
    ks.append(Key("user/sw/org/kde/elektratest/#0/current/Text Editor/preferred", KEY_VALUE, "kate", KEY_END));
    ks.append(Key("user/sw/org/kde/elektratest/#0/current/Text Editor/Font/Name", KEY_VALUE, "Arial", KEY_END));

    kdb_local.set(ks, "user/sw/org/kde/elektratest/#0/current/");

    kdb_local.close();
}

void KConfigElektraTest::cleanupTestCase ()
{
    KDB kdb_local;

    KeySet ks;

    kdb_local.get(ks, "user/sw/org/kde/elektratest/");

    ks.cut(Key("user/sw/org/kde/elektratest/", KEY_END));

    kdb_local.set(ks, "user/sw/org/kde/elektratest/");

    ks.clear();

    kdb_local.get(ks, "user/sw/org/kde/elektratestglobals/");

    ks.cut(Key("user/sw/org/kde/elektratestglobals/", KEY_END));

    kdb_local.set(ks, "user/sw/org/kde/elektratestglobals/");

    kdb_local.close();
}

void KConfigElektraTest::testKConfigElektraRead ()
{
    KConfig kConfig(ElektraInfo{"elektratest", 0, "current"});

    KConfigGroup group_default = kConfig.group("<default>");

    QCOMPARE(group_default.readEntry("hello", ""), "hello");

    KConfigGroup group_text_editor = kConfig.group("Text Editor");

    QCOMPARE(group_text_editor.readEntry("preferred", ""), "kate");

    KConfigGroup group_text_editor_font = group_text_editor.group("Font");

    QCOMPARE(group_text_editor_font.readEntry("Name", ""), "Arial");
}

void KConfigElektraTest::testKConfigElektraWrite ()
{
    KConfig kConfig(ElektraInfo("elektratest", 0, "current"));

    KConfigGroup group_default = kConfig.group("<default>");
    KConfigGroup group_test = kConfig.group("Test");
    KConfigGroup group_test_with_space = kConfig.group("Test With Space");
    KConfigGroup group_with_subgroup = group_test_with_space.group("Subgroup");

    group_test.writeEntry("Testing", "In Progress");
    group_test_with_space.writeEntry("Still in", "Progress");
    group_with_subgroup.writeEntry("This subgroup is", "also being tested!");
    group_default.writeEntry("This is default", "or is it...");

    kConfig.sync();

    KDB kdb_local_check;
    KeySet ks = KeySet();

    kdb_local_check.get(ks, "user/sw/org/kde/elektratest/#0/current");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/This is default"), "or is it...");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Test/Testing"), "In Progress");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Test With Space/Still in"), "Progress");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#0/current/Test With Space/Subgroup/This subgroup is"),
             "also being tested!");
}

void KConfigElektraTest::testKConfigElektraOpenSimpleName ()
{

    KConfig kConfig("elektratest");

    KConfigGroup group_default = kConfig.group("<default>");
    KConfigGroup group_test = kConfig.group("Test");
    KConfigGroup group_test_with_space = kConfig.group("Test With Space");
    KConfigGroup group_with_subgroup = group_test_with_space.group("Subgroup");

    group_test.writeEntry("Testing", "In Progress");
    group_test_with_space.writeEntry("Still in", "Progress");
    group_with_subgroup.writeEntry("This subgroup is", "also being tested!");
    group_default.writeEntry("This is default", "or is it...");

    kConfig.sync();

    KDB kdb_local_check;
    KeySet ks = KeySet();

    kdb_local_check.get(ks, "user/sw/org/kde/elektratest/#5/current");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#5/current/This is default"), "or is it...");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#5/current/Test/Testing"), "In Progress");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#5/current/Test With Space/Still in"), "Progress");
    QCOMPARE(ks.get<std::string>("user/sw/org/kde/elektratest/#5/current/Test With Space/Subgroup/This subgroup is"),
             "also being tested!");
}

static void setCommonData(KConfig* config) {
    KConfigGroup group = config->group("Common");

    group.writeEntry("Test1", "blah blah blah");
    group.writeEntry("Test2", "turn it up");
}

void KConfigElektraTest::testThreewayMerge ()
{
    KConfig a (ElektraInfo {"elektratest", 1});
    KConfig b (ElektraInfo {"elektratest", 1});

    setCommonData(&a);
    setCommonData(&b);

    KConfigGroup group1 = a.group("Specific 1");

    group1.writeEntry("Test1", "the tribe");
    group1.writeEntry("Test2", "namaste");

    KConfigGroup group2 = b.group("Specific 2");

    group2.writeEntry("Test1", "shiva");
    group2.writeEntry("Test2", "mandala");

    KConfigGroup conflictGroupA = a.group("Conflict");

    conflictGroupA.writeEntry("Test1", "oracle");
    conflictGroupA.writeEntry("Test2", "the people");

    KConfigGroup configGroupB = b.group("Conflict");

    configGroupB.writeEntry("Test1", "neverland");
    configGroupB.writeEntry("Test2", "opa");

    a.sync();

    //TODO assert B

    b.sync();

    KConfig config (ElektraInfo {"elektratest", 1});

    KConfigGroup group = config.group("Common");

    QCOMPARE(group.readEntry("Test1"), "blah blah blah");
    QCOMPARE(group.readEntry("Test2"), "turn it up");

    group = config.group("Specific 1");

    QCOMPARE(group.readEntry("Test1"), "the tribe");
    QCOMPARE(group.readEntry("Test2"), "namaste");

    group = config.group("Specific 2");

    QCOMPARE(group.readEntry("Test1"), "shiva");
    QCOMPARE(group.readEntry("Test2"), "mandala");

    group = config.group("Conflict");

    QCOMPARE(group.readEntry("Test1"), "neverland");
    QCOMPARE(group.readEntry("Test2"), "opa");

}

#endif //FEAT_ELEKTRA