#ifndef WAVREADER_H
#define WAVREADER_H

#include <QAudioBuffer>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudio>
#include <QObject>
#include <QAudioDecoder>
#include "wavfile.h"

#define FileNotFound 0
#define Succes 1
#define AudioOutputDeviceNotRecognized 2
#define FileFormatNotSupported 3



class WavReader : public QObject
{
    Q_OBJECT
public:
    explicit WavReader(QObject *parent = 0);
//    explicit WavReader(QObject *parent, const QAudioDeviceInfo &audioOutputDevice);
    void setNotifyInterval(const int &ms);
    int setSource(const QString &fileName);
    QAudioOutput *audioOutput() const;

    void readAll();
    QAudioFormat format() const;


    QAudio::State state();
    QString fileName() const;

    void release();
signals:
    void bufferReady(const QAudioBuffer& buffer);
    void processedUSecs(const qint64 us);
    void readAllData(const QVector<double>& data);
    void stateChanged(const QAudio::State& state);
public slots:
    void play();
    void stop();
private slots:
    void audioNotify();
private:

    QAudioDeviceInfo    _audioOutputDevice;

    QScopedPointer<QAudioOutput>    _audioOutput;
    QScopedPointer<WavFile>         _file;
    QScopedPointer<QAudioDecoder>   _decoder;

};

#endif // WAVREADER_H
