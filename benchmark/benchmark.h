/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef BENCHMARK_H
#define	BENCHMARK_H

#ifdef _MIOSIX

#include <vector>
#include "mxgui/display.h"
#include "miosix.h"


/**
 * The result of a benchmark
 */
class BenchmarkResult
{
public:
    /**
     * \param name name of benchmark
     * \param time time (in microseconds) taken to do the benchmark
     * \param fps number of fps resulting
     */
    BenchmarkResult(const char str[25], unsigned int time, double fps);

    /**
     * \return the benchmark name
     */
    const char *getName() const;

    /**
     * \return the benchmark time, in microseconds
     */
    unsigned int getTime() const;

    /**
     * \return the benchmark resulting fps
     */
    unsigned int getFps() const;

    /**
     * Print the result on the display d, at point p
     */
    void print(mxgui::Display& d, mxgui::Point p);

private:
    char name[25];
    unsigned int time;
    double fps;
};

/**
 * Benchmark code is here. Benchmark is designed for a 240x320 screen,
 * orientation vertical
 */
class Benchmark
{
public:
    /**
     * \param display the display that will be benchmarked
     */
    Benchmark(mxgui::Display& display);

    /**
     * Starts the benchmark.
     * At the end result are directly printed on screen, but it is possible
     * to get them with getResults()
     */
    void start();

private:
    void fixedWidthTextBenchmark();

    void variableWidthTextBenchmark();

    void antialiasingBenchmark();

    void horizontalLineBenchmark();

    void verticalLineBenchmark();

    void obliqueLineBenchmark();

    void clearScreenBenchmark();
	
    void imageBenchmark();

    void scanLineBenchmark();

    void clippedDrawBenchmark();

    void clippedWriteBenchmark();

    mxgui::Display& display;
    std::vector<BenchmarkResult> results;
    miosix::Timer timer;
};

#endif //_MIOSIX

#endif	/* BENCHMARK_H */