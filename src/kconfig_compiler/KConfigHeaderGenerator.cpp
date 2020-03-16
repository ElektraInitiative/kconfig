/* This file is part of the KDE libraries
    Copyright (C) 2020 Tomaz Cananbrava (tcanabrava@kde.org)
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2003 Zack Rusin <zack@kde.org>
    Copyright (c) 2006 Michaël Larouche <michael.larouche@kdemail.net>
    Copyright (c) 2008 Allen Winter <winter@kde.org>
    Copyright (C) 2020 Tomaz Cananbrava (tcanabrava@kde.org)

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

#include "KConfigHeaderGenerator.h"
#include "KConfigCommonStructs.h"

#include <QTextStream>
#include <QDebug>
#include <iostream>

KConfigHeaderGenerator::KConfigHeaderGenerator(
    const QString &inputFile,
    const QString &baseDir,
    const KConfigParameters &cfg,
    ParseResult &result)
    : KConfigCodeGeneratorBase(inputFile, baseDir, baseDir + cfg.baseName + QLatin1Char('.') + cfg.headerExtension, cfg, result)
{
}

void KConfigHeaderGenerator::start() 
{
    KConfigCodeGeneratorBase::start();
    startHeaderGuards();
    createHeaders();

    beginNamespaces();

    createForwardDeclarations();

    doClassDefinition();

    endNamespaces();
    endHeaderGuards();
}

void KConfigHeaderGenerator::doClassDefinition()
{
    stream() << "class " << cfg().visibility << cfg().className << " : public " << cfg().inherits << '\n';
    startScope();

    // Add Q_OBJECT macro if the config need signals.
    if (!parseResult.signalList.isEmpty() || cfg().generateProperties) {
        stream() << "  Q_OBJECT\n";
    }
    stream() << "  public:\n";
    implementEnums();
    createConstructor();
    createDestructor();

    for (auto *entry : parseResult.entries) {
        const QString n = entry->name;
        const QString t = entry->type;

        const QString returnType = (cfg().useEnumTypes && t == QLatin1String("Enum"))
            ? enumType(entry, cfg().globalEnums)
            : cppType(t);

        createSetters(entry);
        createProperties(entry, returnType);
        createImmutableProperty(entry);
        createGetters(entry, returnType);
        createImmutableGetters(entry);
        createDefaultValueMember(entry);
        createItemAcessors(entry, returnType);
    }

    createSignals();
    stream() << "  protected:\n";
    createSingleton();

    // TODO: Move those to functions too.
    if (parseResult.hasNonModifySignals) {
        stream() << whitespace() << "bool usrSave() override;\n";
    }

    // Member variables
    if (!cfg().memberVariables.isEmpty()
        && cfg().memberVariables != QLatin1String("private")
        && cfg().memberVariables != QLatin1String("dpointer")) {
        stream() << "  " << cfg().memberVariables << ":\n";
    }

    // Class Parameters
    for (const auto &parameter : parseResult.parameters) {
        stream() << whitespace() << "" << cppType(parameter.type) << " mParam" << parameter.name << ";\n";
    }

    createNonDPointerHelpers();
    createDPointer();

    if (cfg().customAddons) {
        stream() << whitespace() << "// Include custom additions\n";
        stream() << whitespace() << "#include \"" << cfg().baseName << "_addons." << cfg().headerExtension << "\"\n";
    }

    endScope(ScopeFinalizer::Semicolon);
}

void KConfigHeaderGenerator::createHeaders()
{
    addHeaders(cfg().headerIncludes);
    if (cfg().headerIncludes.size()) {
        stream() << '\n';
    }

    if (!cfg().singleton && parseResult.parameters.isEmpty()) {
        addHeaders({QStringLiteral("qglobal.h")});
    }

    if (cfg().inherits == QLatin1String("KCoreConfigSkeleton")) {
        addHeaders({QStringLiteral("kcoreconfigskeleton.h")});
    } else {
        addHeaders({QStringLiteral("kconfigskeleton.h")});
    }

    addHeaders({QStringLiteral("QCoreApplication"), QStringLiteral("QDebug")});
    stream() << '\n';

    addHeaders(parseResult.includes);
    if (parseResult.includes.size()) {
        stream() << '\n';
    }
}

void KConfigHeaderGenerator::startHeaderGuards()
{
    const bool hasNamespace = !cfg().nameSpace.isEmpty();
    const QString namespaceName = QString(QString(cfg().nameSpace).replace(QLatin1String("::"), QLatin1String("_"))).toUpper();
    const QString namespaceStr = hasNamespace ? namespaceName + QLatin1Char('_') : QStringLiteral("");
    const QString defineName = namespaceStr + cfg().className.toUpper() + QStringLiteral("_H");

    stream() << "#ifndef " << defineName << '\n';
    stream() << "#define " << defineName << '\n';
    stream() << '\n';
}

void KConfigHeaderGenerator::endHeaderGuards() 
{
    stream() << '\n';
    stream() << "#endif";
    stream() << '\n';
    // HACK: Original files ended with two last newlines, add them.
    stream() << '\n';
}

void KConfigHeaderGenerator::implementChoiceEnums(const CfgEntry *entry, const CfgEntry::Choices &choices)
{
    const QList<CfgEntry::Choice> chlist = choices.choices;

    if (chlist.isEmpty()) {
        return;
    }

    QStringList values;
    for (const auto &choice : qAsConst(chlist)) {
        values.append(choices.prefix + choice.name);
    }

    if (choices.name().isEmpty()) {
        if (cfg().globalEnums) {
            stream() << whitespace() << "enum " << enumName(entry->name, entry->choices) << " { " << values.join(QStringLiteral(", ")) << " };\n";
        } else {
            // Create an automatically named enum
            stream() << whitespace() << "class " << enumName(entry->name, entry->choices) << '\n';
            stream() << whitespace() << "{\n";
            stream() << whitespace() << "  public:\n";
            stream() << whitespace() << "  enum type { " << values.join(QStringLiteral(", ")) << ", COUNT };\n";
            stream() << whitespace() << "};\n";
        }
    } else if (!choices.external()) {
        // Create a named enum
        stream() << whitespace() << "enum " << enumName(entry->name, entry->choices) << " { " << values.join(QStringLiteral(", ")) << " };\n";
    }
}

void KConfigHeaderGenerator::implementValueEnums(const CfgEntry *entry, const QStringList &values)
{
    if (values.isEmpty()) {
        return;
    }

    if (cfg().globalEnums) {
        // ### FIXME!!
        // make the following string table an index-based string search!
        // ###
        stream() << whitespace() << "enum " << enumName(entry->param) << " { " << values.join(QStringLiteral(", ")) << " };\n";
        stream() << whitespace() << "static const char* const " << enumName(entry->param) << "ToString[];\n";
    } else {
        stream() << whitespace() << "class " << enumName(entry->param) << '\n';
        stream() << whitespace() << "{\n";
        stream() << whitespace() << "  public:\n";
        stream() << whitespace() << "  enum type { " << values.join(QStringLiteral(", ")) << ", COUNT };\n";
        stream() << whitespace() << "  static const char* const enumToString[];\n";
        stream() << whitespace() << "};\n";
    }
}

void KConfigHeaderGenerator::implementEnums()
{
    if (!parseResult.entries.size()) {
        return;
    }

    for (const auto entry : parseResult.entries) {
        const CfgEntry::Choices &choices = entry->choices;
        const QStringList values = entry->paramValues;

        implementChoiceEnums(entry, choices);
        implementValueEnums(entry, values);
    }
   stream() << '\n';
}

void KConfigHeaderGenerator::createSignals()
{
    // Signal definition.
    const bool hasSignals = !parseResult.signalList.isEmpty();

    unsigned val = 1 << parseResult.signalList.size();
    if (!val) {
        std::cerr << "Too many signals to create unique bit masks" << std::endl;
        exit(1);
    }

    if (!hasSignals) {
        return;
    }

    stream() << "\n    enum {\n";
    val = 1;

    // HACK: Use C-Style for add a comma in all but the last element,
    // just to make the source generated code equal to the old one.
    // When we are sure, revert this to a range-based-for and just add
    // a last comma, as it's valid c++.
    for (int i = 0, end = parseResult.signalList.size(); i < end; i++) {
        auto signal = parseResult.signalList.at(i);
        stream() << whitespace() << "  " << signalEnumName(signal.name) << " = 0x"
                 <<
            #if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
                    hex
            #else
                    Qt::hex
            #endif
                 << val;
        if (i != end-1) {
            stream() << ",\n";
        }

        val <<= 1;
    }
    stream() << '\n';
    stream() << whitespace() << "};"
             <<
            #if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
                    dec
            #else
                    Qt::dec
            #endif
             << "\n\n";

    stream() << "  Q_SIGNALS:";
    for (const Signal &signal : parseResult.signalList) {
        stream() << '\n';
        if (!signal.label.isEmpty()) {
            stream() << whitespace() << "/**\n";
            stream() << whitespace() << "  " << signal.label << '\n';
            stream() << whitespace() << "*/\n";
        }
        stream() << whitespace() << "void " << signal.name << "(";
        QList<Param>::ConstIterator it, itEnd = signal.arguments.constEnd();
        for (it = signal.arguments.constBegin(); it != itEnd;) {
            Param argument = *it;
            QString type = param(argument.type);
            if (cfg().useEnumTypes && argument.type == QLatin1String("Enum")) {
                for (auto *entry : parseResult.entries) {
                    if (entry->name == argument.name) {
                        type = enumType(entry, cfg().globalEnums);
                        break;
                    }
                }
            }
            stream() << type << " " << argument.name;
            if (++it != itEnd) {
                stream() << ", ";
            }
        }
        stream() << ");\n";
    }
    stream() << '\n';

    stream() << "  private:\n";
    stream() << whitespace() << "void itemChanged(quint64 flags);\n";
    stream() << '\n';
}

void KConfigHeaderGenerator::createDPointer()
{
    if (!cfg().dpointer) {
        return;
    }

    // use a private class for both member variables and items
    stream() << "  private:\n";
    for (const auto &entry : parseResult.entries) {
        if (cfg().allDefaultGetters || cfg().defaultGetters.contains(entry->name)) {
            stream() << whitespace() << "";
            if (cfg().staticAccessors) {
                stream() << "static ";
            }
            stream() << cppType(entry->type) << " " << getDefaultFunction(entry->name) << "_helper(";
            if (!entry->param.isEmpty()) {
                stream() << " " << cppType(entry->paramType) << " i ";
            }
            stream() << ")" << Const() << ";\n";
        }
    }
    stream() << whitespace() << "" << cfg().className << "Private *d;\n";
}

void KConfigHeaderGenerator::createConstructor()
{
    if (cfg().singleton) {
        stream() << whitespace() << "static " << cfg().className << " *self();\n";
        if (parseResult.cfgFileNameArg) {
            stream() << whitespace() << "static void instance(const QString& cfgfilename);\n";
            stream() << whitespace() << "static void instance(KSharedConfig::Ptr config);\n";
        }
        return;
    }

    stream() << whitespace() << "" << cfg().className << "(";
    if (parseResult.cfgFileNameArg) {
        if (cfg().forceStringFilename)
            stream() << " const QString &cfgfilename" << (parseResult.parameters.isEmpty() ? " = QString()" : ", ");
        else
            stream() << " KSharedConfig::Ptr config" << (parseResult.parameters.isEmpty() ? " = KSharedConfig::openConfig()" : ", ");
    }

    bool first = true;
    for (const auto parameter : parseResult.parameters) {
        if (first) {
            first = false;
        } else {
            stream() << ",";
        }

        stream() << " " << param(parameter.type) << " " << parameter.name;
    }

    if (cfg().parentInConstructor) {
        if (parseResult.cfgFileNameArg || !parseResult.parameters.isEmpty()) {
            stream() << ",";
        }
        stream() << " QObject *parent = nullptr";
    }
    stream() << " );\n";
}

void KConfigHeaderGenerator::createDestructor()
{
    stream() << whitespace() << "~" << cfg().className << "();\n\n";
}

void KConfigHeaderGenerator::createForwardDeclarations()
{
    // Private class declaration
    if (cfg().dpointer) {
        stream() << "class " << cfg().className << "Private;\n\n";
    }
}

void KConfigHeaderGenerator::createProperties(const CfgEntry *entry, const QString& returnType)
{
    if (!cfg().generateProperties) {
        return;
    }
    stream() << whitespace() << "Q_PROPERTY(" << returnType << ' ' << getFunction(entry->name);
    stream() << " READ " << getFunction(entry->name);

    if (cfg().allMutators || cfg().mutators.contains(entry->name)) {
        const QString signal = changeSignalName(entry->name);
        stream() << " WRITE " << setFunction(entry->name);
        stream() << " NOTIFY " << signal;

        //If we have the modified signal, we'll also need
        //the changed signal as well
        Signal s;
        s.name = signal;
        s.modify = true;
        parseResult.signalList.append(s);
    } else {
        stream() << " CONSTANT";
    }
    stream() << ")\n";
}

void KConfigHeaderGenerator::createImmutableProperty(const CfgEntry *entry)
{
    if (!cfg().generateProperties) {
        return;
    }
    stream() << whitespace();
    stream() << "Q_PROPERTY(bool " << immutableFunction(entry->name);
    stream() << " READ " << immutableFunction(entry->name);
    stream() << " CONSTANT)\n";
}

void KConfigHeaderGenerator::createSetters(const CfgEntry *entry)
{
    // Manipulator
    if (!cfg().allMutators && !cfg().mutators.contains(entry->name)) {
        return;
    }

    stream() << whitespace() << "/**\n";
    stream() << whitespace() << "  Set " << entry->label << '\n';
    stream() << whitespace() << "*/\n";

    if (cfg().staticAccessors) {
        stream() << whitespace() << "static\n";
    }

    stream() << whitespace() << "void " << setFunction(entry->name) << "( ";
    if (!entry->param.isEmpty()) {
        stream() <<cppType(entry->paramType) << " i, ";
    }

    stream() << (cfg().useEnumTypes && entry->type == QLatin1String("Enum")
        ? enumType(entry, cfg().globalEnums)
        : param(entry->type));

    stream() << " v )";

    // function body inline only if not using dpointer
    // for BC mode
    if (!cfg().dpointer) {
        stream() << '\n';
        startScope();
        memberMutatorBody(entry);
        endScope();
        stream() << '\n';
    } else {
        stream() << ";\n\n";
    }
}

void KConfigHeaderGenerator::createGetters(const CfgEntry *entry, const QString &returnType)
{
    // Accessor
    stream() << whitespace() << "/**\n";
    stream() << whitespace() << "  Get " << entry->label << '\n';
    stream() << whitespace() << "*/\n";
    if (cfg().staticAccessors) {
        stream() << whitespace() << "static\n";
    }
    stream() << whitespace() << "";
    stream() << returnType;
    stream() << " " << getFunction(entry->name) << "(";
    if (!entry->param.isEmpty()) {
        stream() << " " << cppType(entry->paramType) << " i ";
    }
    stream() << ")" << Const();

    // function body inline only if not using dpointer
    // for BC mode
    if (!cfg().dpointer) {
        stream() << '\n';
        startScope();
        stream() << whitespace() << memberAccessorBody(entry, cfg().globalEnums);
        endScope();
        stream() << '\n';
    } else {
        stream() << ";\n\n";
    }
}

void KConfigHeaderGenerator::createImmutableGetters(const CfgEntry *entry)
{
    stream() << whitespace() << "/**\n";
    stream() << whitespace() << "  Is " << entry->label << " Immutable\n";
    stream() << whitespace() << "*/\n";
    // Immutable
    if (cfg().staticAccessors) {
        stream() << whitespace() << "static\n";
    }
    stream() << whitespace() << "";
    stream() << "bool " << immutableFunction(entry->name) << "(";
    if (!entry->param.isEmpty()) {
        stream() << " " << cppType(entry->paramType)<< " i ";
    }
    stream() << ")" << Const();
    // function body inline only if not using dpointer
    // for BC mode
    if (!cfg().dpointer) {
        stream() << '\n';
        startScope();
        memberImmutableBody(entry, cfg().globalEnums);
        endScope();
        stream() << '\n';
    } else {
        stream() << ";\n\n";
    }
}

void KConfigHeaderGenerator::createItemAcessors(const CfgEntry *entry, const QString &returnType)
{
    Q_UNUSED(returnType)

    // Item accessor
    if (!cfg().itemAccessors) {
        return;
    }
    stream() << whitespace() << "/**\n";
    stream() << whitespace() << "  Get Item object corresponding to " << entry->name << "()"
        << '\n';
    stream() << whitespace() << "*/\n";
    stream() << whitespace() << "Item" << itemType(entry->type) << " *"
        << getFunction(entry->name) << "Item(";
    if (!entry->param.isEmpty()) {
        stream() << " " << cppType(entry->paramType) << " i ";
    }
    stream() << ")";
    if (!cfg().dpointer) {
        stream() << '\n';
        startScope();
        stream() << whitespace() << itemAccessorBody(entry, cfg());
        endScope();
    } else {
        stream() << ";\n";
    }

    stream() << '\n';
}

void KConfigHeaderGenerator::createDefaultValueMember(const CfgEntry *entry)
{
    // Default value Accessor
    if (! ((cfg().allDefaultGetters || cfg().defaultGetters.contains(entry->name)) && !entry->defaultValue.isEmpty())) {
        return;
    }
    stream() << whitespace() << "/**\n";
    stream() << whitespace() << "  Get " << entry->label << " default value\n";
    stream() << whitespace() << "*/\n";
    if (cfg().staticAccessors) {
        stream() << whitespace() << "static\n";
    }
    stream() << whitespace() << "";
    if (cfg().useEnumTypes && entry->type == QLatin1String("Enum")) {
        stream() << enumType(entry, cfg().globalEnums);
    } else {
        stream() << cppType(entry->type);
    }
    stream() << " " << getDefaultFunction(entry->name) << "(";
    if (!entry->param.isEmpty()) {
        stream() << " " << cppType(entry->paramType) << " i ";
    }
    stream() << ")" << Const() << '\n';
    stream() << whitespace() << "{\n";
    stream() << whitespace() << "    return ";
    if (cfg().useEnumTypes && entry->type == QLatin1String("Enum")) {
        stream() << "static_cast<" << enumType(entry, cfg().globalEnums) << ">(";
    }
    stream() << getDefaultFunction(entry->name) << "_helper(";
    if (!entry->param.isEmpty()) {
        stream() << " i ";
    }
    stream() << ")";
    if (cfg().useEnumTypes && entry->type == QLatin1String("Enum")) {
        stream() << ")";
    }
    stream() << ";\n";
    stream() << whitespace() << "}\n";
    stream() << '\n';
}

void KConfigHeaderGenerator::createSingleton()
{
    // Private constructor for singleton
    if (!cfg().singleton) {
        return;
    }

    stream() << whitespace() << "" << cfg().className << "(";
    if (parseResult.cfgFileNameArg) {
        stream() << "KSharedConfig::Ptr config";
    }
    if (cfg().parentInConstructor) {
        if (parseResult.cfgFileNameArg) {
            stream() << ", ";
        }
        stream() << "QObject *parent = nullptr";
    }
    stream() << ");\n";
    stream() << whitespace() << "friend class " << cfg().className << "Helper;\n\n";
}

void KConfigHeaderGenerator::createNonDPointerHelpers()
{
    if (cfg().memberVariables == QLatin1String("dpointer")) {
        return;
    }

    QString group;
    for (auto *entry : parseResult.entries) {
        if (entry->group != group) {
            group = entry->group;
            stream() << '\n';
            stream() << whitespace() << "// " << group << '\n';
        }
        stream() << whitespace() << "" << cppType(entry->type) << " " << varName(entry->name, cfg());
        if (!entry->param.isEmpty()) {
            stream() << QStringLiteral("[%1]").arg(entry->paramMax + 1);
        }
        stream() << ";\n";

        if (cfg().allDefaultGetters || cfg().defaultGetters.contains(entry->name)) {
            stream() << whitespace() << "";
            if (cfg().staticAccessors) {
                stream() << "static ";
            }
            stream() << cppType(entry->type) << " " << getDefaultFunction(entry->name) << "_helper(";
            if (!entry->param.isEmpty()) {
                stream() << " " << cppType(entry->paramType) << " i ";
            }
            stream() << ")" << Const() << ";\n";
        }
    }

    stream() << "\n  private:\n";
    if (cfg().itemAccessors) {
        for (auto *entry : parseResult.entries) {
            stream() << whitespace() << "Item" << itemType(entry->type) << " *" << itemVar(entry, cfg());
            if (!entry->param.isEmpty()) {
                stream() << QStringLiteral("[%1]").arg(entry->paramMax + 1);
            }
            stream() << ";\n";
        }
    }

    if (parseResult.hasNonModifySignals) {
        stream() << whitespace() << "uint " << varName(QStringLiteral("settingsChanged"), cfg()) << ";\n";
    }
}
