/*
 * Keyboard.cpp
 *
 *  Created on: Nov 4, 2015
 *      Author: xdtthng
 */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

#include "Keyboard.h"
#include "AppTrace.h"


using namespace std;

//******************************************************************************
// Description:
//    Constructor.
//******************************************************************************

Keyboard::Keyboard():
		   m_handle(STDIN_FILENO),
		   m_status(Keyboard::OK),
		   m_savedTermios(),
		   m_termSaved(false)
{

	// Check if input is from a tty
	if (!isatty(m_handle))
	{
		//EVENT(EventReporter::SubSystemError,
		//      PROBLEM_DATA("isatty(STDIN_FILNO) says No"),
		//      PROBLEM_TEXT(""));
		m_status = HandleCreationFailed;
	}

}

Keyboard::~Keyboard()
{
	int32_t savedErrno = 0;

	// Set terminal back to normal, if needed
	if(ttyReset(savedErrno) == -1)
	{
		//EVENT(EventReporter::SubSystemError,
		//      PROBLEM_DATA("ttyReset() failed, fd:%d, errno:%d",
		//                   m_handle, savedErrno),
		//      PROBLEM_TEXT(""));
	}
}

//******************************************************************************
// Description:
//    Sets the terminal to raw mode, and save the original terminal settings.
// Return value:
//    0 ok
//    -1 NOK
//******************************************************************************

int16_t Keyboard::ttyRaw(int32_t& savedErrno)
{
	struct termios  buf;

	if (tcgetattr(STDIN_FILENO, &m_savedTermios) < 0) /* get the original state */
	{
		savedErrno = errno;
		TRACE(("tcgetattr() failed saving original terminal state, errno:%d", savedErrno));
		return -1;
	}

	buf = m_savedTermios;

	// Mokeys see, Monkeys do here
	//cout << "before changing" << endl;
	//cout << "buff.c_iflag " << buf.c_iflag << endl;
	//cout << "buff.c_lflag " << buf.c_lflag << endl;
	//cout << "buff.c_cflag " << buf.c_cflag << endl;


	buf.c_iflag &= ~(ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXANY|IXOFF);
	buf.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN | ECHOE | ECHOK);
	buf.c_cflag &= ~(CSIZE|PARENB);
	buf.c_cflag |= CS8;

	if(tcsetattr(STDIN_FILENO, TCSANOW, &buf) < 0)
	{
		savedErrno = errno;
		TRACE(("tcgetattr() failed setting terminal state to raw, errno:%d",
				savedErrno));
		return -1;
	}

	m_termSaved = true;
	TRACE(("Terminal set to RAW\r"));
	return 0;
}

//******************************************************************************
// Description:
//    Resets the terminal to saved mode.
// Return value:
//    0 ok
//    -1 NOK
//******************************************************************************

int16_t Keyboard::ttyReset(int32_t& savedErrno)
{
	/* set it to normal! */
	if (m_termSaved)
	{
		if (tcsetattr(m_handle, TCSAFLUSH, &m_savedTermios) < 0)
		{
			savedErrno = errno;
			TRACE(("tcgetattr() failed resetting terminal state, errno:%d\r",
					savedErrno));
			return -1;
		}
		m_termSaved = false;
		TRACE(("Terminal reset"));
	}
	return 0;
}

