#include <cstddef>
#include <gtest/gtest.h>
#include <vector>
#include "../../src/drivers/d_default_input.h"


using namespace d_default_input_internals;
using namespace std;


class ParserPartsTestFixture : public ::testing::Test {
protected:
    // This runs BEFORE every single TEST_F
    void SetUp() override {
        events.clear();
        s = State::Ground;
        d.clear();
        // Initialize the map with standard keys so we don't repeat this in every test
        con.csi_map = {
            {make_key(1, 'A'), Key::UP},
            {make_key(1, 'B'), Key::DOWN},
            {make_key(2, '~'), Key::INSERT},
        };
    }
    ParserPartsTestFixture(): d(events){}

    Context con;
    State s;
    std::vector<Event> events;
    Data d;
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
TEST(parse_csi_data, ModifedArrow){
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
TEST_F(ParserPartsTestFixture, csiStanerd){
    size_t idx = csi("[A", s, d, con, 0);
    EXPECT_EQ(s, State::Ground);
    EXPECT_EQ(idx, 2);
    Event c{Kind::Key, modifers::none, "", Key::UP};
    EXPECT_EQ(d.event, c);
}
TEST_F(ParserPartsTestFixture, csiModifer){
    size_t idx = csi("[1;5A", s, d, con, 0);
    EXPECT_EQ(s, State::Ground);
    EXPECT_EQ(idx, 5);
    Event c{Kind::Key, modifers::ctrl, "", Key::UP};
    EXPECT_EQ(d.event, c);
}
TEST_F(ParserPartsTestFixture, csiInsert){
    size_t idx = csi("[2~", s, d, con, 0);
    EXPECT_EQ(s, State::Ground);
    EXPECT_EQ(idx, 3);
    Event c{Kind::Key, modifers::none, "", Key::INSERT};
    EXPECT_EQ(d.event, c);
}
TEST_F(ParserPartsTestFixture, csiIndex){
    size_t idx = csi("[2~A", s, d, con, 0);
    EXPECT_EQ(s, State::Ground);
    EXPECT_EQ(idx, 3);
    Event c{Kind::Key, modifers::none, "", Key::INSERT};
    EXPECT_EQ(d.event, c);
}
TEST_F(ParserPartsTestFixture, csiPart){
    s = State::Csi;
    size_t idx = csi("[2", s, d, con, 0);
    EXPECT_EQ(s, State::Csi);
    EXPECT_EQ(idx, 0);
    EXPECT_TRUE(d.exit);
}
TEST_F(ParserPartsTestFixture, csiPaste){
    size_t idx = csi("[200~", s, d, con, 0);
    EXPECT_EQ(s, State::Paste);
    EXPECT_EQ(idx, 5);
}

// Testing paste
TEST_F(ParserPartsTestFixture, pasteStanerd){
    size_t idx = paste("Hello world\x1b[201~", s, d, con, 0);
    EXPECT_EQ(s, State::Ground);
    EXPECT_EQ(idx, 16);
    Event c{Kind::Paste, modifers::none, "Hello world", Key::NONE};
    EXPECT_EQ(d.event, c);
}
TEST_F(ParserPartsTestFixture, pasteContainsEsc){
    size_t idx = paste("Hello\x1bworld\x1b[201~", s, d, con, 0);
    EXPECT_EQ(s, State::Ground);
    EXPECT_EQ(idx, 16);
    Event c{Kind::Paste, modifers::none, "Hello\x1bworld", Key::NONE};
    EXPECT_EQ(d.event, c);
}
TEST_F(ParserPartsTestFixture, pastePart){
    s = State::Paste;
    size_t idx = paste("Hello\x1bworld", s, d, con, 0);
    EXPECT_EQ(s, State::Paste);
    EXPECT_EQ(idx, 0);
    EXPECT_TRUE(d.exit);
}
TEST_F(ParserPartsTestFixture, pasteShort){
    s = State::Paste;
    size_t idx = paste("hi\x1b[201~", s, d, con, 0);
    EXPECT_EQ(s, State::Ground);
    EXPECT_EQ(idx, 7);
    Event c{Kind::Paste, modifers::none, "hi", Key::NONE};
    EXPECT_EQ(d.event, c);
}
