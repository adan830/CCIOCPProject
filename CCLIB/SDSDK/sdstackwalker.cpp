#include "SDuPCH.h"
#include "sdstackwalker.h"
#include "StackWalker.h"

#ifdef WIN
void Crown::SdStackWalker::Dump()
{
	StackWalker sw;
	sw.ShowCallstack();
}

Crown::SdStackWalker::SdStackWalker()
{

}

#else


#endif
