//
// Created by felix on 28.10.19.
//

#ifndef KCONFIG_KCONFIGELEKTRATEST_H
#define KCONFIG_KCONFIGELEKTRATEST_H

#include <QObject>

class KConfigElektraTest : public QObject {

    Q_OBJECT

private Q_SLOTS:

    void testBackend();
    void initTestCase();
    void cleanupTestCase();

};


#endif //KCONFIG_KCONFIGELEKTRATEST_H
