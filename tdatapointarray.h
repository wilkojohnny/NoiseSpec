/* TDataPointArray: Class which holds an array of data harvested from the input
 *
 *
 */

#ifndef TDATAPOINTARRAY_H
#define TDATAPOINTARRAY_H

#include <QObject>
#include <QVector>
#include <QIODevice>
#include <QDataStream>
#include <QAudioFormat>
#include <cmath>
#include <QLineSeries>
#include <noisespecmaths.h>

class TDataPointArray : public QObject
{
    Q_OBJECT
public:
    explicit TDataPointArray(QObject *parent = 0);
    explicit TDataPointArray(QVector<float> *data, QObject *parent = 0);
    explicit TDataPointArray(QIODevice *IODevice, QAudioFormat *m_AudioFormat, QObject *parent = 0);
    explicit TDataPointArray(QIODevice *IODevice, QAudioFormat *m_AudioFormat, QString UIName, QObject *parent = 0);
    explicit TDataPointArray(QIODevice *IODevice, QAudioFormat *m_AudioFormat, QString UIName, float DesiredTime, QObject *parent = 0);

    QtCharts::QLineSeries* GetScatterData(bool FFT = false);
    QVector<float> GetData(bool FFT = false);

    float getInterpolatedData(float time); // get data in

    void SetData(QVector<float> *data); // Set data with a vector
    void SetData(QIODevice *IODevice, QAudioFormat *m_AudioFormat4); // Set data with an IODevice (microphone)

  //  void four1(int isign); // FFT algorithm - moved to NSmaths so can be re-used elsewhere

    // GUI parameters
    inline void SetUiName(QString Name) { UI_Name = Name; }
    inline QString GetUiName() { return UI_Name; }
    void generateColor();

    float getTotalTime() { return 1.*(this->DataPointVector.size()-1)/(1.*SampleRate); } // size-1 due to DPV[0]<=>(t=0)

    QColor plotColor;

    int SampleRate = 1;
    int BitRate = 0;
signals:

public slots:

private:
    QVector<float> DataPointVector; // The actual data
    QVector<float> FFTData;
    void DoFFT(); // Do FFT

    QString UI_Name; // name to put on the UI
};

#endif // TDATAPOINTARRAY_H
