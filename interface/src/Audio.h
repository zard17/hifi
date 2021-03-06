//
//  Audio.h
//  interface/src
//
//  Created by Stephen Birarda on 1/22/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Audio_h
#define hifi_Audio_h

#ifdef _WIN32
#include <Systime.h>
#endif

#include <fstream>
#include <vector>

#include "InterfaceConfig.h"

#include <QAudio>
#include <QAudioInput>
#include <QGLWidget>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtMultimedia/QAudioFormat>
#include <QVector>

#include <AbstractAudioInterface.h>
#include <AudioRingBuffer.h>
#include <StdDev.h>

static const int NUM_AUDIO_CHANNELS = 2;

class QAudioInput;
class QAudioOutput;
class QIODevice;

class Audio : public AbstractAudioInterface {
    Q_OBJECT
public:
    // setup for audio I/O
    Audio(int16_t initialJitterBufferSamples, QObject* parent = 0);

    float getLastInputLoudness() const { return glm::max(_lastInputLoudness - _noiseGateMeasuredFloor, 0.f); }
    float getTimeSinceLastClip() const { return _timeSinceLastClip; }
    float getAudioAverageInputLoudness() const { return _lastInputLoudness; }

    void setNoiseGateEnabled(bool noiseGateEnabled) { _noiseGateEnabled = noiseGateEnabled; }
        
    void setJitterBufferSamples(int samples) { _jitterBufferSamples = samples; }
    int getJitterBufferSamples() { return _jitterBufferSamples; }
    
    void lowPassFilter(int16_t* inputBuffer);
    
    virtual void startCollisionSound(float magnitude, float frequency, float noise, float duration, bool flashScreen);
    virtual void startDrumSound(float volume, float frequency, float duration, float decay);
    
    float getCollisionSoundMagnitude() { return _collisionSoundMagnitude; }
    
    bool getCollisionFlashesScreen() { return _collisionFlashesScreen; }
    
    bool getMuted() { return _muted; }
    
    void init(QGLWidget *parent = 0);
    bool mousePressEvent(int x, int y);
    
    void renderToolBox(int x, int y, bool boxed);
    
    int getNetworkSampleRate() { return SAMPLE_RATE; }
    int getNetworkBufferLengthSamplesPerChannel() { return NETWORK_BUFFER_LENGTH_SAMPLES_PER_CHANNEL; }

public slots:
    void start();
    void stop();
    void addReceivedAudioToBuffer(const QByteArray& audioByteArray);
    void handleAudioInput();
    void reset();
    void toggleMute();
    void toggleAudioNoiseReduction();
    void toggleToneInjection();
    
    virtual void handleAudioByteArray(const QByteArray& audioByteArray);

    bool switchInputToAudioDevice(const QString& inputDeviceName);
    bool switchOutputToAudioDevice(const QString& outputDeviceName);
    QString getDeviceName(QAudio::Mode mode) const { return (mode == QAudio::AudioInput) ?
                                                            _inputAudioDeviceName : _outputAudioDeviceName; }
    QString getDefaultDeviceName(QAudio::Mode mode);
    QVector<QString> getDeviceNames(QAudio::Mode mode);

    float getInputVolume() const { return (_audioInput) ? _audioInput->volume() : 0.0f; }
    void setInputVolume(float volume) { if (_audioInput) _audioInput->setVolume(volume); }

signals:
    bool muteToggled();
    
private:

    QByteArray firstInputFrame;
    QAudioInput* _audioInput;
    QAudioFormat _desiredInputFormat;
    QAudioFormat _inputFormat;
    QIODevice* _inputDevice;
    int _numInputCallbackBytes;
    int16_t _localProceduralSamples[NETWORK_BUFFER_LENGTH_SAMPLES_PER_CHANNEL];
    QAudioOutput* _audioOutput;
    QAudioFormat _desiredOutputFormat;
    QAudioFormat _outputFormat;
    QIODevice* _outputDevice;
    int _numOutputCallbackBytes;
    QAudioOutput* _loopbackAudioOutput;
    QIODevice* _loopbackOutputDevice;
    QAudioOutput* _proceduralAudioOutput;
    QIODevice* _proceduralOutputDevice;
    AudioRingBuffer _inputRingBuffer;
    AudioRingBuffer _ringBuffer;

    QString _inputAudioDeviceName;
    QString _outputAudioDeviceName;
    
    StDev _stdev;
    timeval _lastReceiveTime;
    float _averagedLatency;
    float _measuredJitter;
    int16_t _jitterBufferSamples;
    float _lastInputLoudness;
    float _timeSinceLastClip;
    float _dcOffset;
    float _noiseGateMeasuredFloor;
    float* _noiseSampleFrames;
    int _noiseGateSampleCounter;
    bool _noiseGateOpen;
    bool _noiseGateEnabled;
    bool _toneInjectionEnabled;
    int _noiseGateFramesToClose;
    int _totalPacketsReceived;
    int _totalInputAudioSamples;
    
    float _collisionSoundMagnitude;
    float _collisionSoundFrequency;
    float _collisionSoundNoise;
    float _collisionSoundDuration;
    bool _collisionFlashesScreen;
    
    // Drum sound generator
    float _drumSoundVolume;
    float _drumSoundFrequency;
    float _drumSoundDuration;
    float _drumSoundDecay;
    int _drumSoundSample;
    
    int _proceduralEffectSample;
    int _numFramesDisplayStarve;
    bool _muted;
    bool _localEcho;
    GLuint _micTextureId;
    GLuint _muteTextureId;
    GLuint _boxTextureId;
    QRect _iconBounds;
    
    // Audio callback in class context.
    inline void performIO(int16_t* inputLeft, int16_t* outputLeft, int16_t* outputRight);
    
    // Process procedural audio by
    //  1. Echo to the local procedural output device
    //  2. Mix with the audio input
    void processProceduralAudio(int16_t* monoInput, int numSamples);

    // Add sounds that we want the user to not hear themselves, by adding on top of mic input signal
    void addProceduralSounds(int16_t* monoInput, int numSamples);
    
    // Process received audio
    void processReceivedAudio(const QByteArray& audioByteArray);

    bool switchInputToAudioDevice(const QAudioDeviceInfo& inputDeviceInfo);
    bool switchOutputToAudioDevice(const QAudioDeviceInfo& outputDeviceInfo);

    // Callback acceleration dependent calculations
    static const float CALLBACK_ACCELERATOR_RATIO;
    int calculateNumberOfInputCallbackBytes(const QAudioFormat& format);
    int calculateNumberOfFrameSamples(int numBytes);
    float calculateDeviceToNetworkInputRatio(int numBytes);

};


#endif // hifi_Audio_h
