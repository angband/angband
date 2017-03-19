/**
 * \file list-mon-timed.h
 * \brief Monster timed flags
 *
 * Fields:
 * - name         - the index name for this timed flag
 * - gets_save    - does this effect get a saving throw?
 * - resist_flag  - monsters with this monster race flag will resist this effect
 * - max_time     - maximum that the timer for this effect can reach (must be below 32767)
 * (messages)
 * - message_begin - the argument to the message code when the effect begins
 * - message_end - the argument to the message code when the effect ends
 * - message_increase - the argument to the message code when the effect increases
 */
/*    name  gets_save  can_stack  resist_flag  max_time  message_begin           message_end             message_increase       */
MON_TMD(SLEEP,  true,   false,    RF_NO_SLEEP, 10000,    MON_MSG_FALL_ASLEEP,    MON_MSG_WAKES_UP,       0                      )
MON_TMD(STUN,   true,   true,     RF_NO_STUN,  200,      MON_MSG_DAZED,          MON_MSG_NOT_DAZED,      MON_MSG_MORE_DAZED     )
MON_TMD(CONF,   true,   true,     RF_NO_CONF,  200,      MON_MSG_CONFUSED,       MON_MSG_NOT_CONFUSED,   MON_MSG_MORE_CONFUSED  )
MON_TMD(FEAR,   true,   true,     RF_NO_FEAR,  10000,    MON_MSG_FLEE_IN_TERROR, MON_MSG_NOT_AFRAID,     MON_MSG_MORE_AFRAID    )
MON_TMD(SLOW,   true,   true,     RF_NO_SLOW,  50,       MON_MSG_SLOWED,         MON_MSG_NOT_SLOWED,     MON_MSG_MORE_SLOWED    )
MON_TMD(FAST,   false,  true,     0,           50,       MON_MSG_HASTED,         MON_MSG_NOT_HASTED,     MON_MSG_MORE_HASTED    )
MON_TMD(HOLD,   false,  false,    RF_NO_SLEEP, 20,       MON_MSG_HELD,           MON_MSG_NOT_HELD,       0                      )
MON_TMD(MAX,    true,   true,     0,           0,        0,                      0,                      0                      )
