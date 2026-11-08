#include "tui.h"
#include "drivers.h"
#include <vector>

using namespace std;
using namespace core;

c_eventBus::c_eventBus(drivers::d_input* driver, c_surfaces& surfs): m_inputDriver(driver), m_surfs(surfs){}
c_eventBus::~c_eventBus(){}


void c_eventBus::tick(int timeout_ms){
    std::vector<Event> events = m_inputDriver->d_getEvents(timeout_ms);
    for (c_surface& surf : m_surfs){
        if (surf.inputFunc == nullptr){
            continue;
        }
        for (Event e : events){
            if (surf.inputFunc(e)){
                break;
            }
        }
    }
}
