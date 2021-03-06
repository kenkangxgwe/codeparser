
#pragma once

#include "WLCharacter.h" // for WLCharacter
#include "Source.h" // for Source

#include <string>
#include <unordered_set> // for unordered_set

extern std::unordered_set<std::string> undocumentedLongNames;


std::ostream& set_graphical(std::ostream& stream);

std::ostream& clear_graphical(std::ostream& stream);

int get_graphical_i();



class Utils {
public:
    
    static bool isStrangeLetterlikeLongName(std::string s);
    
    static bool isVeryStrangeLetterlikeLongName(std::string s);
    
    static bool isUndocumentedLongName(std::string s);
    
    static bool isMBNonCharacter(codepoint point);
    
#if !NISSUES
    static void strangeLetterlikeWarning(Source Src, WLCharacter c);
    
    static void mbStrangeLetterlikeWarning(Source Src, WLCharacter c);
    
    static bool isStrange(codepoint point);
    
    static bool isMBStrange(codepoint point);
#endif // !NISSUES
    
    static uint8_t toDigit(unsigned char val);
    
    static SourceConvention parseSourceConvention(std::string s);
};

