#include "audiodriver.h"
#include "constants.h"
#include "adikdrum.h"
#include <iostream>
#include <portaudio.h>

namespace adikdrum {

AudioDriver::AudioDriver() : stream_(nullptr), lastError_(paNoError) {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Erreur lors de l'initialisation de PortAudio : " << Pa_GetErrorText(err) << std::endl;
        lastError_ = err;
    }
    std::cout << "PortAudio is initialized successfully" << std::endl;
}
//----------------------------------------

AudioDriver::~AudioDriver() {
    close();
}
//----------------------------------------

bool AudioDriver::init(int numChannels, int sampleRate, int framesPerBuffer, void *userData) {
    PaStreamParameters outputParameters;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "Erreur : Pas de périphérique de sortie audio par défaut trouvé." << std::endl;
        return false;
    }

    outputParameters.sampleFormat = paFloat32;
    outputParameters.channelCount = numChannels;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &stream_,
        nullptr, // no input
        &outputParameters,
        sampleRate,
        framesPerBuffer,
        paClipOff,      // we won't clip signals that exceed range
        drumMachineCallback,
        userData);

    if (err != paNoError) {
        std::cerr << "Erreur lors de l'ouverture du flux audio : " << Pa_GetErrorText(err) << std::endl;
        lastError_ = err;
        return false;
    }

    if (stream_ == nullptr) {
        std::cerr << "Erreur : Le flux audio n'a pas été ouvert correctement (stream_ est nullptr)." << std::endl;
        return false;
    }

    return true;
}
//----------------------------------------

bool AudioDriver::start() {
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
//----------------------------------------

bool AudioDriver::stop() {
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
//----------------------------------------

bool AudioDriver::close() {
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
//----------------------------------------

int AudioDriver::drumMachineCallback(const void* inputBuffer, void* outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void* userData) {
    // Note: Casting parameters in void here, to avoid compiler warnings: inused parameter.
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;
    AdikDrum::DrumMachineData* data = static_cast<AdikDrum::DrumMachineData*>(userData);
    if (data && data->mixer) {
        float* out = static_cast<float*>(outputBuffer);
        static size_t frameCounter = 0;
        const size_t outputNumChannels = 2; // Assumons stéréo pour l'instant
        size_t samplesPerStep = static_cast<unsigned long>(data->sampleRate * data->player->secondsPerStep);
        const size_t numSamples = framesPerBuffer * outputNumChannels;
        std::vector<float> bufData(numSamples, 0.0f);

        if (frameCounter >= samplesPerStep) {
            frameCounter = 0;
            auto numSteps = data->player->getNumSteps();
            if (data->player->isPlaying()) {
                data->player->clickStep_ = data->player->getCurrentStep();
                if (data->player->isClicking() && data->player->clickStep_ % 4 == 0) {
                  data->player->playMetronome();
                }
                data->player->playPattern(numSteps);
            } else if (data->player->isClicking()) {
                if (data->player->clickStep_ % 4 == 0) {
                    data->player->playMetronome();
                }
                data->player->clickStep_ = (data->player->clickStep_ + 1) % numSteps;
            }
        }

        // Mixer les sons en utilisant la fonction dédiée
        data->mixer->mixSoundData(bufData, framesPerBuffer, outputNumChannels);

        // Copie du buffer de mixage vers le buffer de sortie PortAudio
        for (size_t i =0; i < numSamples; ++i) {
          // Note: il est recommandé de convertir la sortie en static_cast float, pour éviter des comportements inattendus de convertion de types implicites.  
          out[i] = static_cast<float>(data->player->hardClip(bufData[i] * data->mixer->getGlobalVolume() * GLOBAL_GAIN));
        
        }

        if (data->player->isPlaying() || data->player->isClicking()) {
            frameCounter += framesPerBuffer;
        }
        return paContinue;
    }
    return paContinue;
}
//----------------------------------------
//==== End of class AudioDriver ====

} // namespace adikdrum
