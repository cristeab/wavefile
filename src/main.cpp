#include <iostream>
#include "Wave.h"

#define ROOT_DIR "/Users/bogdan/projects/wavefile/test/"
static
const char* fileList[] = {"ref_float_32bit.wav", "ref_float_32bit_stereo.wav", 
    "ref_pcm_16bit.wav", "ref_pcm_16bit_stereo.wav"};
static
const uint32_t fileListSize = sizeof(fileList)/sizeof(fileList[0]);

static
void dumpWaveHeader(const Wave &wav)
{
    std::cout << "Size " << wav.getSize() << std::endl;
    std::cout << "Channels " << wav.getChannels() << std::endl;
    std::cout << "Sample rate [Hz] " << wav.getSampleRateHz() << std::endl;
    std::cout << "Bits/sample " << wav.getBitsPerSample() << std::endl;
    std::cout << "Wave format " << wav.getWaveFormat() << std::endl;    
}

int main()
{
    Wave wav;
    for (uint32_t i = 0; i < fileListSize; ++i) {
        std::cout << "Processing " << fileList[i] << std::endl;
        std::string filePath(ROOT_DIR);
        filePath += fileList[i];
        int rc = wav.load(filePath);
        if (EXIT_SUCCESS != rc) {
            std::cout << "Cannot load " << fileList[i] << std::endl;
            continue;
        }
        dumpWaveHeader(wav);

        //test save
        {
            std::string newFilename(fileList[i]);
            newFilename[0] = 'a';
            newFilename[1] = 'c';
            newFilename[2] = 't';
            rc = wav.save(newFilename);
            if (EXIT_SUCCESS != rc) {
                std::cout << "Cannot save " << newFilename << std::endl;
            }
        }

        //test mix
        do {
            std::string newFilename(fileList[i]);
            newFilename[0] = 'm';
            newFilename[1] = 'i';
            newFilename[2] = 'x';
            rc = wav.mix();
            if (EXIT_SUCCESS != rc) {
                std::cout << "Cannot mix " << fileList[i] << std::endl;
                break;
            }
            dumpWaveHeader(wav);
            rc = wav.save(newFilename);
            if (EXIT_SUCCESS != rc) {
                std::cout << "Cannot save " << newFilename << std::endl;
            }
        } while (false);
    }

    return EXIT_SUCCESS;
}