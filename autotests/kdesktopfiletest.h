/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KDESKTOPFILETEST_H
#define KDESKTOPFILETEST_H

#include <QObject>
#include <kconfigcore_export.h>

class KDesktopFileTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testRead();
#if KCONFIGCORE_ENABLE_DEPRECATED_SINCE(5, 0)
    void testReadDirectory();
#endif
    void testReadLocalized_data();
    void testReadLocalized();
    void testUnsuccessfulTryExec();
    void testSuccessfulTryExec();
    void testActionGroup();
    void testIsAuthorizedDesktopFile();
    void testTryExecWithAuthorizeAction();
    void testLocateLocal_data();
    void testLocateLocal();

};

#endif /* KDESKTOPFILETEST_H */
