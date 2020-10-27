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
//    void readAll();

    void setNotifyInterval(const int &ms);
    int setSource(const QString &fileName);

    QAudioOutput *audioOutput() const;    
    QAudioFormat format() const;
    QString fileName() const;
    QAudio::State state();    

public slots:
    void play();
    void stop();
    void release();

signals:
    void bufferReady(const QAudioBuffer& buffer);
    void processedUSecs(const qint64 us);
    void readAllData(const QVector<double>& data);
    void stateChanged(const QAudio::State& state);

private slots:
    void audioNotify();

private:

    QAudioDeviceInfo    _audioOutputDevice;

    QScopedPointer<QAudioOutput>    _audioOutput;
    QScopedPointer<WavFile>         _file;
    QScopedPointer<QAudioDecoder>   _decoder;

    QVector<double>     _decodedSamples;
    QVector<char>       _rawSamples;

};

#endif // WAVREADER_H
