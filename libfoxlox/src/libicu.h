#pragma once

#ifdef FOXLOX_USE_WINSDK_ICU
import <icu.h>;
#pragma comment(lib, "icu.lib") 
#else
import <unicode/ustring.h>;
import <unicode/uchar.h>;
#endif

#undef FALSE
#undef TRUE