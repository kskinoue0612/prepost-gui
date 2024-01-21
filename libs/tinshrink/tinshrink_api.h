#ifndef TINSHRINK_API_H
#define TINSHRINK_API_H

#include <QtCore/QtGlobal>

#if defined(TINSHRINK_LIBRARY)
#  define TINSHRINK_API Q_DECL_EXPORT
#else
#  define TINSHRINK_API Q_DECL_IMPORT
#endif

#endif // TINSHRINK_API_H
