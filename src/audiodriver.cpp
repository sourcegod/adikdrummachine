#include "audiodriver.h"
#include <iostream>
#include <portaudio.h>


#include "audiodriver.h"
#include <iostream>

AudioDriver::AudioDriver() : stream_(nullptr), lastError_(paNoError) {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Erreur lors de l'initialisation de PortAudio : " << Pa_GetErrorText(err) << std::endl;
        lastError_ = err;
    }
}

AudioDriver::~AudioDriver() {
    closeAudioDriver();
}

bool AudioDriver::initAudioDriver(PaStreamCallback *callback, void *userData, int sampleRate, int framesPerBuffer) {
    PaStreamParameters outputParameters;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "Erreur : Pas de périphérique de sortie audio par défaut trouvé." << std::endl;
        return false;
    }

    outputParameters.sampleFormat = paFloat32;
    outputParameters.channelCount = 1;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &stream_,
        nullptr, // no input
        &outputParameters,
        sampleRate,
        framesPerBuffer,
        paClipOff,      // we won't clip signals that exceed range
        callback,
        userData);

    if (err != paNoError) {
        std::cerr << "Erreur lors de l'ouverture du flux audio : " << Pa_GetErrorText(err) << std::endl;
        lastError_ = err;
        return false;
    }

    return true;
}

bool AudioDriver::startAudioDriver() {
    if (stream_ == nullptr) {
        std::cerr << "Erreur : Le flux audio n'a pas été initialisé." << std::endl;
        return false;
    }

    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "Erreur lors du démarrage du flux audio : " << Pa_GetErrorText(err) << std::endl;
        lastError_ = err;
        return false;
    }

    return true;
}

bool AudioDriver::stopAudioDriver() {
    if (stream_ != nullptr) {
        PaError err = Pa_StopStream(stream_);
        if (err != paNoError) {
            std::cerr << "Erreur lors de l'arrêt du flux audio : " << Pa_GetErrorText(err) << std::endl;
            lastError_ = err;
            return false;
        }

        err = Pa_CloseStream(stream_);
        if (err != paNoError) {
            std::cerr << "Erreur lors de la fermeture du flux audio : " << Pa_GetErrorText(err) << std::endl;
            lastError_ = err;
            return false;
        }
        stream_ = nullptr;
    }
    return true;
}

bool AudioDriver::closeAudioDriver() {
    if (stream_ != nullptr) {
        PaError err = Pa_CloseStream(stream_);
        if (err != paNoError) {
            std::cerr << "Erreur lors de la fermeture du flux audio : " << Pa_GetErrorText(err) << std::endl;
            lastError_ = err;
            return false;
        }
        stream_ = nullptr;
    }
    PaError err = Pa_Terminate();
    if (err != paNoError) {
        std::cerr << "Erreur lors de la terminaison de PortAudio : " << Pa_GetErrorText(err) << std::endl;
        lastError_ = err;
        return false;
    }
    return true;
}

 
/*
AudioDriver::AudioDriver() : stream_(nullptr), is_initialized_(false) {}

AudioDriver::~AudioDriver() {
    terminate();
}

PaError AudioDriver::initialize() {
    PaError err = Pa_Initialize();
    if (err == paNoError) {
        is_initialized_ = true;
    } else {
        std::cerr << "Erreur lors de l'initialisation de PortAudio: " << Pa_GetErrorText(err) << std::endl;
    }
    return err;
}

PaError AudioDriver::openStream(int sampleRate, PaStreamCallback* callback, void* userData) {
    if (!is_initialized_) {
        std::cerr << "PortAudio n'est pas initialisé." << std::endl;
        return paNotInitialized;
    }

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "Pas de périphérique de sortie audio par défaut trouvé après l'initialisation." << std::endl;
        return paNoDevice;
    }
    outputParameters.channelCount = 1;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &stream_,
        nullptr, // no input
        &outputParameters,
        sampleRate,
        256, // framesPerBuffer
        paClipOff,
        callback,
        userData);
    if (err != paNoError) {
        std::cerr << "Erreur lors de l'ouverture du flux audio: " << Pa_GetErrorText(err) << std::endl;
        stream_ = nullptr;
    }
    return err;
}

PaError AudioDriver::startStream() {
    if (!stream_) {
        std::cerr << "Le flux audio n'est pas ouvert." << std::endl;
        return paBadStreamPtr;
    }
    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "Erreur lors du démarrage du flux audio: " << Pa_GetErrorText(err) << std::endl;
    }
    return err;
}

PaError AudioDriver::stopStream() {
    if (stream_) {
        PaError err = Pa_StopStream(stream_);
        if (err != paNoError) {
            std::cerr << "Erreur lors de l'arrêt du flux audio: " << Pa_GetErrorText(err) << std::endl;
        }
        return err;
    }
    return paNoError;
}

PaError AudioDriver::closeStream() {
    if (stream_) {
        PaError err = Pa_CloseStream(stream_);
        if (err != paNoError) {
            std::cerr << "Erreur lors de la fermeture du flux audio: " << Pa_GetErrorText(err) << std::endl;
        }
        stream_ = nullptr;
        return err;
    }
    return paNoError;
}

PaError AudioDriver::terminate() {
    if (is_initialized_) {
        PaError err = Pa_Terminate();
        if (err != paNoError) {
            std::cerr << "Erreur lors de la terminaison de PortAudio: " << Pa_GetErrorText(err) << std::endl;
        }
        is_initialized_ = false;
        return err;
    }
    return paNoError;
}
*/

