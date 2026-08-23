#ifndef CLOUDLOG_HPP_
#define CLOUDLOG_HPP_

#include <QObject>

#include "pimpl_h.hpp"

class QByteArray;
class Configuration;
class QString;

// Uploads logged ADIF records to the compatible Cloudlog and Wavelog APIs.
// Each instance owns its network manager so unrelated services cannot disrupt
// an in-flight upload by reconfiguring or destroying a shared manager.
class Cloudlog final : public QObject
{
  Q_OBJECT

public:
  // Uses the supplied configuration as the source of the accepted API URL,
  // key, and station profile. The configuration must outlive this object.
  explicit Cloudlog (Configuration const * config, QObject * parent = nullptr);
  ~Cloudlog ();

  // Sends one ADIF record. The API requires the end-of-record marker, which
  // this method appends so callers continue to pass the application's normal
  // marker-free ADIF representation.
  void log_qso (QByteArray const& adif);

  // Tests credentials entered in the settings dialog without first accepting
  // them. Results are reported asynchronously through the API-key signals.
  Q_SLOT void test_api (QString const& url, QString const& api_key);

  // Reports that the tested API key permits QSO uploads.
  Q_SIGNAL void api_key_writable () const;

  // Reports that the tested API key is valid but read-only.
  Q_SIGNAL void api_key_read_only () const;

  // Reports invalid credentials, an invalid URL, or a network-level failure.
  Q_SIGNAL void api_key_invalid () const;

  // Reports a rejected upload or transport failure with a user-facing reason.
  Q_SIGNAL void upload_failed (QString const& reason) const;

private:
  class impl;
  pimpl<impl> m_;
};

#endif
