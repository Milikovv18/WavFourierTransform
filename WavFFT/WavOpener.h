#pragma once

#include <filesystem>
#include <fstream>


// Only 16 bit uncompressed
class WavOpener
{
public:
	WavOpener() {}
	WavOpener(const char* name)
	{
		setFile(name);
	}

	void setFile(const char* name)
	{
		std::ifstream file(name, std::ios::binary);
		if (!file.is_open())
			throw "Bad file";

		m_fileName = std::filesystem::path(name).filename().string();

		file.read((char*)&chunkId, 4);
		file.read((char*)&chunkSize, 4);
		file.read((char*)&format, 4);
		file.read((char*)&subchunk1Id, 4);
		file.read((char*)&subchunk1Size, 4);
		file.read((char*)&audioFormat, 2);
		file.read((char*)&numChannel, 2);
		file.read((char*)&sampleRate, 4);
		file.read((char*)&byteRate, 4);
		file.read((char*)&blockAlign, 2);
		file.read((char*)&bitsPerSample, 2);
		file.read((char*)&subchunk2Id, 4);
		file.read((char*)&subchunk2Size, 4);

		data = new int16_t[(chunkSize - 36) / 2]; // 2 times larger than char
		file.read((char*)data, chunkSize - 36);
	}

	int16_t* getData()
	{
		return data;
	}

	size_t getDataSize()
	{
		return (chunkSize - 36) / 2;
	}

	uint32_t getDuration()
	{
		uint32_t numSamples = (chunkSize - 36) / (numChannel * (bitsPerSample / 8));
		uint32_t durationSeconds = numSamples / sampleRate;
		return durationSeconds;
	}

	uint16_t getChannelCount()
	{
		return numChannel;
	}

	uint32_t getSampleRate()
	{
		return sampleRate;
	}

	const char* getFileName()
	{
		return m_fileName.c_str();
	}


	~WavOpener()
	{
		delete[] data;
	}

private:

	std::ifstream m_file;
	std::string m_fileName;

	uint16_t extra{};
	char chunkId[4]{};
	uint32_t chunkSize{};
	char format[4]{};
	char subchunk1Id[4]{};
	uint32_t subchunk1Size{};
	uint16_t audioFormat{};
	uint16_t numChannel{};
	uint32_t sampleRate{};
	uint32_t byteRate{};
	uint16_t blockAlign{};
	uint16_t bitsPerSample{};
	char subchunk2Id[4]{};
	uint32_t subchunk2Size{};

	int16_t* data = nullptr;
};