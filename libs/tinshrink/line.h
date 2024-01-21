#ifndef LINE_H
#define LINE_H

#include <vtkIdList.h>

#include <vector>

class Line
{
public:
	bool merge(const Line& line, vtkIdType* oldEnd, vtkIdType* newEnd);

	std::vector<vtkIdType> ids;
};


#endif // LINE_H
