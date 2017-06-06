#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "../points/points.h"

class Benchmark : public Points
{
	Q_OBJECT

public:
	Benchmark(DataItem* parent);
	~Benchmark();

	QStandardItem* buildPreProcessorStandardItem() const override;
	DataItemView* buildPreProcessorDataItemView(Model*) override;
};

#endif // BENCHMARK_H
