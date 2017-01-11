/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */
#ifndef ArchiveSF_Plugin_H
#define ArchiveSF_Plugin_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>
#include <QTimer>
#include <QThread>
#include <qwt.h>
#include "cacartesianplot.h"
#include "controlsinterface.h"
#include "archiveSF_plugin.h"
#include "archiverCommon.h"
#include "sfRetrieval.h"

class Q_DECL_EXPORT WorkerSF : public QObject
{
    Q_OBJECT

public:
    WorkerSF() {
        qRegisterMetaType<indexes>("indexes");
        qRegisterMetaType<QVector<double> >("QVector<double>");
    }

private:
    QVector<double>  TimerN, YValsN;

public slots:

    void workerFinish() {
        deleteLater();
    }

    void getFromArchive(QWidget *w, indexes indexNew) {
        Q_UNUSED(w);

        QMutex *mutex = indexNew.mutexP;
        mutex->lock();

        struct timeb now;
        QUrl url = QUrl("http://data-api.psi.ch/sf/query");
        QString fields, agg;
        bool isBinned;

        QString key = indexNew.pv;
        int nbVal = 0;

        ftime(&now);
        double endSeconds = (double) now.time + (double) now.millitm / (double)1000;
        double startSeconds = endSeconds - indexNew.secondsPast;
        QString response ="'response':{'format':'csv'}";
        QString channels = "'channels': [ '" + key + "' ]";
        QString range = "'range': { 'startSeconds' : '" + QString::number(startSeconds, 'g', 10) + "', 'endSeconds' : '" + QString::number(endSeconds, 'g', 10) + "'}";

        if(indexNew.nrOfBins != -1) {
            isBinned = true;
            fields = "'fields':['channel','iocSeconds','value']";
            agg = tr(", 'aggregation': {'aggregationType':'value', 'aggregations':['min','mean','max'], 'nrOfBins' : %1}").arg(indexNew.nrOfBins);
        } else {
            isBinned = false;
            fields = "'fields':['channel','iocSeconds','value']}";
            agg = "";
        }
        QString total = "{" + response + "," + range + "," + channels + "," + fields + agg + "}";
        total = total.replace("'", "\"");
        QByteArray json_str = total.toUtf8();

        sfRetrieval *fromArchive = new sfRetrieval();

        if(fromArchive->requestUrl(url, json_str, indexNew.secondsPast, isBinned)) {

            if((nbVal = fromArchive->getCount()) > 0) {
                //qDebug() << nbVal << total;
                TimerN.resize(fromArchive->getCount());
                YValsN.resize(fromArchive->getCount());
                fromArchive->getData(TimerN, YValsN);
            }
        }
        fromArchive->deleteLater();

        //qDebug() << "number of values received" << nbVal;

        emit resultReady(indexNew, nbVal, TimerN, YValsN);

        mutex->unlock();
    }

signals:
    void resultReady(indexes indexNew, int nbVal, QVector<double> TimerN, QVector<double> YValsN);

public:

};

class Q_DECL_EXPORT ArchiveSF_Plugin : public QObject, ControlsInterface
{
    Q_OBJECT

    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.democontrols")
#endif

public:
    QString pluginName();
    ArchiveSF_Plugin();
    ~ArchiveSF_Plugin(){}

    int initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);
    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
    int pvGetTimeStamp(char *pv, char *timestamp);
    int pvGetDescription(char *pv, char *description);
    int pvClearEvent(void * ptr);
    int pvAddEvent(void * ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();

public slots:
    void handleResults(indexes, int, QVector<double>, QVector<double>);
signals:
    void operate(QWidget*, const indexes);

private slots:
    void Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes);

private:
    QMutex mutex;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;
    ArchiverCommon *archiverCommon;
    QMap<QString, QThread*> listOfThreads;
};

#endif