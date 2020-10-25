#ifndef WAVREADER_H
#define WAVREADER_H

#include "wavfile.h"

#include <QAudioBuffer>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudio>
#include <QObject>
#include <QAudioDecoder>

#define FileNotFound 0
#define Succes 1
#define AudioOutputDeviceNotRecognized 2
#define FileFormatNotSupported 3



class WavReader : public QObject
{
    Q_OBJECT
public:
    explicit WavReader(QObject *parent = 0);
    explicit WavReader(QObject *parent, const QAudioDeviceInfo &audioOutputDevice);
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
    void stateChangeg(const QAudio::State& state);
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

    QByteArray          _buffer;
    qint64              _bufferPosition;
    qint64              _bufferLength;
    qint64              _dataLength;
    qint64              _headerLength;
    qint64              _playPosition;

};

#endif // WAVREADER_H
