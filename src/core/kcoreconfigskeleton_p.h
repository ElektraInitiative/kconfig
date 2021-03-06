/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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

#ifndef KCORECONFIGSKELETON_P_H
#define KCORECONFIGSKELETON_P_H

#include "kcoreconfigskeleton.h"

class KCoreConfigSkeletonPrivate
{
public:
    KCoreConfigSkeletonPrivate()
        : mCurrentGroup(QStringLiteral("No Group")), mUseDefaults(false)
    {}
    ~KCoreConfigSkeletonPrivate()
    {
        KConfigSkeletonItem::List::ConstIterator it;
        for (it = mItems.constBegin(); it != mItems.constEnd(); ++it) {
            delete *it;
        }
    }
    QString mCurrentGroup;

    KSharedConfig::Ptr mConfig; // pointer to KConfig object

    KConfigSkeletonItem::List mItems;
    KConfigSkeletonItem::Dict mItemDict;

    bool mUseDefaults;
};

class KConfigSkeletonItemPrivate
{
public:
    KConfigSkeletonItemPrivate()
        : mIsImmutable(true)
        , mWriteFlags(KConfigBase::Normal)
    {}
    virtual ~KConfigSkeletonItemPrivate();
    bool mIsImmutable; ///< Indicates this item is immutable
    KConfigBase::WriteConfigFlags mWriteFlags; ///< The flags to pass to calls of writeEntry() and revertToDefault()

    QString mLabel; ///< The label for this item
    QString mToolTip; ///< The ToolTip text for this item
    QString mWhatsThis; ///< The What's This text for this item
    KConfigGroup mConfigGroup; ///< KConfigGroup, allow to read/write item in nested groups
    QHash<QString, QString> mValues; /// The values used for ItemEnum's choices, name -> value (if set)

    // HACK: Necessary to avoid introducing new virtuals in KConfigSkeletonItem
    std::function<bool()> mIsDefaultImpl;
    std::function<bool()> mIsSaveNeededImpl;
};

class KPropertySkeletonItemPrivate : public KConfigSkeletonItemPrivate
{
public:
    KPropertySkeletonItemPrivate(QObject *object, const QByteArray &propertyName, const QVariant &defaultValue)
        : KConfigSkeletonItemPrivate()
        , mObject(object)
        , mPropertyName(propertyName)
        , mDefaultValue(defaultValue)
        , mConstDefaultValue(defaultValue)
    {
        mIsImmutable = false;
    }

    QObject *mObject;
    const QByteArray mPropertyName;
    QVariant mDefaultValue;
    const QVariant mConstDefaultValue;
    QVariant mReference;
    QVariant mLoadedValue;
    std::function<void()> mNotifyFunction;
};


#endif
