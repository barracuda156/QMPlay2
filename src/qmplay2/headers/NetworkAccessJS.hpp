#pragma once

#include <QtQml/QJSValue>
#include <QVariant>
#include <QObject>

class NetworkAccess;

class NetworkAccessJS : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE NetworkAccessJS();
    NetworkAccessJS(NetworkAccess &net, QObject *parent = nullptr);
    ~NetworkAccessJS();

public:
    Q_INVOKABLE QString urlEncoded();

    Q_INVOKABLE void setCustomUserAgent(const QString &customUserAgent);
    Q_INVOKABLE void setMaxDownloadSize(const int maxSize);
    Q_INVOKABLE void setRetries(const int retries);

    Q_INVOKABLE int getRetries() const;

    Q_INVOKABLE int start(QJSValue args, QJSValue onFinished = {}, QJSValue onProgress = {});
    Q_INVOKABLE QVariantMap startAndWait(QJSValue args, const int ioControllerId);

private:
    NetworkAccess *const m_net;
};
