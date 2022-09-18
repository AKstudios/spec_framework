//==========================================================================================================
// mgmt_server.h - Defines a process-management UDP server
//==========================================================================================================
#pragma once
#include "cthread.h"

class CMgmtServer : public CThread
{
protected:

    void    main();

};


