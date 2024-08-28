/*
 * Keyboard.h
 *
 *  Created on: Nov 4, 2015
 *      Author: xdtthng
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <termios.h>
#include <string>

class Keyboard
{
public:
	enum StatusT
	{
		OK,
		HandleCreationFailed
	};

	enum InputT
	{
		NO_READABLE_INPUT, 	// No readable input from STDIN exists
		CHARACTER,         	// A character has been read from STDIN
		CR = 13,           	// Carriage return has been read from STDIN
		CONTROL_D = 4,     	// CTRL+D has been read from STDIN
		BS = 8, 			// Backspace
		DEL = 127, 			// Delete
		GS = 29				// Ctrol ] shown as ^] as in telnet
	};

	// Constructor
	Keyboard();

	// Destructor
	~Keyboard();

	// Returns the fd associated with the current instance for keyboard
	// input. (STDIN)
	int32_t handle() const;

	// Returns whether the current instance can be used or not.
	StatusT status() const;

	// Set terminal to raw mode.
	int16_t ttyRaw(int32_t& savedErrno);

	// Reset terminal to saved mode.
	int16_t ttyReset(int32_t& savedErrno);

	void ttyClear();

	// Check if terminal is set to raw.
	bool isTtyRaw() const;

private:

	// STDIN fd
	int32_t m_handle;

	// Information telling whether the constructor was able to initialize the
	// instance so it can be used without causing errors.
	StatusT m_status;

	// The terminal's original state
	struct termios m_savedTermios;

	// Is the terminal's original state saved.
	// If true means implicitly that the terminal is set to raw.
	bool m_termSaved;
};

inline
int Keyboard::handle() const
{
	return m_handle;
}

#endif /* KEYBOARD_H_ */
