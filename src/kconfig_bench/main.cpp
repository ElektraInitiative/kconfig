//
// Created by felix on 10/2/19.
//

#include <KConfig>
#include <KConfigGroup>
#include <iostream>
#include <random>
#include <kconfigbackend_p.h>

#define KEYS 100

void serialAccess(KConfig* config) {
    KConfigGroup longGroup = config->group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");

    for(int i = 0; i < KEYS; i++) {
        QString name = QString::fromStdString("key.");
        name.append(QString::number(i));

        assert(longGroup.hasKey(name));
        assert(i % 17 * 4 == longGroup.readEntry(name).toInt());
    }
}

void randomAccess(KConfig* config) {
    KConfigGroup longGroup = config->group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");
    std::vector<int> indices;
    indices.reserve(KEYS);

    for(int i = 0; i < KEYS; i++) {
        indices.push_back(i);
    }

    auto rng = std::mt19937(std::random_device()());
    std::uniform_int_distribution<int> rdist(0, KEYS);

    for(auto i = indices.begin(); i < indices.end(); i++) {
        QString name = QString::fromStdString("key.");
        int j = rdist(rng);

        name.append(QString::number(j));

        if (longGroup.hasKey(name)) {
            assert(j % 17 * 4 == longGroup.readEntry(name).toInt());
        } else {
            longGroup.writeEntry(name, j % 17 * 4);
        }
    }
}

void deleteKeys(KConfig * config, int keys) {
    KConfigGroup longGroup = config->group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");

    for (int j = 0; j < keys; ++j) {
        QString name = QString::fromStdString("key.");
        name.append(QString::number(j));
        longGroup.deleteEntry(name);
    }
}

void writeKeys(KConfig * config) {
    KConfigGroup longGroup = config->group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");

    for(int i = 0; i < KEYS; i++) {
        QString name = QString::fromStdString("key.");
        name.append(QString::number(i));
        longGroup.writeEntry(name, i % 17 * 4);
    }
}

int main() {

#ifdef FEAT_ELEKTRA
    KConfig config(ElektraInfo{"kconf_bench", 0, "default"});
#else
    KConfig config(QString::fromStdString("/home/felix/CLionProjects/kconfig/test/kconfig_bench_testrc"));
#endif //USE_ELEKTRA

    std::cout << "Parsing...";

    KConfigGroup test = config.group("Test");

    if (test.hasKey("keys") && test.readEntry("keys").toInt() == KEYS) {
        std::cout << "Testing...";

        serialAccess(&config);
        randomAccess(&config);

        std::cout << "Done!" << std::endl;
    } else {
        int keys = test.readEntry("keys").toInt();

        test.writeEntry("test", "test");
        test.writeEntry("keys", KEYS);
        KConfigGroup longGroup = config.group("Group with very long group name that should not be necessary but could possibly mess with lookup times...apwj rewuihrt 9uwhh weuihui aehtuihe ");

        deleteKeys(&config, keys);

        writeKeys(&config);

        config.sync();

        std::cout << "Keys written. Run again for testing!" << std::endl;
    }

}