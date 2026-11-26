#include <cstddef>
#include <gtest/gtest.h>
#include <vector>
#include "../../src/drivers/d_default_input.h"
#include "core/tui.h"


using namespace d_default_input_internals;


class ParserPartsTestFixture : public ::testing::Test {
protected:
    // This runs BEFORE every single TEST_F
    void SetUp() override {
        events.clear();
        state = State::Ground;
        data.clear();
        // Initialize the map with standard keys so we don't repeat this in every test
        context.csi_map = {
            {make_key(1, 'A'), Key::UP},
            {make_key(1, 'B'), Key::DOWN},
            {make_key(2, '~'), Key::INSERT},
        };
        context.ss3_map = {
            {'A', Key::UP},
            {'B', Key::DOWN},
            {'~', Key::INSERT},
        };
        context.ctrl_map = {
            {'\x1', {Key::K_A, static_cast<int>(modifers::ctrl)}}
        };
    }
    ParserPartsTestFixture(): data(events){}

    Context context;
    State state;
    std::vector<Event> events;
    Data data;
};
// Testing parse_csi_data
TEST(parse_csi_data, BasicCsi) {
    std::vector<int> results;
    char ch = parse_csi_data("A", results);
    EXPECT_EQ(results, std::vector<int>{});
    EXPECT_EQ(ch, 'A');
}

TEST(parse_csi_data, Parameters) {
    std::vector<int> results;
    std::vector<int> correct {1, 2};
    char ch = parse_csi_data("1;2A", results);
    EXPECT_EQ(results, correct);
    EXPECT_EQ(ch, 'A');
}

TEST(parse_csi_data, NullParameters) {
    std::vector<int> results;
    std::vector<int> correct {0, 0};
    char ch = parse_csi_data(";A", results);
    EXPECT_EQ(results, correct);
    EXPECT_EQ(ch, 'A');
}
TEST(parse_csi_data, InvaliedSequence) {
    std::vector<int> results;
    std::vector<int> correct {0, 0};
    char ch = parse_csi_data(";0", results);
    EXPECT_EQ(ch, '\0');
}
TEST(parse_csi_data, ModifiedArrow){
    std::vector<int> results;
    std::vector<int> correct {1, 5};
    char ch = parse_csi_data("1;5B", results);
    EXPECT_EQ(results, correct);
    EXPECT_EQ(ch, 'B');
}
TEST(parse_csi_data, Paste){
    std::vector<int> results;
    std::vector<int> correct {201};
    char ch = parse_csi_data("201~", results);
    EXPECT_EQ(results, correct);
    EXPECT_EQ(ch, '~');
}

// Testing csi
TEST_F(ParserPartsTestFixture, csiStandard){
    size_t idx = csi("[A", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 2);
    Event c{Kind::Key, modifers::none, "", Key::UP};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, csiModifer){
    size_t idx = csi("[1;5A", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 5);
    Event c{Kind::Key, modifers::ctrl, "", Key::UP};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, csiInsert){
    size_t idx = csi("[2~", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 3);
    Event c{Kind::Key, modifers::none, "", Key::INSERT};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, csiUnknown){
    size_t idx = csi("[181818181~", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 11);
    Event c{Kind::Text, modifers::none, "", Key::NONE};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, csiIndex){
    size_t idx = csi("[2~A", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 3);
    Event c{Kind::Key, modifers::none, "", Key::INSERT};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, csiPart){
    state = State::Csi;
    size_t idx = csi("[2", state, data, context, 0);
    EXPECT_EQ(state, State::Csi);
    EXPECT_EQ(idx, 0);
    EXPECT_TRUE(data.exit);
}
TEST_F(ParserPartsTestFixture, csiPaste){
    size_t idx = csi("[200~", state, data, context, 0);
    EXPECT_EQ(state, State::Paste);
    EXPECT_EQ(idx, 5);
}

// Testing paste
TEST_F(ParserPartsTestFixture, pasteStandard){
    size_t idx = paste("Hello world\x1b[201~", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 16);
    Event c{Kind::Paste, modifers::none, "Hello world", Key::NONE};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, pasteContainsEsc){
    size_t idx = paste("Hello\x1bworld\x1b[201~", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 16);
    Event c{Kind::Paste, modifers::none, "Hello\x1bworld", Key::NONE};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, pastePart){
    state = State::Paste;
    size_t idx = paste("Hello\x1bworld", state, data, context, 0);
    EXPECT_EQ(state, State::Paste);
    EXPECT_EQ(idx, 0);
    EXPECT_TRUE(data.exit);
}
TEST_F(ParserPartsTestFixture, pasteShort){
    state = State::Paste;
    size_t idx = paste("hi\x1b[201~", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 7);
    Event c{Kind::Paste, modifers::none, "hi", Key::NONE};
    EXPECT_EQ(data.event, c);
}

// Testing ss3
TEST_F(ParserPartsTestFixture, ss3Standerd){
    size_t idx = ss3("OA", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 2);
    Event c{Kind::Key, modifers::none, "", Key::UP};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, ss3Unknown){
    size_t idx = ss3("OC", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 2);
    Event c{Kind::Text, modifers::none, "", Key::NONE};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, ss3Exit){
    state = State::Ss3;
    size_t idx = ss3("O", state, data, context, 0);
    EXPECT_EQ(state, State::Ss3);
    EXPECT_EQ(idx, 0);
    EXPECT_TRUE(data.exit);
}
// Testing uf8
TEST_F(ParserPartsTestFixture, utf8Standard){
    size_t idx = utf8("そ", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 3);
    Event c{Kind::Text, modifers::none, "そ", Key::NONE};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, utf8Exit){
    state = State::Utf8;
    size_t idx = utf8("\xC3", state, data, context, 0);
    EXPECT_EQ(state, State::Utf8);
    EXPECT_EQ(idx, 0);
    EXPECT_TRUE(data.exit);
}
TEST_F(ParserPartsTestFixture, utf8Zerowidth){
    data.event.text = "e";
    size_t idx = utf8("\u0301", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 2);
    Event c{Kind::Text, modifers::none, "e\u0301", Key::NONE};
    EXPECT_EQ(data.event, c);
}

// Testing ground
TEST_F(ParserPartsTestFixture, groundStandardASCII){
    size_t idx = ground("a", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 1);
    Event c{Kind::Text, modifers::none, "a", Key::K_A};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, groundModifiedASCII){
    data.event.modifer = modifers::alt;
    size_t idx = ground("a", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 1);
    Event c{Kind::Key, modifers::alt, "", Key::K_A};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, groundUpperASCII){
    size_t idx = ground("A", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 1);
    Event c{Kind::Text, modifers::none, "A", Key::K_A};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, groundPushFromASCII){
    Event r{Kind::Text, modifers::none, "a", Key::K_A};
    data.event = r;
    size_t idx = ground("A", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 1);
    Event c{Kind::Text, modifers::none, "A", Key::K_A};
    EXPECT_EQ(data.event, c);
    EXPECT_EQ(events[0], r);
}
TEST_F(ParserPartsTestFixture, groundStanderdESC){
    size_t idx = ground("\x1b", state, data, context, 0);
    EXPECT_EQ(state, State::Esc);
    EXPECT_EQ(idx, 0);
}
TEST_F(ParserPartsTestFixture, groundPushFromESC){
    Event r{Kind::Text, modifers::none, "a", Key::K_A};
    data.event = r;
    size_t idx = ground("\x1b", state, data, context, 0);
    EXPECT_EQ(state, State::Esc);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(events[0], r);
}
TEST_F(ParserPartsTestFixture, groundStandardCtrl){
    size_t idx = ground("\x1", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 1);
    Event c{Kind::Key, modifers::ctrl, "", Key::K_A};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, groundAltCtrl){
    data.event.modifer = modifers::alt;
    size_t idx = ground("\x1", state, data, context, 0);
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 1);
    Event c{Kind::Key, modifers::ctrl | modifers::alt, "", Key::K_A};
    EXPECT_EQ(data.event, c);
}
TEST_F(ParserPartsTestFixture, groundUtf8){
    size_t idx = ground("\xC3", state, data, context, 0);
    EXPECT_EQ(state, State::Utf8);
    EXPECT_EQ(idx, 0);
}
TEST_F(ParserPartsTestFixture, groundUnknown){
    size_t idx = ground("\x2", state, data, context, 0); // \x2 is a ctrl key that is not mapped in this test environment.
    EXPECT_EQ(state, State::Ground);
    EXPECT_EQ(idx, 1);
    Event c{Kind::Text, modifers::none, "", Key::NONE};
    EXPECT_EQ(data.event, c);
}
