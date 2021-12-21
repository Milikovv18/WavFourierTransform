// WavFFT.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>

#include "PrintHelper.h"
#include "WavOpener.h"
#include "Player.h"
#include "Square.h"
#include "FFT.h"

std::string toTimeFormat(uint32_t seconds);


int main()
{
	console_helper::init();
	console_helper::showCursor(false);

	const size_t bottomY = console_helper::verticalBottomAlign;
	const uint16_t bassCount(10); // First N columns are considered bass
	const uint16_t squeakCount(10); // Last N columns are considered squeak
	const uint16_t squareBaseSize(10);
	const size_t fftSampleSize(2048);
	const uint16_t fileNameYPos(21);
	const uint16_t songDurationYPos(10);
	const uint16_t fftColumnsCount(115);
	const uint16_t fftColumnsHeight(10);
	const uint16_t songDurationProgressSize(30);
	const double squareMaxBassScaling(5.0);
	const double fftDecibelCompression(0.2);
	const double SPF(0.01); // Seconds per frame
	const double squeakOverBassFactor(0.01);
	const char* fileName("Faded(mono).wav");

	// All needed classes
	WavOpener wo(fileName);
	Player pl(wo.getData(), wo.getSampleRate(), wo.getChannelCount());
	Square sq(console_helper::centerAlign, short(bottomY - fileNameYPos), squareBaseSize);
	FFT<fftSampleSize> fft(wo.getChannelCount());

	// Lonely const
	const std::string songDuration = toTimeFormat(wo.getDuration());

	// Startup drawing
	console_helper::setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	// FFT
	for (int i(0); i < fftColumnsCount; ++i) {
		console_helper::printCoords("#", i + 2 /* todo hardcoded */, short(bottomY - 1));
	}
	// Some weird signs
	console_helper::printCoords("Lower notes", 10, short(bottomY - 18) /* todo hardcoded */);
	console_helper::printCoords("Higher notes", console_helper::rightAlign - 22 /* todo hardcoded */,
		short(bottomY - 18) /* todo hardcoded */);
	console_helper::setColor(FOREGROUND_BLUE);
	// Animating square
	sq.draw(true);
	console_helper::unsetColor();
	// Song name and duration
	console_helper::centerPrint(wo.getFileName(), short(bottomY - fileNameYPos));
	console_helper::centerPrint(("0:00 [------------------------------] " + songDuration).c_str(),
		short(bottomY - songDurationYPos));

	// Why not
	Sleep(700);

	// Spectre decay levels
	double* currentDB = new double[fftColumnsCount]{}; // Yellow
	double* shadowDB = new double[fftColumnsCount]{};  // Red
	double* decayedDB = new double[fftColumnsCount]{}; // Empty (clearing shadows)
	auto lastTime = std::chrono::high_resolution_clock::now();

	pl.play(); // Let`s go babe!!!
	while (true)
	{
		// Energy levels defining cube rotation
		double bassEnergy(0.0);
		double squeakEnergy(0.0);
		double loudestDecibel(1.0);

		// Get decibels from sine waves (wav data)
		auto db = fft.computeDecibels(&wo.getData()[pl.getCurrentSamplePos()]);

		console_helper::setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		// Printing FFT result
		for (int h(-fftColumnsHeight); h < 0; ++h)
		{
			for (int col(0); col < fftColumnsCount; ++col)
			{
				// Normalizing?
				currentDB[col] = abs(fftDecibelCompression * db[col]);

				if (decayedDB[col] >= -h && shadowDB[col] < -h) {
					console_helper::printCoords(" ", col + 2 /* todo hardcoded */, short(bottomY + h));
				}
				else if (shadowDB[col] >= -h && currentDB[col] < -h) {
					console_helper::setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
					console_helper::printCoords("#", col + 2 /* todo hardcoded */, short(bottomY + h));
					decayedDB[col] = shadowDB[col];
				}
				else if (currentDB[col] >= -h) {
					console_helper::setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
					console_helper::printCoords("#", col + 2 /* todo hardcoded */, short(bottomY + h));
					shadowDB[col] = currentDB[col];
				}

				if (h == -fftColumnsHeight) // Calculating energy only once
				{
					if (col <= bassCount)
						bassEnergy += db[col];
					else if (col >= fftColumnsCount - squeakCount)
						squeakEnergy += db[col];

					if (db[col] > loudestDecibel) // Peak detection
						loudestDecibel = db[col];
				}
			}
		}

		console_helper::setColor(0);
		sq.draw(); // Clearing previous square

		// Averaging
		bassEnergy /= bassCount;
		squeakEnergy /= squeakCount;
		squeakEnergy *= squeakOverBassFactor; // Lets give higher notes a higher chance

		// Normalizing energy levels
		double compressionFactor = 1.0 / loudestDecibel;
		bassEnergy *= compressionFactor;
		squeakEnergy *= compressionFactor;

		// Drawing square
		sq.setSize(squareBaseSize + uint16_t(squareMaxBassScaling * bassEnergy));
		sq.accelerate(bassEnergy - squeakEnergy); // Too much acceleration
		sq.update();

		console_helper::setColor(FOREGROUND_BLUE);
		sq.draw();

		// Drawing song duration
		std::string progress;
		double currentRelativePos = (double)pl.getCurrentSamplePos() / wo.getDataSize();
		progress += toTimeFormat(uint32_t(currentRelativePos * wo.getDuration())) + " [";

		currentRelativePos *= songDurationProgressSize; // Nasty thing, but optimised
		for (int len(0); len < songDurationProgressSize; ++len)
			progress += len <= currentRelativePos ? '+' : '-';
		progress += "] " + songDuration;

		console_helper::unsetColor();
		console_helper::centerPrint(progress.c_str(), short(bottomY - songDurationYPos));

		while (std::chrono::duration_cast<std::chrono::seconds>
			(std::chrono::high_resolution_clock::now() - lastTime).count() < SPF);
	}

	// Ha-ha dont forget
	delete[] currentDB;
	delete[] shadowDB;
	delete[] decayedDB;
}


// Seconds to time-formatted string (0:00)
std::string toTimeFormat(uint32_t seconds)
{
	std::string timeFormat = std::to_string(seconds / 60) + ':';
	if (seconds % 60 < 10)
		timeFormat += '0';
	timeFormat += std::to_string(seconds % 60);
	return timeFormat;
}


// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
