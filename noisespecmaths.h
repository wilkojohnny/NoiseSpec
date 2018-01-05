#ifndef NOISESPECMATHS_H
#define NOISESPECMATHS_H

/* NoiseSpecMaths:
 * Reusable functions for maths operations.
 */

#include <cmath>
#include <QList>

int round2(int number); // round up to nearest power of 2

void four1(float * data, unsigned int nn, int isign); // FFT from numerical recipes

void Sort(QList<int> & Data); // Sorting routine (bubble sort)

#endif // NOISESPECMATHS_H
