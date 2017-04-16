#include <iostream>
#include "Wave.h"

int main()
{
    Wave wav;
    int rc = wav.load("test/ref_pcm_16bit.wav");
    if (EXIT_SUCCESS != rc) {
        std::cout << "Cannot load ref_pcm_16bit.wav" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Size " << wav.getSize() << std::endl;
    std::cout << "Channels " << wav.getChannels() << std::endl;
    std::cout << "Sample rate [Hz] " << wav.getSampleRateHz() << std::endl;
    std::cout << "Bits/sample " << wav.getBitsPerSample() << std::endl;
    std::cout << "Wave format " << wav.getWaveFormat() << std::endl;

    return EXIT_SUCCESS;
}