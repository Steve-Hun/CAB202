#ifndef TYPES_H
#define TYPES_H 1

#define PB1 PIN4_bm
#define PB2 PIN5_bm
#define PB3 PIN6_bm
#define PB4 PIN7_bm

#define TONE_1 0
#define TONE_2 1
#define TONE_3 2
#define TONE_4 3

typedef enum
{
    CHECK_SCORE,
    RECORD_ENTRY,
    ENTERING,
    DISPLAY_ENTRY,
} High_Score_State;

typedef enum
{
    INIT,
    SIMON_TURN,
    USER_TURN,
    RESULT,
    HIGHSCORE_ENTRY,
} State;

typedef enum
{
    STATUS,
    DISPLAY_STATUS,
    DISPLAY_SCORE,
} Result_State;

typedef enum
{
    PAUSED,
    PLAYING,
} User_Turn_State;

typedef enum
{
    START,
    SILENT,
    PLAY,
} Simon_Turn_State;

typedef enum
{
    AWAITING_COMMAND,
    AWAITING_PAYLOAD,
    AWAITING_NAME,
    AWAITING_NEWLINE,
} Serial_State;

typedef struct
{
    char name[21]; // 20 visible characters and null terminator at the end
    uint16_t score;
} score_entry;

#endif
