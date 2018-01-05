#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set up plot area
    m_chart = new QChart();
    QChartView *chartView = new QChartView(m_chart);
    chartView->setMinimumSize(800, 600);
    m_chart->setDropShadowEnabled(false);
    ui->ChartLayout->addWidget(chartView);

    // Set up table of datasets
    DPATableModel = new QStandardItemModel(0, 5, this);
    DPATableModel->setHorizontalHeaderItem(0, new QStandardItem());
    DPATableModel->setHorizontalHeaderItem(1, new QStandardItem("Name"));
    DPATableModel->setHorizontalHeaderItem(2, new QStandardItem("Duration /s"));
    DPATableModel->setHorizontalHeaderItem(3, new QStandardItem("Sample Rate /Hz"));
    DPATableModel->setHorizontalHeaderItem(4, new QStandardItem("Bitrate"));

    ui->DPATable->setModel(DPATableModel);

    ui->DPATable->horizontalHeader()->resizeSection(0, 30);

    ui->xMinBox->setValidator(new QDoubleValidator(this));
    ui->xMaxBox->setValidator(new QDoubleValidator(this));
    ui->yMinBox->setValidator(new QDoubleValidator(this));
    ui->yMaxBox->setValidator(new QDoubleValidator(this));

    QShortcut * sampledata = new QShortcut(QKeySequence(tr("F7")), this);
    connect(sampledata,SIGNAL(activated()),this, SLOT(generateSampleData()));

    // Set up audio format with default values
    LastAudioFormat = new AudioFormatParameters;
    LastAudioFormat->RecordDevice = QAudioDeviceInfo::defaultInputDevice();
    LastAudioFormat->RecordTime = 1.;
    LastAudioFormat->BitRate = 32;
    LastAudioFormat->SampleRate = 96000;

}

MainWindow::~MainWindow()
{
    delete ui;
    delete LastAudioFormat;
}

void MainWindow::generateSampleData()
{
    float sampleRate = 10000.;
    float maxTime = 2.;
    float frequency = 10000*(rand()/(1.*RAND_MAX));

    QVector<float> * data;

    data = new QVector<float>;

    for (float time=0; time<=maxTime; time+=(1./sampleRate) ) {
        data->append(sin(2.*3.1415926*frequency*time));
    }

    this->DataPointArrayVector.append(new TDataPointArray(data));
    this->DataPointArrayVector[this->DataPointArrayVector.size()-1]->SampleRate = sampleRate;
    this->DataPointArrayVector[this->DataPointArrayVector.size()-1]->SetUiName(QString::number(frequency).append(QString("Hz Sample")));

    this->AddDPATableItem(this->DataPointArrayVector.size()-1);
    RePaintUI();
}

void MainWindow::DoPlot(bool doFFT, bool AutoAxis)
{
    bool doLogX = ui->LogXCheckBox->isChecked();
    bool doLogY = ui->LogYCheckBox->isChecked();

    // Clean up old series and chart objects
    ChartSeries.clear();
    m_chart->removeAllSeries();
    delete m_chart->axisX();
    delete m_chart->axisY();

    QLogValueAxis * LogAxisX;
    QValueAxis * axisX;
    QLogValueAxis * LogAxisY;
    QValueAxis * axisY;

    // set up axis
    if (doLogX) {
        LogAxisX = new QLogValueAxis();
        LogAxisX->setLabelFormat("%g");
    } else {
        axisX = new QValueAxis();
        axisX->setLabelFormat("%g");
    }
    if (doLogY) {
        LogAxisY = new QLogValueAxis();
        LogAxisY->setLabelFormat("%g");
    } else {
        axisY = new QValueAxis();
        axisY->setLabelFormat("%g");
    }


    for (int i = 0; i<DataPointArrayVector.size(); i++) {
        ChartSeries.push_back(DataPointArrayVector[i]->GetScatterData(doFFT));
        m_chart->addSeries(ChartSeries[i]);
    }

    doFFT ? axisX->setTitleText("Frequency /Hz") : axisX->setTitleText("Time /s");

    m_chart->legend()->setVisible(false);

    // Do axis
    m_chart->createDefaultAxes();
    // For each series in chart, attach both axis to it
    for (int i=0; i<ChartSeries.size(); i++) {
        // if axis set to automatic, let the computer decide
        if (AutoAxis) {
            axisX->setRange(0, 0);
            axisY->setRange(0, 0);
        } else {
            axisX->setRange(ui->xMinBox->text().toDouble(), ui->xMaxBox->text().toDouble());
            axisY->setRange(ui->yMinBox->text().toDouble(), ui->yMaxBox->text().toDouble());
        }
        m_chart->setAxisX(axisX, ChartSeries[i]);
        m_chart->setAxisY(axisY, ChartSeries[i]);
    }

    // start at 0 if auto axis set
    if (AutoAxis) axisX->setMin(0);

    // Fill out the axis range boxes
    if (AutoAxis) {
        ui->xMaxBox->setText(QString::number(axisX->max()));
        ui->xMinBox->setText(QString::number(axisX->min()));
        ui->yMinBox->setText(QString::number(axisY->min()));
        ui->yMaxBox->setText(QString::number(axisY->max()));
    }
}

void MainWindow::on_RawDataRadio_toggled(bool checked)
{
    this->DoPlot(!checked);
}

void MainWindow::on_AddButton_clicked()
{
    TSignalSettings * SignalAddBox = new TSignalSettings(LastAudioFormat, this);
    SignalAddBox->exec();

    if (SignalAddBox->result() == QDialog::Accepted) {
        DataPointArrayVector.append(SignalAddBox->OutputDPA);
        DoPlot();
        this->AddDPATableItem(DataPointArrayVector.size()-1);
    }
    delete SignalAddBox;
}

void MainWindow::AddDPATableItem(int VectorIndex) {
    QList<QStandardItem*> DPATableRow;

    // Get information to put into row
    QString Name = DataPointArrayVector[VectorIndex]->GetUiName();
    QString Duration = QString::number(DataPointArrayVector[VectorIndex]->GetData().size() * 1.0 / (1.*DataPointArrayVector[VectorIndex]->SampleRate));
    QString SampleRate = QString::number(DataPointArrayVector[VectorIndex]->SampleRate);
    QString BitRate = QString::number(DataPointArrayVector[VectorIndex]->BitRate);

    QPixmap pixmap(100,100);
    pixmap.fill(DataPointArrayVector[VectorIndex]->plotColor);
    QIcon Icon(pixmap);

    DPATableRow.append(new QStandardItem(Icon, QString("")));
    DPATableRow.append(new QStandardItem(Name));
    DPATableRow.append(new QStandardItem(Duration));
    DPATableRow.append(new QStandardItem(SampleRate));
    DPATableRow.append(new QStandardItem(BitRate));
    DPATableModel->appendRow(DPATableRow);

    ui->DPATable->selectRow(VectorIndex);
    ui->deleteButton->setEnabled(true);

}

void MainWindow::RePaintUI() {
    QModelIndexList SelectedRow = ui->DPATable->selectionModel()->selectedIndexes();
    // Remove all rows from table of plots
    this->DPATableModel->removeRows(0,this->DataPointArrayVector.size());
    // Repopulate the table
    for (int i=0; i<this->DataPointArrayVector.size(); i++) {
        this->AddDPATableItem(i);
    }
    ui->DPATable->selectionModel()->select(SelectedRow[0],QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    // Replot the data
    this->DoPlot(ui->FFTRadio->isChecked());
}


void MainWindow::on_deleteButton_clicked()
{
    QModelIndexList indexList = ui->DPATable->selectionModel()->selectedIndexes();
    int row=-1;
    foreach (QModelIndex index, indexList) {
        // Prevent deleting the same row twice
        if (row!=index.row()) {
            row = index.row();
            DPATableModel->removeRow(row);
            delete(DataPointArrayVector[row]);
            DataPointArrayVector.remove(row);
            row = index.row();
        }
    }

    // if no more datasets left, disable delete button
    if (DPATableModel->rowCount()==0) {
        ui->deleteButton->setEnabled(false);
    }

    // Update the plot
    DoPlot(ui->FFTRadio->isChecked(), true);
}

void MainWindow::on_DPATable_doubleClicked(const QModelIndex &index)
{
    // double click changes colour
    int row = index.row();
    DataPointArrayVector[row]->generateColor();
    this->RePaintUI();
}

void MainWindow::on_xMinBox_returnPressed()
{
    m_chart->axisX()->setMin(ui->xMinBox->text().toDouble());
}

void MainWindow::on_xMaxBox_returnPressed()
{
    m_chart->axisX()->setMax(ui->xMaxBox->text().toDouble());
}

void MainWindow::on_yMinBox_returnPressed()
{
    m_chart->axisY()->setMin(ui->yMinBox->text().toDouble());
}

void MainWindow::on_yMaxBox_returnPressed()
{
    m_chart->axisY()->setMax(ui->yMaxBox->text().toDouble());
}

void MainWindow::on_actionExport_triggered()
{
    // Make a save file dialog to get the location
    QString ExportFileLocation = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().append("/exportFile.csv");
    ExportFileLocation = QFileDialog::getSaveFileName(this, tr("Export file"), ExportFileLocation, tr("CSV file (*.csv)"));

    QFile ExportFile;
    ExportFile.setFileName(ExportFileLocation);

    QTextStream FileWriteStream(&ExportFile);

    // Open file
    if (ExportFile.open( QIODevice::WriteOnly)) {
        // Write to file routine
        QString CurrentFileLine;

        // write the col headings:
        CurrentFileLine = QString("time,");

        /* Due to potentially differing sample rates, the times of the samples may be out of sync.
         * Hence, we need to:
         * - Find the largest sample rate of all the tracks, use this to base the times off of
         * - Get the total time off each of the datasets
         * - Go up in steps of 1/SR_max, incrementing i:
         *      fe TDPA
         *        is i/SR_max < TDPA.Tmax?
         *            y: if SR_max = TDPA.SR, just get the result and print onto line
         *                  otherwise linearly interpolate
         *            n: don't worry
         */

        // Find the largest sample rate and longest time
        float SR_max = 0.;
        float time_max = 0.;
        for (int i = 0; i<this->DataPointArrayVector.size(); i++) {
            if (this->DataPointArrayVector[i]->SampleRate>SR_max) {
                SR_max = this->DataPointArrayVector[i]->SampleRate;
            }
            if (this->DataPointArrayVector[i]->getTotalTime()>time_max) {
                time_max = this->DataPointArrayVector[i]->getTotalTime();
            }

            // while we're looping over the DPAs, might as well add in the file heading here:
            CurrentFileLine.append(this->DataPointArrayVector[i]->GetUiName().append(","));
        }

        // use the largest sample rate as the time increment
        float timeIncrement = 1./(1.*SR_max);

        // go up the time
        for (float time = 0; time<time_max; time+=timeIncrement) {
            // write the time onto the line
            CurrentFileLine = QString::number(time).append(",");
            // go through each DPA
            for (int i=0; i<this->DataPointArrayVector.size(); i++) {
                // print out data at this time (everything else is automatic)
                CurrentFileLine.append(QString::number(this->DataPointArrayVector[i]->getInterpolatedData(time)).append(","));
            }

            // write to file
            FileWriteStream << CurrentFileLine << "\n";
        }

        // close file
        ExportFile.close();


    } else {
        // file write failed
        // Notify the user
        QMessageBox box;
        box.setText("File write failed.");
        box.show();
    }
}
