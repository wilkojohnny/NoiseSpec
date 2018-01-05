/* TMicListener
 *
 * Class which listens to the audio input (line in or mic), and returns the value of the signal.
 *
 * Created 30/09/17
 *
 *
 */

#ifndef TMICLISTENER_H
#define TMICLISTENER_H

#include <QObject>
#include <QAudioInput>
#include <QBuffer>
#include <QTimer>
#include <QDataStream>
#include <QVector>
#include <thread>
#include <chrono>
#include <cmath>
#include <QDebug>

class TMicListener : public QObject
{
    Q_OBJECT
public:
    // Constructor
    explicit TMicListener(QObject *parent, QIODevice *p_IODevice, QAudioFormat *p_AudioFormat);

    // Destructor
    ~TMicListener();

    QAudioInput * m_AudioInput;

    inline void setRecordDuration(float p_RecordTime) { this->RecordDuration = p_RecordTime; }

signals:
    void finished();
public slots:
    // Start and stop
    void stopCapture();
    void Capture();

private:

    // QAudioInput stuff
    QAudioFormat * m_AudioFormat;

    // Output device
    QIODevice * m_AudioIODevice;

    // time to record for
    float RecordDuration;
};

#endif // TMICLISTENER_H
