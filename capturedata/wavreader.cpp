#include <algorithm>
#include <QDebug>
#include "wavreader.h"

WavReader::WavReader(QObject *parent) :
    QObject(parent),
    _audioOutputDevice(QAudioDeviceInfo::defaultOutputDevice()),
    _file(new WavFile),
    _decoder(new QAudioDecoder)
{
    connect(_decoder.get(), &QAudioDecoder::sourceChanged, this, [=](){
//        qDebug() << "sourceChanged";
        _decodedSamples.clear();
        _decoder->start();
    });
//    connect(_decoder.get(), &QAudioDecoder::stateChanged, this, [=](QAudioDecoder::State state){
//        qDebug() << state;
//    });
    connect(_decoder.get(), &QAudioDecoder::bufferReady, this, [=](){
//        qDebug() << "bufferReady";
        QAudioBuffer audioBuffer = this->_decoder->read();
        const qint16 *data = audioBuffer.constData<qint16>();
        const qint8 *data2 = audioBuffer.constData<qint8>();
//        qDebug() << audioBuffer.sampleCount();
        std::copy_n(data, audioBuffer.sampleCount(), std::back_inserter(_decodedSamples));
//        qDebug() << audioBuffer.byteCount();
        std::copy_n(data2, audioBuffer.byteCount(), std::back_inserter(_rawSamples));

    });
    connect(_decoder.get(), &QAudioDecoder::finished, this, [=](){
//        qDebug() << "finished";
        emit readAllData(_decodedSamples);
    });

    QAudioFormat format;
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setChannelCount(1);
    format.setCodec("audio/pcm");
    format.setSampleRate(44100);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SampleType::SignedInt);

    _audioOutput.reset(new QAudioOutput(_audioOutputDevice, format/*, this*/));
    connect(_audioOutput.get(), SIGNAL(notify()), this, SLOT(audioNotify()));
    connect(_audioOutput.get(), SIGNAL(stateChanged(QAudio::State)), this, SIGNAL(stateChanged(QAudio::State)));

}

//WavReader::WavReader(QObject *parent, const QAudioDeviceInfo& audioOutputDevice) :
//    QObject(parent),
//    _audioOutputDevice(audioOutputDevice),

//    _playPosition(0),
//    _bufferPosition(0),
//    _bufferLength(0),
//    _dataLength(0),
//    _file(new WavFile),
//    _decoder(new QAudioDecoder)
//{
//}


bool isPCM(const QAudioFormat &format)
{
    return (format.codec() == "audio/pcm");
}


bool isPCMS16LE(const QAudioFormat &format)
{
    return isPCM(format) &&
           format.channelCount() == 1 &&
           format.sampleType() == QAudioFormat::SignedInt &&
           format.sampleSize() == 16 &&
           format.byteOrder() == QAudioFormat::LittleEndian;
}



void WavReader::setNotifyInterval(const int& ms)
{
    if(_audioOutput) _audioOutput->setNotifyInterval(ms);
}


void WavReader::play()
{
    if (!_file || !isPCMS16LE(_file->fileFormat()) || !_audioOutput)
        return;

    _audioOutput->start(_file.get());
}

void WavReader::stop()
{
    if(_audioOutput) _audioOutput->stop();
}

void WavReader::release()
{
    if(_audioOutput)  _audioOutput->reset();
}

QAudio::State WavReader::state()
{
    if (_audioOutput)
        return _audioOutput->state();
    else
        return QAudio::IdleState;
}

QAudioFormat WavReader::format() const
{
    return _file->fileFormat();
}

QAudioOutput *WavReader::audioOutput() const
{
    return _audioOutput.get();
}


int WavReader::setSource(const QString& fileName)
{
    _decoder->setSourceFilename( fileName );

    if (_file->open(fileName))
    {
        const QAudioFormat format = _file->fileFormat();
        if (isPCMS16LE(format) && _audioOutputDevice.isFormatSupported(format))
        {
            return Succes;
        }
        else
        {
            return FileFormatNotSupported;
        }
    }
    else
    {
        return FileNotFound;
    }

}

qint64 audioLength(const QAudioFormat &format, qint64 microSeconds)
{
    return format.bytesForDuration(microSeconds);
}

void WavReader::audioNotify()
{
    if (!_audioOutput)
        return;

    const QAudioFormat audioFormat = _file->fileFormat();//_decoder->audioFormat();
    qint64 localProcessedUSecs = _audioOutput->processedUSecs();

    const qint64 readLength = _audioOutput->periodSize();
    const qint64 endBufferPosition = audioLength(audioFormat, localProcessedUSecs);
    const qint64 startBufferPosition = endBufferPosition - readLength;

    QByteArray buffer2(_rawSamples.constData()+startBufferPosition, readLength);
    QAudioBuffer audioBuffer2 = QAudioBuffer(buffer2, audioFormat, -1);

    emit bufferReady(audioBuffer2);
    emit processedUSecs(localProcessedUSecs);

}

QString WavReader::fileName() const
{
    return _decoder->sourceFilename();
}

