#include "tmiclistener.h"

TMicListener::TMicListener(QObject *parent, QIODevice *p_IODevice, QAudioFormat *p_AudioFormat) : QObject(parent),
    m_AudioFormat(p_AudioFormat), m_AudioIODevice(p_IODevice)
{
    // Set up audio input handler (QAudioInput)
    this->m_AudioInput = new QAudioInput(*this->m_AudioFormat, nullptr);
    this->RecordDuration = 1.;
}

TMicListener::~TMicListener() {
    delete(this->m_AudioInput);
}

void TMicListener::Capture() {
    this->m_AudioInput->start(m_AudioIODevice);

    // wait for audio to become active
    while (!(this->m_AudioInput->state()==QAudio::ActiveState)) {}

    // time for RecordDuration seconds in advance
    QTimer::singleShot(floor(this->RecordDuration*1000.), this, SLOT(stopCapture()));
}

void TMicListener::stopCapture() {

    this->m_AudioInput->stop();

    emit finished();
}

