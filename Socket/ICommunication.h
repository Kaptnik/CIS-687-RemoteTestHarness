#pragma once

////////////////////////////////////////////////////////////////////////////
// ICommunication.h - An interface class that's inherited by Communication//
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains an interface that exposes the functions of 
* Communication.h through an interface, so that we can get around the
* problem of <mutex> not being supported in the C++/CLI project.
*
* This is largely based on Prof. Fawcett's example code.
*
* Required Files:
* ---------------
* Message.h
*/


#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include "Message.h"

namespace Sockets {
	enum CONTEXT {SERVER, CLIENT, WORKER};

	class ICommunication
	{
	public:
		CONTEXT ProcessContext;
		static ICommunication* CreateInstance(size_t);
		virtual ~ICommunication() {}
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void DeliverMessage(Message) = 0;
		virtual Message CollectMessage() = 0;
	};
}
#endif // !COMMUNICATION_H