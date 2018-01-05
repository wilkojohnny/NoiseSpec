#ifndef TSIGNALSETTINGS_H
#define TSIGNALSETTINGS_H

#include <QDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QAudioInput>
#include <QBuffer>
#include <QVector>
#include <QtWidgets/QWidget>
#include <QThread>
#include <tsignalsettings.h>
#include <tmiclistener.h>
#include <tdatapointarray.h>

namespace Ui {
class TSignalSettings;
}

// AudioFormatParameters for the TSignalSettings Dialog
struct AudioFormatParameters {
    int BitRate;
    float RecordTime;
    float SampleRate;
    QAudioDeviceInfo RecordDevice;
};


class TSignalSettings : public QDialog
{
    Q_OBJECT

public:
    // Constructor
    explicit TSignalSettings(AudioFormatParameters *LastRecordingParameters, QWidget *parent = 0); // Construct with previous values

    ~TSignalSettings();

    QBuffer * m_MicBuffer;

    TDataPointArray * OutputDPA;

    bool Listening;


private slots:

    void on_AudioDeviceSelector_currentIndexChanged(int index);

    void StopListening();

    void on_RecordButton_clicked();

    void UpdateListeningStatus();

signals:
    void FinishedProcessing();

private:
    Ui::TSignalSettings *ui;

    void UpdateParameterBoxes(int selectedDevice);

    QAudioFormat * AudioFormat;
    TMicListener * m_MicListener;
    QTimer * ListeningTimer;
    QTimer * ListeningStatusTimer;
    QThread* ListenerThread;

    // Time to record for
    float RecordTime;

    AudioFormatParameters * m_FormatParameters;

};

#endif // TSIGNALSETTINGS_H

