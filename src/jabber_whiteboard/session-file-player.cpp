/**
 * Whiteboard session file playback mechanism
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <glibmm.h>
#include <glibmm/i18n.h>

#include <gtkmm/textbuffer.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"

#include "jabber_whiteboard/session-file.h"
#include "jabber_whiteboard/session-file-player.h"
#include "jabber_whiteboard/session-manager.h"

namespace Inkscape {

namespace Whiteboard {

SessionFilePlayer::SessionFilePlayer(unsigned int bufsz, SessionManager* sm) 
{
	this->_sm = sm;
	this->_delay = 100;
	this->_playing = false;
	this->_curdir = FORWARD;

	this->_current = this->_next = 0;
	this->_sf = NULL;

	if (sm->session_file() != NULL) {
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Session file: %p", sm->session_file());
		this->_sf = sm->session_file();
	} else {
		g_warning(_("Cannot operate on NULL SessionFile."));
	}
}

SessionFilePlayer::~SessionFilePlayer()
{

}

void
SessionFilePlayer::load(SessionFile* sm)
{
	this->stop();
	if (this->_sm != NULL) {
		this->_sf = sm;
		this->_current = this->_next = 0;
		this->_visited.clear();
	}
}

SessionFile*
SessionFilePlayer::unload()
{
	SessionFile* sf = this->_sf;
	this->stop();
	this->_sf = NULL;
	return sf;
}

Glib::ustring const&
SessionFilePlayer::filename()
{
	return this->_sf->filename();
}

Glib::ustring const&
SessionFilePlayer::curmsg()
{
	return this->_curmsg;
}

void
SessionFilePlayer::start()
{
	this->_playback_dispatcher = Glib::signal_timeout().connect(sigc::bind< 0 > (sigc::mem_fun(*(this), &SessionFilePlayer::step), this->_curdir), this->_delay);
	this->_playing = true;
}

void
SessionFilePlayer::stop()
{
	this->_playback_dispatcher.disconnect();
	this->_playing = false;
}

void
SessionFilePlayer::setDelay(unsigned int delay)
{
	this->_delay = delay;
	if (this->_playing) {
		this->stop();
		this->start();
	}
}

void
SessionFilePlayer::setMessageOutputWidget(Glib::RefPtr<Gtk::TextBuffer> const& widgetptr)
{
	this->_outputwidget = widgetptr;
}

bool
SessionFilePlayer::step(unsigned short dir)
{
	switch (dir) {
		case FORWARD: {
			g_log(NULL, G_LOG_LEVEL_DEBUG, "stepping forward");
		//	Glib::ustring buf;
			this->_current = this->_next;
			this->_next = this->_sf->nextMessageFrom(this->_current, this->_curmsg);
		if (this->_next == this->_current) {
				g_log(NULL, G_LOG_LEVEL_DEBUG, "at eof");
				return false;
			} else {
				this->_visited.push_front(std::make_pair< gint64, gint64 >(this->_current, this->_curmsg.bytes()));
				g_log(NULL, G_LOG_LEVEL_DEBUG, "forward: visited=%u _current=%llu _next=%llu", this->_visited.size(), this->_current, this->_next);
				this->_outputMessageToWidget();
				this->_sm->receiveChange(this->_curmsg);
				sp_document_done(SP_DT_DOCUMENT(this->_sm->desktop()));
				this->_curdir = FORWARD;
				return true;
			}
			break;
					  }
		case BACKWARD:
			if (this->_current == 0) {
				g_log(NULL, G_LOG_LEVEL_DEBUG, "at bof");
				return false;
			} else {
				g_log(NULL, G_LOG_LEVEL_DEBUG, "stepping backward");
				this->_visited.pop_front();
				std::pair< gint64, gint64 > last = this->_visited.front();
				this->_current = last.first;
				this->_next = last.first + last.second;
				this->_sf->nextMessageFrom(this->_current, this->_curmsg);
				this->_outputMessageToWidget();
				g_log(NULL, G_LOG_LEVEL_DEBUG, "backward: visited=%u _current=%llu _next=%llu", this->_visited.size(), this->_current, this->_next);
				sp_document_undo(SP_DT_DOCUMENT(this->_sm->desktop()));
				this->_curdir = BACKWARD;
				return true;
			}
			break;
		default:
			return true;
			break;
	}
}

void
SessionFilePlayer::_outputMessageToWidget()
{
	if (this->_outputwidget) {
		this->_outputwidget->set_text(this->_curmsg);
	}
}

}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
