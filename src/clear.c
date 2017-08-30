/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>

int
main(void)
{
	fputs("\x1b[2J\x1b[H", stdout);
	return 0;
}
