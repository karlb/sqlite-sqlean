// Originally from the spellfix SQLite exension, Public Domain
// https://www.sqlite.org/src/file/ext/misc/spellfix.c
// Modified by Anton Zhiyanov, https://github.com/nalgeon/sqlean/, MIT License

#include <stdlib.h>

#include "common.h"

extern const unsigned char midClass[];
extern const unsigned char initClass[];
extern const unsigned char className[];

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
static const unsigned char sqlite3Utf8Trans1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

/*
** Return the value of the first UTF-8 character in the string.
*/
static int utf8Read(const unsigned char* z, int n, int* pSize) {
    int c, i;

    /* All callers to this routine (in the current implementation)
    ** always have n>0. */
    if (NEVER(n == 0)) {
        c = i = 0;
    } else {
        c = z[0];
        i = 1;
        if (c >= 0xc0) {
            c = sqlite3Utf8Trans1[c - 0xc0];
            while (i < n && (z[i] & 0xc0) == 0x80) {
                c = (c << 6) + (0x3f & z[i++]);
            }
        }
    }
    *pSize = i;
    return c;
}

typedef struct Transliteration Transliteration;
struct Transliteration {
    unsigned short int cFrom;
    unsigned char cTo0, cTo1, cTo2, cTo3;
};

/*
** Table of translations from unicode characters into ASCII.
*/
static const Transliteration translit[] = {
    {0x00A0, 0x20, 0x00, 0x00, 0x00}, /*   to   */
    {0x00B5, 0x75, 0x00, 0x00, 0x00}, /* µ to u */
    {0x00C0, 0x41, 0x00, 0x00, 0x00}, /* À to A */
    {0x00C1, 0x41, 0x00, 0x00, 0x00}, /* Á to A */
    {0x00C2, 0x41, 0x00, 0x00, 0x00}, /* Â to A */
    {0x00C3, 0x41, 0x00, 0x00, 0x00}, /* Ã to A */
    {0x00C4, 0x41, 0x65, 0x00, 0x00}, /* Ä to Ae */
    {0x00C5, 0x41, 0x61, 0x00, 0x00}, /* Å to Aa */
    {0x00C6, 0x41, 0x45, 0x00, 0x00}, /* Æ to AE */
    {0x00C7, 0x43, 0x00, 0x00, 0x00}, /* Ç to C */
    {0x00C8, 0x45, 0x00, 0x00, 0x00}, /* È to E */
    {0x00C9, 0x45, 0x00, 0x00, 0x00}, /* É to E */
    {0x00CA, 0x45, 0x00, 0x00, 0x00}, /* Ê to E */
    {0x00CB, 0x45, 0x00, 0x00, 0x00}, /* Ë to E */
    {0x00CC, 0x49, 0x00, 0x00, 0x00}, /* Ì to I */
    {0x00CD, 0x49, 0x00, 0x00, 0x00}, /* Í to I */
    {0x00CE, 0x49, 0x00, 0x00, 0x00}, /* Î to I */
    {0x00CF, 0x49, 0x00, 0x00, 0x00}, /* Ï to I */
    {0x00D0, 0x44, 0x00, 0x00, 0x00}, /* Ð to D */
    {0x00D1, 0x4E, 0x00, 0x00, 0x00}, /* Ñ to N */
    {0x00D2, 0x4F, 0x00, 0x00, 0x00}, /* Ò to O */
    {0x00D3, 0x4F, 0x00, 0x00, 0x00}, /* Ó to O */
    {0x00D4, 0x4F, 0x00, 0x00, 0x00}, /* Ô to O */
    {0x00D5, 0x4F, 0x00, 0x00, 0x00}, /* Õ to O */
    {0x00D6, 0x4F, 0x65, 0x00, 0x00}, /* Ö to Oe */
    {0x00D7, 0x78, 0x00, 0x00, 0x00}, /* × to x */
    {0x00D8, 0x4F, 0x00, 0x00, 0x00}, /* Ø to O */
    {0x00D9, 0x55, 0x00, 0x00, 0x00}, /* Ù to U */
    {0x00DA, 0x55, 0x00, 0x00, 0x00}, /* Ú to U */
    {0x00DB, 0x55, 0x00, 0x00, 0x00}, /* Û to U */
    {0x00DC, 0x55, 0x65, 0x00, 0x00}, /* Ü to Ue */
    {0x00DD, 0x59, 0x00, 0x00, 0x00}, /* Ý to Y */
    {0x00DE, 0x54, 0x68, 0x00, 0x00}, /* Þ to Th */
    {0x00DF, 0x73, 0x73, 0x00, 0x00}, /* ß to ss */
    {0x00E0, 0x61, 0x00, 0x00, 0x00}, /* à to a */
    {0x00E1, 0x61, 0x00, 0x00, 0x00}, /* á to a */
    {0x00E2, 0x61, 0x00, 0x00, 0x00}, /* â to a */
    {0x00E3, 0x61, 0x00, 0x00, 0x00}, /* ã to a */
    {0x00E4, 0x61, 0x65, 0x00, 0x00}, /* ä to ae */
    {0x00E5, 0x61, 0x61, 0x00, 0x00}, /* å to aa */
    {0x00E6, 0x61, 0x65, 0x00, 0x00}, /* æ to ae */
    {0x00E7, 0x63, 0x00, 0x00, 0x00}, /* ç to c */
    {0x00E8, 0x65, 0x00, 0x00, 0x00}, /* è to e */
    {0x00E9, 0x65, 0x00, 0x00, 0x00}, /* é to e */
    {0x00EA, 0x65, 0x00, 0x00, 0x00}, /* ê to e */
    {0x00EB, 0x65, 0x00, 0x00, 0x00}, /* ë to e */
    {0x00EC, 0x69, 0x00, 0x00, 0x00}, /* ì to i */
    {0x00ED, 0x69, 0x00, 0x00, 0x00}, /* í to i */
    {0x00EE, 0x69, 0x00, 0x00, 0x00}, /* î to i */
    {0x00EF, 0x69, 0x00, 0x00, 0x00}, /* ï to i */
    {0x00F0, 0x64, 0x00, 0x00, 0x00}, /* ð to d */
    {0x00F1, 0x6E, 0x00, 0x00, 0x00}, /* ñ to n */
    {0x00F2, 0x6F, 0x00, 0x00, 0x00}, /* ò to o */
    {0x00F3, 0x6F, 0x00, 0x00, 0x00}, /* ó to o */
    {0x00F4, 0x6F, 0x00, 0x00, 0x00}, /* ô to o */
    {0x00F5, 0x6F, 0x00, 0x00, 0x00}, /* õ to o */
    {0x00F6, 0x6F, 0x65, 0x00, 0x00}, /* ö to oe */
    {0x00F7, 0x3A, 0x00, 0x00, 0x00}, /* ÷ to : */
    {0x00F8, 0x6F, 0x00, 0x00, 0x00}, /* ø to o */
    {0x00F9, 0x75, 0x00, 0x00, 0x00}, /* ù to u */
    {0x00FA, 0x75, 0x00, 0x00, 0x00}, /* ú to u */
    {0x00FB, 0x75, 0x00, 0x00, 0x00}, /* û to u */
    {0x00FC, 0x75, 0x65, 0x00, 0x00}, /* ü to ue */
    {0x00FD, 0x79, 0x00, 0x00, 0x00}, /* ý to y */
    {0x00FE, 0x74, 0x68, 0x00, 0x00}, /* þ to th */
    {0x00FF, 0x79, 0x00, 0x00, 0x00}, /* ÿ to y */
    {0x0100, 0x41, 0x00, 0x00, 0x00}, /* Ā to A */
    {0x0101, 0x61, 0x00, 0x00, 0x00}, /* ā to a */
    {0x0102, 0x41, 0x00, 0x00, 0x00}, /* Ă to A */
    {0x0103, 0x61, 0x00, 0x00, 0x00}, /* ă to a */
    {0x0104, 0x41, 0x00, 0x00, 0x00}, /* Ą to A */
    {0x0105, 0x61, 0x00, 0x00, 0x00}, /* ą to a */
    {0x0106, 0x43, 0x00, 0x00, 0x00}, /* Ć to C */
    {0x0107, 0x63, 0x00, 0x00, 0x00}, /* ć to c */
    {0x0108, 0x43, 0x68, 0x00, 0x00}, /* Ĉ to Ch */
    {0x0109, 0x63, 0x68, 0x00, 0x00}, /* ĉ to ch */
    {0x010A, 0x43, 0x00, 0x00, 0x00}, /* Ċ to C */
    {0x010B, 0x63, 0x00, 0x00, 0x00}, /* ċ to c */
    {0x010C, 0x43, 0x00, 0x00, 0x00}, /* Č to C */
    {0x010D, 0x63, 0x00, 0x00, 0x00}, /* č to c */
    {0x010E, 0x44, 0x00, 0x00, 0x00}, /* Ď to D */
    {0x010F, 0x64, 0x00, 0x00, 0x00}, /* ď to d */
    {0x0110, 0x44, 0x00, 0x00, 0x00}, /* Đ to D */
    {0x0111, 0x64, 0x00, 0x00, 0x00}, /* đ to d */
    {0x0112, 0x45, 0x00, 0x00, 0x00}, /* Ē to E */
    {0x0113, 0x65, 0x00, 0x00, 0x00}, /* ē to e */
    {0x0114, 0x45, 0x00, 0x00, 0x00}, /* Ĕ to E */
    {0x0115, 0x65, 0x00, 0x00, 0x00}, /* ĕ to e */
    {0x0116, 0x45, 0x00, 0x00, 0x00}, /* Ė to E */
    {0x0117, 0x65, 0x00, 0x00, 0x00}, /* ė to e */
    {0x0118, 0x45, 0x00, 0x00, 0x00}, /* Ę to E */
    {0x0119, 0x65, 0x00, 0x00, 0x00}, /* ę to e */
    {0x011A, 0x45, 0x00, 0x00, 0x00}, /* Ě to E */
    {0x011B, 0x65, 0x00, 0x00, 0x00}, /* ě to e */
    {0x011C, 0x47, 0x68, 0x00, 0x00}, /* Ĝ to Gh */
    {0x011D, 0x67, 0x68, 0x00, 0x00}, /* ĝ to gh */
    {0x011E, 0x47, 0x00, 0x00, 0x00}, /* Ğ to G */
    {0x011F, 0x67, 0x00, 0x00, 0x00}, /* ğ to g */
    {0x0120, 0x47, 0x00, 0x00, 0x00}, /* Ġ to G */
    {0x0121, 0x67, 0x00, 0x00, 0x00}, /* ġ to g */
    {0x0122, 0x47, 0x00, 0x00, 0x00}, /* Ģ to G */
    {0x0123, 0x67, 0x00, 0x00, 0x00}, /* ģ to g */
    {0x0124, 0x48, 0x68, 0x00, 0x00}, /* Ĥ to Hh */
    {0x0125, 0x68, 0x68, 0x00, 0x00}, /* ĥ to hh */
    {0x0126, 0x48, 0x00, 0x00, 0x00}, /* Ħ to H */
    {0x0127, 0x68, 0x00, 0x00, 0x00}, /* ħ to h */
    {0x0128, 0x49, 0x00, 0x00, 0x00}, /* Ĩ to I */
    {0x0129, 0x69, 0x00, 0x00, 0x00}, /* ĩ to i */
    {0x012A, 0x49, 0x00, 0x00, 0x00}, /* Ī to I */
    {0x012B, 0x69, 0x00, 0x00, 0x00}, /* ī to i */
    {0x012C, 0x49, 0x00, 0x00, 0x00}, /* Ĭ to I */
    {0x012D, 0x69, 0x00, 0x00, 0x00}, /* ĭ to i */
    {0x012E, 0x49, 0x00, 0x00, 0x00}, /* Į to I */
    {0x012F, 0x69, 0x00, 0x00, 0x00}, /* į to i */
    {0x0130, 0x49, 0x00, 0x00, 0x00}, /* İ to I */
    {0x0131, 0x69, 0x00, 0x00, 0x00}, /* ı to i */
    {0x0132, 0x49, 0x4A, 0x00, 0x00}, /* Ĳ to IJ */
    {0x0133, 0x69, 0x6A, 0x00, 0x00}, /* ĳ to ij */
    {0x0134, 0x4A, 0x68, 0x00, 0x00}, /* Ĵ to Jh */
    {0x0135, 0x6A, 0x68, 0x00, 0x00}, /* ĵ to jh */
    {0x0136, 0x4B, 0x00, 0x00, 0x00}, /* Ķ to K */
    {0x0137, 0x6B, 0x00, 0x00, 0x00}, /* ķ to k */
    {0x0138, 0x6B, 0x00, 0x00, 0x00}, /* ĸ to k */
    {0x0139, 0x4C, 0x00, 0x00, 0x00}, /* Ĺ to L */
    {0x013A, 0x6C, 0x00, 0x00, 0x00}, /* ĺ to l */
    {0x013B, 0x4C, 0x00, 0x00, 0x00}, /* Ļ to L */
    {0x013C, 0x6C, 0x00, 0x00, 0x00}, /* ļ to l */
    {0x013D, 0x4C, 0x00, 0x00, 0x00}, /* Ľ to L */
    {0x013E, 0x6C, 0x00, 0x00, 0x00}, /* ľ to l */
    {0x013F, 0x4C, 0x2E, 0x00, 0x00}, /* Ŀ to L. */
    {0x0140, 0x6C, 0x2E, 0x00, 0x00}, /* ŀ to l. */
    {0x0141, 0x4C, 0x00, 0x00, 0x00}, /* Ł to L */
    {0x0142, 0x6C, 0x00, 0x00, 0x00}, /* ł to l */
    {0x0143, 0x4E, 0x00, 0x00, 0x00}, /* Ń to N */
    {0x0144, 0x6E, 0x00, 0x00, 0x00}, /* ń to n */
    {0x0145, 0x4E, 0x00, 0x00, 0x00}, /* Ņ to N */
    {0x0146, 0x6E, 0x00, 0x00, 0x00}, /* ņ to n */
    {0x0147, 0x4E, 0x00, 0x00, 0x00}, /* Ň to N */
    {0x0148, 0x6E, 0x00, 0x00, 0x00}, /* ň to n */
    {0x0149, 0x27, 0x6E, 0x00, 0x00}, /* ŉ to 'n */
    {0x014A, 0x4E, 0x47, 0x00, 0x00}, /* Ŋ to NG */
    {0x014B, 0x6E, 0x67, 0x00, 0x00}, /* ŋ to ng */
    {0x014C, 0x4F, 0x00, 0x00, 0x00}, /* Ō to O */
    {0x014D, 0x6F, 0x00, 0x00, 0x00}, /* ō to o */
    {0x014E, 0x4F, 0x00, 0x00, 0x00}, /* Ŏ to O */
    {0x014F, 0x6F, 0x00, 0x00, 0x00}, /* ŏ to o */
    {0x0150, 0x4F, 0x00, 0x00, 0x00}, /* Ő to O */
    {0x0151, 0x6F, 0x00, 0x00, 0x00}, /* ő to o */
    {0x0152, 0x4F, 0x45, 0x00, 0x00}, /* Œ to OE */
    {0x0153, 0x6F, 0x65, 0x00, 0x00}, /* œ to oe */
    {0x0154, 0x52, 0x00, 0x00, 0x00}, /* Ŕ to R */
    {0x0155, 0x72, 0x00, 0x00, 0x00}, /* ŕ to r */
    {0x0156, 0x52, 0x00, 0x00, 0x00}, /* Ŗ to R */
    {0x0157, 0x72, 0x00, 0x00, 0x00}, /* ŗ to r */
    {0x0158, 0x52, 0x00, 0x00, 0x00}, /* Ř to R */
    {0x0159, 0x72, 0x00, 0x00, 0x00}, /* ř to r */
    {0x015A, 0x53, 0x00, 0x00, 0x00}, /* Ś to S */
    {0x015B, 0x73, 0x00, 0x00, 0x00}, /* ś to s */
    {0x015C, 0x53, 0x68, 0x00, 0x00}, /* Ŝ to Sh */
    {0x015D, 0x73, 0x68, 0x00, 0x00}, /* ŝ to sh */
    {0x015E, 0x53, 0x00, 0x00, 0x00}, /* Ş to S */
    {0x015F, 0x73, 0x00, 0x00, 0x00}, /* ş to s */
    {0x0160, 0x53, 0x00, 0x00, 0x00}, /* Š to S */
    {0x0161, 0x73, 0x00, 0x00, 0x00}, /* š to s */
    {0x0162, 0x54, 0x00, 0x00, 0x00}, /* Ţ to T */
    {0x0163, 0x74, 0x00, 0x00, 0x00}, /* ţ to t */
    {0x0164, 0x54, 0x00, 0x00, 0x00}, /* Ť to T */
    {0x0165, 0x74, 0x00, 0x00, 0x00}, /* ť to t */
    {0x0166, 0x54, 0x00, 0x00, 0x00}, /* Ŧ to T */
    {0x0167, 0x74, 0x00, 0x00, 0x00}, /* ŧ to t */
    {0x0168, 0x55, 0x00, 0x00, 0x00}, /* Ũ to U */
    {0x0169, 0x75, 0x00, 0x00, 0x00}, /* ũ to u */
    {0x016A, 0x55, 0x00, 0x00, 0x00}, /* Ū to U */
    {0x016B, 0x75, 0x00, 0x00, 0x00}, /* ū to u */
    {0x016C, 0x55, 0x00, 0x00, 0x00}, /* Ŭ to U */
    {0x016D, 0x75, 0x00, 0x00, 0x00}, /* ŭ to u */
    {0x016E, 0x55, 0x00, 0x00, 0x00}, /* Ů to U */
    {0x016F, 0x75, 0x00, 0x00, 0x00}, /* ů to u */
    {0x0170, 0x55, 0x00, 0x00, 0x00}, /* Ű to U */
    {0x0171, 0x75, 0x00, 0x00, 0x00}, /* ű to u */
    {0x0172, 0x55, 0x00, 0x00, 0x00}, /* Ų to U */
    {0x0173, 0x75, 0x00, 0x00, 0x00}, /* ų to u */
    {0x0174, 0x57, 0x00, 0x00, 0x00}, /* Ŵ to W */
    {0x0175, 0x77, 0x00, 0x00, 0x00}, /* ŵ to w */
    {0x0176, 0x59, 0x00, 0x00, 0x00}, /* Ŷ to Y */
    {0x0177, 0x79, 0x00, 0x00, 0x00}, /* ŷ to y */
    {0x0178, 0x59, 0x00, 0x00, 0x00}, /* Ÿ to Y */
    {0x0179, 0x5A, 0x00, 0x00, 0x00}, /* Ź to Z */
    {0x017A, 0x7A, 0x00, 0x00, 0x00}, /* ź to z */
    {0x017B, 0x5A, 0x00, 0x00, 0x00}, /* Ż to Z */
    {0x017C, 0x7A, 0x00, 0x00, 0x00}, /* ż to z */
    {0x017D, 0x5A, 0x00, 0x00, 0x00}, /* Ž to Z */
    {0x017E, 0x7A, 0x00, 0x00, 0x00}, /* ž to z */
    {0x017F, 0x73, 0x00, 0x00, 0x00}, /* ſ to s */
    {0x0192, 0x66, 0x00, 0x00, 0x00}, /* ƒ to f */
    {0x0218, 0x53, 0x00, 0x00, 0x00}, /* Ș to S */
    {0x0219, 0x73, 0x00, 0x00, 0x00}, /* ș to s */
    {0x021A, 0x54, 0x00, 0x00, 0x00}, /* Ț to T */
    {0x021B, 0x74, 0x00, 0x00, 0x00}, /* ț to t */
    {0x0386, 0x41, 0x00, 0x00, 0x00}, /* Ά to A */
    {0x0388, 0x45, 0x00, 0x00, 0x00}, /* Έ to E */
    {0x0389, 0x49, 0x00, 0x00, 0x00}, /* Ή to I */
    {0x038A, 0x49, 0x00, 0x00, 0x00}, /* Ί to I */
    {0x038C, 0x4f, 0x00, 0x00, 0x00}, /* Ό to O */
    {0x038E, 0x59, 0x00, 0x00, 0x00}, /* Ύ to Y */
    {0x038F, 0x4f, 0x00, 0x00, 0x00}, /* Ώ to O */
    {0x0390, 0x69, 0x00, 0x00, 0x00}, /* ΐ to i */
    {0x0391, 0x41, 0x00, 0x00, 0x00}, /* Α to A */
    {0x0392, 0x42, 0x00, 0x00, 0x00}, /* Β to B */
    {0x0393, 0x47, 0x00, 0x00, 0x00}, /* Γ to G */
    {0x0394, 0x44, 0x00, 0x00, 0x00}, /* Δ to D */
    {0x0395, 0x45, 0x00, 0x00, 0x00}, /* Ε to E */
    {0x0396, 0x5a, 0x00, 0x00, 0x00}, /* Ζ to Z */
    {0x0397, 0x49, 0x00, 0x00, 0x00}, /* Η to I */
    {0x0398, 0x54, 0x68, 0x00, 0x00}, /* Θ to Th */
    {0x0399, 0x49, 0x00, 0x00, 0x00}, /* Ι to I */
    {0x039A, 0x4b, 0x00, 0x00, 0x00}, /* Κ to K */
    {0x039B, 0x4c, 0x00, 0x00, 0x00}, /* Λ to L */
    {0x039C, 0x4d, 0x00, 0x00, 0x00}, /* Μ to M */
    {0x039D, 0x4e, 0x00, 0x00, 0x00}, /* Ν to N */
    {0x039E, 0x58, 0x00, 0x00, 0x00}, /* Ξ to X */
    {0x039F, 0x4f, 0x00, 0x00, 0x00}, /* Ο to O */
    {0x03A0, 0x50, 0x00, 0x00, 0x00}, /* Π to P */
    {0x03A1, 0x52, 0x00, 0x00, 0x00}, /* Ρ to R */
    {0x03A3, 0x53, 0x00, 0x00, 0x00}, /* Σ to S */
    {0x03A4, 0x54, 0x00, 0x00, 0x00}, /* Τ to T */
    {0x03A5, 0x59, 0x00, 0x00, 0x00}, /* Υ to Y */
    {0x03A6, 0x46, 0x00, 0x00, 0x00}, /* Φ to F */
    {0x03A7, 0x43, 0x68, 0x00, 0x00}, /* Χ to Ch */
    {0x03A8, 0x50, 0x73, 0x00, 0x00}, /* Ψ to Ps */
    {0x03A9, 0x4f, 0x00, 0x00, 0x00}, /* Ω to O */
    {0x03AA, 0x49, 0x00, 0x00, 0x00}, /* Ϊ to I */
    {0x03AB, 0x59, 0x00, 0x00, 0x00}, /* Ϋ to Y */
    {0x03AC, 0x61, 0x00, 0x00, 0x00}, /* ά to a */
    {0x03AD, 0x65, 0x00, 0x00, 0x00}, /* έ to e */
    {0x03AE, 0x69, 0x00, 0x00, 0x00}, /* ή to i */
    {0x03AF, 0x69, 0x00, 0x00, 0x00}, /* ί to i */
    {0x03B1, 0x61, 0x00, 0x00, 0x00}, /* α to a */
    {0x03B2, 0x62, 0x00, 0x00, 0x00}, /* β to b */
    {0x03B3, 0x67, 0x00, 0x00, 0x00}, /* γ to g */
    {0x03B4, 0x64, 0x00, 0x00, 0x00}, /* δ to d */
    {0x03B5, 0x65, 0x00, 0x00, 0x00}, /* ε to e */
    {0x03B6, 0x7a, 0x00, 0x00, 0x00}, /* ζ to z */
    {0x03B7, 0x69, 0x00, 0x00, 0x00}, /* η to i */
    {0x03B8, 0x74, 0x68, 0x00, 0x00}, /* θ to th */
    {0x03B9, 0x69, 0x00, 0x00, 0x00}, /* ι to i */
    {0x03BA, 0x6b, 0x00, 0x00, 0x00}, /* κ to k */
    {0x03BB, 0x6c, 0x00, 0x00, 0x00}, /* λ to l */
    {0x03BC, 0x6d, 0x00, 0x00, 0x00}, /* μ to m */
    {0x03BD, 0x6e, 0x00, 0x00, 0x00}, /* ν to n */
    {0x03BE, 0x78, 0x00, 0x00, 0x00}, /* ξ to x */
    {0x03BF, 0x6f, 0x00, 0x00, 0x00}, /* ο to o */
    {0x03C0, 0x70, 0x00, 0x00, 0x00}, /* π to p */
    {0x03C1, 0x72, 0x00, 0x00, 0x00}, /* ρ to r */
    {0x03C3, 0x73, 0x00, 0x00, 0x00}, /* σ to s */
    {0x03C4, 0x74, 0x00, 0x00, 0x00}, /* τ to t */
    {0x03C5, 0x79, 0x00, 0x00, 0x00}, /* υ to y */
    {0x03C6, 0x66, 0x00, 0x00, 0x00}, /* φ to f */
    {0x03C7, 0x63, 0x68, 0x00, 0x00}, /* χ to ch */
    {0x03C8, 0x70, 0x73, 0x00, 0x00}, /* ψ to ps */
    {0x03C9, 0x6f, 0x00, 0x00, 0x00}, /* ω to o */
    {0x03CA, 0x69, 0x00, 0x00, 0x00}, /* ϊ to i */
    {0x03CB, 0x79, 0x00, 0x00, 0x00}, /* ϋ to y */
    {0x03CC, 0x6f, 0x00, 0x00, 0x00}, /* ό to o */
    {0x03CD, 0x79, 0x00, 0x00, 0x00}, /* ύ to y */
    {0x03CE, 0x69, 0x00, 0x00, 0x00}, /* ώ to i */
    {0x0400, 0x45, 0x00, 0x00, 0x00}, /* Ѐ to E */
    {0x0401, 0x45, 0x00, 0x00, 0x00}, /* Ё to E */
    {0x0402, 0x44, 0x00, 0x00, 0x00}, /* Ђ to D */
    {0x0403, 0x47, 0x00, 0x00, 0x00}, /* Ѓ to G */
    {0x0404, 0x45, 0x00, 0x00, 0x00}, /* Є to E */
    {0x0405, 0x5a, 0x00, 0x00, 0x00}, /* Ѕ to Z */
    {0x0406, 0x49, 0x00, 0x00, 0x00}, /* І to I */
    {0x0407, 0x49, 0x00, 0x00, 0x00}, /* Ї to I */
    {0x0408, 0x4a, 0x00, 0x00, 0x00}, /* Ј to J */
    {0x0409, 0x49, 0x00, 0x00, 0x00}, /* Љ to I */
    {0x040A, 0x4e, 0x00, 0x00, 0x00}, /* Њ to N */
    {0x040B, 0x44, 0x00, 0x00, 0x00}, /* Ћ to D */
    {0x040C, 0x4b, 0x00, 0x00, 0x00}, /* Ќ to K */
    {0x040D, 0x49, 0x00, 0x00, 0x00}, /* Ѝ to I */
    {0x040E, 0x55, 0x00, 0x00, 0x00}, /* Ў to U */
    {0x040F, 0x44, 0x00, 0x00, 0x00}, /* Џ to D */
    {0x0410, 0x41, 0x00, 0x00, 0x00}, /* А to A */
    {0x0411, 0x42, 0x00, 0x00, 0x00}, /* Б to B */
    {0x0412, 0x56, 0x00, 0x00, 0x00}, /* В to V */
    {0x0413, 0x47, 0x00, 0x00, 0x00}, /* Г to G */
    {0x0414, 0x44, 0x00, 0x00, 0x00}, /* Д to D */
    {0x0415, 0x45, 0x00, 0x00, 0x00}, /* Е to E */
    {0x0416, 0x5a, 0x68, 0x00, 0x00}, /* Ж to Zh */
    {0x0417, 0x5a, 0x00, 0x00, 0x00}, /* З to Z */
    {0x0418, 0x49, 0x00, 0x00, 0x00}, /* И to I */
    {0x0419, 0x49, 0x00, 0x00, 0x00}, /* Й to I */
    {0x041A, 0x4b, 0x00, 0x00, 0x00}, /* К to K */
    {0x041B, 0x4c, 0x00, 0x00, 0x00}, /* Л to L */
    {0x041C, 0x4d, 0x00, 0x00, 0x00}, /* М to M */
    {0x041D, 0x4e, 0x00, 0x00, 0x00}, /* Н to N */
    {0x041E, 0x4f, 0x00, 0x00, 0x00}, /* О to O */
    {0x041F, 0x50, 0x00, 0x00, 0x00}, /* П to P */
    {0x0420, 0x52, 0x00, 0x00, 0x00}, /* Р to R */
    {0x0421, 0x53, 0x00, 0x00, 0x00}, /* С to S */
    {0x0422, 0x54, 0x00, 0x00, 0x00}, /* Т to T */
    {0x0423, 0x55, 0x00, 0x00, 0x00}, /* У to U */
    {0x0424, 0x46, 0x00, 0x00, 0x00}, /* Ф to F */
    {0x0425, 0x4b, 0x68, 0x00, 0x00}, /* Х to Kh */
    {0x0426, 0x54, 0x63, 0x00, 0x00}, /* Ц to Tc */
    {0x0427, 0x43, 0x68, 0x00, 0x00}, /* Ч to Ch */
    {0x0428, 0x53, 0x68, 0x00, 0x00}, /* Ш to Sh */
    {0x0429, 0x53, 0x68, 0x63, 0x68}, /* Щ to Shch */
    {0x042A, 0x61, 0x00, 0x00, 0x00}, /*  to A */
    {0x042B, 0x59, 0x00, 0x00, 0x00}, /* Ы to Y */
    {0x042C, 0x59, 0x00, 0x00, 0x00}, /*  to Y */
    {0x042D, 0x45, 0x00, 0x00, 0x00}, /* Э to E */
    {0x042E, 0x49, 0x75, 0x00, 0x00}, /* Ю to Iu */
    {0x042F, 0x49, 0x61, 0x00, 0x00}, /* Я to Ia */
    {0x0430, 0x61, 0x00, 0x00, 0x00}, /* а to a */
    {0x0431, 0x62, 0x00, 0x00, 0x00}, /* б to b */
    {0x0432, 0x76, 0x00, 0x00, 0x00}, /* в to v */
    {0x0433, 0x67, 0x00, 0x00, 0x00}, /* г to g */
    {0x0434, 0x64, 0x00, 0x00, 0x00}, /* д to d */
    {0x0435, 0x65, 0x00, 0x00, 0x00}, /* е to e */
    {0x0436, 0x7a, 0x68, 0x00, 0x00}, /* ж to zh */
    {0x0437, 0x7a, 0x00, 0x00, 0x00}, /* з to z */
    {0x0438, 0x69, 0x00, 0x00, 0x00}, /* и to i */
    {0x0439, 0x69, 0x00, 0x00, 0x00}, /* й to i */
    {0x043A, 0x6b, 0x00, 0x00, 0x00}, /* к to k */
    {0x043B, 0x6c, 0x00, 0x00, 0x00}, /* л to l */
    {0x043C, 0x6d, 0x00, 0x00, 0x00}, /* м to m */
    {0x043D, 0x6e, 0x00, 0x00, 0x00}, /* н to n */
    {0x043E, 0x6f, 0x00, 0x00, 0x00}, /* о to o */
    {0x043F, 0x70, 0x00, 0x00, 0x00}, /* п to p */
    {0x0440, 0x72, 0x00, 0x00, 0x00}, /* р to r */
    {0x0441, 0x73, 0x00, 0x00, 0x00}, /* с to s */
    {0x0442, 0x74, 0x00, 0x00, 0x00}, /* т to t */
    {0x0443, 0x75, 0x00, 0x00, 0x00}, /* у to u */
    {0x0444, 0x66, 0x00, 0x00, 0x00}, /* ф to f */
    {0x0445, 0x6b, 0x68, 0x00, 0x00}, /* х to kh */
    {0x0446, 0x74, 0x63, 0x00, 0x00}, /* ц to tc */
    {0x0447, 0x63, 0x68, 0x00, 0x00}, /* ч to ch */
    {0x0448, 0x73, 0x68, 0x00, 0x00}, /* ш to sh */
    {0x0449, 0x73, 0x68, 0x63, 0x68}, /* щ to shch */
    {0x044A, 0x61, 0x00, 0x00, 0x00}, /*  to a */
    {0x044B, 0x79, 0x00, 0x00, 0x00}, /* ы to y */
    {0x044C, 0x79, 0x00, 0x00, 0x00}, /*  to y */
    {0x044D, 0x65, 0x00, 0x00, 0x00}, /* э to e */
    {0x044E, 0x69, 0x75, 0x00, 0x00}, /* ю to iu */
    {0x044F, 0x69, 0x61, 0x00, 0x00}, /* я to ia */
    {0x0450, 0x65, 0x00, 0x00, 0x00}, /* ѐ to e */
    {0x0451, 0x65, 0x00, 0x00, 0x00}, /* ё to e */
    {0x0452, 0x64, 0x00, 0x00, 0x00}, /* ђ to d */
    {0x0453, 0x67, 0x00, 0x00, 0x00}, /* ѓ to g */
    {0x0454, 0x65, 0x00, 0x00, 0x00}, /* є to e */
    {0x0455, 0x7a, 0x00, 0x00, 0x00}, /* ѕ to z */
    {0x0456, 0x69, 0x00, 0x00, 0x00}, /* і to i */
    {0x0457, 0x69, 0x00, 0x00, 0x00}, /* ї to i */
    {0x0458, 0x6a, 0x00, 0x00, 0x00}, /* ј to j */
    {0x0459, 0x69, 0x00, 0x00, 0x00}, /* љ to i */
    {0x045A, 0x6e, 0x00, 0x00, 0x00}, /* њ to n */
    {0x045B, 0x64, 0x00, 0x00, 0x00}, /* ћ to d */
    {0x045C, 0x6b, 0x00, 0x00, 0x00}, /* ќ to k */
    {0x045D, 0x69, 0x00, 0x00, 0x00}, /* ѝ to i */
    {0x045E, 0x75, 0x00, 0x00, 0x00}, /* ў to u */
    {0x045F, 0x64, 0x00, 0x00, 0x00}, /* џ to d */
    {0x1E02, 0x42, 0x00, 0x00, 0x00}, /* Ḃ to B */
    {0x1E03, 0x62, 0x00, 0x00, 0x00}, /* ḃ to b */
    {0x1E0A, 0x44, 0x00, 0x00, 0x00}, /* Ḋ to D */
    {0x1E0B, 0x64, 0x00, 0x00, 0x00}, /* ḋ to d */
    {0x1E1E, 0x46, 0x00, 0x00, 0x00}, /* Ḟ to F */
    {0x1E1F, 0x66, 0x00, 0x00, 0x00}, /* ḟ to f */
    {0x1E40, 0x4D, 0x00, 0x00, 0x00}, /* Ṁ to M */
    {0x1E41, 0x6D, 0x00, 0x00, 0x00}, /* ṁ to m */
    {0x1E56, 0x50, 0x00, 0x00, 0x00}, /* Ṗ to P */
    {0x1E57, 0x70, 0x00, 0x00, 0x00}, /* ṗ to p */
    {0x1E60, 0x53, 0x00, 0x00, 0x00}, /* Ṡ to S */
    {0x1E61, 0x73, 0x00, 0x00, 0x00}, /* ṡ to s */
    {0x1E6A, 0x54, 0x00, 0x00, 0x00}, /* Ṫ to T */
    {0x1E6B, 0x74, 0x00, 0x00, 0x00}, /* ṫ to t */
    {0x1E80, 0x57, 0x00, 0x00, 0x00}, /* Ẁ to W */
    {0x1E81, 0x77, 0x00, 0x00, 0x00}, /* ẁ to w */
    {0x1E82, 0x57, 0x00, 0x00, 0x00}, /* Ẃ to W */
    {0x1E83, 0x77, 0x00, 0x00, 0x00}, /* ẃ to w */
    {0x1E84, 0x57, 0x00, 0x00, 0x00}, /* Ẅ to W */
    {0x1E85, 0x77, 0x00, 0x00, 0x00}, /* ẅ to w */
    {0x1EF2, 0x59, 0x00, 0x00, 0x00}, /* Ỳ to Y */
    {0x1EF3, 0x79, 0x00, 0x00, 0x00}, /* ỳ to y */
    {0xFB00, 0x66, 0x66, 0x00, 0x00}, /* ﬀ to ff */
    {0xFB01, 0x66, 0x69, 0x00, 0x00}, /* ﬁ to fi */
    {0xFB02, 0x66, 0x6C, 0x00, 0x00}, /* ﬂ to fl */
    {0xFB05, 0x73, 0x74, 0x00, 0x00}, /* ﬅ to st */
    {0xFB06, 0x73, 0x74, 0x00, 0x00}, /* ﬆ to st */
};

static const Transliteration* spellfixFindTranslit(int c, int* pxTop) {
    *pxTop = (sizeof(translit) / sizeof(translit[0])) - 1;
    return translit;
}

/*
** Convert the input string from UTF-8 into pure ASCII by converting
** all non-ASCII characters to some combination of characters in the
** ASCII subset.
**
** The returned string might contain more characters than the input.
**
** Space to hold the returned string comes from sqlite3_malloc() and
** should be freed by the caller.
*/
unsigned char* transliterate(const unsigned char* zIn, int nIn) {
    unsigned char* zOut = malloc(nIn * 4 + 1);
    int c, sz, nOut;
    if (zOut == 0)
        return 0;
    nOut = 0;
    while (nIn > 0) {
        c = utf8Read(zIn, nIn, &sz);
        zIn += sz;
        nIn -= sz;
        if (c <= 127) {
            zOut[nOut++] = (unsigned char)c;
        } else {
            int xTop, xBtm, x;
            const Transliteration* tbl = spellfixFindTranslit(c, &xTop);
            xBtm = 0;
            while (xTop >= xBtm) {
                x = (xTop + xBtm) / 2;
                if (tbl[x].cFrom == c) {
                    zOut[nOut++] = tbl[x].cTo0;
                    if (tbl[x].cTo1) {
                        zOut[nOut++] = tbl[x].cTo1;
                        if (tbl[x].cTo2) {
                            zOut[nOut++] = tbl[x].cTo2;
                            if (tbl[x].cTo3) {
                                zOut[nOut++] = tbl[x].cTo3;
                            }
                        }
                    }
                    c = 0;
                    break;
                } else if (tbl[x].cFrom > c) {
                    xTop = x - 1;
                } else {
                    xBtm = x + 1;
                }
            }
            if (c)
                zOut[nOut++] = '?';
        }
    }
    zOut[nOut] = 0;
    return zOut;
}

/*
** Return the number of characters in the shortest prefix of the input
** string that transliterates to an ASCII string nTrans bytes or longer.
** Or, if the transliteration of the input string is less than nTrans
** bytes in size, return the number of characters in the input string.
*/
int translen_to_charlen(const char* zIn, int nIn, int nTrans) {
    int i, c, sz, nOut;
    int nChar;

    i = nOut = 0;
    for (nChar = 0; i < nIn && nOut < nTrans; nChar++) {
        c = utf8Read((const unsigned char*)&zIn[i], nIn - i, &sz);
        i += sz;

        nOut++;
        if (c >= 128) {
            int xTop, xBtm, x;
            const Transliteration* tbl = spellfixFindTranslit(c, &xTop);
            xBtm = 0;
            while (xTop >= xBtm) {
                x = (xTop + xBtm) / 2;
                if (tbl[x].cFrom == c) {
                    if (tbl[x].cTo1) {
                        nOut++;
                        if (tbl[x].cTo2) {
                            nOut++;
                            if (tbl[x].cTo3) {
                                nOut++;
                            }
                        }
                    }
                    break;
                } else if (tbl[x].cFrom > c) {
                    xTop = x - 1;
                } else {
                    xBtm = x + 1;
                }
            }
        }
    }

    return nChar;
}

/*
 * Try to determine the dominant script used by the word zIn of length nIn
 * and return its ISO 15924 numeric code.
 */
int script_code(const unsigned char* zIn, int nIn) {
    int c, sz;
    int scriptMask = 0;
    int res;
    int seenDigit = 0;

    while (nIn > 0) {
        c = utf8Read(zIn, nIn, &sz);
        zIn += sz;
        nIn -= sz;
        if (c < 0x02af) {
            if (c >= 0x80 || midClass[c & 0x7f] < CCLASS_DIGIT) {
                scriptMask |= SCRIPT_LATIN;
            } else if (c >= '0' && c <= '9') {
                seenDigit = 1;
            }
        } else if (c >= 0x0400 && c <= 0x04ff) {
            scriptMask |= SCRIPT_CYRILLIC;
        } else if (c >= 0x0386 && c <= 0x03ce) {
            scriptMask |= SCRIPT_GREEK;
        } else if (c >= 0x0590 && c <= 0x05ff) {
            scriptMask |= SCRIPT_HEBREW;
        } else if (c >= 0x0600 && c <= 0x06ff) {
            scriptMask |= SCRIPT_ARABIC;
        }
    }
    if (scriptMask == 0 && seenDigit)
        scriptMask = SCRIPT_LATIN;
    switch (scriptMask) {
        case 0:
            res = 999;
            break;
        case SCRIPT_LATIN:
            res = 215;
            break;
        case SCRIPT_CYRILLIC:
            res = 220;
            break;
        case SCRIPT_GREEK:
            res = 200;
            break;
        case SCRIPT_HEBREW:
            res = 125;
            break;
        case SCRIPT_ARABIC:
            res = 160;
            break;
        default:
            res = 998;
            break;
    }
    return res;
}
