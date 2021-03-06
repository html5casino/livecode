/* Copyright (C) 2003-2013 Runtime Revolution Ltd.

This file is part of LiveCode.

LiveCode is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License v3 as published by the Free
Software Foundation.

LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

#include "lnxprefix.h"

#include "globdefs.h"
#include "filedefs.h" 
#include "objdefs.h"
#include "parsedef.h" 

#include "util.h"

#include "transfer.h"
#include "execpt.h" 
#include "dispatch.h"
#include "image.h"
#include "globals.h"

#include "lnxdc.h"
#include "lnxtransfer.h"

Atom XdndAware;
Atom XdndSelection;
Atom XdndEnter;
Atom XdndLeave;
Atom XdndPosition;
Atom XdndDrop;
Atom XdndFinished;
Atom XdndStatus;
Atom XdndActionCopy;
Atom XdndActionMove;
Atom XdndActionLink;
Atom XdndActionAsk;
Atom XdndActionPrivate;
Atom XdndTypeList;
Atom XdndActionList;
Atom XdndActionDescription;

Atom XdndMyAtom ;
Atom XdndRevolutionWindow ;

Atom XA_TARGETS ;


Atom WMname ;
Atom text ;
Atom html ;
Atom rtf ;

 
Time LastEventTime ;
Time LastPositionTime ;


char * xdnd_get_window_title ( Window w ) ;

static XdndMimeTable XTransfer_lookup_table[XDND_TYPE_TABLE_SIZE] = 
	{ 	
		{ "text/uri-list", 	TRANSFER_TYPE_FILES, 		5, ConvertFile_MIME_to_rev, ConvertFile_rev_to_MIME } ,
		{ "image/gif",	 	TRANSFER_TYPE_IMAGE, 		5, NULL, NULL },
		{ "image/jpeg",	 	TRANSFER_TYPE_IMAGE, 		5, NULL, NULL },
		{ "image/png",	 	TRANSFER_TYPE_IMAGE, 		5, NULL, NULL },
		{ REV_MIME_STR, 	TRANSFER_TYPE_OBJECTS, 		5, NULL, NULL },
		{ REV_TEXT_STR,		TRANSFER_TYPE_STYLED_TEXT,	4, NULL, NULL },
		{ "text/richtext", 	TRANSFER_TYPE_STYLED_TEXT,	2, ConvertStyled_RTF_to_rev, ConvertStyled_rev_to_RTF },
		{ "STRING",		 	TRANSFER_TYPE_TEXT,	 		3, NULL, NULL }, 
		{ "COMPOUND_TEXT", 	TRANSFER_TYPE_TEXT,	 		3, NULL, NULL }, 
		{ "UTF8_STRING", 	TRANSFER_TYPE_UNICODE_TEXT,	3, ConvertUnicodeToText, ConvertTextToUnicode },
		{ "STRING",		 	TRANSFER_TYPE_STYLED_TEXT, 	2, ConvertStyled_Text_to_rev, ConvertStyled_rev_to_TEXT  }, 	
		{ "COMPOUND_TEXT",	TRANSFER_TYPE_STYLED_TEXT, 	2, ConvertStyled_Text_to_rev, ConvertStyled_rev_to_TEXT  }, 	
		{ "UTF8_STRING",	TRANSFER_TYPE_STYLED_TEXT, 	2, ConvertUnicodeToStyled, ConvertStyledToUnicode }, 	
		{ "text/plain", 	TRANSFER_TYPE_TEXT, 		1, NULL, NULL }
	} ; 

////////////////////////////////////////////////////////////////////////////////
//
// MCMIMEtype implimentation


MCMIMEtype::MCMIMEtype( Display * p_display, Atom p_atom )
{
	m_display = p_display ;
	m_atom = p_atom ;
	m_atom_name = NULL ;
}


MCMIMEtype::MCMIMEtype ( Display * p_display, char * p_atom )
{
	m_display = p_display ;
	m_atom = XInternAtom ( m_display, p_atom, false );
	m_atom_name = NULL ;
}

MCMIMEtype::~MCMIMEtype(void)
{
	if ( m_atom_name != NULL)
		XFree(m_atom_name);
}


char * MCMIMEtype::asString(void)
{
	if ( m_atom_name == NULL)
		m_atom_name = XGetAtomName ( m_display, m_atom );
	return (m_atom_name);
}

MCTransferType MCMIMEtype::asRev ( void ) 
{
	for ( uint4 a = 0 ; a < XDND_TYPE_TABLE_SIZE ; a++)
		if ( XInternAtom(m_display, XTransfer_lookup_table[a] . mime_type, false) == m_atom)
			return XTransfer_lookup_table[a] . rev_type ;
	return TRANSFER_TYPE_NULL ;
}

 

/*=========================================================================================

					T R A N S F E R S T O R E     O B J E C T

=========================================================================================*/



MCXTransferStore::MCXTransferStore ( Display *p_display ) 
{
	m_entries = NULL ;
	m_entry_count = 0 ;
	m_display = p_display ;
	m_internal_dnd = false ; 
	m_types_list = NULL ;
	m_types_count = 0 ;
}


MCXTransferStore::~MCXTransferStore(void)
{
	cleartypes();
}

void	MCXTransferStore::cleartypes ( void ) 
{

	if ( m_entries != NULL )
	{
		for ( uint4 a=0; a<m_entry_count; a++)
		{
			if (m_entries[a].data != NULL)
				m_entries[a].data->Release();
			//if ( m_entries[a].format != NULL)
				delete m_entries[a].format ;
		}
	}

	free(m_entries ) ;	
	m_entries = NULL ;
	m_entry_count = 0 ;
	m_internal_dnd = false ;
	
	if ( m_types_list != NULL)
	{
		free(m_types_list);
		m_types_list = NULL;
		m_types_count = 0 ;
	}
}




///////////////////////////////////////////////////////////////////////////////////////////
//
// Functions to add data into the TransferStore


// Adds an explicit MIME type to the type list
void MCXTransferStore::addType ( MCMIMEtype * p_mime, MCTransferType p_type, MCSharedString * p_data  ) 
{
	MCTransferType t_type ;
	
	if ( p_mime == NULL ) 
		return ;
	
	t_type = ( p_type == TRANSFER_TYPE_NULL ? p_mime->asRev() : p_type ) ;
	
	if ( t_type != TRANSFER_TYPE_NULL ) 
	{
		
		m_entry_count ++ ;
		
		if ( m_entries == NULL ) 
			m_entries = (Entry*)malloc(sizeof(Entry)) ;
		else
			m_entries = (Entry*)realloc(m_entries, (sizeof(Entry) * m_entry_count ) ) ;
		
		m_entries[m_entry_count-1] . format = p_mime ;
		m_entries[m_entry_count-1] . type = t_type ;
		m_entries[m_entry_count - 1 ] . data = p_data ;
		
		// Need to retain the string as it can be used multiple times.
		if ( p_data != NULL)
			p_data -> Retain() ;
	}
}



void MCXTransferStore::addAtom ( Atom p_atom )
{
	MCMIMEtype * t_mime ;
	t_mime = new MCMIMEtype ( m_display, p_atom ) ;
	addType ( t_mime, TRANSFER_TYPE_NULL, NULL ) ;
}




// Adds a Revolution DnD type -- the MIME types are then derived from this base type
void 	MCXTransferStore::addRevType ( MCTransferType p_type, MCSharedString * p_data  )
{
		
	switch ( p_type )
	{
		case TRANSFER_TYPE_TEXT: 
			addType ( new MCMIMEtype(m_display, "text/plain"), p_type, p_data );
			addType ( new MCMIMEtype( m_display, "STRING"), TRANSFER_TYPE_TEXT, p_data ) ;
			addType ( new MCMIMEtype( m_display, "COMPOUND_TEXT"), TRANSFER_TYPE_TEXT, p_data ) ;
		break ; 
		 
		case TRANSFER_TYPE_STYLED_TEXT:
			addType ( new MCMIMEtype( m_display, "text/richtext"), TRANSFER_TYPE_STYLED_TEXT, p_data ) ;
			addType ( new MCMIMEtype( m_display, "STRING"), TRANSFER_TYPE_STYLED_TEXT, p_data ) ;
			addType ( new MCMIMEtype( m_display, "COMPOUND_TEXT"), TRANSFER_TYPE_STYLED_TEXT, p_data ) ;
			addType ( new MCMIMEtype( m_display, "UTF8_STRING"), TRANSFER_TYPE_STYLED_TEXT, p_data );
		break ; 
			
		case TRANSFER_TYPE_IMAGE:
			if ( MCFormatImageIsPNG ( p_data ) ) 
				addType ( new MCMIMEtype( m_display, "image/png"), p_type, p_data );
			if ( MCFormatImageIsJPEG ( p_data ) )
				addType ( new MCMIMEtype( m_display, "image/jpeg"), p_type, p_data );
			if ( MCFormatImageIsGIF ( p_data ) ) 
				addType ( new MCMIMEtype( m_display, "image/gif"), p_type, p_data );
		break ;
			
		case TRANSFER_TYPE_FILES:
			addType(new MCMIMEtype( m_display, "text/uri-list"), p_type, p_data );
		break;
		
		case TRANSFER_TYPE_OBJECTS:
			addType(new MCMIMEtype( m_display, REV_MIME_STR), p_type, p_data );
		break;
	} 
	
}




///////////////////////////////////////////////////////////////////////////////////////////
//
// X11 Specific functions

// Applies the whole typelist to a specified window
void	MCXTransferStore::apply_to_window ( Window w ) 
{
	Atom * typelist ;
	
    if ( m_entries != NULL ) 
	{
		typelist = (Atom*)malloc(sizeof(Atom) * m_entry_count ) ;
		for (uint4 a = 0 ; a < m_entry_count; a++)
			typelist[a] = m_entries[a] . format->asAtom() ;
		XChangeProperty (m_display, w, XdndTypeList, XA_ATOM, 32, PropModeReplace, (unsigned char *) typelist, m_entry_count);
		free ( typelist ) ;
	}
	
}


// Applies the first 3 types in the typelist to the xevent
void	MCXTransferStore::apply_to_message ( XEvent *xevent ) 
{
	if ( xevent->type == ClientMessage  && m_entries != NULL )
	{
		if ( xevent->xclient.message_type == XdndEnter )
			for ( uint4 a = 0 ; a < ( m_entry_count < 3 ? m_entry_count : 3 ) ; a ++ )
				XDND_ENTER_TYPE( xevent, a ) = m_entries[a] . format->asAtom();
	}
	
}


char * buffer_append ( char * p_old, uint4 p_old_size, char * p_new, uint4 p_new_size )
{
	char * t_new;
	t_new = (char*)malloc ( p_old_size + p_new_size );
	memcpy(t_new, p_old, p_old_size);
	memcpy(t_new+p_old_size, p_new, p_new_size);
	return t_new ;
}

	

char * MCXTransferStore::GetSelection ( Window w, Atom property, uint4& p_count )
{
    long read;
    int error = 0;
    unsigned long remaining;
	unsigned long count;
	char * t_ret = NULL ;

	uint4 t_bytes = 0 ;
	uint4 t_total_bytes = 0 ;
	unsigned long t_count = 0;
	
	read = 0;
	count = 0 ;
    do 
	{
		unsigned char *s = NULL ;
		Atom actual;
		int format;
		if (XGetWindowProperty (m_display, w, property, t_total_bytes / 4, 65536, false,
					AnyPropertyType, &actual, &format,
					&count, &remaining,
					&s) != Success) {
			XFree (s);
			return NULL;
			}
		
		
		if ( s != NULL ) 
		{
			t_bytes = count * ( format / 8 );
			t_count += count ;
			
			t_ret = buffer_append ( t_ret, t_total_bytes, (char*)s, t_bytes );
			t_total_bytes += t_bytes ;

#ifdef DEBUG_DND
  	fprintf(stderr, "GetSelection() : Have got %d items of size %d = %d bytes\n", count, format, t_bytes ) ;
	if ( format == 8 ) 
		fprintf(stderr, "\tString(?) : %s\n", s);
#endif
	
			XFree (s);
		}
    } 
	while (remaining);
	
	p_count = t_count ;
#ifdef DEBUG_DND
	fprintf(stderr, "Returning %d items\n", t_count ) ;
#endif

	
	return (t_ret) ;

}



void MCXTransferStore::GetExternalTypes ( Atom p_public_atom, Atom p_private_atom, Window w)
{
	XEvent xevent ;
	XSetSelectionOwner(m_display, p_private_atom, w, LastEventTime);
	XConvertSelection (m_display, p_public_atom, XA_TARGETS, p_private_atom, w, LastEventTime ); 
	if ( WaitForEventCompletion(xevent) )
	{
		Atom * t_data ;
		uint4 t_count ;
		
		cleartypes();
		t_data = (Atom*)GetSelection ( xevent.xselection.requestor, p_private_atom , t_count) ;		
		
#ifdef DEBUG_DND
		fprintf(stderr, "\t I have retrieved %d types....\n", t_count ) ;
#endif
		for ( uint4 a =0 ; a < t_count ; a++)
		{
#ifdef DEBUG_DND
			fprintf(stderr, "\t%s\n", AtomAsString((Atom)t_data[a]));
#endif
			addAtom( (Atom)t_data[a]);
		}
		
		free(t_data) ;
	}
	
#ifdef DEBUG_DND
	dumpList("GetExternalTypes() -- So, this is what I think is on the clipboard!");
#endif
	
}






bool MCXTransferStore::WaitForEventCompletion(XEvent &p_xevent)
{

	XEvent xevent ;
	bool xdnd_target_done = false ;
	
	Time t_loop_time = LastEventTime ; 
	
	while (!xdnd_target_done )
	{

		XAllowEvents (m_display , SyncPointer, CurrentTime);
		XNextEvent( m_display , &xevent);
	
		
		if ( xevent.type == SelectionNotify ) 
		{
			xdnd_target_done = true ;
			p_xevent = xevent ;
			return true ;
		}
		
		
		
		
		if ( xevent.type == MotionNotify ) 
		{
			if (xevent.xmotion.time > ( t_loop_time + 1000) )
			  return false ;
		}
		
		if ( ( xevent.type == Expose ) || ( xevent.type == GraphicsExpose ) )
		{
			XPutBackEvent(m_display , &xevent);
			MCscreen -> expose();
		}
		
		
	}
}
	



MCSharedString * MCXTransferStore::GetExternalData ( MCTransferType p_type, Atom p_public_atom, Atom p_private_atom, Window p_source_window, Window p_target_window, Time p_lock_time)  
{
	XEvent xevent ;
	MCSharedString *t_ret = NULL ;
	
	// Check that the Source application actually owns the XdndSelection.
	if ( XGetSelectionOwner ( m_display, p_public_atom ) == p_source_window ) 
	{

		// Take ownership of the Xdnd Atom
		XSetSelectionOwner (m_display, p_private_atom, p_target_window, p_lock_time);
		
		// Now try and convert XdndSelection to XdndMyAtom as type "text/plain"
		// NOTE: here we need to seach through out entries and find the MIME type of the first matching
		//       rev type.
		
		MCMIMEtype * p_mime_type ;
		int4   idx ;
		idx = find_entry_with_rev_type ( p_type );
		if ( idx > -1 )
		{
				p_mime_type = m_entries[idx] . format ;
			
			
			XConvertSelection (m_display, p_public_atom, p_mime_type -> asAtom(), p_private_atom , p_target_window, p_lock_time);
			
			// Now we want to wait until we get a SelectionNotify message or we time-out
			// waiting for it.
			if ( WaitForEventCompletion(xevent) )
			{

				char * t_data ;
				uint4 t_count ;
				t_data = GetSelection ( xevent.xselection.requestor, p_private_atom , t_count) ;			
				t_ret = MCSharedString::Create ( t_data, t_count ) ;
				
				MCSharedString *t_tmp ;
				t_tmp = Convert_MIME_to_REV ( t_ret, p_mime_type ); 
				if ( t_tmp != t_ret)
				{
					t_ret->Release();
					t_ret = t_tmp;
				}
			}
		}
	
	}
	
	return ( t_ret ) ;
}



////////////////////////////////////////////////////////////////////////////////
//
// Utility functions for searching tables etc.


char * MCXTransferStore::asString( uint4 p_idx ) 
{
	if ( ( m_entries != NULL ) && ( p_idx <= m_entry_count ) )
		return ( m_entries[p_idx] . format->asString() ) ;	 
	return NULL ;
}


Atom   MCXTransferStore::asAtom  ( uint4 p_idx ) 
{
	if  ( ( m_entries != NULL ) && ( p_idx <= m_entry_count ) ) 
		return m_entries[p_idx] . format->asAtom() ;
	return 0 ;
	
}



int4 	MCXTransferStore::find_type ( MCMIMEtype * p_type )
{
	for ( uint4 a = 0 ; a < m_entry_count ; a++)
		if ( strcmp ( ( const char*)p_type->asString(), asString( a ) ) == 0 )
			return a ;
	return -1;
}




int4  MCXTransferStore::find_entry_with_rev_type ( MCTransferType p_type ) 
{
	for ( uint4 a = 0 ; a < m_entry_count ; a++)
		if ( m_entries[a] . type == p_type )
			return a;
	
	return -1 ;
}

int4  MCXTransferStore::find_table_entry_with_rev_type ( MCTransferType p_type ) 
{
	for ( uint4 a = 0 ; a < XDND_TYPE_TABLE_SIZE ; a++)
		if ( XTransfer_lookup_table [a] . rev_type == p_type )
			return a;
	
	return -1 ;
}

int4  MCXTransferStore::find_table_entry_with_MIME_type ( MCMIMEtype * p_MIME ) 
{
	for ( uint4 a = 0 ; a < XDND_TYPE_TABLE_SIZE ; a++)
		if ( strcmp ( ( const char*)p_MIME->asString(), (char*)XTransfer_lookup_table [a] . mime_type ) == 0 )

			return a;
	
	return -1 ;
} 

int4  MCXTransferStore::find_table_entry_with_full_types ( MCTransferType p_type, MCMIMEtype * p_MIME ) 
{
	for ( uint4 a = 0 ; a < XDND_TYPE_TABLE_SIZE ; a++)
		if (( strcmp ( p_MIME->asString(), (char*)XTransfer_lookup_table [a] . mime_type ) == 0 ) &&
		   ( p_type == XTransfer_lookup_table[a] . rev_type ))
			return a;
	
	return -1 ;
} 


///////////////////////////////////////////////////////////////////////////////
//
// Functions that convert between the REV and System type descriptors


MCMIMEtype * MCXTransferStore::rev_to_MIME_stored ( MCTransferType p_type )
{
	int4 idx ;
	idx = find_entry_with_rev_type ( p_type ) ;
	if ( idx > -1 )
		return ( m_entries[idx].format ) ;
	else 
		return None ;
}



/////////////////////////////////////////////////////////////////////////////////
//
// Interface with the Clipboard


Atom * MCXTransferStore::QueryAtoms (uint4 & r_count)
{
	
#ifdef DEBUG_DND
	dumpList("QueryAtoms");
#endif
	
	Atom * t_ret = NULL;
	uint4 t_top = 0 ; 
	uint4 idx ;
	uint4 t_count = 0 ;
	
	if ( m_entries != NULL ) 
	{
		// First we need to find the highest priority type we hold. 
		for ( uint4 a = 0 ; a < m_entry_count; a++)
		{
			idx = find_table_entry_with_full_types ( m_entries[a] . type, m_entries[a] . format ) ;
			if ( XTransfer_lookup_table[idx] . priority > t_top )
				t_top = XTransfer_lookup_table[idx] . priority ;
		}
 
		t_ret = (Atom*)malloc(sizeof(Atom) * 3);
		t_ret[0] = XInternAtom(m_display,"TIMESTAMP", false);	// We MUST respond to TIMESTAMP requests
		t_ret[1] = XInternAtom(m_display,"TARGETS", false);		// We MUST respond to TARGETS requests. 
		t_ret[2] = XInternAtom(m_display,"SAVE_TARGETS", false);		// We MUST respond to TARGETS requests.
		
		t_count +=3 ;
		// Now will will return all data types we hold with that highest priority
		for ( uint4 a = 0 ; a < m_entry_count; a++)
		{
			idx = find_table_entry_with_full_types ( m_entries[a] . type, m_entries[a] . format ) ;
#ifdef DEBUG_DND
			fprintf(stderr, "TopPri = %d, IDX = %d, MIME = %s, REV = %s\n", t_top, idx, m_entries[a].format->asString(), m_entries[a].format->asRevString());
#endif 
			{
				t_count ++ ;
				t_ret = (Atom*)realloc(t_ret, (sizeof(Atom) * t_count ) ) ;
				t_ret[t_count - 1 ] = m_entries[a] . format->asAtom() ;
			}
		}

		r_count = t_count ;
	}
	return t_ret ;
}

	

////////////////////////////////////////////////////////////////////////////////
//
// Interface with MCXPasteboard


bool in_list ( MCTransferType * p_list, uint4 p_count, MCTransferType p_type)
{
	for (uint4 a = 0 ; a < p_count ; a++)
		if ( p_list[a] == p_type)
			return true ;
	return false ;
}


bool should_include ( MCTransferType * p_list, uint4 p_count, MCTransferType p_type)
{
	
	if ( (( p_type == TRANSFER_TYPE_TEXT ) || ( p_type == TRANSFER_TYPE_UNICODE_TEXT )) && in_list(p_list, p_count, TRANSFER_TYPE_STYLED_TEXT ))
		return false ;
	
	if (( p_type == TRANSFER_TYPE_TEXT ) && in_list(p_list, p_count, TRANSFER_TYPE_UNICODE_TEXT ))
		return false ;
	
	return ( !in_list ( p_list, p_count, p_type));	
}



bool MCXTransferStore::Query(MCTransferType*& r_types, unsigned int& r_type_count) 
{

	
	MCTransferType * t_list = NULL;
	uint4 t_top = 0 ; 
	uint4 idx ;
	uint4 t_count = 0 ; 
	
	if ( m_entries != NULL ) 
	{
		if ( m_types_list == NULL)
		{
				 
			// First we need to find the highest priority type we hold.
			for ( uint4 a = 0 ; a < m_entry_count; a++)
			{
				idx = find_table_entry_with_full_types ( m_entries[a].type, m_entries[a] . format );
				if ( XTransfer_lookup_table[idx] . priority > t_top )
					t_top = XTransfer_lookup_table[idx] . priority ; 
			}

			// Now will will return all data types we hold with that highest priority
			for ( uint4 a = 0 ; a < m_entry_count; a++)
			{
				idx = find_table_entry_with_full_types ( m_entries[a].type, m_entries[a] . format );
				if (( XTransfer_lookup_table[idx] . priority == t_top ) && should_include( t_list, t_count, m_entries[a].type) )
				{
					t_count ++ ;
					if ( t_list == NULL )
						t_list = (MCTransferType*)malloc(sizeof(MCTransferType)) ;
					else 
						t_list = (MCTransferType*)realloc(t_list, sizeof(MCTransferType) * t_count  );
					
					t_list[t_count - 1 ] = m_entries[a] . type ;
				}
			}

			r_type_count = t_count ;
			m_types_count = t_count ;
			r_types = t_list ;
			m_types_list = t_list ;
		}
		else 
		{
			r_types = m_types_list ;
			r_type_count = m_types_count ;
		}

#ifdef DEBUG_DND
		fprintf(stderr, "\tDump\t\n----\n");
		for ( uint4 a = 0 ; a < r_type_count ; a ++)
			fprintf(stderr, "\t%s\n", RevTypeToString(m_types_list[a]));
#endif
		return true ;
	}
	return false ;
}




bool MCXTransferStore::Fetch(MCMIMEtype * p_mime_type, MCSharedString*& r_data, Atom p_public_atom, Atom p_private_atom, Window p_source_window, Window p_target_window, Time p_lock_time) 
{
#ifdef DEBUG_DND
	fprintf(stderr, "Fetch_MIME() : Type = %s\n", p_mime_type -> asString() ) ;
#endif 
	if ( m_entries != NULL )
	{
		int4 t_idx = find_type ( p_mime_type ) ;
		if ( t_idx > -1 )
		{
			if ( m_entries[t_idx] . data == NULL )	// We SHOULD only need to get the data if we are the TARGET
			{
				m_entries[t_idx] . data = GetExternalData( m_entries[t_idx] . type , p_public_atom, p_private_atom, p_source_window, p_target_window, p_lock_time) ;
				r_data = m_entries[t_idx] . data ;
			}
			else 
			{
				if (!m_internal_dnd) 
					r_data = Convert_REV_to_MIME ( m_entries[ t_idx ] . data , m_entries[t_idx] . type,  m_entries [t_idx] . format ) ;
				else
					r_data = m_entries[t_idx] . data ;
			}

	
			if ( r_data == NULL )		// This may happen if the selection owner has changed.
				return false ;
		
			return true ;
		}
	}
	return false ;
}



bool MCXTransferStore::Fetch(MCTransferType p_type, MCSharedString*& r_data, Atom p_public_atom, Atom p_private_atom, Window p_source_window, Window p_target_window, Time p_lock_time) 
{
#ifdef DEBUG_DND
	fprintf(stderr, "Fecth() : Asked for REV type %s\n", RevTypeToString (p_type));
#endif
	return ( Fetch ( rev_to_MIME_stored ( p_type ), r_data, p_public_atom, p_private_atom, p_source_window, p_target_window, p_lock_time ) ) ;
}



////////////////////////////////////////////////////////////////////////////////
//
// Utility and Misc functions


void MCXTransferStore::dumpClipboard(const char * p_msg)
{
Window w ;
	fprintf(stderr, "------- CLIPBOARD [ %s ] ----------\n", p_msg );
	w = XGetSelectionOwner(m_display, MCclipboardatom);
	fprintf(stderr, "Clipboard owner is : %x\n", w );
	xdnd_get_window_title ( w ) ;
	w = ((MCScreenDC*)MCscreen) -> GetNullWindow () ;
	fprintf(stderr, "Types on the clipboard : \n");

	XEvent xevent ;
	XConvertSelection (m_display, MCclipboardatom, XA_TARGETS, XdndMyAtom, w, LastEventTime ); 
	if ( WaitForEventCompletion(xevent) )
	{
		Atom * t_data ;
		uint4 t_count ;

		cleartypes();
		t_data = (Atom*)GetSelection ( xevent.xselection.requestor, XdndMyAtom , t_count) ;			
		for ( uint4 a =0 ; a < t_count ; a++)
			fprintf(stderr, "\t%s\n", AtomAsString((Atom)t_data[a]));
	}

	
	
	
}




void MCXTransferStore::dumpList ( char * p_msg )
{
	Entry t_entry ;
	fprintf(stderr, "MCXdndTypeList Dump -[ %s ]- \n", p_msg);
	for ( uint4 a = 0; a < m_entry_count ; a++)
	{
		t_entry = m_entries[a] ;
	  	fprintf(stderr, "\t RevType = %s \t Atom ID = %d \t Atom name = %s \t Data pointer = %x\n", RevTypeToString ( t_entry . type ) ,  t_entry . format, asString ( a ), t_entry . data ) ;
	}
}




const char *RevTypeToString(MCTransferType p_type)
{
	switch(p_type)
	{
	case TRANSFER_TYPE_TEXT:
		return "TRANSFER_TYPE_TEXT";
	case TRANSFER_TYPE_UNICODE_TEXT:
		return "TRANSFER_TYPE_UNICODE_TEXT";
	case TRANSFER_TYPE_STYLED_TEXT:
		return "TRANSFER_TYPE_STYLED_TEXT";
	case TRANSFER_TYPE_RTF_TEXT:
		return "TRANSFER_TYPE_RTF_TEXT";
	case TRANSFER_TYPE_HTML_TEXT:
		return "TRANSFER_TYPE_HTML_TEXT";
	case TRANSFER_TYPE_IMAGE:
		return "TRANSFER_TYPE_IMAGE";
	case TRANSFER_TYPE_FILES:
		return "TRANSFER_TYPE_FILES";
	case TRANSFER_TYPE_PRIVATE:
		return "TRANSFER_TYPE_PRIVATE";
	case TRANSFER_TYPE_OBJECTS:
		return "TRANSFER_TYPE_OBJECTS";
	}

	return "";
}




////////////////////////////////////////////////////////////////////////////////
//
// Format conversion routines


MCSharedString * MCXTransferStore::Convert_REV_to_MIME ( MCSharedString * p_in, MCTransferType p_type,  MCMIMEtype * p_mime ) 
{
#ifdef DEBUG_DND
	fprintf(stderr, "--> Converting type [%s] into [%s]\n",  RevTypeToString(p_type), p_mime->asString() ) ;
#endif
		
	int4 t_idx ;
	t_idx = find_table_entry_with_full_types ( p_type , p_mime ) ;
	if ( t_idx > -1 )
		if ( XTransfer_lookup_table [ t_idx ] . f_converter_to_mime != NULL )
		{
#ifdef DEBUG_DND
			fprintf(stderr, "\tHave got converter function.\n");
#endif
			return ( XTransfer_lookup_table [ t_idx ] . f_converter_to_mime ( p_in, p_mime->asRev() ) ) ;
		}
		else
		{
#ifdef DEBUG_DND
			fprintf(stderr, "\tno conversion function defined... doing nothing\n");
#endif
			return ( p_in ) ;
		}
	
	return NULL ;
}



MCSharedString * MCXTransferStore::Convert_MIME_to_REV ( MCSharedString * p_in, MCMIMEtype * p_MIME ) 
{
#ifdef DEBUG_DND
	fprintf(stderr, "<-- Converting type [%s] into [%s]\n", p_MIME->asString(), p_MIME->asRevString() ) ;
#endif

	int4 t_idx ;
	t_idx = find_table_entry_with_MIME_type ( p_MIME ) ;
	if ( t_idx > -1 )
		if ( XTransfer_lookup_table [ t_idx ] . f_converter_to_rev != NULL )
		{
#ifdef DEBUG_DND
			fprintf(stderr, "\tHave got converter function.\n");
#endif
			return ( XTransfer_lookup_table [ t_idx ] . f_converter_to_rev ( p_in, p_MIME ) ) ;
		}
		else 
		{
			return ( p_in ) ;
#ifdef DEBUG_DND
			fprintf(stderr, "\tno conversion function defined... doing nothing\n");
#endif
		}
	
	
	return NULL ; 
	
}

// Convert from UTF8 to TRANSFER_TYPE_STYLED_TEXT
MCSharedString * ConvertUnicodeToStyled ( MCSharedString * p_in, MCMIMEtype * p_MIME ) 
{
	MCExecPoint ep ( NULL, NULL, NULL) ;
	ep.setsvalue(p_in -> Get());
	ep.utf8toutf16();
	return MCConvertUnicodeToStyledText(ep . getsvalue());
}


// Convert from TRANSFER_TYPE_STYLED_TEXT to UTF8
MCSharedString *ConvertStyledToUnicode ( MCSharedString * p_in, MCTransferType p_type ) 
{
	MCExecPoint ep ( NULL, NULL, NULL ) ;
	
	// Convert from Styles to UTF16
	MCSharedString *t_unicode_text;
	t_unicode_text = MCConvertStyledTextToUnicode(p_in);
	
	ep . setsvalue (  t_unicode_text -> Get() ) ;
	ep . utf16toutf8() ;
	
	t_unicode_text -> Release();
	
	return ( MCSharedString::Create( ep.getsvalue() ) ) ;
}



// Convert from UTF8 to TRANSFER_TYPE_TEXT
MCSharedString * ConvertUnicodeToText ( MCSharedString * p_in, MCMIMEtype * p_MIME ) 
{
	MCExecPoint ep ( NULL, NULL, NULL ) ;
	
	ep . setsvalue(p_in -> Get());
	ep . utf8toutf16();
	
	return MCSharedString::Create(ep . getsvalue());
}
	
// Convert from TRANSFER_TYPE_TEXT to UTF8
MCSharedString * ConvertTextToUnicode ( MCSharedString * p_in, MCTransferType p_type ) 
{
	MCExecPoint ep ( NULL, NULL, NULL ) ;
	
	ep . setsvalue(p_in -> Get());
	ep . utf16toutf8();

	return MCSharedString::Create(ep . getsvalue());
}
	
MCSharedString * ConvertStyled_rev_to_HTML ( MCSharedString * p_in, MCTransferType p_type ) 
{
	return ( MCConvertStyledTextToHTML ( p_in )) ;
}


MCSharedString * ConvertStyled_rev_to_RTF ( MCSharedString * p_in, MCTransferType p_type ) 
{
	return ( MCConvertStyledTextToRTF ( p_in ) ) ;
}


MCSharedString * ConvertStyled_RTF_to_rev ( MCSharedString * p_in, MCMIMEtype * p_MIME ) 
{
	return ( MCConvertRTFToStyledText ( p_in ) ); 
}


MCSharedString * ConvertStyled_HTML_to_rev ( MCSharedString * p_in, MCMIMEtype * p_MIME ) 
{
	return ( MCConvertHTMLToStyledText ( p_in ) ); 
}


MCSharedString * ConvertStyled_rev_to_TEXT ( MCSharedString * p_in, MCTransferType p_type ) 
{
	return ( MCConvertStyledTextToText ( p_in )) ;
}

MCSharedString * ConvertStyled_Text_to_rev ( MCSharedString * p_in, MCMIMEtype * p_MIME ) 
{
	p_in -> Retain();
	return ( p_in) ;
}







MCSharedString * ConvertFile_rev_to_MIME ( MCSharedString * p_in, MCTransferType p_type ) 
{
	MCString *t_strings = NULL ;
	MCExecPoint ep(NULL, NULL, NULL );
	uint2 	 n_strings ;

	MCU_break_string( p_in -> Get() , t_strings, n_strings, false);
	
	for (uint4 a=0; a < n_strings; a++)
	{
		ep.concatcstring("file://", EC_RETURN, (a==0) );
		ep.concatmcstring(t_strings[a], EC_NONE, false ); 
	}
	
	delete t_strings;

	return ( MCSharedString::Create(ep . getsvalue()) ) ;
}



 
 



MCSharedString * ConvertFile_MIME_to_rev ( MCSharedString * p_in, MCMIMEtype * p_MIME )  
{
	MCString *t_strings = NULL ;
	MCExecPoint ep(NULL, NULL, NULL );
	uint2 	 n_strings ;
	char * t_part ;
	bool first = true ;
	
	ep.setsvalue(p_in -> Get());
	ep.texttobinary() ;
	MCU_urldecode(ep);
	MCU_break_string( ep.getsvalue(), t_strings, n_strings, false);
	
	ep.clear() ;
	
	for (uint4 a=0; a < n_strings; a++)
	{
		t_part = (char*)t_strings[a].clone();
		if ( strstr(t_part, "file://") != NULL)
		{
			ep.concatcstring((t_part + 7), EC_RETURN, first );
			first=false;
		}
		delete t_part;
	}
	delete t_strings ;
	
	return ( MCSharedString::Create(ep . getsvalue()) ) ;
}


