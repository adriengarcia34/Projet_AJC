/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "themewidget.h"

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QAbstractBarSeries>
#include <QtCharts/QPercentBarSeries>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLegend>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtCore/QTime>
#include <QtCharts/QBarCategoryAxis>
#include <QMessageBox>
#include <QSqlQuery>
#include <QFileDialog>
#include <QDebug>
#include <QDateTimeAxis>
#include <QValueAxis>

ThemeWidget::ThemeWidget(QWidget *parent) :
    QWidget(parent),

    m_themeComboBox(createThemeBox()),
    m_antialiasCheckBox(new QCheckBox("Anti-aliasing")),
    m_animatedComboBox(createAnimationBox()),
    m_legendComboBox(createLegendBox()),
    m_dateComboBox(createDateBox()),
    m_envComboBox(createEnvBox())
{
    connectSignals();
    // create layout
    /*QGridLayout *baseLayout = new QGridLayout();
    QHBoxLayout *settingsLayout = new QHBoxLayout();
    settingsLayout->addWidget(new QLabel("Env.:"));
    settingsLayout->addWidget(m_envComboBox);
    settingsLayout->addWidget(new QLabel("Time:"));
    settingsLayout->addWidget(m_dateComboBox);
    settingsLayout->addWidget(m_antialiasCheckBox);
    settingsLayout->addStretch();
    baseLayout->addLayout(settingsLayout, 0, 0, 1, 3);
*/
    bool connected = false;

    QSplineSeries *series1 = new QSplineSeries();
    QSplineSeries *series2 = new QSplineSeries();
    do
    {
         QString SQLPath = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", QString(), "SQLite3 (*.sqlite3)");
         QMessageBox::information(this, "Fichier", "Vous avez sélectionné :\n" + SQLPath);

        malouDB = QSqlDatabase::addDatabase("QSQLITE");
        malouDB.setDatabaseName(SQLPath);


        if(!malouDB.open())
            {

                QMessageBox errorBox;
                errorBox.critical(0, "Error", "Connexion à la base de donnée à distance échouée");
            }
        else
        {
            QMessageBox errorBox;
            errorBox.information(0, "GG", "Connexion OK\n"+malouDB.databaseName());
            connected = true;

            m_listCount = 1;
            m_valueMax = 40;
            QSqlQuery lineCount = malouDB.exec("SELECT COUNT(*) FROM Mesures;");
            lineCount.next();
            m_valueCount = lineCount.value(0).toInt();

            QDateTime moment;


            QSqlQuery result= malouDB.exec("SELECT * FROM mesures");
            while(result.next())
            {
                //Point Tempetarure / Date
                /*
                 * Exemple Guillaume P
                 * Gestion de la date
                 * moment=QDateTime(QDate(values[2].toInt(),values[3].toInt(),values[4].toInt()),QTime(values[5].toInt(),values[6].toInt(),0));
                 * series1->append(moment.toMSecsSinceEpoch(), values[0].toDouble());
                 */

               moment=QDateTime(result.value(3).toDateTime());
               series1->append(moment.toMSecsSinceEpoch(), result.value(1).toDouble());
               series2->append(moment.toMSecsSinceEpoch(), result.value(2).toDouble());
               seriesTemp.append(moment.toMSecsSinceEpoch(), result.value(1).toDouble());
               seriesPress.append(moment.toMSecsSinceEpoch(), result.value(2).toDouble());
               m_listTemp.push_back(result.value(1).toDouble());
               m_listPress.push_back(result.value(2).toDouble());
               dataMoment.push_back(moment);
            }


        }
    }while(!connected);


    QGridLayout *baseLayout = new QGridLayout();
    QHBoxLayout *settingsLayout = new QHBoxLayout();
    settingsLayout->addWidget(new QLabel(" * Debut/Fin : "));
    settingsLayout->addWidget(m_themeComboBox);
    settingsLayout->addWidget(new QLabel(" * Fin/Debut : "));
    settingsLayout->addWidget(m_animatedComboBox);
    /*settingsLayout->addWidget(picture);
    settingsLayout->addWidget(reload);*/
    //create charts

    /*QChartView *chartView;

    chartView = new QChartView(createSplineChart(true));
    baseLayout->addWidget(chartView, 1, 1);
    m_charts << chartView;

    chartView = new QChartView(createSplineChart(false));
    baseLayout->addWidget(chartView, 2, 1);
    m_charts << chartView;

    setLayout(baseLayout);*/

    settingsLayout->addStretch();
    baseLayout->addLayout(settingsLayout, 0, 0, 1, 3);
    QChartView *chartView;
    chartView = new QChartView(createSplineChart("Temperature (°C)",series1)); //series1
    baseLayout->addWidget(chartView, 1, 0);
    m_charts << chartView;
    chartView->setRenderHint(QPainter::Antialiasing, true);
    chartView->chart()->setTheme(static_cast<QChart::ChartTheme>(1));

    chartView = new QChartView(createSplineChart("Pression (hPa)", series2)); //series2
    baseLayout->addWidget(chartView, 1, 1);
    m_charts << chartView;
    chartView->setRenderHint(QPainter::Antialiasing, true);
    chartView->chart()->setTheme(static_cast<QChart::ChartTheme>(1));


    //chargecombobox
    int longueur=dataMoment.size();
    int i=0;
    //m_themeComboBox->clear();
    //m_animatedComboBox->clear();
    for(i=0;i<longueur;i++){
         m_themeComboBox->addItem(dataMoment.at(i).toString("yyyy-MM-dd/hh:mm"));
         m_animatedComboBox->addItem(dataMoment.at(i).toString("yyyy-MM-dd/hh:mm"));
    }
    m_animatedComboBox->setCurrentIndex(dataMoment.size()-1);//fixe la valeur de la deuxième combobox à la fin de la liste

    // Set defaults
    /*m_antialiasCheckBox->setChecked(true);
    updateUI();*/
    setLayout(baseLayout);
    updateUI();
}

ThemeWidget::~ThemeWidget()
{
}

void ThemeWidget::connectSignals()
{
    connect(m_themeComboBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &ThemeWidget::updateUI);
    connect(m_antialiasCheckBox, &QCheckBox::toggled, this, &ThemeWidget::updateUI);
    connect(m_animatedComboBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &ThemeWidget::updateUI);
    connect(m_legendComboBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &ThemeWidget::updateUI);
}

/*DataTable ThemeWidget::generateRandomData(int listCount, int valueMax, int valueCount) const
{
    DataTable dataTable;

    // set seed for random stuff
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    // generate random data
    for (int i(0); i < listCount; i++) {
        DataList dataList;
        qreal yValue(0);
        for (int j(0); j < valueCount; j++) {
            yValue = yValue + (qreal)(qrand() % valueMax) / (qreal) valueCount;
            QPointF value((j + (qreal) rand() / (qreal) RAND_MAX) * ((qreal) m_valueMax / (qreal) valueCount),
                          yValue);
            QString label = "Slice " + QString::number(i) + ":" + QString::number(j);
            dataList << Data(value, label);
        }
        dataTable << dataList;
    }

    return dataTable;
}*/

/*DataTable ThemeWidget::getSQLData(bool isTemp) const
{
    DataTable dataTable;
    DataList dataList;


    return dataTable;
}
*/

QComboBox *ThemeWidget::createThemeBox() const
{
    // settings layout
    QComboBox *themeComboBox = new QComboBox();
    themeComboBox->addItem("Dark", QChart::ChartThemeDark);
    themeComboBox->addItem("Light", QChart::ChartThemeLight);
    themeComboBox->addItem("Blue Cerulean", QChart::ChartThemeBlueCerulean);
    themeComboBox->addItem("Brown Sand", QChart::ChartThemeBrownSand);
    themeComboBox->addItem("Blue NCS", QChart::ChartThemeBlueNcs);
    themeComboBox->addItem("High Contrast", QChart::ChartThemeHighContrast);
    themeComboBox->addItem("Blue Icy", QChart::ChartThemeBlueIcy);
    themeComboBox->addItem("Qt", QChart::ChartThemeQt);
    return themeComboBox;
}

QComboBox *ThemeWidget::createAnimationBox() const
{
    // settings layout
    QComboBox *animationComboBox = new QComboBox();
    animationComboBox->addItem("GridAxis Animations", QChart::GridAxisAnimations);
    animationComboBox->addItem("Series Animations", QChart::SeriesAnimations);
    animationComboBox->addItem("All Animations", QChart::AllAnimations);
    animationComboBox->addItem("No Animations", QChart::NoAnimation);
    return animationComboBox;
}

QComboBox *ThemeWidget::createLegendBox() const
{
    QComboBox *legendComboBox = new QComboBox();
    legendComboBox->addItem("Legend Top", Qt::AlignTop);
    legendComboBox->addItem("Legend Bottom", Qt::AlignBottom);
    legendComboBox->addItem("Legend Left", Qt::AlignLeft);
    legendComboBox->addItem("Legend Right", Qt::AlignRight);
    legendComboBox->addItem("No Legend ", 0);
    return legendComboBox;
}

QComboBox *ThemeWidget::createDateBox() const
{
    QComboBox *dateComboBox = new QComboBox();
    dateComboBox->addItem("Minutes", 0);
    dateComboBox->addItem("Hours", 0);
    dateComboBox->addItem("Days", 0);
    dateComboBox->addItem("Weeks", 0);
    dateComboBox->addItem("Months", 0);
    return dateComboBox;
}

QComboBox *ThemeWidget::createEnvBox() const
{
    QComboBox *envComboBox = new QComboBox();
    envComboBox->addItem("Jardin", 0);
    envComboBox->addItem("Salon", 0);
    envComboBox->addItem("Chambre", 0);
    envComboBox->addItem("Cuisine", 0);
    envComboBox->addItem("Salle de bain", 0);
    return envComboBox;
}

QChart *ThemeWidget::createBarChart(int valueCount) const
{
   /* Q_UNUSED(valueCount);
    QChart *chart = new QChart();
    chart->setTitle("Pression");

    QStackedBarSeries *series = new QStackedBarSeries(chart);
    for (int i(0); i < m_dataTable.count(); i++) {
        QBarSet *set = new QBarSet("Bar set " + QString::number(i));
        for (const Data &data : m_dataTable[i])
            *set << data.first.y();
        series->append(set);
    }
    chart->addSeries(series);
    chart->createDefaultAxes();

    return chart;*/

    return nullptr;
}

QChart *ThemeWidget::createSplineChart(QString Title, QSplineSeries* series) const
{
    QChart *chart = new QChart();
        chart->setTitle(Title);
        chart->addSeries(series);
        chart->legend()->hide();

        QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setFormat("yyyy-MM-dd/hh:mm");
        chart->setAxisX(axisX, series);

        QValueAxis *axisY = new QValueAxis;
        if (Title=="Temperature (°C)"){
        axisY->setRange(0, 40);
        }else{axisY->setRange(0, 0.25);}
        chart->setAxisY(axisY, series);

        return chart;
}

void ThemeWidget::updateUI()
{
    const auto charts = m_charts;
        int value= m_themeComboBox->currentIndex();
        int value2= m_animatedComboBox->currentIndex();
        if (value2==-1){value2=0;};
        int depart=0;
        int fin=0;
        for (QChartView *chartView : charts){
            QString titre= chartView->chart()->title();
            QDateTimeAxis *axisX = new QDateTimeAxis;
            QValueAxis *axisY = new QValueAxis;
            chartView->chart()->removeAllSeries();
            chartView->chart()->removeAxis(chartView->chart()->axisX());
            chartView->chart()->removeAxis(chartView->chart()->axisY());
            depart=0;
            fin=0;
            if (value<value2){
                depart=value;
                fin=value2;
            }
            else if(value>value2){
                depart=value2;
                fin=value;
            }
            else if (value==value2){
                depart=0;
                fin=dataMoment.size()-1;
            }
            if (titre=="Temperature (°C)"){
                QSplineSeries *seriesT=new QSplineSeries;
                for (int i=depart;i<fin+1;i++)
                {
                    seriesT->append(seriesTemp.at(i));
                }
                chartView->chart()->addSeries(seriesT);
                axisX->setRange(dataMoment.at(depart),dataMoment.at(fin));
                chartView->chart()->setAxisX(axisX, seriesT);
               /* minTemp = minData(dataTemp, depart, fin);
                maxTemp = maxData(dataTemp, depart, fin);
                axisY->setRange(minTemp*0.9, maxTemp*1.1);*/
                chartView->chart()->setAxisY(axisY, seriesT);
            }else{//"Pression (hPa)"
                QSplineSeries *seriesP=new QSplineSeries;
                for (int i=depart;i<fin+1;i++)
                {
                    seriesP->append(seriesPress.at(i));
                }
                chartView->chart()->addSeries(seriesP);
                axisX->setRange(dataMoment.at(depart),dataMoment.at(fin));
                chartView->chart()->setAxisX(axisX, seriesP);
                axisY->setRange(0, 0.25);
                /*minPress = minData(dataPress, depart, fin);
                maxPress = maxData(dataPress, depart, fin);
                axisY->setRange(minPress*0.99, maxPress*1.01);*/
                chartView->chart()->setAxisY(axisY, seriesP);
            }
        }
       /* QString texte=  " [ Température (°C) => Minimum : " + QString::number(minTemp) +" - Maximum : " + QString::number(maxTemp) + " ] [ Pression (hPa) => Minimum : " + QString::number(minPress) + " - Maximum : " + QString::number(maxPress) + " ]";
        extremum->setText(texte);*/
}

