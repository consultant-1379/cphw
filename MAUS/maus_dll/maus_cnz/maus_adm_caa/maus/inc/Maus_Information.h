/*
 * Maus_Information.h
 *
 *  Created on: Apr 30, 2015
 *      Author: xdtthng
 */

#ifndef MAUS_INFORMATION_H_
#define MAUS_INFORMATION_H_

struct MausInformation
{
	int side;
	int shlf;
	char* viewName;
	char* interface[2];     //Backplane NIC interface
	char* logRoot;          //Root directory for MAU logs
	int   instance;         //Connection instance
	int	sysNo;
	int fcState;
	int apgState;
};


#endif // MAUS_INFORMATION_H_
