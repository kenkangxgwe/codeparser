
#include "CharacterDecoder.h"
#include "ByteBuffer.h"
#include "API.h"
//#include "CodePoint.h"

#include "gtest/gtest.h"

#include <sstream>


class CharacterDecoderTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        
        TheParserSession = std::unique_ptr<ParserSession>(new ParserSession);
    }
    
    static void TearDownTestSuite() {
        
        TheParserSession.reset(nullptr);
    }
    
    void SetUp() override {
        
    }
    
    void TearDown() override {
        TheParserSession->deinit();
    }
};

TEST_F(CharacterDecoderTest, Basic1) {
    
    auto strIn = std::string("1+2");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('2'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, LongName) {
    
    auto strIn = std::string("1+\\[Alpha]");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(0x03b1, ESCAPE_LONGNAME));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, 4Hex) {
    
    auto strIn = std::string("1+\\:03b1");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(0x03b1, ESCAPE_4HEX));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, 2Hex) {
    
    auto strIn = std::string("1+\\.f2");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(242, ESCAPE_2HEX));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, Octal) {
    
    auto strIn = std::string("1+\\333");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(219, ESCAPE_OCTAL));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, 6Hex) {
    
    auto strIn = std::string("1+\\|0000f2");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(242, ESCAPE_6HEX));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, Raw) {
    
    auto strIn = std::string("1+\\[RawWedge]");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('^', ESCAPE_RAW));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, LongNameError1) {
    
    auto strIn = std::string("1+\\[Alpha+2");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
//    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('\\'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('['));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('A'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('l'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('p'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('h'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('a'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('2'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, LongNameError2) {
    
    auto strIn = std::string("1+\\[Alpa]+2");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('\\'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('['));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('A'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('l'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('p'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('a'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(']'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('2'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, 4HexError1) {
    
    auto strIn = std::string("1+\\:03b+1");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('\\'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(':'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('0'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('3'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('b'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('+'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('1'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, LineContinuation1) {
    
    auto strIn = std::string("ab\\\ncd");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('a'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('b'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('c'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('d'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, LineContinuation2) {
    
    auto strIn = std::string("ab\\\r\ncd");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('a'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('b'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('c'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('d'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

TEST_F(CharacterDecoderTest, LineContinuation3) {
    
    auto strIn = std::string("ab\\\rcd");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('a'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('b'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('c'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter('d'));
    
    //    TheCharacterDecoder->nextWLCharacter(TOPLEVEL);
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(INSIDE_SYMBOL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
}

//
// There was a bug where UnexpectedEscapeSequence issues were being added by mistake
//
TEST_F(CharacterDecoderTest, UnexpectedEscapeSequence) {
    
    auto strIn = std::string("\"\\[Alpha]\"");
    
    auto str = reinterpret_cast<Buffer>(strIn.c_str());
    
    TheParserSession->init(BufferAndLength(str, strIn.size()), nullptr, INCLUDE_SOURCE, SOURCECONVENTION_LINECOLUMN);
    
    auto c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('"'));
    
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL | PRESERVE_WS_AFTER_LC | LC_IS_MEANINGFUL);
    
    EXPECT_EQ(c, WLCharacter(0x03b1, ESCAPE_LONGNAME));
    
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter('"'));
    
    TheByteBuffer->buffer = TheCharacterDecoder->lastBuf;
    
    c = TheCharacterDecoder->currentWLCharacter(TOPLEVEL);
    
    EXPECT_EQ(c, WLCharacter(CODEPOINT_ENDOFFILE));
    
    auto& Issues = TheCharacterDecoder->getIssues();
    
    EXPECT_EQ(Issues.size(), (size_t) 0);
}

