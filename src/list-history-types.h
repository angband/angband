/**
 * \file list-history-types.h
 * \brief History message types
 */
/* index				description */
HIST(NONE,				"")
HIST(PLAYER_BIRTH,		"Player was born")
HIST(ARTIFACT_UNKNOWN,	"Player found but not IDd an artifact")
HIST(ARTIFACT_KNOWN,	"Player has IDed an artifact")
HIST(ARTIFACT_LOST,		"Player had an artifact and lost it")
HIST(PLAYER_DEATH,		"Player has been slain")
HIST(SLAY_UNIQUE,		"Player has slain a unique monster")
HIST(USER_INPUT,		"User-added note")
HIST(SAVEFILE_IMPORT,	"Added when an older version savefile is imported")
HIST(GAIN_LEVEL,		"Player gained a level")
HIST(GENERIC,			"Anything else not covered here (unused)")
