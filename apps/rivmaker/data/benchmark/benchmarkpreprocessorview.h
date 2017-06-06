#ifndef BENCHMARKPREPROCESSORVIEW_H
#define BENCHMARKPREPROCESSORVIEW_H

#include "../points/pointspreprocessorview.h"

class Benchmark;

class BenchmarkPreProcessorView : public PointsPreProcessorView
{
public:
	BenchmarkPreProcessorView(Model* model, Benchmark* item);
	~BenchmarkPreProcessorView();
};

#endif // BENCHMARKPREPROCESSORVIEW_H
