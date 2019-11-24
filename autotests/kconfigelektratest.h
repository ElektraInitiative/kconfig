/* This file is part of the KDE libraries
    Copyright (C) 2019 Felix Resch (resch.felix@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifdef FEAT_ELEKTRA

#ifndef KCONFIG_KCONFIGELEKTRATEST_H
#define KCONFIG_KCONFIGELEKTRATEST_H

#include <QObject>

class KConfigElektraTest : public QObject
{

Q_OBJECT

private Q_SLOTS:

    void testBackend ();

    void initTestCase ();

    void cleanupTestCase ();

    void testKConfigElektraRead ();

    void testKConfigElektraWrite ();

    void testKConfigElektraOpenSimpleName ();
};

#endif //KCONFIG_KCONFIGELEKTRATEST_H
#endif //FEAT_ELEKTRA