#include "audiomanager.h"
#include <iostream>
#include "audiodriver.h" // Assure-toi d'inclure ton AudioDriver
namespace adikdrum {
AudioManager::AudioManager() {
}

AudioManager::~AudioManager() {
    close();
}

bool AudioManager::init(PaStreamCallback *callback, void *userData, int sampleRate, int framesPerBuffer) {
    // Note: casting to avoid warnings
    (void)callback;
    (void)userData;
    (void)sampleRate;
    (void)framesPerBuffer;
    /*
    if (audioDriver_.init(callback, userData, sampleRate, framesPerBuffer) == paNoError) {
        std::cout << "Audio driver initialized successfully." << std::endl;
        if (audioDriver_.start() == paNoError) {
            std::cout << "Audio stream started successfully." << std::endl;
            return true;
        } else {
            std::cerr << "Failed to start audio stream: " << Pa_GetErrorText(audioDriver_.getLastError()) << std::endl;
            audioDriver_.close();
            return false;
        }

    } else {
        std::cerr << "Hey man, Failed to initialize audio driver: " << Pa_GetErrorText(audioDriver_.getLastError()) << std::endl;
        return false;
    }
    */

    return true;
}

void AudioManager::close() {
    audioDriver_.stop();
    audioDriver_.close();
    std::cout << "Audio driver stopped and terminated." << std::endl;
}

} // namespace adikdrum
