/**
 * Main functions for libnmea: publicly exported functions for interpreting
 * messages, and converting them to more usable structures.
 *
 * Since the message type may not be known beforehand, these functions will
 * allocate memory for their structures, and return a pointer to this memory. It
 * is the responsibility of the caller to free() this memory once they are done
 * with using the structure.
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h> 

#include "libnmea.h"

/**
 * Private parsing functions. These are passed a copy of the message, so that
 * they can modify it.
 */
extern nmea_message_gga_t* parse_gga(char *msg, nmea_error_t *outErr);
extern nmea_message_gsa_t* parse_gsa(char *msg, nmea_error_t *outErr);
extern nmea_message_gsv_t* parse_gsv(char *msg, nmea_error_t *outErr);
extern nmea_message_vtg_t* parse_vtg(char *msg, nmea_error_t *outErr);

/**
 * Maps an NMEA message identifier (such as "$GPVTG") to a more easily workable
 * equate.
 */
static const struct {
	char *nmea;
	nmea_message_type_t type;
} type_map[] = {
	{"$GPGGA", kNMEAMessageGGA},
	{"$GPGSA", kNMEAMessageGSA},
	{"$GPGSV", kNMEAMessageGSV},
	{"$GPVTF", kNMEAMessageVTG},
	{NULL, kNMEAMessageUnknown}
};

/**
 * Determines the type of message from the given string buffer.
 *
 * This works by comparing the first few characters of the message against
 * ones stored in an internal structure.
 */
LIBNMEA_PUBLIC nmea_message_type_t nmea_get_message_type(char *message) {
	assert(message != NULL);
	int i = 0;
 
	do {
		// if char pointer is NULL, we haven't got this type of message.
		if(type_map[i].nmea == NULL) {
			return kNMEAMessageUnknown;
		}

		// check if the message[1..5] match the string.
		if(!strncmp(message, type_map[i].nmea, 6)) {
			return type_map[i].type;
		}

		// go on to the next type;
		i++;
	} while(1);
}

/**
 * Parses a message. If the message could not be parsed, NULL is returned. The
 * return value be cast to the proper struct, based on the determined message
 * type. 
 *
 * If the message could not be parsed, and if the outError pointer is non-NULL,
 * its contents will be set to the appropriate error code.
 */
LIBNMEA_PUBLIC nmea_message_t *nmea_parse_message(char *message, nmea_error_t *outError) {
	assert(message != NULL);

	// If we can parse the message, point to it here
	nmea_message_t *parsed;

	// Create a copy of the input message.
	int len = strlen(message);
	char *copy = malloc(len + 2);
	strncpy(copy, message, len + 2);

	// Figure out what function will parse this message
	nmea_message_type_t t = nmea_get_message_type(message);

	switch(t) {
		case kNMEAMessageGGA:
			parsed = (nmea_message_t *) parse_gga(copy, outError);
			break;

		case kNMEAMessageGSA:
			parsed = (nmea_message_t *) parse_gsa(copy, outError);
			break;

		case kNMEAMessageGSV:
			parsed = (nmea_message_t *) parse_gsv(copy, outError);
			break;

		case kNMEAMessageVTG:
			parsed = (nmea_message_t *) parse_vtg(copy, outError);
			break;

		// The message was of an unknown type. (Handles kNMEAMessageUnknown)
		default:
			if(outError) *outError = kNMEAErrorTypeNotUnderstood;
			parsed = NULL;
			break;
	}

	// Ensure we don't pass back invalid data.
	assert(parsed);

	// Clean up
	free(copy);

	parsed->type = t;

	// We're done.
	return parsed;
}