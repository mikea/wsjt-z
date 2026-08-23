#include "Cloudlog.hpp"

#include <QByteArray>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

#include "Configuration.hpp"
#include "pimpl_impl.hpp"

#include "moc_Cloudlog.cpp"

namespace
{
  // Accepts either a deployment root or the QSO endpoint users commonly copy
  // from Cloudlog/Wavelog, then produces one canonical endpoint URL.
  QUrl api_url (QString url, QString const& endpoint)
  {
    url = url.trimmed ();
    while (url.endsWith ('/'))
      {
        url.chop (1);
      }

    QString const qso_path {"/index.php/api/qso"};
    if (url.endsWith (qso_path, Qt::CaseInsensitive))
      {
        url.chop (qso_path.size ());
      }

    return QUrl {url + "/index.php/api/" + endpoint};
  }

  // Some hosting providers reject anonymous HTTP clients. This identity uses
  // the application metadata populated by main(), including its exact version.
  QByteArray http_user_agent ()
  {
    auto name = QCoreApplication::applicationName ();
    if (name.isEmpty ())
      {
        name = QStringLiteral ("WSJT-Z");
      }
    auto version = QCoreApplication::applicationVersion ();
    return (version.isEmpty () ? name : name + '/' + version).toUtf8 ();
  }
}

class Cloudlog::impl final : public QObject
{
  Q_OBJECT

public:
  // Keeps the API client independent from the application's other network
  // services while retaining the public object's signal interface.
  impl (Cloudlog * self, Configuration const * config)
    : self_ {self}
    , config_ {config}
    , network_manager_ {this}
  {
  }

  // Serializes with Qt's JSON implementation so quotes and non-ASCII content
  // in ADIF fields cannot corrupt the request body.
  void log_qso (QByteArray const& adif)
  {
    QJsonObject payload {
      {"key", config_->cloudlog_api_key ()},
      {"station_profile_id", QString::number (config_->cloudlog_api_station_id ())},
      {"type", "adif"},
      {"string", QString::fromUtf8 (adif) + "<eor>"},
    };

    QNetworkRequest request {api_url (config_->cloudlog_api_url (), "qso")};
    request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader ("User-Agent", http_user_agent ());
    auto reply = network_manager_.post (request, QJsonDocument {payload}.toJson (QJsonDocument::Compact));
    connect (reply, &QNetworkReply::finished, this, [this, reply] {handle_upload_reply (reply);});
  }

  // Tests the current editor values rather than the last accepted settings.
  // The key is percent-encoded as a URL path segment by QUrl.
  void test_api (QString const& url, QString const& api_key)
  {
    auto endpoint = api_url (url, "auth/" + QString::fromUtf8 (QUrl::toPercentEncoding (api_key)));
    if (!endpoint.isValid () || endpoint.host ().isEmpty ())
      {
        Q_EMIT self_->api_key_invalid ();
        return;
      }

    QNetworkRequest request {endpoint};
    request.setRawHeader ("User-Agent", http_user_agent ());
    request.setOriginatingObject (this);
    auto reply = network_manager_.get (request);
    connect (reply, &QNetworkReply::finished, this, [this, reply] {handle_test_reply (reply);});
  }

private:
  // Classifies the small XML response used by both Cloudlog and Wavelog. Any
  // transport error or unrecognized response is deliberately treated as an
  // invalid test so the UI never suggests credentials are usable.
  void handle_test_reply (QNetworkReply * reply)
  {
    auto const result = reply->readAll ();
    auto const succeeded = QNetworkReply::NoError == reply->error ();
    reply->deleteLater ();

    if (succeeded && result.contains ("<status>Valid</status>"))
      {
        if (result.contains ("<rights>rw</rights>"))
          {
            Q_EMIT self_->api_key_writable ();
          }
        else
          {
            Q_EMIT self_->api_key_read_only ();
          }
      }
    else
      {
        Q_EMIT self_->api_key_invalid ();
      }
  }

  // Interprets both HTTP failures and the API's JSON failure object. Successful
  // uploads need no signal because logging remains fire-and-forget to the UI.
  void handle_upload_reply (QNetworkReply * reply)
  {
    auto const body = reply->readAll ();
    if (QNetworkReply::NoError != reply->error ())
      {
        auto const reason = reply->errorString ();
        reply->deleteLater ();
        Q_EMIT self_->upload_failed (reason);
        return;
      }
    reply->deleteLater ();

    auto const response = QJsonDocument::fromJson (body).object ();
    if (response.value ("status").toString () == "failed")
      {
        auto reason = response.value ("reason").toString ();
        if (reason.isEmpty ())
          {
            reason = tr ("The server rejected the QSO without an explanation.");
          }
        Q_EMIT self_->upload_failed (reason);
      }
  }

  Cloudlog * self_;
  Configuration const * config_;
  QNetworkAccessManager network_manager_;
};

#include "Cloudlog.moc"

// Constructs the private asynchronous client with the same lifetime as this
// signal-emitting facade.
Cloudlog::Cloudlog (Configuration const * config, QObject * parent)
  : QObject {parent}
  , m_ {this, config}
{
}

// Out-of-line destruction permits the private implementation to remain hidden
// from users of this header.
Cloudlog::~Cloudlog () = default;

// Delegates upload ownership and response handling to the private client.
void Cloudlog::log_qso (QByteArray const& adif)
{
  m_->log_qso (adif);
}

// Delegates the test while intentionally using the supplied, unaccepted values.
void Cloudlog::test_api (QString const& url, QString const& api_key)
{
  m_->test_api (url, api_key);
}
