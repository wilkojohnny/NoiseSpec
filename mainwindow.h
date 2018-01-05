#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioInput>
#include <QBuffer>
#include <QFile>
#include <QtCharts>
#include <QVector>
#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtWidgets/QVBoxLayout>
#include <QtCharts/QValueAxis>

#include <tsignalsettings.h>
#include <tmiclistener.h>
#include <tdatapointarray.h>
#include <noisespecmaths.h>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QVector<TDataPointArray*> DataPointArrayVector;

private slots:

    void on_RawDataRadio_toggled(bool checked);

    void on_AddButton_clicked();

    void on_deleteButton_clicked();

    void on_DPATable_doubleClicked(const QModelIndex &index);

    void on_xMinBox_returnPressed();

    void on_xMaxBox_returnPressed();

    void on_yMinBox_returnPressed();

    void on_yMaxBox_returnPressed();

    void on_actionExport_triggered();

    // generate sample data
    void generateSampleData();

private:
    void DoPlot(bool doFFT = false, bool AutoXaxis = true); // Plot the graph on the GUI
    void RePaintUI();

    void AddDPATableItem(int VectorIndex);

    Ui::MainWindow *ui;

    QVector<QColor*> Colors;

//    QFile * OutputFile; // Uncomment for write to file

    // Chart stuff
    QChart *m_chart;
    QChartView *chartView;
    QLineSeries *m_series;
    QVector<QLineSeries*> ChartSeries;

    // Model to populate the table
    QStandardItemModel * DPATableModel;

    // Settings for recording
    AudioFormatParameters * LastAudioFormat;
};

#endif // MAINWINDOW_H
