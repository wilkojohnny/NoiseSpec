#include "tsignalsettings.h"
#include "ui_tsignalsettings.h"

TSignalSettings::TSignalSettings(AudioFormatParameters * LastRecordingParameters, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TSignalSettings)
{
    ui->setupUi(this);

    // Set the format parameters
    m_FormatParameters = LastRecordingParameters;

    // Not listening initially
    Listening = false;

    // Create and open buffer for data
    m_MicBuffer = new QBuffer(this);
    m_MicBuffer->open(QIODevice::ReadWrite);

    // Populate the table of audio input devices, and select the previous device
    QStandardItemModel * AudioDeviceInfoModel = new QStandardItemModel(this);
    AudioDeviceInfoModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Device Name")));
    ui->AudioDeviceSelector->setModel(AudioDeviceInfoModel);
    int i = 0;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
       AudioDeviceInfoModel->setItem(i, new QStandardItem(deviceInfo.deviceName()));
       if (LastRecordingParameters->RecordDevice==deviceInfo) {
            ui->AudioDeviceSelector->setCurrentIndex(i);
            on_AudioDeviceSelector_currentIndexChanged(i);
       }
       i++;
    }

    // Select the previous bit rate
    int BitRateIndex = ui->AudioBitRateSelector->findData(QString::number(LastRecordingParameters->BitRate)); // find data in the combobox
    if (BitRateIndex!=-1) ui->AudioBitRateSelector->setCurrentIndex(BitRateIndex); // if found, select it (-1==not found)

    // Select the previous sample rate
    int SampleRateIndex = ui->AudioSampleRateBox->findData(QString::number(LastRecordingParameters->SampleRate));
    if (SampleRateIndex!=-1) ui->AudioSampleRateBox->setCurrentIndex(SampleRateIndex);

    // Set the previously specified time
    ui->RecordTimeBox->setValue(LastRecordingParameters->RecordTime);

}

TSignalSettings::~TSignalSettings()
{
    delete ui;
}


void TSignalSettings::on_AudioDeviceSelector_currentIndexChanged(int index)
{
    UpdateParameterBoxes(index);
}

void TSignalSettings::UpdateParameterBoxes(int selectedDevice) {
    QList<QAudioDeviceInfo> Devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    // Clear the bitrates and sample rate boxes
    for (int i = ui->AudioBitRateSelector->count(); i>0; --i) {
        ui->AudioBitRateSelector->removeItem(i);
    }
    for (int i = ui->AudioSampleRateBox->count(); i>0; --i) {
        ui->AudioSampleRateBox->removeItem(i);
    }

    // Get the selected audio device
    for (int i=0; i<Devices.size(); i++) {
        if (Devices[i].deviceName()==ui->AudioDeviceSelector->itemText(selectedDevice)) {
            // This device is selected... so list out what the device can cope with

            // Sample rate
            QList<int> SampleRates = Devices[i].supportedSampleRates();
            Sort(SampleRates);
            for (int j=0; j<SampleRates.size(); j++) {
                ui->AudioSampleRateBox->addItem(QString::number(SampleRates[j]), SampleRates[j]);
            }

            // Bit rate
            QList<int> BitRates = Devices[i].supportedSampleSizes();
            Sort(BitRates);
            for (int j=0; j<BitRates.size(); j++) {
                // Only allow power of 2 bitrates (anything else is harder to code)
                if (log(BitRates[j])/log(2)==floor(log(BitRates[j])/log(2))) {
                  ui->AudioBitRateSelector->addItem(QString::number(BitRates[j]), BitRates[j]);
                }
            }

        }
    }
}

void TSignalSettings::on_RecordButton_clicked()
{
    m_MicBuffer = new QBuffer(this);
    m_MicBuffer->open(QIODevice::ReadWrite);

    AudioFormat = new QAudioFormat();

    AudioFormat->setByteOrder(QAudioFormat::LittleEndian); // Same as example
    AudioFormat->setChannelCount(1);  // Same as example
    AudioFormat->setCodec("audio/pcm"); // pcm is the codec which samples the amplitude
    AudioFormat->setSampleRate(ui->AudioSampleRateBox->currentText().toInt());
    AudioFormat->setSampleSize(ui->AudioBitRateSelector->currentText().toInt());  // Same as example
    AudioFormat->setSampleType(QAudioFormat::SignedInt);  // Should be alright

    // Check this format is supported
    QAudioDeviceInfo info = QAudioDeviceInfo::availableDevices(QAudio::AudioInput)[ui->AudioDeviceSelector->currentIndex()];
    if (!info.isFormatSupported(*AudioFormat)) {
        // If not supported, create dummy audio format so that we can delete the one just made
        QAudioFormat * OldAudioFormat;
        OldAudioFormat = AudioFormat;
        AudioFormat = new QAudioFormat(info.nearestFormat(*OldAudioFormat));
        delete OldAudioFormat;

        // Notify the user the format has changed
        QMessageBox box;
        box.setText("Bad audio format, using nearest...");
        box.show();
    }

    Listening = true;
    ui->RecordTimeBox->setEnabled(false);
    RecordTime = ui->RecordTimeBox->text().toFloat();

    // Construct MicListener
    m_MicListener = new TMicListener(this, m_MicBuffer, AudioFormat);
    m_MicListener->setRecordDuration(RecordTime);

    connect(m_MicListener, SIGNAL(finished()), this, SLOT(StopListening()));

    // tell miclistener to start listening
    m_MicListener->Capture();

    // Start GUI timers
    ListeningStatusTimer = new QTimer(this);
    connect(ListeningStatusTimer, SIGNAL(timeout()), this, SLOT(UpdateListeningStatus()));
    ListeningStatusTimer->setSingleShot(false);
    ListeningStatusTimer->start(10);

    ui->AudioBitRateSelector->setEnabled(false);
    ui->AudioDeviceSelector->setEnabled(false);
    ui->AudioSampleRateBox->setEnabled(false);
    ui->NameBox->setEnabled(false);
    ui->RecordTimeBox->setEnabled(false);

    ui->ListeningStatusLabel->setText("Listening...");
}

void TSignalSettings::UpdateListeningStatus() {
    float ElapsedTime = 1.e-6*m_MicListener->m_AudioInput->elapsedUSecs();
    int statusbar = round(ElapsedTime*100./RecordTime);
    ui->ListeningStatusBar->setValue(statusbar);
}

void TSignalSettings::StopListening() {

    // Turn listening flag off
    Listening = false;

    // stop the progress bar timer
    ListeningStatusTimer->stop();

    // Add results to a TDataPointArray
    this->OutputDPA = new TDataPointArray(m_MicBuffer, AudioFormat, ui->NameBox->text(), RecordTime);

    // Set the format parameters
    m_FormatParameters->BitRate = AudioFormat->sampleSize();
    m_FormatParameters->SampleRate = AudioFormat->sampleRate();
    m_FormatParameters->RecordDevice = QAudioDeviceInfo::availableDevices(QAudio::AudioInput)[ui->AudioDeviceSelector->currentIndex()];
    m_FormatParameters->RecordTime = ui->RecordTimeBox->value();

    // accept dialog
    this->accept();
}
