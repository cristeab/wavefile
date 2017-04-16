#ifndef _WAVE_H_
#define _WAVE_H_

#include <string>
#include <memory>

class Wave
{
public:
	enum {WAVE_FORMAT_PCM = 0x0001,
		WAVE_FORMAT_IEEE_FLOAT = 0x0003,
		WAVE_FORMAT_ALAW = 0x0006,
		WAVE_FORMAT_MULAW = 0x0007,
		WAVE_FORMAT_EXTENSIBLE = 0xFFFE};
	Wave();
	virtual ~Wave() = default;
	int load(const std::string &filePath);
	int save(const std::string &filePath);
	uint32_t getSize() const { return size_; }
	uint16_t getChannels() const { return format_.channels; }
	uint32_t getSampleRateHz() const { return format_.sampleRate; }
	uint16_t getBitsPerSample() const { return format_.bitsPerSample; }
	uint16_t getWaveFormat() const { return format_.format; }
	int mix();
	std::unique_ptr<Wave> getChannel(uint16_t channel);
private:
	Wave(const Wave&) = delete;
	Wave& operator=(const Wave&) = delete;
	int raw2float(float *&samp, uint32_t &sampSize, const char *raw,
		uint32_t rawSize, uint16_t bitsPerSample);
	int float2raw(char *&raw, uint32_t &rawSize, const float *samp,
		uint32_t sampSize, uint16_t bitsPerSample);
	float mixSamples(float left, float right);
	struct WaveDescr {
		char riff[4];
		uint32_t size;
		char wave[4];

	} descriptor_;
	struct WaveFormat {
		char id[4];
		uint32_t size;
		uint16_t format;
		uint16_t channels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;

	} format_;
	const uint32_t headerSize;
	std::unique_ptr<char> data_;
	uint32_t size_ = 0;
};

#endif
