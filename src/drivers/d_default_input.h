#include "../core/drivers.h"

namespace d_default_input_internals {
    enum State : uint8_t{
        Ground = 0,
        Utf8 = 1,
        Esc = 2,
        Csi = 3,
        Ss3 = 4,
        String = 5,
        Paste = 6,
    };

    struct Context {
        std::unordered_map<char, std::pair<Key, int>> ctrl_map {};
        std::unordered_map<char, Key> ss3_map {};
        std::unordered_map<long long, Key> csi_map {};
    };
    struct Data{
        bool exit {false};

        Event event{Kind::Text, modifers::none, "", Key::NONE};

        std::vector<Event>& events;

        Data(std::vector<Event>& i_events): events(i_events){}
        void clear(){
            event = Event{Kind::Text, modifers::none, "", Key::NONE};
        }
        bool has_data(){
                return (event.kind == Kind::Text && event.text.size() > 0) || (event.kind == Kind::Key && event.key != Key::NONE) || (event.kind == Kind::Paste && event.text.size() > 0);
        }
        void push(){
            events.push_back(event);
            clear();
        }
    };
    char parse_csi_data(std::string_view s, std::vector<int>& results);
    long long make_key(int i, char c);
    size_t csi(std::string_view m_buff, State& state, Data& data, Context& cont, size_t index);
    size_t paste(std::string_view m_buff, State& state, Data& data, Context& cont, size_t index);

}

namespace drivers {
    class d_default_input: public drivers::d_input{
    private:
        struct termios m_old;
        std::string m_buff;
        std::unordered_map<std::string, Key> m_keydb;
        std::unordered_map<char, char> m_asciiCtrldb;
        std::vector<Event> m_events {};
        d_default_input_internals::Data m_data;
        d_default_input_internals::Context m_cont;
        d_default_input_internals::State m_state;
        void m_fillDb();
        void m_setRawMode();
        void m_restoreMode();
    public:
        ~d_default_input() override;
        d_default_input();
        std::vector<Event> d_getEvents(int timeout_ms) override;
        void d_clean() override{
            m_restoreMode();
        }
        // int d_getCharacterWidth(uint32_t ch) override;
    };
}
