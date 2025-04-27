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
    "''", "b", "p", "m", "f", "d", "t", "n", "l", "g",
    "k", "h", "j", "q", "x", "z", "c", "s", "r", "uu",
    "ii", "i", "u", "v", "a", "ia", "ua", "o", "uo",
    "e", "ie", "ai", "uai", "ei", "van", "ve", "uei",
    "ao", "iao", "ou", "ing", "iou", "ian", "uan", "an",
    "en", "in", "uen", "iang", "uang", "eng", "ong",
    "iong", "er", "oov", "aa", "ang", "ch", "ee", "oo",
    "sh", "vn", "vv", "zh", "#"
};

static const char *g_syllable_table_fuzzy[] __attribute__((unused)) = {
    "''", "[bplfdgh]", "[pbxjh]", "m", "[fhb]", "[dtfbjzn]", "[tdxpf]", "[nmdl]", "[lrhn]", "[gkh]",
    "[kgt]", "[hf]", "[jqxz]", "[qjx]", "[xjqs]h?", "[zbd]h?", "[czs]h?", "[szcf]h?", "[rld]", "[uml]u?",
    "[isz]i?", "[iv]", "u", "[vi]", "a", "iao?", "ua", "u?o", "uo",
    "[ea]", "ie", "a+[in]?", "uai", "u?ei", "van", "ve", "u?ei",
    "ao", "iao", "a?o+u?", "[ieo]ng?", "iou", "iang?", "uan?g?", "u?ang?",
    "[ie]a?ng?", "in?g?", "uen", "iang?", "uang?", "eng?", "ou?n?g?",
    "iong", "er", "oov", "aa", "ang?", "[cqsz]h?", "ee", "oo",
    "[szf]h?", "i*v?[nvr]?g?", "i*v?[nv]?g?", "[zs]?d?h?", "#"
};
#endif /* __SYLLABLE_TABLE_H__ */
