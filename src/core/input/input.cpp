#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <algorithm>
#include <term.h>
#include <stdexcept>
#include <vector>
#include "../tui.h"
#include "../utils/logging.h"
#include "../drivers.h"

using namespace std;
using namespace core;
// static helper functions
static int getDataSize(){
    int size;
    ioctl(0, FIONREAD, &size);
    return size;
}

static string getData(){
    int dataSize = getDataSize();
    if (dataSize){
        char buff[dataSize + 1];
        read(0, &buff, dataSize);
        buff[dataSize] = '\0';
        return buff;
    }
    return "";
}

// Input memeber functinos
Input::Input(drivers::d_input* driver): m_driver(driver){
    logger::info("[Input] Starting up");
    // m_setRawMode();
    // cout << "\r\x1b[6n\x1b]66;w=2; \a\x1b[6n\x1b]66;s=2; \a\x1b[6n\x1b[6n";
    // cout.flush();
    //
    // std::vector<std::array<string, 2>> pos;
    // string buff = "";
    // while (pos.size() < 3) {
    //     buff += getData();
    //
    //     // Look for the start of the sequence: \x1b[
    //     size_t start_pos = buff.find("\x1b[");
    //     // Look for the end of the sequence: R
    //     size_t end_pos = buff.find('R', start_pos);
    //
    //     if (start_pos != std::string::npos && end_pos != std::string::npos) {
    //         // We found a full sequence! Let's parse the content between '[' and ';' and 'R'
    //         // Format expected: \x1b[row;colR
    //         size_t semi_colon = buff.find(';', start_pos);
    //
    //         if (semi_colon != std::string::npos && semi_colon < end_pos) {
    //             std::string row = buff.substr(start_pos + 2, semi_colon - (start_pos + 2));
    //             std::string col = buff.substr(semi_colon + 1, end_pos - (semi_colon + 1));
    //
    //             pos.push_back({row, col});
    //
    //             // Clear the processed part of the buffer
    //             buff.erase(0, end_pos + 1);
    //         } else {
    //             // If we found the start but the format is incomplete,
    //             // we might need to wait for more data.
    //             // We'll let the loop continue.
    //         }
    //     }
    // }
    //
    // for (const auto& p : pos) {
    //     logger::debug("x: " + p[0] + " y: " + p[1]);
    // }
}
Input::~Input() {}
bool Input::update(c_surfaces& surfs, int timeout_ms){
    vector<Event> events = m_driver->d_getEvents(timeout_ms);
    for (Event e : events){
        logger::debug("Event: kind == Text " + std::to_string(e.kind == Event::Kind::Text) + " text '" + e.text + "' key:" + std::to_string((uint32_t)e.key) + " modifer:" + std::to_string((uint8_t)e.modifer));
        for (c_surface& surf : surfs){
            auto it = find(surf.keys.begin(), surf.keys.end(), e.text);
            if (it != surf.keys.end()){
                surf.events[e.text] = true;
            }
        }
    }
    return events.empty();
};
