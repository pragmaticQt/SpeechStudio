#include <algorithm>
#include <QDebug>
#include "wavreader.h"

WavReader::WavReader(QObject *parent) :
    QObject(parent),
    _audioOutputDevice(QAudioDeviceInfo::defaultOutputDevice()),
    _file(new WavFile),
    _decoder(new QAudioDecoder)
{
    connect(_decoder.get(), &QAudioDecoder::sourceChanged, this, [=](){ qDebug() << "sourceChanged"; });
    connect(_decoder.get(), &QAudioDecoder::stateChanged, this, [=](QAudioDecoder::State state){
        qDebug() << state;
    });
    connect(_decoder.get(), &QAudioDecoder::bufferReady, this, [=](){
        qDebug() << "bufferReady";
        QAudioBuffer audioBuffer = this->_decoder->read();
        const qint16 *data = audioBuffer.constData<qint16>();
        QVector<double> readAll(audioBuffer.sampleCount(),0);
        std::copy_n(data, audioBuffer.sampleCount(), readAll.data());
        emit readAllData(readAll);
    });
    connect(_decoder.get(), &QAudioDecoder::finished, this, [=](){
        qDebug() << "finished";
    });
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



void WavReader::setNotifyInterval(const int& ms) {
    if(_audioOutput) _audioOutput->setNotifyInterval(ms);
}


void WavReader::play(){
    if (!_file)
        return;
    if (!isPCMS16LE(_file->fileFormat()))
        return;
    if (!_audioOutput)
        return ;
    _audioOutput->start(_file.get());
}

void WavReader::stop(){
    if(_audioOutput) _audioOutput->stop();
}

void WavReader::release(){
    if(_audioOutput) {
        _audioOutput->reset();
        _audioOutput.reset();
    }
}

QAudio::State WavReader::state(){
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


int WavReader::setSource(const QString& fileName){

    _decoder->setSourceFilename( fileName );

    if (_file->open(fileName)) {
        if (isPCMS16LE(_file->fileFormat()) && _audioOutputDevice.isFormatSupported(_file->fileFormat())){

            //            _bufferLength = _file->size();
            //            _headerLength = _file->headerLength();
            _audioOutput.reset(new QAudioOutput(_audioOutputDevice, _file->fileFormat()/*, this*/));
            connect(_audioOutput.get(), SIGNAL(notify()), this, SLOT(audioNotify()));
            connect(_audioOutput.get(), SIGNAL(stateChanged(QAudio::State)), this, SIGNAL(stateChanged(QAudio::State)));

            return Succes;
        } else {
            //            _decoder->setSourceFilename( "Error" );
            return FileFormatNotSupported;
        }
    } else {
        //        _decoder->setSourceFilename( "Error" );
        return FileNotFound;
    }

}


qint64 audioLength(const QAudioFormat &format, qint64 microSeconds)
{
    return format.bytesForDuration(microSeconds);
}

void WavReader::readAll(){
    //    _decoder->start();
    WavFile analysisFile;
    if (analysisFile.open(fileName()) && analysisFile.seek(analysisFile.headerLength())){
        QByteArray buffer;
        buffer.resize(analysisFile.size() - analysisFile.headerLength());
        /*_dataLength = */analysisFile.read(buffer.data(),buffer.size());
        QAudioBuffer audioBuffer = QAudioBuffer(buffer, analysisFile.fileFormat(), -1);
        const qint16 *data = audioBuffer.constData<qint16>();
        QVector<double> readAll(audioBuffer.sampleCount(),0);
        std::copy_n(data, audioBuffer.sampleCount(), readAll.data());

        emit readAllData(readAll);
    }
}



void WavReader::audioNotify(){
    if (!_audioOutput)
        return;

    QAudioFormat localAudioFormat = _file->fileFormat();//_decoder->audioFormat();
    const qint64 endBufferPosition = audioLength(localAudioFormat, _audioOutput->processedUSecs());
    const qint64 readLength = _audioOutput->periodSize();
    const qint64 startBufferPosition = endBufferPosition - readLength;

    WavFile analysisFile;
    if (analysisFile.open(fileName())
        && analysisFile.seek(analysisFile.headerLength() + startBufferPosition)
        && endBufferPosition < analysisFile.size()) {
        QByteArray buffer;
        buffer.resize(readLength);
        qint64 dataLength = analysisFile.read(buffer.data(), readLength);
        if (dataLength == readLength){
            QAudioBuffer audioBuffer = QAudioBuffer(buffer, localAudioFormat, -1);
            emit bufferReady(audioBuffer);
            emit processedUSecs(_audioOutput->processedUSecs());
        } else {
            qDebug() << "Problem with audio buffer";
        }
    } else {
        qDebug() << "Elapsed buffer position";
    }
}

QString WavReader::fileName() const
{
    return _decoder->sourceFilename();
}

