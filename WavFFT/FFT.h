#pragma once

#include <corecrt_math_defines.h>


template <unsigned N>
class FFT
{
	struct Complex {
		double real;
		double imag;
	};

public:
	FFT(uint16_t channelNum) :
		m_channelNum(channelNum)
	{
		static_assert(N && ((N & (N - 1)) == 0), "Not a power of 2");

		m_hannWindow = new double[N];
		m_fftArray = new Complex[N];
		m_decibels = new int16_t[N];

		// Generating hamming window
		for (int i(0); i < N; i++)
			m_hannWindow[i] = 0.54 - 0.46 * cos(2.0 * M_PI * i / (double)N);
	}

	// Applies hamming window transformation to i'th element of m_fftInputArray
	void hammingWindow(double& sample, int i)
	{
		sample *= m_hannWindow[i];
	}

	void computeFFT()
	{
		unsigned int k = N, n;
		double thetaT = 3.14159265358979323846264338328L / N;
		Complex phiT{ cos(thetaT), -sin(thetaT) }, T{};

		while (k > 1)
		{
			n = k;
			k >>= 1;
			phiT = { phiT.real * phiT.real - phiT.imag * phiT.imag, 2.0 * phiT.real * phiT.imag };
			T.real = 1.0L;
			for (unsigned int l = 0; l < k; l++)
			{
				for (unsigned int a = l; a < N; a += n)
				{
					unsigned int b = a + k;
					Complex t{ m_fftArray[a].real - m_fftArray[b].real, m_fftArray[a].imag - m_fftArray[b].imag };
					m_fftArray[a] = { m_fftArray[a].real + m_fftArray[b].real, m_fftArray[a].imag + m_fftArray[b].imag };
					m_fftArray[b] = { t.real * T.real - t.imag * T.imag, t.real * T.imag + T.real * t.imag };
				}
				T = { T.real * phiT.real - T.imag * phiT.imag, T.real * phiT.imag + phiT.real * T.imag };
			}
		}
		// Decimate
		unsigned int m = (unsigned int)log2(N);
		for (unsigned int a = 0; a < N; a++)
		{
			unsigned int b = a;
			// Reverse bits
			b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
			b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
			b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
			b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
			b = ((b >> 16) | (b << 16)) >> (32 - m);
			if (b > a)
			{
				Complex t = m_fftArray[a];
				m_fftArray[a] = m_fftArray[b];
				m_fftArray[b] = t;
			}
		}
	}

	int16_t* computeDecibels(const int16_t* soundData)
	{
		for (int i(0); i < N; ++i)
		{
			m_fftArray[i] = { (double)soundData[i * m_channelNum], 0.0 };
			hammingWindow(m_fftArray[i].real, i);
			m_fftArray[i].real /= 32'768;
		}

		computeFFT();

		for (int i(0); i < N / 2; ++i)
		{
			m_decibels[i] = (int16_t)sqrt(m_fftArray[i].real * m_fftArray[i].real +
				m_fftArray[i].imag * m_fftArray[i].imag);
			m_decibels[i] = int16_t(20.0 * log10(double(m_decibels[i])));
		}

		return m_decibels;
	}

private:
	uint16_t m_channelNum;

	double* m_hannWindow;
	Complex* m_fftArray;
	int16_t* m_decibels;
};
