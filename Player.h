#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

#pragma comment(lib, "winmm.lib") // Standard lib import


class Player
{
public:
	Player(int16_t* soundData,
		unsigned int nSampleRate = 44100,
		unsigned int nChannels = 1,
		std::wstring sOutputDevice = Player::Enumerate()[0]) :

		m_rawData(soundData),
		m_sampleRate(nSampleRate),
		m_channels(nChannels),
		m_waveHeaders{},
		m_samplesPlayed(0),
		m_playSpeed(1.0)
	{
		if (m_rawData == nullptr)
			throw "Nothing to play";

		// Validate device
		std::vector<std::wstring> devices = Enumerate();
		auto d = std::find(devices.begin(), devices.end(), sOutputDevice);
		if (d != devices.end())
		{
			// Device is available
			size_t nDeviceID = distance(devices.begin(), d);
			WAVEFORMATEX waveFormat;
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nSamplesPerSec = m_sampleRate;
			waveFormat.wBitsPerSample = 16; // bitsPerSample
			waveFormat.nChannels = m_channels;
			waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;

			m_done = CreateEvent(0, false, false, 0);

			// Open Device if valid
			if (waveOutOpen(&m_hwDevice, (uint32_t)nDeviceID, &waveFormat, (DWORD_PTR)m_done, (DWORD_PTR)this, CALLBACK_EVENT) != S_OK)
			{
				throw "Invalid device";
				return;
			}
		}

		// Allocate Wave|Block Memory
		for (int i(0); i < 2; ++i)
		{
			m_waveHeaders[i].lpData = new char[2 * FRAME_SIZE];
			m_waveHeaders[i].dwBufferLength = 2 * FRAME_SIZE;
			if (!m_waveHeaders[i].lpData)
			{
				throw "No memory allocated";
				return;
			}
			ZeroMemory(m_waveHeaders[i].lpData, 2 * FRAME_SIZE);

			waveOutPrepareHeader(m_hwDevice, &m_waveHeaders[i], sizeof(WAVEHDR));
			waveOutWrite(m_hwDevice, &m_waveHeaders[i], sizeof(WAVEHDR));
		}
	}

	void play()
	{
		m_thread = std::thread(&Player::MainThread, this);
	}

	~Player()
	{
		m_thread.join();
	}

	double GetTime()
	{
		return m_globalTime;
	}

	static std::vector<std::wstring> Enumerate()
	{
		int nDeviceCount = waveOutGetNumDevs();
		std::vector<std::wstring> sDevices;
		WAVEOUTCAPS woc;
		for (int n = 0; n < nDeviceCount; n++)
			if (waveOutGetDevCaps(n, &woc, sizeof(WAVEOUTCAPS)) == S_OK)
				sDevices.push_back(woc.szPname);
		return sDevices;
	}

	int16_t clip(int16_t dSample, int16_t dMax)
	{
		if (dSample >= 0.0)
			return min(dSample, dMax);
		else
			return max(dSample, -dMax);
	}


	void setRawDataArray(int16_t* data)
	{
		m_rawData = data;
	}

	void setPlaySpeed(double speed)
	{
		m_playSpeed = speed;
	}

	uint64_t getCurrentSamplePos()
	{
		return m_samplesPlayed;
	}


	static constexpr size_t FRAME_SIZE = 8'000; // 16 bit samples
private:
	unsigned int m_sampleRate;
	unsigned int m_channels;
	unsigned int m_blockSamples;
	double m_playSpeed;

	HANDLE m_done;
	WAVEHDR m_waveHeaders[2]; // Multi-buffering
	HWAVEOUT m_hwDevice;
	int16_t* m_rawData;
	size_t m_samplesPlayed;

	std::thread m_thread;
	std::atomic<double> m_globalTime;
	std::mutex soundBufferMux[2];

	void MainThread()
	{
		m_globalTime = 0.0;
		double dTimeStep = 1.0 / (double)m_sampleRate;

		while (true)
		{
			WaitForSingleObject(m_done, INFINITE);
			ResetEvent(m_done);

			for (int bufId(0); bufId < 2; ++bufId)
			{
				if (m_waveHeaders[bufId].dwFlags & WHDR_DONE)
				{
					soundBufferMux[bufId].lock();

					for (int blockPos(0); blockPos < FRAME_SIZE; ++blockPos)
					{
						int16_t& sample = *(int16_t*)&m_waveHeaders[bufId].lpData[2 * blockPos];
						sample = m_rawData[size_t(m_playSpeed * m_samplesPlayed)];
						sample = clip(sample, 32767);

						m_globalTime = m_globalTime + dTimeStep;
						m_samplesPlayed++;
					}

					waveOutWrite(m_hwDevice, &m_waveHeaders[bufId], sizeof(WAVEHDR));

					soundBufferMux[bufId].unlock();
				}
			}
		}
	}
};