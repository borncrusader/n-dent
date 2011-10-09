/*
 * ------------------------
 * |   32 bits seq num    |
 * ------------------------
 * | checksum |type |flags|
 * ------------------------
 * |                      |
 * |         data         |
 * |                      |
 *
 * flags = | | | | | | |FNAME|EOM|
 *
 * FNAME - Set to 1 for first packet of message. The data contains the file name.
 * EOM - End of message. Set as 1 to indicate the last packet of the file.
