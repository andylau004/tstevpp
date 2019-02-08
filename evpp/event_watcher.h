#pragma once


#include "inner_pre.h"

#include "duration.h"

struct event;
struct event_base;

namespace evpp {

class Eventloop;

class EVPP_EXPORT EventWatcher {
public:
    typedef std::function< void() > Handler;
    virtual ~EventWatcher();

public:

private:


};

}
