/**
 * \file list-room-flags.h
 * \brief List flags for room types
 *
 * Changing these flags would not affect savefiles but would affect the parsing
 * of vault.txt and room_template.txt.
 *
 * Fields:
 * name
 * help string
 */
ROOMF(FEW_ENTRANCES, "select alternate tunneling for a room since it can only be entered from a few directions or the entrances involve digging")
ROOMF(MAX, "")
