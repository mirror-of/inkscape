#ifndef __SP_KDE_PRIVATE_H__
#define __SP_KDE_PRIVATE_H__

/*
 * KDE utilities for Sodipodi
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <qobject.h>

class SPKDEBridge : public QObject {
	Q_OBJECT
public:
	SPKDEBridge (const char *name) : QObject (NULL, name) { /* NOP */ }

public slots:
	void EventHook (void);
	void TimerHook (void);
};

#endif
