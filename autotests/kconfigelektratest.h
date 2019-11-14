//
// Created by felix on 28.10.19.
//

#ifdef FEAT_ELEKTRA

#ifndef KCONFIG_KCONFIGELEKTRATEST_H
#define KCONFIG_KCONFIGELEKTRATEST_H

#include <QObject>

class KConfigElektraTest : public QObject {

    Q_OBJECT

private Q_SLOTS:

    void testBackend();
    void initTestCase();
    void cleanupTestCase();
    void testKConfigElektraRead();

    void testKConfigElektraWrite();
    void testKConfigElektraOpenSimpleName();
};


#endif //KCONFIG_KCONFIGELEKTRATEST_H
#endif //FEAT_ELEKTRA