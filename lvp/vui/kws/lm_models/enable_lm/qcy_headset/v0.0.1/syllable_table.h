/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * syllable_table.h:
 *
 */

#ifndef __SYLLABLE_TABLE_H__
#define __SYLLABLE_TABLE_H__

static const char *g_syllable_table[] __attribute__((unused)) = {
    "''", "b", "p", "m", "f", "d", "t", "n", "l", "g",/*0-9*/
    "k", "h", "j", "q", "x", "z", "c", "s", "r", "w",/*10-19*/
    "y", "i", "u", "v", "a", "ia", "ua", "o", "uo",/*20-28*/
    "e", "ie", "ai", "uai", "ei", "van", "ue", "ei",/*29-36*/
    "ao", "iao", "ou", "ing", "ou", "ian", "uan", "an",/*37-44*/
    "en", "in", "uen", "iang", "uang", "eng", "ong",/*45-51*/
    "iong", "er", "oov", "", "ang", "ch", "", "",/*52-59*/
    "sh", "un", "y", "zh", "#"/*60-64*/
};
/*{
    "''", "b", "p", "m", "f", "d", "t", "n", "l", "g",
    "k", "h", "j", "q", "x", "z", "c", "s", "r", "uu",
    "ii", "i", "u", "v", "a", "ia", "ua", "o", "uo",
    "e", "ie", "ai", "uai", "ei", "van", "ve", "uei",
    "ao", "iao", "ou", "ing", "iou", "ian", "uan", "an",
    "en", "in", "uen", "iang", "uang", "eng", "ong",
    "iong", "er", "oov", "aa", "ang", "ch", "ee", "oo",
    "sh", "vn", "vv", "zh", "#"
};*/

static const char *g_syllable_table_fuzzy[] __attribute__((unused)) = {
    "''", "[bpl]", "[pbxj]", "m", "[fhb]", "[dtf]", "[tdx]", "n", "[lrh]", "[gkh]",
    "[kg]", "[hf]", "[jtqx]", "[qjx]", "[xjq]", "zh?", "ch?", "sh?", "[rld]", "uu",
    "ii", "i", "u", "v", "a", "ia", "ua", "o", "uo",
    "e", "ie", "ai", "uai", "ei?", "van", "ve", "uei",
    "ao", "iao", "ou", "ing?", "iou", "iang?", "uang?", "ang?",
    "eng?", "ing?", "uen", "iang?", "uang?", "eng?", "ong?",
    "iong", "er", "oov", "aa", "ang?", "ch", "ee", "oo",
    "[sz]h", "vn", "vv", "[zs]h", "#"
};



#endif /* __SYLLABLE_TABLE_H__ */

