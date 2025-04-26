#ifndef QT_BLL_PROXY_GLOBAL_H
#define QT_BLL_PROXY_GLOBAL_H

#include <QtCore/qglobal.h> 

#if defined(QT_BLL_PROXY_LIBRARY)
#  define QT_BLL_PROXY_EXPORT Q_DECL_EXPORT
#else
#  define QT_BLL_PROXY_EXPORT Q_DECL_IMPORT
#endif

#endif // QT_BLL_PROXY_GLOBAL_H
