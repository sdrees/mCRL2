// Author(s): Frank Stappers 
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file editor.h

#ifndef MCRL2XI_EDITOR_H_
#define MCRL2XI_EDITOR_H_
#include "xstceditor.h"
#include "outputpanel.h"
#include "wx/textctrl.h"
#include <wx/aui/auibook.h>


class xEditor: public wxAuiNotebook {
public:
	xEditor(wxWindow *parent, wxWindowID id, outputpanel *output );
	void AddEmptyPage();
	bool LoadFile( const wxString &filename );
	bool SaveFile( const wxString &filename );
	wxString GetStringFromDataEditor();
	wxString GetFileInUse();
private:
	outputpanel *p_output;
	xStcEditor *p_data_editor;
};


#endif
