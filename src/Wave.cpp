#include <cstdio>
#include <limits>
#include <cstdlib>
#include "Wave.h"

struct FileLock {
	FileLock(FILE *f) : file_(f) {}
	~FileLock() {
		if (NULL != file_) {
			fclose(file_);
		}
	}
	operator FILE*() const { return file_; }
	bool isValid() const { return (NULL != file_); }
private:
	FILE *file_ = NULL;
};

Wave::Wave() : headerSize(sizeof(descriptor_)+sizeof(format_)+8)
{
	// Init members
	memset(&descriptor_, 0, sizeof(descriptor_));
	memset(&format_, 0, sizeof(format_));
}

int Wave::load(const std::string &filePath)
{
	// Load .WAV file
	FileLock file = fopen(filePath.c_str(), "rb");
	if (!file.isValid()) {
		return EXIT_FAILURE;
	}
	// Read .WAV descriptor
	size_t count = fread(&descriptor_, sizeof(descriptor_), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}
	// Check for valid .WAV file
	if ((0 != strncmp(descriptor_.riff, "RIFF", 4)) ||
		(0 != strncmp(descriptor_.wave, "WAVE", 4))) {
		return EXIT_FAILURE;
	}
	// Read .WAV format
	count = fread(&format_, sizeof(format_), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}
	// Check for valid .WAV file
	if (0 != strncmp(format_.id, "fmt ", 4)) {
		return EXIT_FAILURE;
	}
	// Read next chunk
	char id[4];
	do {
		count = fread(id, sizeof(id), 1, file);
		if (1 != count) {
			return EXIT_FAILURE;
		}
	} while (0 != strncmp(id, "data", 4));
	count = fread(&size_, sizeof(size_), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}
	uint32_t offset = ftell(file);
	// Read .WAV data
	data_.reset(new char[size_]);
	count = fread(data_.get(), size_*sizeof(char), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Wave::save(const std::string &filePath)
{
	// Save .WAV file
	FileLock file = fopen(filePath.c_str(), "wb");
	if (!file.isValid()) {
		return EXIT_FAILURE;
	}
	// Save .WAV descriptor
	size_t count = fwrite(&descriptor_, sizeof(descriptor_), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}
	// Save .WAV format
	count = fwrite(&format_, sizeof(format_), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}
	// Write .WAV data
	static const char id[4] = {'d', 'a', 't', 'a'};
	count = fwrite(id, sizeof(id), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}
	count = fwrite(&size_, sizeof(size_), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}
	count = fwrite(data_.get(), size_*sizeof(char), 1, file);
	if (1 != count) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Wave::raw2float(float *&samp, uint32_t &sampSize, const char *raw,
		uint32_t rawSize, uint16_t bitsPerSample)
{
	sampSize = rawSize/(bitsPerSample/8);
	samp = new float[sampSize];
	switch (bitsPerSample) {
		case 8:
		{
			const float max = std::numeric_limits<char>::max();
			for (uint32_t i = 0; i < sampSize; ++i) {
				samp[i] = static_cast<float>(raw[i])/max;		
			}
		}
		break;
		case 16:
		{
			const float max = std::numeric_limits<int16_t>::max();
			const int16_t *ptr = reinterpret_cast<const int16_t*>(raw);
			for (uint32_t i = 0; i < sampSize; ++i) {
				samp[i] = static_cast<float>(ptr[i])/max;
			}
		}
		break;
		case 24:
		{
			const float max = std::numeric_limits<int32_t>::max()-256;
			for (uint32_t i = 0; i < sampSize; ++i) {
				int tmpSamp = (raw[2] << 24) | (raw[1] << 16) | (raw[0] << 8);
				raw += 3;
				samp[i] = tmpSamp/max;
			}
		}
		break;
		case 32:
		{
			const float max = std::numeric_limits<int32_t>::max();
			const int32_t *ptr = reinterpret_cast<const int32_t*>(raw);
			for (uint32_t i = 0; i < sampSize; ++i) {
				samp[i] = static_cast<float>(ptr[i])/max;
			}
		}
		break;
		default:
		delete[] samp;
		samp = nullptr;
		sampSize = 0;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Wave::float2raw(char *&raw, uint32_t &rawSize, const float *samp,
		uint32_t sampSize, uint16_t bitsPerSample)
{
	rawSize = sampSize*(bitsPerSample/8);
	raw = new char[rawSize];
	switch (bitsPerSample) {
		case 8:
		{
			const float max = std::numeric_limits<char>::max();
			for (uint32_t i = 0; i < sampSize; ++i) {
				raw[i] = static_cast<char>(max*samp[i]);
			}
		}
		break;
		case 16:
		{
			const float max = std::numeric_limits<int16_t>::max();
			int16_t *ptr = reinterpret_cast<int16_t*>(raw);
			for (uint32_t i = 0; i < sampSize; ++i) {
				ptr[i] = static_cast<int16_t>(max*samp[i]);
			}
		}
		break;
		case 24:
		{
			const float max = std::numeric_limits<int32_t>::max()-256;
			char *ptr = raw;
			for (uint32_t i = 0; i < sampSize; ++i) {
				const int tmpSamp = static_cast<int>(max*samp[i]);
				ptr[2] = static_cast<char>(tmpSamp >> 24);
				ptr[1] = static_cast<char>(tmpSamp >> 16);
				ptr[0] = static_cast<char>(tmpSamp >> 8);
				ptr += 3;
			}
		}
		break;
		case 32:
		{
			const float max = std::numeric_limits<int32_t>::max();
			int32_t *ptr = reinterpret_cast<int32_t*>(raw);
			for (uint32_t i = 0; i < sampSize; ++i) {
				ptr[i] = static_cast<int32_t>(max*samp[i]);
			}
		}
		break;
		default:
		delete []raw;
		raw = nullptr;
		rawSize = 0;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

float Wave::mixSamples(float left, float right)
{
	float mix = left+right;
	if ((left < 0) && (right < 0)) {
		mix += left*right;
	} else if ((left > 0) && (right > 0)) {
		mix -= left*right;
	}
	return mix;
}

int Wave::mix()
{
	if (1 == format_.channels) {
		return EXIT_SUCCESS;
	}
	float *data = nullptr;
	uint32_t dataSize = 0;
	int rc = EXIT_FAILURE;
	switch (format_.format) {
		case WAVE_FORMAT_PCM:
		rc = raw2float(data, dataSize, data_.get(), size_, format_.bitsPerSample);
		if (EXIT_SUCCESS != rc) {
			return rc;
		}
		break;
		case WAVE_FORMAT_IEEE_FLOAT:
		data = reinterpret_cast<float*>(data_.release());
		dataSize = size_/sizeof(float);
		break;
		default:
		return EXIT_FAILURE;
	}
	size_ = dataSize*sizeof(float)/format_.channels;
	data_.reset(new char[size_]);
	float *dst = reinterpret_cast<float*>(data_.get());
	const uint32_t numSamples = dataSize/format_.channels;
	for (uint32_t i = 0; i < numSamples; ++i) {
		dst[i] = data[format_.channels*i];
		for (uint16_t c = 1; c < format_.channels; ++c) {
			dst[i] = mixSamples(dst[i], data[format_.channels*i+c]);
		}
	}
	delete[] data;
	//restore format if needed
	if (WAVE_FORMAT_PCM == format_.format) {
		char *rawData = nullptr;
		uint32_t rawDataSize = 0;
		rc = float2raw(rawData, rawDataSize, reinterpret_cast<float*>(data_.get()),
			size_/sizeof(float), format_.bitsPerSample);
		if (EXIT_SUCCESS != rc) {
			return rc;
		}
		data_.reset(rawData);
		size_ = rawDataSize;
	}
	format_.channels = 1;
	descriptor_.size = size_+headerSize-8;
	format_.byteRate = format_.sampleRate*format_.channels*format_.bitsPerSample/8;
	format_.blockAlign = format_.channels*format_.bitsPerSample/8;
	return EXIT_SUCCESS;
}

std::unique_ptr<Wave> Wave::getChannel(uint16_t channel)
{
	if (channel >= format_.channels) {
		return std::unique_ptr<Wave>();
	}
	std::unique_ptr<Wave> chWave(new Wave());
	memcpy(&chWave->descriptor_, &descriptor_, sizeof(descriptor_));
	memcpy(&chWave->format_, &format_, sizeof(format_));
	if (1 == format_.channels) {
		chWave->size_ = size_;
		chWave->data_.reset(new char[size_]);
		memcpy(chWave->data_.get(), data_.get(), size_);
		return chWave;//just return a copy of itself
	}
	chWave->format_.channels = 1;
	chWave->size_ = size_/format_.channels;
	chWave->data_.reset(new char[chWave->size_]);
	chWave->descriptor_.size = chWave->size_+headerSize-8;
	chWave->format_.byteRate = chWave->format_.sampleRate*chWave->format_.channels*
		chWave->format_.bitsPerSample/8;
	chWave->format_.blockAlign = chWave->format_.channels*chWave->format_.bitsPerSample/8;

	//copy the requested channel
	switch (chWave->format_.format) {
		case WAVE_FORMAT_PCM:
		{
			switch (format_.bitsPerSample) {
				case 8:
				{
					char *src = data_.get();
					char *dst = chWave->data_.get();
					for (uint32_t i = 0; i < chWave->size_; ++i) {
						dst[i] = src[i*format_.channels+channel];
					}
				}
				break;
				case 16:
				{
					int16_t *src = reinterpret_cast<int16_t*>(data_.get());
					int16_t *dst = reinterpret_cast<int16_t*>(chWave->data_.get());
					for (uint32_t i = 0; i < chWave->size_/sizeof(int16_t); ++i) {
						dst[i] = src[i*format_.channels+channel];
					}
				}
				break;
				case 24:
				{
					char *src = data_.get();
					char *dst = chWave->data_.get();
					src += 3*channel;
					for (uint32_t i = 0; i < chWave->size_/3; ++i) {
						dst[0] = src[0];
						dst[1] = src[1];
						dst[2] = src[2];
						dst += 3;
						src += 3*format_.channels;
					}					
				}
				break;
				case 32:
				{
					int32_t *src = reinterpret_cast<int32_t*>(data_.get());
					int32_t *dst = reinterpret_cast<int32_t*>(chWave->data_.get());
					for (uint32_t i = 0; i < chWave->size_/sizeof(int32_t); ++i) {
						dst[i] = src[i*format_.channels+channel];
					}
				}
				break;
				default:
				return std::unique_ptr<Wave>();
			}
		}
		break;
		case WAVE_FORMAT_IEEE_FLOAT:
		{
			float *src = reinterpret_cast<float*>(data_.get());
			float *dst = reinterpret_cast<float*>(chWave->data_.get());
			for (uint32_t i = 0; i < chWave->size_/sizeof(float); ++i) {
				dst[i] = src[i*format_.channels+channel];
			}
		}
		break;
		default:
		return std::unique_ptr<Wave>();
	}
	return chWave;
}
