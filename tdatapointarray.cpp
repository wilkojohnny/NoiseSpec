#include "tdatapointarray.h"
#include <fstream>

TDataPointArray::TDataPointArray(QObject *parent) : QObject(parent)
{
    this->generateColor();
}

TDataPointArray::TDataPointArray(QVector<float> *data, QObject *parent) : QObject(parent)
{
    this->SetData(data);
    this->generateColor();
    this->DoFFT();
}

TDataPointArray::TDataPointArray(QIODevice *IODevice, QAudioFormat *m_AudioFormat, QObject *parent) : QObject(parent)
{
    this->SetData(IODevice, m_AudioFormat);
    this->generateColor();
    this->DoFFT();
}

TDataPointArray::TDataPointArray(QIODevice *IODevice, QAudioFormat *m_AudioFormat, QString p_UIName, QObject *parent):
    QObject(parent), UI_Name(p_UIName)
{
    this->SetData(IODevice, m_AudioFormat);
    this->generateColor();
    this->DoFFT();
}


TDataPointArray::TDataPointArray(QIODevice *IODevice, QAudioFormat *m_AudioFormat, QString p_UIName, float DesiredTime, QObject *parent):
    QObject(parent), UI_Name(p_UIName)
{
    this->SetData(IODevice, m_AudioFormat);

    // shave off the extra time
    int noDesiredElements = round(m_AudioFormat->sampleRate()*DesiredTime);
    this->DataPointVector.resize(noDesiredElements);

    this->generateColor();
    this->DoFFT();
}

QVector<float> TDataPointArray::GetData(bool FFT) {
    return FFT ? this->FFTData : this->DataPointVector;
}

float TDataPointArray::getInterpolatedData(float time)
{
    // is the time requested in the time range for this signal?
    if (time<=this->getTotalTime()) {
        // Interpolate between two points to get the signal at the time requested.
        int beginningIndex, endIndex;
        float index;

        // index = the exact index
        index = time*this->SampleRate;
        beginningIndex = floor(index);
        endIndex = ceil(index);

        // is the time exact?
        if (index==1.*beginningIndex) {
            // no interpolation needed
            return this->DataPointVector[index];
        } else {
            // need to interpolate
            return this->DataPointVector[beginningIndex]*(1.+index-beginningIndex*1.) + this->DataPointVector[endIndex]*(index-beginningIndex*1.);
        }
    } else {
        // time not in signal range
        return 0.;
    }
}

void TDataPointArray::SetData(QVector<float> *data) {
    this->DataPointVector = *data;
    this->DoFFT();
}

void TDataPointArray::SetData(QIODevice *IODevice, QAudioFormat * m_AudioFormat) {
    // Delete all previous array elements
    this->DataPointVector.clear();
    this->FFTData.clear();

    // Define a stream to get the data from the IODevice
    QDataStream InStream(IODevice);
    // Go to the beginning of the data
    IODevice->seek(0);
    // Same byte order as audio input
    InStream.setByteOrder(QDataStream::LittleEndian);

    this->BitRate = m_AudioFormat->sampleSize();

    // For each xx bits as defined by the format, convert to float and add to the end of the array
    switch (m_AudioFormat->sampleSize()) {
    case 8:
        qint8 AudioSegment8;
        while (!InStream.atEnd()) {
            InStream >> AudioSegment8;
            this->DataPointVector.append(AudioSegment8*1.0);
        }
        break;
    case 16:
        qint16 AudioSegment16;
        while (!InStream.atEnd()) {
            InStream >> AudioSegment16;
            this->DataPointVector.append(AudioSegment16*1.0);
        }
        break;
    case 32:
        qint32 AudioSegment32;
        while (!InStream.atEnd()) {
            InStream >> AudioSegment32;
            this->DataPointVector.append(AudioSegment32*1.0);
        }
        break;
    case 64:
        qint64 AudioSegment64;
        while (!InStream.atEnd()) {
            InStream >> AudioSegment64;
            this->DataPointVector.append(AudioSegment64*1.0);
        }
        break;
    }

    this->SampleRate = m_AudioFormat->sampleRate();
}

QtCharts::QLineSeries* TDataPointArray::GetScatterData(bool FFT) {
    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();

    if (FFT) {
        float FreqIncrement = (this->SampleRate*1.0)/(2.0*this->FFTData.size());
        for (int i=0; i<this->FFTData.size(); i++) {
            series->append(FreqIncrement*(i), this->FFTData[i]);
        }
    } else {
        // not FFT, plot raw data
        for (int i=0; i<this->DataPointVector.size(); i++) {
            series->append((i+1)/(1.0*this->SampleRate), this->DataPointVector[i]);
        }
    }
    series->setColor(this->plotColor);
    return series;
}

void TDataPointArray::generateColor() {
    // Set color
    int red = 0;
    int green = 0;
    int blue = 0;
    // Enforce Prettiness rules - dont allow dark or white-like colours
    while (((red>200) && (green>200) && (blue>200)) || ((red<150) && (green<150) && (blue<150))) {
        red = round(rand()%255);
        green = round(rand()%255);
        blue = round(rand()%255);
    }

    this->plotColor = QColor(red, green, blue);
}

void TDataPointArray::DoFFT() {
    /*
     Because four1(..) wants the input data to be [re im re im re im.....]
     this converts the DataPointVector into that form..
     */
    this->FFTData.resize(round2(this->DataPointVector.size())*2); // resize the length of data array
    for (int i=0; i<this->DataPointVector.size(); i++) {
        this->FFTData[2*i] = this->DataPointVector[i];
        this->FFTData[2*i+1] = 0;
    }

    four1(&(this->FFTData[0]), (this->FFTData.size())/2,-1);
    //this->four1(-1);

    // Our code
    for (int i=0; i<(this->FFTData.size()/2); i++)
    {
        this->FFTData[i]=sqrt(this->FFTData[2*i]*this->FFTData[2*i]+this->FFTData[2*i+1]*this->FFTData[2*i+1]); //find the amplitude for each frequency bin from RE and IM parts
//        outfile << FFTData[i];
//        outfile << "\n";
    }

    this->FFTData.resize(this->FFTData.size()/4);
}
