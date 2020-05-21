#ifndef HIME_QT5_PLUGIN_H
#define HIME_QT5_PLUGIN_H

#include <QtCore/QStringList>
#include <QtGui/qpa/qplatforminputcontextplugin_p.h>

#include "hime-imcontext-qt.h"


class QHimePlatformInputContextPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "hime.json")
    QStringList keys() const;
    QHimePlatformInputContext *create(const QString& system, const QStringList& paramList);
};

#endif
