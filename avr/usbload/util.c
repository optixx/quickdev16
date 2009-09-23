/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *util_strupper(uint8_t *s)
{
	uint8_t *p;
	for (p = s; *p != '\0'; p++)
		if (*p >= 'a' && *p <= 'z')
			*p += 'A' - 'a';
	return s;
}

uint8_t *util_strlower(uint8_t *s)
{
	uint8_t *p;
	for (p = s; *p != '\0'; p++)
		if (*p >= 'A' && *p <= 'Z')
			*p += 'a' - 'A';
	return s;
}

void util_chomp(uint8_t *s)
{
	uint16_t len;

	len = strlen((char*)s);
	if (len >= 2 && s[len - 1] == '\n' && s[len - 2] == '\r')
		s[len - 2] = '\0';
	else if (len >= 1 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
		s[len - 1] = '\0';
}

void util_trim(uint8_t *s)
{
	uint8_t *p = s;
	uint8_t *q;
	/* skip leading whitespace */
	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
		p++;
	/* now p points at the first non-whitespace uint8_tacter */

	if (*p == '\0') {
		/* only whitespace */
		*s = '\0';
		return;
	}

	q = s + strlen((char*)s);
	/* skip trailing whitespace */
	/* we have found p < q such that *p is non-whitespace,
	   so this loop terminates with q >= p */
	do
		q--;
	while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n');

	/* now q points at the last non-whitespace uint8_tacter */
	/* cut off trailing whitespace */
	*++q = '\0';

	/* move to string */
	memmove(s, p, q + 1 - p);
}

uint32_t util_sscandec(const uint8_t *s)
{
	uint32_t result;
	if (*s == '\0')
		return -1;
	result = 0;
	for (;;) {
		if (*s >= '0' && *s <= '9')
			result = 10 * result + *s - '0';
		else if (*s == '\0')
			return result;
		else
			return -1;
		s++;
	}
}

uint32_t util_sscanhex(const uint8_t *s)
{
	int32_t result;
	if (*s == '\0')
		return -1;
	result = 0;
	for (;;) {
		if (*s >= '0' && *s <= '9')
			result = 16 * result + *s - '0';
		else if (*s >= 'A' && *s <= 'F')
			result = 16 * result + *s - 'A' + 10;
		else if (*s >= 'a' && *s <= 'f')
			result = 16 * result + *s - 'a' + 10;
		else if (*s == '\0')
			return result;
		else
			return -1;
		s++;
	}
}

uint8_t util_sscanbool(const uint8_t *s)
{
	if (*s == '0' && s[1] == '\0')
		return 0;
	if (*s == '1' && s[1] == '\0')
		return 1;
	return -1;
}


