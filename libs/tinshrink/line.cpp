#include "line.h"

bool Line::merge(const Line& line, vtkIdType* oldEnd, vtkIdType* newEnd)
{
	if (*ids.rbegin() == *line.ids.begin()) {
		*oldEnd = *ids.rbegin();
		*newEnd = *line.ids.rbegin();
		auto it = line.ids.begin() + 1;
		while (it != line.ids.end()) {
			ids.push_back(*it);
			++ it;
		}
		return true;
	} else if (*ids.rbegin() == *line.ids.rbegin()) {
		*oldEnd = *ids.rbegin();
		*newEnd = *line.ids.begin();
		auto it = line.ids.rbegin() + 1;
		while (it != line.ids.rend()) {
			ids.push_back(*it);
			++ it;
		}
		return true;
	} else if (*ids.begin() == *line.ids.begin()) {
		*oldEnd = *ids.begin();
		*newEnd = *line.ids.rbegin();
		std::reverse(ids.begin(), ids.end());
		auto it = line.ids.begin() + 1;
		while (it != line.ids.end()) {
			ids.push_back(*it);
			++ it;
		}
		return true;
	} else if (*ids.begin() == *line.ids.rbegin()) {
		*oldEnd = *ids.begin();
		*newEnd = *line.ids.begin();
		std::reverse(ids.begin(), ids.end());
		auto it = line.ids.rbegin() + 1;
		while (it != line.ids.rend()) {
			ids.push_back(*it);
			++ it;
		}
		return true;
	}

	return false;
}
