
#include "Parselet.h"

#include "API.h" // for ParserSession
#include "ParseletRegistration.h"


LeafParselet::LeafParselet(Precedence precedence) : precedence(precedence) {}

NodePtr LeafParselet::parse(Token TokIn, ParserContext Ctxt) const {
    
    TheParser->nextToken(TokIn);
    
    auto Left = NodePtr(new LeafNode(TokIn));
    
    return TheParser->infixLoop(std::move(Left), Ctxt);
}

Precedence LeafParselet::getPrecedence(ParserContext Ctxt) const {
    assert(precedence != PRECEDENCE_ASSERTFALSE);
    return precedence;
}


PrefixAssertFalseParselet::PrefixAssertFalseParselet() {}

NodePtr PrefixAssertFalseParselet::parse(Token TokIn, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}

Precedence PrefixAssertFalseParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}


NodePtr PrefixErrorParselet::parse(Token TokIn, ParserContext Ctxt) const {
    
    assert(TokIn.Tok.isError());
    
    TheParser->nextToken(TokIn);
    
    //
    // If there is a Token error, then use that specific error
    //
    
    return NodePtr(new ErrorNode(TokIn));
}

Precedence PrefixErrorParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}


NodePtr PrefixCloserParselet::parse(Token TokIn, ParserContext Ctxt) const {
    
    assert(TokIn.Tok.isCloser());
    
    if (TokenToCloser(TokIn.Tok) == Ctxt.Closr) {
        //
        // Handle the special cases of:
        // { + }
        // { a + }
        // { a @ }
        // We are here parsing the operators, but we don't want to descend and treat the } as the problem
        //
        
        //
        // Do not take next token
        //
        
        auto createdToken = Token(TOKEN_ERROR_EXPECTEDOPERAND, BufferAndLength(TokIn.BufLen.buffer), Source(TokIn.Src.Start));
        
        return NodePtr(new ErrorNode(createdToken));
    }
    
    //
    // Handle  { a ) }
    // which ends up being  MissingCloser[ { a ) ]   UnexpectedCloser[ } ]
    //
    
    TheParser->nextToken(TokIn);
    
    NodeSeq Args(1);
    Args.append(NodePtr(new LeafNode(TokIn)));
    
    auto Error = NodePtr(new SyntaxErrorNode(SYNTAXERROR_UNEXPECTEDCLOSER, std::move(Args)));
    
    return Error;
}

Precedence PrefixCloserParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}


NodePtr PrefixEndOfFileParselet::parse(Token TokIn, ParserContext Ctxt) const {
    
    //
    // Something like  a+<EOF>
    //
    
    auto createdToken = Token(TOKEN_ERROR_EXPECTEDOPERAND, BufferAndLength(TokIn.BufLen.buffer), Source(TokIn.Src.Start));
    
    return NodePtr(new ErrorNode(createdToken));
}

Precedence PrefixEndOfFileParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}


NodePtr PrefixUnsupportedTokenParselet::parse(Token TokIn, ParserContext Ctxt) const {
    
    TheParser->nextToken(TokIn);
    
    auto createdToken = Token(TOKEN_ERROR_UNSUPPORTEDTOKEN, TokIn.BufLen, TokIn.Src);
    
    return NodePtr(new ErrorNode(createdToken));
}

Precedence PrefixUnsupportedTokenParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}


NodePtr PrefixUnhandledParselet::parse(Token TokIn, ParserContext Ctxt) const {
    
    assert(!TokIn.Tok.isPossibleBeginning() && "handle at call site");
    
    //
    // FIXME: clear other flags here also?
    //
    Ctxt.Flag &= ~(PARSER_INSIDE_COLON | PARSER_INSIDE_TILDE);
    
    auto NotPossible = NodePtr(new ErrorNode(Token(TOKEN_ERROR_EXPECTEDOPERAND, BufferAndLength(TokIn.BufLen.buffer), Source(TokIn.Src.Start))));
    
    auto I = infixParselets[TokIn.Tok.value()];
    
    auto TokenPrecedence = I->getPrecedence(Ctxt);
    
    //
    // if (Ctxt.Prec > TokenPrecedence)
    //   goto prefixUnhandledParseletRet;
    // else if (Ctxt.Prec == TokenPrecedence && Ctxt.Prec.Associativity is NonRight)
    //   goto prefixUnhandledParseletRet;
    //
    if ((Ctxt.Prec | 0x1) > TokenPrecedence) {
        
        //
        // Something like  a + | 2
        //
        // Make sure that the error leaf is with the + and not the |
        //
        
        return NotPossible;
    }
    
    //
    // Handle something like  f[,1]
    //
    // We want to make EXPECTEDOPERAND the first arg of the Comma node.
    //
    // Do not take next token
    //
    // Important to not duplicate token's Str here, it may also appear later
    //
    // Also, invent Source
    //
    
    NodeSeq LeftSeq(1);
    LeftSeq.append(std::move(NotPossible));
    
    auto NotPossible2 = infixParselets[TokIn.Tok.value()]->parse(std::move(LeftSeq), TokIn, Ctxt);
    
    return NotPossible2;
}

Precedence PrefixUnhandledParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}


NodePtr InfixToplevelNewlineParselet::parse(NodeSeq Left, Token firstTok, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}


//
// something like  x  or x_
//
NodePtr SymbolParselet::parse(Token TokIn, ParserContext Ctxt) const {
    
    auto Sym = NodePtr(new LeafNode(TokIn));
    
    TheParser->nextToken(TokIn);
    
    auto Tok = TheParser->currentToken(Ctxt);
    
    //
    // if we are here, then we know that Sym could bind to _
    //
    
    switch (Tok.Tok.value()) {
        case TOKEN_UNDER.value(): {
            
            NodeSeq Args(1);
            Args.append(std::move(Sym));
            
            return contextSensitiveUnder1Parselet->parseContextSensitive(std::move(Args), Tok, Ctxt);
        }
        case TOKEN_UNDERUNDER.value(): {
            
            NodeSeq Args(1);
            Args.append(std::move(Sym));
            
            return contextSensitiveUnder2Parselet->parseContextSensitive(std::move(Args), Tok, Ctxt);
        }
        case TOKEN_UNDERUNDERUNDER.value(): {
            
            NodeSeq Args(1);
            Args.append(std::move(Sym));
            
            return contextSensitiveUnder3Parselet->parseContextSensitive(std::move(Args), Tok, Ctxt);
        }
        case TOKEN_UNDERDOT.value(): {

            NodeSeq Args(1);
            Args.append(std::move(Sym));
            
            return contextSensitiveUnderDotParselet->parseContextSensitive(std::move(Args), Tok, Ctxt);
        }
            break;
        default: {
            
            {
                LeafSeq ArgsTest;
            
                Tok = TheParser->eatTriviaButNotToplevelNewlines(Tok, Ctxt, ArgsTest);
            
                //
                // when parsing a in a:b  then PARSER_INSIDE_COLON bit is 0
                // when parsing b in a:b  then PARSER_INSIDE_COLON bit is 1
                //
                // It is necessary to go to colonParselet->parse here (even though it seems non-contextSensitive)
                // because in e.g.,  a_*b:f[]  the b is the last node in the Times expression and needs to bind with  :f[]
                // Parsing  a_*b  completely, and then parsing  :f[]  would be wrong.
                //
                if (Tok.Tok == TOKEN_COLON) {
                    
                    if ((Ctxt.Flag & PARSER_INSIDE_COLON) != PARSER_INSIDE_COLON) {
                        
                        NodeSeq Args(1 + 1);
                        Args.append(std::move(Sym));
                        Args.appendIfNonEmpty(std::move(ArgsTest));
                        
                        Sym = infixParselets[TOKEN_COLON.value()]->parse(std::move(Args), Tok, Ctxt);
                    }
                }
            }
            
            return TheParser->infixLoop(std::move(Sym), Ctxt);
        }
    }
    
    assert(false);
    return nullptr;
}

NodePtr SymbolParselet::parseContextSensitive(Token TokIn, ParserContext Ctxt) const {
    
    auto Sym = NodePtr(new LeafNode(TokIn));
    
    TheParser->nextToken(TokIn);
    
    return Sym;
}


NodePtr PrefixOperatorParselet::parse(Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = getPrecedence(Ctxt);
    
    TheParser->nextToken(TokIn);
    
    NodePtr Left;
    {
        LeafSeq ArgsTest;
        
        auto Tok = TheParser->currentToken(Ctxt);
        Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest);
        
        auto Operand = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
        
        NodeSeq Args(1 + 1 + 1);
        Args.append(NodePtr(new LeafNode(TokIn)));
        Args.appendIfNonEmpty(std::move(ArgsTest));
        Args.append(std::move(Operand));
        
        Left = NodePtr(new PrefixNode(Op, std::move(Args)));
    }
    
    return TheParser->infixLoop(std::move(Left), CtxtIn);
}


//
// Does not "parse" so much as inserts the implicit Times token, and then allows the rest of the system to do the work
//
NodePtr InfixImplicitTimesParselet::parse(NodeSeq Left, Token TokIn, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}

Precedence InfixImplicitTimesParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}

Token InfixImplicitTimesParselet::procesImplicitTimes(Token TokIn) const {
    
    auto token = Token(TOKEN_FAKE_IMPLICITTIMES, BufferAndLength(TokIn.BufLen.buffer), Source(TokIn.Src.Start));
    
    return token;
}


NodePtr InfixAssertFalseParselet::parse(NodeSeq Left, Token firstTok, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}


Precedence InfixAssertFalseParselet::getPrecedence(ParserContext Ctxt) const {
    assert(false);
    return PRECEDENCE_ASSERTFALSE;
}


NodePtr InfixCloserParselet::parse(NodeSeq Left, Token firstTok, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}


NodePtr InfixEndOfFileParselet::parse(NodeSeq Left, Token firstTok, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}


NodePtr InfixUnsupportedTokenParselet::parse(NodeSeq Left, Token firstTok, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}


NodePtr InfixErrorParselet::parse(NodeSeq Left, Token firstTok, ParserContext Ctxt) const {
    assert(false);
    return nullptr;
}


NodePtr BinaryOperatorParselet::parse(NodeSeq Left, Token TokIn, ParserContext Ctxt) const {
    
    Ctxt.Prec = getPrecedence(Ctxt);
    
    TheParser->nextToken(TokIn);
    
    NodePtr L;
    {
        LeafSeq ArgsTest;
    
        auto Tok = TheParser->currentToken(Ctxt);
        Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest);
    
        auto Right = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
    
        NodeSeq Args(1 + 1 + 1 + 1);
        Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
        Args.append(NodePtr(new LeafNode(TokIn)));
        Args.appendIfNonEmpty(std::move(ArgsTest));
        Args.append(std::move(Right));
    
        L = NodePtr(new BinaryNode(Op, std::move(Args)));
    }
    
    return TheParser->infixLoop(std::move(L), Ctxt);
}


NodePtr InfixOperatorParselet::parse(NodeSeq Left, Token TokIn, ParserContext Ctxt) const {
    
    NodeSeq Args(1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    
    Ctxt.Prec = getPrecedence(Ctxt);
    
    while (true) {

#if !NABORT
        //
        // Check isAbort() inside loops
        //
        if (TheParserSession->isAbort()) {
            
            return TheParserSession->handleAbort();
        }
#endif // !NABORT
        
        LeafSeq ArgsTest1;
        
        auto Tok1 = TheParser->currentToken(Ctxt);
        Tok1 = TheParser->eatTriviaButNotToplevelNewlines(Tok1, Ctxt, ArgsTest1);
        
        auto I = infixParselets[Tok1.Tok.value()];
        
        Tok1 = I->procesImplicitTimes(Tok1);
        I = infixParselets[Tok1.Tok.value()];
        
        //
        // Cannot just compare tokens
        //
        // May be something like  a * b c \[Times] d
        //
        // and we want only a single Infix node created
        //
        if (I->getOp() != Op) {
            
            //
            // Tok.Tok != TokIn.Tok, so break
            //
            
            break;
        }
        
        TheParser->nextToken(Tok1);
        
        LeafSeq ArgsTest2;
        
        auto Tok2 = TheParser->currentToken(Ctxt);
        Tok2 = TheParser->eatTrivia(Tok2, Ctxt, ArgsTest2);
        
        auto Operand = prefixParselets[Tok2.Tok.value()]->parse(Tok2, Ctxt);
        
        //
        // Do not reserve inside loop
        // Allow default resizing strategy, which is hopefully exponential
        //
        Args.appendIfNonEmpty(std::move(ArgsTest1));
        Args.append(NodePtr(new LeafNode(Tok1)));
        Args.appendIfNonEmpty(std::move(ArgsTest2));
        Args.append(std::move(Operand));
        
    } // while
    
    auto L = NodePtr(new InfixNode(Op, std::move(Args)));
    
    return TheParser->infixLoop(std::move(L), Ctxt);
}


NodePtr PostfixOperatorParselet::parse(NodeSeq Left, Token TokIn, ParserContext Ctxt) const {
    
    TheParser->nextToken(TokIn);
    
    NodeSeq Args(1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(TokIn)));
    
    auto L = NodePtr(new PostfixNode(Op, std::move(Args)));
    
    return TheParser->infixLoop(std::move(L), Ctxt);
}


NodePtr GroupParselet::parse(Token firstTok, ParserContext CtxtIn) const {
    
    auto OpenerT = firstTok;
    
    auto Ctxt = CtxtIn;
    
    TheParser->nextToken(firstTok);
    
    Ctxt.Closr = Closr;
    
    //
    // FIXME: clear other flags here also?
    //
    Ctxt.Flag &= ~(PARSER_INSIDE_COLON | PARSER_INSIDE_TILDE);
    Ctxt.Prec = PRECEDENCE_LOWEST;
    
    NodeSeq Args(1);
    Args.append(NodePtr(new LeafNode(std::move(OpenerT))));
    
    //
    // There will only be 1 "good" node (either a LeafNode or a CommaNode)
    // But there might be multiple error nodes
    //
    // ADDENDUM: Actually, there may be more than 1 "good" node
    // e.g. {1\\2}
    //
    
    NodePtr group;
    
    while (true) {
        
#if !NABORT
        //
        // Check isAbort() inside loops
        //
        if (TheParserSession->isAbort()) {
            
            return TheParserSession->handleAbort();
        }
#endif // !NABORT
        
        LeafSeq ArgsTest1;
        
        auto Tok = TheParser->currentToken(Ctxt);
        Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest1);
        
        if (TokenToCloser(Tok.Tok) == Closr) {
            
            //
            // Everything is good
            //
            
            TheParser->nextToken(Tok);
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.appendIfNonEmpty(std::move(ArgsTest1));
            Args.append(NodePtr(new LeafNode(std::move(Tok))));
            
            group = NodePtr(new GroupNode(Op, std::move(Args)));
            
            break;
        }
        if (Tok.Tok.isCloser()) {
            
            //
            // some other closer
            //
            // e.g.,   { ( }
            //
            // FIXME: { ) }  is not handled well
            //
            
            group = NodePtr(new GroupMissingCloserNode(Op, std::move(Args)));
            
            break;
        }
        if (Tok.Tok == TOKEN_ENDOFFILE) {
            
            //
            // Handle something like   { a EOF
            //
            
            group = NodePtr(new GroupMissingCloserNeedsReparseNode(Op, std::move(Args)));
            
            break;
        }
        
        //
        // Handle the expression
        //
        
        auto Operand = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
        
        //
        // Do not reserve inside loop
        // Allow default resizing strategy, which is hopefully exponential
        //
        
        //
        // Always append here
        //
        Args.appendIfNonEmpty(std::move(ArgsTest1));
        Args.append(std::move(Operand));
        
    } // while
    
    return TheParser->infixLoop(std::move(group), CtxtIn);
}


NodePtr CallParselet::parse(NodeSeq Head, Token TokIn, ParserContext CtxtIn) const {
    
    //
    // if we used PRECEDENCE_CALL here, then e.g., a[]?b should technically parse as   a <call> []?b
    //
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_HIGHEST;
    
    auto Right = GP->parse(TokIn, Ctxt);
    
    NodeSeq Args(1);
    Args.append(std::move(Right));
    
    auto L = NodePtr(new CallNode(std::move(Head), std::move(Args)));
    
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


#if STARTOFLINE

//
// StartOfLine
//

NodePtr StartOfLineParselet::parse(ParserContext CtxtIn) const {
    
    auto TokIn = TheParser->currentToken();
    
    auto Ctxt = CtxtIn;
    
    TheParser->nextToken_stringifyCurrentLine(Ctxt);
    
    auto Tok = TheParser->currentToken();
    
    //
    // We know there is just a token here, either TOKEN_ERROR_EMPTYSTRING or a legit string
    //
    TheParser->nextToken(Ctxt);
    
    NodeSeq Args;
    Args.reserve(1 + 1);
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.append(NodePtr(new LeafNode(Tok)));
    
    return NodePtr(new StartOfLineNode(StartOfLineOperatorToSymbol(TokIn.Tok()), std::move(Args)));
}


//
// StartOfFile
//

NodePtr StartOfFileParselet::parse(ParserContext CtxtIn) const {
    
    auto TokIn = TheParser->currentToken();
    
    auto Ctxt = CtxtIn;
    
    TheParser->nextToken_stringifyCurrentLine(Ctxt);
    
    auto Tok = TheParser->currentToken();
    
    //
    // We know there is just a token here, either TOKEN_ERROR_EMPTYSTRING or a legit string
    //
    TheParser->nextToken(Ctxt);
    
    NodeSeq Args;
    Args.reserve(1 + 1);
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.append(NodePtr(new LeafNode(Tok)));
    
    return NodePtr(new StartOfFileNode(StartOfFileOperatorToSymbol(TokIn.Tok()), std::move(Args)));
}

#endif // STARTOFLINE


//
// Something like  a ~f~ b
//
NodePtr TildeParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    auto FirstTilde = TokIn;
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_LOWEST;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest1;
    
    auto FirstTok = TheParser->currentToken(Ctxt);
    FirstTok = TheParser->eatTrivia(FirstTok, Ctxt, ArgsTest1);
    
    if ((Ctxt.Flag & PARSER_INSIDE_TILDE) == PARSER_INSIDE_TILDE) {
        
        auto Last = prefixParselets[FirstTok.Tok.value()]->parse(FirstTok, Ctxt);
        
        return Last;
    }
    
    Ctxt.Flag |= PARSER_INSIDE_TILDE;
    //
    // FIXME: clear other flags here also?
    //
    Ctxt.Flag &= ~(PARSER_INSIDE_COLON);
    
    auto Middle = prefixParselets[FirstTok.Tok.value()]->parse(FirstTok, Ctxt);
    
    LeafSeq ArgsTest2;
    
    auto Tok1 = TheParser->currentToken(Ctxt);
    Tok1 = TheParser->eatTrivia(Tok1, Ctxt, ArgsTest2);
    
    if (Tok1.Tok != TOKEN_TILDE) {
        
        TheParser->nextToken(Tok1);
        
        //
        // Something like   a ~f b
        //
        
        NodeSeq Args(1 + 1 + 1 + 1);
        Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
        Args.append(NodePtr(new LeafNode(FirstTilde)));
        Args.appendIfNonEmpty(std::move(ArgsTest1));
        Args.append(std::move(Middle));
        
        auto Error = NodePtr(new SyntaxErrorNode(SYNTAXERROR_EXPECTEDTILDE, std::move(Args)));
        
        return Error;
    }
    
    LeafSeq ArgsTest3;
    
    //
    // Reset back to "outside" precedence
    //
    Ctxt.Prec = PRECEDENCE_TILDE;
    Ctxt.Flag &= (~PARSER_INSIDE_TILDE);
    
    TheParser->nextToken(Tok1);
    
    auto Tok2 = TheParser->currentToken(Ctxt);
    Tok2 = TheParser->eatTrivia(Tok2, Ctxt, ArgsTest3);
    
    auto Right = prefixParselets[Tok2.Tok.value()]->parse(Tok2, Ctxt);
    
    NodeSeq Args(1 + 1 + 1 + 1 + 1 + 1 + 1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(FirstTilde)));
    Args.appendIfNonEmpty(std::move(ArgsTest1));
    Args.append(std::move(Middle));
    Args.appendIfNonEmpty(std::move(ArgsTest2));
    Args.append(NodePtr(new LeafNode(Tok1)));
    Args.appendIfNonEmpty(std::move(ArgsTest3));
    Args.append(std::move(Right));
    
    auto L = NodePtr(new TernaryNode(SYMBOL_CODEPARSER_TERNARYTILDE, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// Something like  symbol:object
//
// when parsing a in a:b  then ColonFlag is false
// when parsing b in a:b  then ColonFlag is true
//
NodePtr ColonParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    assert((CtxtIn.Flag & PARSER_INSIDE_COLON) != PARSER_INSIDE_COLON);
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_FAKE_PATTERNCOLON;
    Ctxt.Flag |= PARSER_INSIDE_COLON;
    
    TheParser->nextToken(TokIn);
    
    NodePtr Pat;
    {
        LeafSeq ArgsTest1;
        
        auto Tok = TheParser->currentToken(Ctxt);
        Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest1);
        
        auto Right = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
        
        NodeSeq Args(1 + 1 + 1 + 1);
        Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
        Args.append(NodePtr(new LeafNode(TokIn)));
        Args.appendIfNonEmpty(std::move(ArgsTest1));
        Args.append(std::move(Right));
        
        Pat = NodePtr(new BinaryNode(SYMBOL_PATTERN, std::move(Args)));
        
        LeafSeq ArgsTest2;
        
        Tok = TheParser->currentToken(Ctxt);
        Tok = TheParser->eatTriviaButNotToplevelNewlines(Tok, Ctxt, ArgsTest2);
        
        if (Tok.Tok == TOKEN_COLON) {
            
            Ctxt.Flag &= ~(PARSER_INSIDE_COLON);
            
            NodeSeq PatSeq(1 + 1);
            PatSeq.append(std::move(Pat));
            PatSeq.appendIfNonEmpty(std::move(ArgsTest2));
            
            auto L = parseContextSensitive(std::move(PatSeq), Tok, Ctxt);
            return L;
        }
    }
    
    return TheParser->infixLoop(std::move(Pat), CtxtIn);
}

//
// Something like  pattern:optional
//
// Called from other parselets
//
NodePtr ColonParselet::parseContextSensitive(NodeSeq Left, Token TokIn, ParserContext Ctxt) const {
    
    //
    // when parsing a in a:b  then ColonFlag is false
    // when parsing b in a:b  then ColonFlag is true
    //
    assert((Ctxt.Flag & PARSER_INSIDE_COLON) != PARSER_INSIDE_COLON);
    
    Ctxt.Prec = PRECEDENCE_FAKE_OPTIONALCOLON;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest;
    
    auto Tok = TheParser->currentToken(Ctxt);
    Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest);
    
    auto Right = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
    
    NodeSeq Args(1 + 1 + 1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.appendIfNonEmpty(std::move(ArgsTest));
    Args.append(std::move(Right));
    
    auto L = NodePtr(new BinaryNode(SYMBOL_OPTIONAL, std::move(Args)));
    return TheParser->infixLoop(std::move(L), Ctxt);
}


//
// Something like  a /: b = c
//
// a   /:   b   =   c
// ^~~~~ Args at the start
//       ^~~ ArgsTest1
//           ^~~ ArgsTest2
//
NodePtr SlashColonParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_SLASHCOLON;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest1;
    
    auto Tok = TheParser->currentToken(Ctxt);
    Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest1);
    
    auto Middle = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
    
    LeafSeq ArgsTest2;
    
    Tok = TheParser->currentToken(Ctxt);
    Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest2);
    
    switch (Tok.Tok.value()) {
        case TOKEN_EQUAL.value(): {
            
            NodeSeq Args2(1 + 1 + 1 + 1 + 1);
            Args2.append(NodePtr(new NodeSeqNode(std::move(Left))));
            Args2.append(NodePtr(new LeafNode(TokIn)));
            Args2.appendIfNonEmpty(std::move(ArgsTest1));
            Args2.append(std::move(Middle));
            Args2.appendIfNonEmpty(std::move(ArgsTest2));
            
            Ctxt.Flag |= PARSER_INSIDE_SLASHCOLON;
            
            auto N = infixParselets[TOKEN_EQUAL.value()]->parse(std::move(Args2), Tok, Ctxt);
            
            return N;
        }
        case TOKEN_COLONEQUAL.value(): {
            
            NodeSeq Args2(1 + 1 + 1 + 1 + 1);
            Args2.append(NodePtr(new NodeSeqNode(std::move(Left))));
            Args2.append(NodePtr(new LeafNode(TokIn)));
            Args2.appendIfNonEmpty(std::move(ArgsTest1));
            Args2.append(std::move(Middle));
            Args2.appendIfNonEmpty(std::move(ArgsTest2));
            
            Ctxt.Flag |= PARSER_INSIDE_SLASHCOLON;
            
            auto N = infixParselets[TOKEN_COLONEQUAL.value()]->parse(std::move(Args2), Tok, Ctxt);
            
            return N;
        }
        case TOKEN_EQUALDOT.value(): {
            
            NodeSeq Args2(1 + 1 + 1 + 1 + 1);
            Args2.append(NodePtr(new NodeSeqNode(std::move(Left))));
            Args2.append(NodePtr(new LeafNode(TokIn)));
            Args2.appendIfNonEmpty(std::move(ArgsTest1));
            Args2.append(std::move(Middle));
            Args2.appendIfNonEmpty(std::move(ArgsTest2));
            
            Ctxt.Flag |= PARSER_INSIDE_SLASHCOLON;
            
            auto N = infixParselets[TOKEN_EQUALDOT.value()]->parse(std::move(Args2), Tok, Ctxt);
            
            return N;
        }
        case TOKEN_ENDOFFILE.value(): {
            
            NodeSeq Args(1 + 1 + 1 + 1);
            Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
            Args.append(NodePtr(new LeafNode(TokIn)));
            Args.appendIfNonEmpty(std::move(ArgsTest1));
            Args.append(std::move(Middle));
            
            auto Error = NodePtr(new SyntaxErrorNode(SYNTAXERROR_EXPECTEDSET, std::move(Args)));
            
            return Error;
        }
        default: {
            
            //
            // Anything other than:
            // a /: b = c
            // a /: b := c
            // a /: b =.
            // a /: b <EOF>
            //
            
            TheParser->nextToken(Tok);
            
            NodeSeq Args(1 + 1 + 1 + 1 + 1 + 1);
            Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
            Args.append(NodePtr(new LeafNode(TokIn)));
            Args.appendIfNonEmpty(std::move(ArgsTest1));
            Args.append(std::move(Middle));
            Args.appendIfNonEmpty(std::move(ArgsTest2));
            Args.append(NodePtr(new LeafNode(Tok)));
            
            auto Error = NodePtr(new SyntaxErrorNode(SYNTAXERROR_EXPECTEDSET, std::move(Args)));
            
            return Error;
        }
    }
}


//
// Something like  \( x \)
//
NodePtr LinearSyntaxOpenParenParselet::parse(Token firstTok, ParserContext CtxtIn) const {
    
    auto Opener = firstTok;
    
    TheParser->nextToken(firstTok);
    
    auto Ctxt = CtxtIn;
    Ctxt.Closr = CLOSER_LINEARSYNTAX_CLOSEPAREN;
    
    NodeSeq Args(1);
    Args.append(NodePtr(new LeafNode(Opener)));
    
    NodePtr group;
    
    while (true) {
    
#if !NABORT
        //
        // Check isAbort() inside loops
        //
        if (TheParserSession->isAbort()) {
            
            return TheParserSession->handleAbort();
        }
#endif // !NABORT
        
        auto Tok = TheParser->currentToken(Ctxt);
        
        if (Tok.Tok == TOKEN_ENDOFFILE) {
            
            //
            // Handle something like   \( a EOF
            //
            
            group = NodePtr(new GroupMissingCloserNeedsReparseNode(SYMBOL_CODEPARSER_GROUPLINEARSYNTAXPAREN, std::move(Args)));
            
            break;
        }
        if (Tok.Tok == TOKEN_LINEARSYNTAX_CLOSEPAREN) {
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.append(NodePtr(new LeafNode(Tok)));
            
            TheParser->nextToken(Tok);
            
            group = NodePtr(new GroupNode(SYMBOL_CODEPARSER_GROUPLINEARSYNTAXPAREN, std::move(Args)));
            
            break;
        }
        
        //
        // Do not check for other closers here
        //
        // As long as \( \) parses by just doing tokenization, then cannot reliably test for other closers
        //
        // e.g., \( ( \)  and  \( ) \)  is completely valid syntax
        //
        
        if (Tok.Tok == TOKEN_LINEARSYNTAX_OPENPAREN) {
            
            auto Sub = this->parse(Tok, Ctxt);
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.append(std::move(Sub));
            
            Tok = TheParser->currentToken(Ctxt);
            
        } else {
            
            //
            // COMMENT, WHITESPACE, and NEWLINE are handled here
            //
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.append(NodePtr(new LeafNode(Tok)));
            
            TheParser->nextToken(Tok);
            
            Tok = TheParser->currentToken(Ctxt);
        }
        
    } // while
    
    return TheParser->infixLoop(std::move(group), CtxtIn);
}


//
// a /: b = c  and  a /: b = .  are handled here
//
NodePtr EqualParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_EQUAL;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest;
    
    auto Tok = TheParser->currentToken(Ctxt);
    Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest);
    
    if (Tok.Tok == TOKEN_DOT) {
        
        //
        // Something like a = .
        //
        // tutorial/OperatorInputForms
        // Spaces to Avoid
        //
        
        TheParser->nextToken(Tok);
        
        NodeSeq Args(1 + 1 + 1 + 1);
        Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
        Args.append(NodePtr(new LeafNode(TokIn)));
        Args.appendIfNonEmpty(std::move(ArgsTest));
        Args.append(NodePtr(new LeafNode(Tok)));
        
        if ((Ctxt.Flag & PARSER_INSIDE_SLASHCOLON) == PARSER_INSIDE_SLASHCOLON) {
            
            auto L = NodePtr(new TernaryNode(SYMBOL_TAGUNSET, std::move(Args)));
            return TheParser->infixLoop(std::move(L), CtxtIn);
        }
        
        auto L = NodePtr(new BinaryNode(SYMBOL_UNSET, std::move(Args)));
        return TheParser->infixLoop(std::move(L), CtxtIn);
    }
    
    auto wasInsideSlashColon = ((Ctxt.Flag & PARSER_INSIDE_SLASHCOLON) == PARSER_INSIDE_SLASHCOLON);
    
    Ctxt.Flag &= ~(PARSER_INSIDE_SLASHCOLON);
    
    auto Right = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
    
    NodeSeq Args(1 + 1 + 1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.appendIfNonEmpty(std::move(ArgsTest));
    Args.append(std::move(Right));
    
    if (wasInsideSlashColon) {
        auto L = NodePtr(new TernaryNode(SYMBOL_TAGSET, std::move(Args)));
        return TheParser->infixLoop(std::move(L), CtxtIn);
    }
    
    auto L = NodePtr(new BinaryNode(SYMBOL_SET, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// a /: b := c  is handled here
//
NodePtr ColonEqualParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_EQUAL;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest;
    
    auto Tok = TheParser->currentToken(Ctxt);
    Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest);
    
    auto wasInsideSlashColon = ((Ctxt.Flag & PARSER_INSIDE_SLASHCOLON) == PARSER_INSIDE_SLASHCOLON);
    
    Ctxt.Flag &= ~(PARSER_INSIDE_SLASHCOLON);
    
    auto Right = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
    
    NodeSeq Args(1 + 1 + 1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.appendIfNonEmpty(std::move(ArgsTest));
    Args.append(std::move(Right));
    
    if (wasInsideSlashColon) {
        
        auto L = NodePtr(new TernaryNode(SYMBOL_TAGSETDELAYED, std::move(Args)));
        return TheParser->infixLoop(std::move(L), CtxtIn);
    }
    
    auto L = NodePtr(new BinaryNode(SYMBOL_SETDELAYED, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// Something like  a =.
//
// a /: b =.  is also handled here
//
NodePtr EqualDotParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_EQUAL;
    
    TheParser->nextToken(TokIn);
    
    NodeSeq Args(1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(TokIn)));
    
    if ((Ctxt.Flag & PARSER_INSIDE_SLASHCOLON) == PARSER_INSIDE_SLASHCOLON) {
        
        auto L = NodePtr(new TernaryNode(SYMBOL_TAGUNSET, std::move(Args)));
        return TheParser->infixLoop(std::move(L), CtxtIn);
    }
    
    auto L = NodePtr(new BinaryNode(SYMBOL_UNSET, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// Something like  \[Integral] f \[DifferentialD] x
//
NodePtr IntegralParselet::parse(Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_CLASS_INTEGRATIONOPERATORS;
    Ctxt.Flag |= PARSER_INSIDE_INTEGRAL;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest1;
    
    auto Tok = TheParser->currentToken(Ctxt);
    Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest1);
    
    auto operand = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
    
    Ctxt.Flag &= ~(PARSER_INSIDE_INTEGRAL);
    
    LeafSeq ArgsTest2;
    
    Tok = TheParser->currentToken(Ctxt);
    Tok = TheParser->eatTrivia(Tok, Ctxt, ArgsTest2);
    
    if (!Tok.Tok.isDifferentialD()) {
        
        NodeSeq Args(1 + 1 + 1);
        Args.append(NodePtr(new LeafNode(TokIn)));
        Args.appendIfNonEmpty(std::move(ArgsTest1));
        Args.append(std::move(operand));
        
        auto L = NodePtr(new PrefixNode(Op1, std::move(Args)));
        return TheParser->infixLoop(std::move(L), CtxtIn);
    }

    auto variable = prefixParselets[Tok.Tok.value()]->parse(Tok, Ctxt);
    
    NodeSeq Args(1 + 1 + 1 + 1 + 1);
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.appendIfNonEmpty(std::move(ArgsTest1));
    Args.append(std::move(operand));
    Args.appendIfNonEmpty(std::move(ArgsTest2));
    Args.append(std::move(variable));
    
    auto L = NodePtr(new PrefixBinaryNode(Op2, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


NodePtr InfixOperatorWithTrailingParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    NodeSeq Args(1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    
    auto lastOperatorToken = TokIn;
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = getPrecedence(Ctxt);
    
    NodePtr L;
    
    while (true) {
        
#if !NABORT
        //
        // Check isAbort() inside loops
        //
        if (TheParserSession->isAbort()) {
            
            return TheParserSession->handleAbort();
        }
#endif // !NABORT
        
        LeafSeq ArgsTest1;
        
        auto Tok1 = TheParser->currentToken(Ctxt);
        Tok1 = TheParser->eatTriviaButNotToplevelNewlines(Tok1, Ctxt, ArgsTest1);
        
        //
        // Cannot just compare tokens
        //
        // May be something like  a,b\[InvisibleComma]c
        //
        // and we want only a single Infix node created
        //
        if (infixParselets[Tok1.Tok.value()]->getOp() != Op) {
            
            L = NodePtr(new InfixNode(Op, std::move(Args)));
            
            break;
        }
            
        lastOperatorToken = Tok1;
        
        //
        // Something like  a;b  or  a,b
        //
        
        TheParser->nextToken(Tok1);
        
        LeafSeq ArgsTest2;
        
        auto Tok2 = TheParser->currentToken(Ctxt);
        Tok2 = TheParser->eatTriviaButNotToplevelNewlines(Tok2, Ctxt, ArgsTest2);
        
        if (infixParselets[Tok2.Tok.value()]->getOp() == Op) {
            
            //
            // Something like  a; ;
            //
            
            auto Implicit = Token(TOKEN_FAKE_IMPLICITNULL, BufferAndLength(lastOperatorToken.BufLen.end), Source(lastOperatorToken.Src.End));
            
            lastOperatorToken = Tok2;
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.appendIfNonEmpty(std::move(ArgsTest1));
            Args.append(NodePtr(new LeafNode(Tok1)));
            Args.append(NodePtr(new LeafNode(Implicit)));
            
        } else if (Tok2.Tok.isPossibleBeginning()) {
            
            auto operand = prefixParselets[Tok2.Tok.value()]->parse(Tok2, Ctxt);
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.appendIfNonEmpty(std::move(ArgsTest1));
            Args.append(NodePtr(new LeafNode(Tok1)));
            Args.appendIfNonEmpty(std::move(ArgsTest2));
            Args.append(std::move(operand));
            
        } else {
            
            //
            // Not beginning of an expression
            //
            // For example:  a;&
            //
            
            auto Implicit = Token(TOKEN_FAKE_IMPLICITNULL, BufferAndLength(lastOperatorToken.BufLen.end), Source(lastOperatorToken.Src.End));
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.appendIfNonEmpty(std::move(ArgsTest1));
            Args.append(NodePtr(new LeafNode(Tok1)));
            Args.append(NodePtr(new LeafNode(Implicit)));
            
            L = NodePtr(new InfixNode(Op, std::move(Args)));
            
            break;
        }
        
    } // while
    
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// a::b
//
NodePtr ColonColonParselet::parse(NodeSeq Left, Token TokIn, ParserContext Ctxt) const {
    
    NodeSeq Args(1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    
    //
    // Not used here because of the special rules for tokenizing after ::
    //
    //auto Ctxt = CtxtIn;
    //Ctxt.Prec = PRECEDENCE_COLONCOLON;
    //Ctxt.Assoc = ASSOCIATIVITY_NONE;
    
    while (true) {
        
#if !NABORT
        //
        // Check isAbort() inside loops
        //
        if (TheParserSession->isAbort()) {
            
            return TheParserSession->handleAbort();
        }
#endif // !NABORT
        
        LeafSeq ArgsTest1;
        
        auto Tok1 = TheParser->currentToken(Ctxt);
        Tok1 = TheParser->eatTriviaButNotToplevelNewlines(Tok1, Ctxt, ArgsTest1);
        
        if (Tok1.Tok == TOKEN_COLONCOLON) {
            
            TheParser->nextToken(Tok1);
            
            //
            // Special tokenization, so must do parsing here
            //
            
            auto Tok2 = TheParser->currentToken_stringifySymbol();
            
            TheParser->nextToken_stringifySymbol();
            
            bool possible;
            if (Tok2.Tok.isPossibleBeginning()) {
                assert(Tok2.Tok == TOKEN_STRING);
                possible = true;
            } else {
                assert(Tok2.Tok.isError());
                possible = false;
            }
            
            NodePtr Operand;
            if (possible) {
                Operand = NodePtr(new LeafNode(std::move(Tok2)));
            } else {
                Operand = NodePtr(new ErrorNode(std::move(Tok2)));
            }
            
            
            //
            // Do not reserve inside loop
            // Allow default resizing strategy, which is hopefully exponential
            //
            Args.appendIfNonEmpty(std::move(ArgsTest1));
            Args.append(NodePtr(new LeafNode(Tok1)));
            Args.append(std::move(Operand));
            
            if (!possible) {
                auto L = NodePtr(new InfixNode(SYMBOL_MESSAGENAME, std::move(Args)));
                return TheParser->infixLoop(std::move(L), Ctxt);
            }
            
        } else {
            
            //
            // Tok.Tok != TokIn.Tok, so break
            //
            
            auto L = NodePtr(new InfixNode(SYMBOL_MESSAGENAME, std::move(Args)));
            return TheParser->infixLoop(std::move(L), Ctxt);
        }
        
    } // while
}


//
// a>>b
//
NodePtr GreaterGreaterParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_GREATERGREATER;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest;
    
    //
    // Special tokenization, so must do parsing here
    //
    
    auto Tok = TheParser->currentToken_stringifyFile();
    Tok = TheParser->eatTrivia_stringifyFile(Tok, Ctxt, ArgsTest);
    
    TheParser->nextToken_stringifyFile();

    auto Operand = NodePtr(new LeafNode(std::move(Tok)));
    
    NodeSeq Args(1 + 1 + 1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.appendIfNonEmpty(std::move(ArgsTest));
    Args.append(std::move(Operand));
    
    auto L = NodePtr(new BinaryNode(SYMBOL_PUT, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// a>>>b
//
NodePtr GreaterGreaterGreaterParselet::parse(NodeSeq Left, Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_GREATERGREATERGREATER;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest;
    
    //
    // Special tokenization, so must do parsing here
    //
    
    auto Tok = TheParser->currentToken_stringifyFile();
    Tok = TheParser->eatTrivia_stringifyFile(Tok, Ctxt, ArgsTest);
    
    TheParser->nextToken_stringifyFile();
    
    auto Operand = NodePtr(new LeafNode(std::move(Tok)));
    
    NodeSeq Args(1 + 1 + 1 + 1);
    Args.append(NodePtr(new NodeSeqNode(std::move(Left))));
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.appendIfNonEmpty(std::move(ArgsTest));
    Args.append(std::move(Operand));
    
    auto L = NodePtr(new BinaryNode(SYMBOL_PUTAPPEND, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// <<a
//
NodePtr LessLessParselet::parse(Token TokIn, ParserContext CtxtIn) const {
    
    auto Ctxt = CtxtIn;
    Ctxt.Prec = PRECEDENCE_LESSLESS;
    
    TheParser->nextToken(TokIn);
    
    LeafSeq ArgsTest;
    
    //
    // Special tokenization, so must do parsing here
    //
    
    auto Tok = TheParser->currentToken_stringifyFile();
    Tok = TheParser->eatTrivia_stringifyFile(Tok, Ctxt, ArgsTest);
    
    TheParser->nextToken_stringifyFile();
    
    auto Operand = NodePtr(new LeafNode(std::move(Tok)));
    
    NodeSeq Args(1 + 1 + 1);
    Args.append(NodePtr(new LeafNode(TokIn)));
    Args.appendIfNonEmpty(std::move(ArgsTest));
    Args.append(std::move(Operand));
    
    auto L = NodePtr(new PrefixNode(SYMBOL_GET, std::move(Args)));
    return TheParser->infixLoop(std::move(L), CtxtIn);
}


//
// InfixDifferentialDParselet only exists to properly supply precedence, depending on context
//
Precedence InfixDifferentialDParselet::getPrecedence(ParserContext Ctxt) const {
    
    if ((Ctxt.Flag & PARSER_INSIDE_INTEGRAL) == PARSER_INSIDE_INTEGRAL) {

        //
        // Inside \[Integral], so \[DifferentialD] is treated specially
        //

        return PRECEDENCE_LOWEST;
    }

    return PRECEDENCE_FAKE_IMPLICITTIMES;
}

//
// Does not "parse" so much as inserts the implicit Times token, and then allows the rest of the system to do the work
//
NodePtr InfixDifferentialDParselet::parse(NodeSeq Left, Token firstTok, ParserContext Ctxt) const {
    
    auto token = Token(TOKEN_FAKE_IMPLICITTIMES, BufferAndLength(firstTok.BufLen.buffer), Source(firstTok.Src.Start));
    
    TheParser->prepend(token);
    
    auto L = NodePtr(new NodeSeqNode(std::move(Left)));
    return L;
}
