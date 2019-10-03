//
// Created by felix on 10/2/19.
//

#include <KConfig>
#include <KConfigGroup>
#include <iostream>
#include <random>

void serialAccess(KConfig* config) {
    KConfigGroup longGroup = config->group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");

    for(int i = 0; i < 1000; i++) {
        QString name = QString::fromStdString("key.");
        name.append(QString::number(i));

        assert(longGroup.hasKey(name));
        assert(i % 17 * 4 == longGroup.readEntry(name).toInt());
    }
}

void randomAccess(KConfig* config) {
    KConfigGroup longGroup = config->group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");
    std::vector<int> indices;
    indices.reserve(1000);

    for(int i = 0; i < 1000; i++) {
        indices.push_back(i);
    }

    std::shuffle(indices.begin(), indices.end(), std::mt19937(std::random_device()()));

    for(auto i = indices.begin(); i < indices.end(); i++) {
        QString name = QString::fromStdString("key.");
        int j = *i.base();

        name.append(QString::number(j));

        if (longGroup.hasKey(name)) {
            assert(j % 17 * 4 == longGroup.readEntry(name).toInt());
        } else {
            longGroup.writeEntry(name, j % 17 * 4);
        }
    }
}

int main() {
    KConfig config(QString::fromStdString("/home/felix/CLionProjects/kconfig/test/kconfig_bench_testrc"));

    KConfigGroup test = config.group("Test");

    if (test.hasKey("test")) {
        std::cout << "Ready for testing!" << std::endl;

        serialAccess(&config);
        randomAccess(&config);
    } else {
        test.writeEntry("test", "test");
        KConfigGroup longGroup = config.group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");

        for(int i = 0; i < 1000; i++) {
            QString name = QString::fromStdString("key.");
            name.append(QString::number(i));
            longGroup.writeEntry(name, i % 17 * 4);
        }

        config.sync();

        std::cout << "Keys written. Run again for testing!" << std::endl;
    }

}