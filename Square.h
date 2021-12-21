#pragma once

#include "PrintHelper.h"


class Square
{
	struct Vertex {
		double x;
		double y;
	};

public:
	Square(uint16_t x, uint16_t y, uint16_t size, double angle = 0) :
		m_center{ (double)x, (double)y },
		m_baseVerts{{ -1.0, -1.0 },
					{ -1.0, +1.0 },
					{ +1.0, +1.0 },
					{ +1.0, -1.0 }}
	{
		setSize(size);
		setAngle(angle);

		m_velocity = 0.0;
		m_acceleration = 0.0;
	}

	bool setSize(uint16_t newSize)
	{
		// Can be resized?
		if (newSize <= 0 || newSize == m_size)
			return false;

		m_size = newSize;
		return true;
	}

	void setAngle(double newAngle)
	{
		m_angle = newAngle;
	}

	void setCenter(uint16_t x, uint16_t y)
	{
		m_center.x = x;
		m_center.y = y;
	}

	void accelerate(double acc)
	{
		m_acceleration = acc;
	}

	void update()
	{
		// Updating derivative
		m_velocity += m_acceleration * dTime;
		m_angle += m_velocity * dTime;
	}

	// Extremely fast line drawing algorithm (with animation)
	void drawLine(const Vertex& a, const Vertex& b, bool anime) const
	{
		bool yLonger = false;
		int incrementVal;
		int shortLen = int(b.y - a.y);
		int longLen = int(b.x - a.x);

		if (abs(shortLen) > abs(longLen)) {
			int swap = shortLen;
			shortLen = longLen;
			longLen = swap;
			yLonger = true;
		}

		if (longLen < 0) incrementVal = -1;
		else incrementVal = 1;

		double multDiff;
		if (longLen == 0.0) multDiff = (double)shortLen;
		else multDiff = (double)shortLen / (double)longLen;
		if (yLonger) {
			for (int i = 0; i != longLen; i += incrementVal) {
				console_helper::printCoords("@", short(a.x + (double)i * multDiff), short(a.y + i));
				if (anime) Sleep(10);
			}
		}
		else {
			for (int i = 0; i != longLen; i += incrementVal) {
				console_helper::printCoords("@", short(a.x + i), short(a.y + (double)i * multDiff));
				if (anime) Sleep(10);
			}
		}
	}

	void draw(bool smoothAnime = false)
	{
		Vertex toDraw[4];
		memcpy(toDraw, m_baseVerts, 4 * sizeof(Vertex));

		// ATTENTION! Transformation order: scaling -> rotating -> translating
		for (int i(0); i < 4; ++i)
		{
			resize(toDraw[i]);
			rotate(toDraw[i]);
			toDraw[i].y *= 0.5; // Bc console stretching symbol height X2
			translate(toDraw[i]);
		}

		drawLine(toDraw[0], toDraw[1], smoothAnime);
		drawLine(toDraw[1], toDraw[2], smoothAnime);
		drawLine(toDraw[2], toDraw[3], smoothAnime);
		drawLine(toDraw[3], toDraw[0], smoothAnime);
	}

	inline static const double dTime = 0.05; // Every draw time increases
private:
	int64_t m_size;
	double m_angle;
	Vertex m_center;

	double m_velocity;
	double m_acceleration;

	const Vertex m_baseVerts[4];


	void resize(Vertex& vert)
	{
		vert.x *= m_size;
		vert.y *= m_size;
	}

	void rotate(Vertex& vert)
	{
		double tempX = vert.x;
		vert.x = cos(m_angle) * vert.x - sin(m_angle) * vert.y;
		vert.y = sin(m_angle) * tempX + cos(m_angle) * vert.y;
	}

	void translate(Vertex& vert)
	{
		vert.x += m_center.x;
		vert.y += m_center.y;
	}
};