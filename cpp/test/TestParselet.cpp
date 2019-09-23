
#include "Parser.h"
#include "API.h"

#include "gtest/gtest.h"

#include <sstream>

static std::unique_ptr<MLSession> mlSession;

class ParseletTest : public ::testing::Test {
protected:
    
    static void SetUpTestSuite() {
        
        mlSession = std::unique_ptr<MLSession>(new MLSession);
        
        TheParserSession = std::unique_ptr<ParserSession>(new ParserSession);
    }
    
    static void TearDownTestSuite() {
        
        TheParserSession.reset(nullptr);
        
        mlSession.reset(nullptr);
    }
    
    void SetUp() override {
        
    }
    
    void TearDown() override {
        TheParserSession->deinit();
    }

};

TEST_F(ParseletTest, Bug1) {
    
    auto iss = std::stringstream("a /: b := c");
    
    TheParserSession->init(nullptr, iss, false);
    
    ParserContext Ctxt;
    
    auto NP = TheParser->parse(Ctxt);
    
    auto N = NP.get();
    
    EXPECT_TRUE(dynamic_cast<TernaryNode*>(N));
}