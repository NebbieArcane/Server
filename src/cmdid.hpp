/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: cmdid.h,v 1.2 2002/02/13 12:30:57 root Exp $
*/
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
#ifndef _CMDID_H
#define _CMDID_H
namespace Alarmud {
#define CMD_NORTH                 1
#define CMD_EAST                  2
#define CMD_SOUTH                 3
#define CMD_WEST                  4
#define CMD_UP                    5
#define CMD_DOWN                  6
#define CMD_ENTER                 7
#define CMD_EXITS                 8
#define CMD_KISS                  9
#define CMD_GET                  10
#define CMD_DRINK                11
#define CMD_EAT                  12
#define CMD_WEAR                 13
#define CMD_WIELD                14
#define CMD_LOOK                 15
#define CMD_SCORE                16
#define CMD_SAY                  17
#define CMD_SHOUT                18
#define CMD_TELL                 19
#define CMD_INVENTORY            20
#define CMD_QUI                  21
#define CMD_BOUNCE               22
#define CMD_SMILE                23
#define CMD_DANCE                24
#define CMD_KILL                 25
#define CMD_CACKLE               26
#define CMD_LAUGH                27
#define CMD_GIGGLE               28
#define CMD_SHAKE                29
#define CMD_PUKE                 30
#define CMD_GROWL                31
#define CMD_SCREAM               32
#define CMD_INSULT               33
#define CMD_COMFORT              34
#define CMD_NOD                  35
#define CMD_SIGH                 36
#define CMD_SULK                 37
#define CMD_HELP                 38
#define CMD_WHO                  39
#define CMD_EMOTE                40
#define CMD_ECHO                 41
#define CMD_STAND                42
#define CMD_SIT                  43
#define CMD_REST                 44
#define CMD_SLEEP                45
#define CMD_WAKE                 46
#define CMD_FORCE                47
#define CMD_TRANSFER             48
#define CMD_HUG                  49
#define CMD_SNUGGLE              50
#define CMD_CUDDLE               51
#define CMD_NUZZLE               52
#define CMD_CRY                  53
#define CMD_NEWS                 54
#define CMD_EQUIPMENT            55
#define CMD_BUY                  56
#define CMD_SELL                 57
#define CMD_VALUE                58
#define CMD_LIST                 59
#define CMD_DROP                 60
#define CMD_GOTO                 61
#define CMD_WEATHER              62
#define CMD_READ                 63
#define CMD_POUR                 64
#define CMD_DUMMY                65
#define CMD_REMOVE               66
#define CMD_PUT                  67
#define CMD_SHUTDOW              68
#define CMD_SAVE                 69
#define CMD_HIT                  70
#define CMD_STRING               71
#define CMD_GIVE                 72
#define CMD_QUIT                 73
#define CMD_WIZNEWS              74
#define CMD_GUARD                75
#define CMD_TIME                 76
#define CMD_OLOAD                77
#define CMD_PURGE                78
#define CMD_SHUTDOWN             79
#define CMD_IDEA                 80
#define CMD_TYPO                 81
#define CMD_BUG                  82
#define CMD_WHISPER              83
#define CMD_CAST                 84
#define CMD_AT                   85
#define CMD_ASK                  86
#define CMD_ORDER                87
#define CMD_SIP                  88
#define CMD_TASTE                89
#define CMD_SNOOP                90
#define CMD_FOLLOW               91
#define CMD_RENT                 92
#define CMD_OFFER                93
#define CMD_POKE                 94
#define CMD_ADVANCE              95
#define CMD_ACCUSE               96
#define CMD_GRIN                 97
#define CMD_BOW                  98
#define CMD_OPEN                 99
#define CMD_CLOSE               100
#define CMD_LOCK                101
#define CMD_UNLOCK              102
#define CMD_LEAVE               103
#define CMD_APPLAUD             104
#define CMD_BLUSH               105
#define CMD_BURP                106
#define CMD_CHUCKLE             107
#define CMD_NOSE                108
#define CMD_COUGH               109
#define CMD_CURTSEY             110
#define CMD_RISE                111
#define CMD_FLIP                112
#define CMD_FONDLE              113
#define CMD_FROWN               114
#define CMD_GASP                115
#define CMD_GLARE               116
#define CMD_GROAN               117
#define CMD_GROPE               118
#define CMD_HICCUP              119
#define CMD_LICK                120
#define CMD_LOVE                121
#define CMD_MOAN                122
#define CMD_NIBBLE              123
#define CMD_POUT                124
#define CMD_PURR                125
#define CMD_RUFFLE              126
#define CMD_SHIVER              127
#define CMD_SHRUG               128
#define CMD_SING                129
#define CMD_SLAP                130
#define CMD_SMIRK               131
#define CMD_SUPPORT             132
#define CMD_SNEEZE              133
#define CMD_SNICKER             134
#define CMD_SNIFF               135
#define CMD_SNORE               136
#define CMD_SPIT                137
#define CMD_SQUEEZE             138
#define CMD_STARE               139
#define CMD_ASSIST              140
#define CMD_THANK               141
#define CMD_TWIDDLE             142
#define CMD_WAVE                143
#define CMD_WHISTLE             144
#define CMD_WIGGLE              145
#define CMD_WINK                146
#define CMD_YAWN                147
#define CMD_SNOWBALL            148
#define CMD_WRITE               149
#define CMD_HOLD                150
#define CMD_FLEE                151
#define CMD_SNEAK               152
#define CMD_HIDE                153
#define CMD_BACKSTAB            154
#define CMD_PICK                155
#define CMD_STEAL               156
#define CMD_BASH                157
#define CMD_RESCUE              158
#define CMD_KICK                159
#define CMD_FRENCHKISS          160
#define CMD_COMB                161
#define CMD_MASSAGE             162
#define CMD_TICKLE              163
#define CMD_PRACTICE            164
#define CMD_PAT                 165
#define CMD_EXAMINE             166
#define CMD_TAKE                167
#define CMD_INFO                168
#define CMD_SAY_APICE           169
#define CMD_THINK_SUPERNI       170
#define CMD_CURSE               171
#define CMD_USE                 172
#define CMD_WHERE               173
#define CMD_LEVELS              174
#define CMD_REGISTER            175
#define CMD_PRAY                176
#define CMD_EMOTE_VIRGOLA       177
#define CMD_BEG                 178
#define CMD_FORGE               179     /* Aggiunto da ACIDUS 2003 */
#define CMD_CRINGE              180
#define CMD_DAYDREAM            181
#define CMD_FUME                182
#define CMD_GROVEL              183
#define CMD_HOP                 184
#define CMD_NUDGE               185
#define CMD_PEER                186
#define CMD_POINT               187
#define CMD_PONDER              188
#define CMD_PUNCH               189
#define CMD_SNARL               190
#define CMD_SPANK               191
#define CMD_STEAM               192
#define CMD_TACKLE              193
#define CMD_TAUNT               194
#define CMD_THINK               195
#define CMD_WHINE               196
#define CMD_WORSHIP             197
#define CMD_YODEL               198
#define CMD_BRIEF               199
#define CMD_WIZLIST             200
#define CMD_CONSIDER            201
#define CMD_GROUP               202
#define CMD_RESTORE             203
#define CMD_RETURN              204
#define CMD_SWITCH              205
#define CMD_QUAFF               206
#define CMD_RECITE              207
#define CMD_USERS               208
#define CMD_POSE                209
#define CMD_NOSHOUT             210
#define CMD_WIZHELP             211
#define CMD_CREDITS             212
#define CMD_COMPACT             213
#define CMD_DAIMOKU             214
#define CMD_DEAFEN              215
#define CMD_SLAY                216
#define CMD_WIMPY               217
#define CMD_JUNK                218
#define CMD_DEPOSIT             219
#define CMD_WITHDRAW            220
#define CMD_BALANCE             221
#define CMD_NOHASSLE            222
#define CMD_SYSTEM              223
#define CMD_PULL                224
#define CMD_STEALTH             225
#define CMD_EDIT                226
#define CMD_CHIOCCIOLA          227
#define CMD_RSAVE               228
#define CMD_RLOAD               229
#define CMD_TRACK               230
#define CMD_WIZLOCK             231
#define CMD_HIGHFIVE            232
#define CMD_TITLE               233
#define CMD_WHOZONE             234
#define CMD_ASSOCIA             235
#define CMD_ATTRIBUTE           236
#define CMD_WORLD               237
#define CMD_ALLSPELLS           238
#define CMD_BREATH              239
#define CMD_SHOW                240
#define CMD_DEBUG               241
#define CMD_INVISIBLE           242
#define CMD_GAIN                243
#define CMD_MLOAD               244
#define CMD_DISARM              245
#define CMD_BONK                246
#define CMD_CHPWD               247
#define CMD_FILL                248
#define CMD_MANTRA              249
#define CMD_SHOOT               250
#define CMD_SILENCE             251
#define CMD_TEAMS               252     // da implementare
#define CMD_PLAYER              253     // da implementare
#define CMD_CREATE              254
#define CMD_BAMFIN              255
#define CMD_BAMFOUT             256
#define CMD_VIS                 257
#define CMD_DOORBASH            258
#define CMD_MOSH                259
#define CMD_ALIAS               260
#define CMD_1                   261
#define CMD_2                   262
#define CMD_3                   263
#define CMD_4                   264
#define CMD_5                   265
#define CMD_6                   266
#define CMD_7                   267
#define CMD_8                   268
#define CMD_9                   269
#define CMD_0                   270
#define CMD_SWIM                271
#define CMD_SPY                 272
#define CMD_SPRINGLEAP          273
#define CMD_QUIVERING           274
#define CMD_FEIGH_DEATH         275
#define CMD_MOUNT               276
#define CMD_DISMOUNT            277
#define CMD_RIDE                278
#define CMD_SIGN                279
#define CMD_SET                 280
#define CMD_FIRST_AID           281
#define CMD_LOG                 282
#define CMD_RECALL              283
#define CMD_RELOAD              284
#define CMD_EVENT               285
#define CMD_DISGUISE            286
#define CMD_CLIMB               287
#define CMD_BEEP                288
#define CMD_BITE                289
#define CMD_REDIT               290
#define CMD_DISPLAY             291
#define CMD_RESIZE              292
#define CMD_RIPUDIA             293
#define CMD_DIESIS              294
#define CMD_SPELL               295
#define CMD_NAME                296
#define CMD_STAT                297
#define CMD_IMMORT              298
#define CMD_AUTH                299
#define CMD_SACRIFICE           300     // per Thanatos
#define CMD_NOGOSSIP            301
#define CMD_GOSSIP              302
#define CMD_NOAUCTION           303
#define CMD_AUCTION             304
#define CMD_DISCON              305
#define CMD_FREEZE              306
#define CMD_DRAIN               307
#define CMD_OEDIT               308
#define CMD_REPORT              309
#define CMD_INTERVEN            310
#define CMD_GTELL               311
#define CMD_RAISE               312
#define CMD_TAP                 313
#define CMD_LIEGE               314
#define CMD_SNEER               315
#define CMD_HOWL                316
#define CMD_KNEEL               317
#define CMD_FINGER              318
#define CMD_PACE                319
#define CMD_TONGUE              320
#define CMD_FLEX                321
#define CMD_ACK                 322
#define CMD_CKEQ                323
#define CMD_CARESS              324
#define CMD_CHEER               325
#define CMD_JUMP                326
#define CMD_JOIN                327
#define CMD_SPLIT               328
#define CMD_BERSERK             329
#define CMD_TAN                 330
#define CMD_MEMORIZE            331
#define CMD_FIND                332
#define CMD_BELLOW              333
#define CMD_STORE               334
#define CMD_CARVE               335
#define CMD_NUKE                336
#define CMD_SKILLS              337
#define CMD_DOORWAY             338
#define CMD_PORTAL              339
#define CMD_SUMMON              340
#define CMD_CANIBALIZE          341
#define CMD_FLAME               342
#define CMD_AURA                343
#define CMD_GREAT               344
#define CMD_PSIONIC_INVISIBILIY 345
#define CMD_BLAST               346
#define CMD_MEDIT               347
#define CMD_HYPNOTIZE           348
#define CMD_SCRY                349
#define CMD_ADRENALIZE          350
#define CMD_BREW                351     /* NON IMPLEMENTATO */
#define CMD_MEDITATE            352
#define CMD_FORCERENT           353
#define CMD_WARCRY              354
#define CMD_LAY_ON_HANDS        355
#define CMD_BLESSING            356
#define CMD_HEROIC              357
#define CMD_SCAN                358
#define CMD_SHIELD              359
#define CMD_NOTELL              360
#define CMD_COMMANDS            361
#define CMD_GHOST               362
#define CMD_SPEAK               363
#define CMD_SETSEV              364
#define CMD_ESP                 365
#define CMD_MAIL                366
#define CMD_CHECK               367
#define CMD_RECEIVE             368
#define CMD_TELEPATHY           369
#define CMD_MIND                370
#define CMD_TWIST               371
#define CMD_TURN                372
#define CMD_LIFT                373
#define CMD_PUSH                374
#define CMD_ZLOAD               375
#define CMD_ZSAVE               376
#define CMD_ZCLEAN              377
#define CMD_WREBUILD            378
#define CMD_GWHO                379
#define CMD_MFORCE              380
#define CMD_CLONE               381
#define CMD_BODYGUARD           382
#define CMD_THROW               383
#define CMD_RUN                 384
#define CMD_NOTCH               385
#define CMD_SPELLID             386
#define CMD_SPOT                387
#define CMD_VIEW                388     // inutile?
#define CMD_AFK                 389
#define CMD_STOPFIGHT           390
#define CMD_PRINCE              391
#define CMD_TSPY                392
#define CMD_BID                 393
#define CMD_EAVESDROP           394
#define CMD_PQUEST              395
#define CMD_SETALIGN            396
#define CMD_WRESET              397	    // SALVO aggiunto comando wreset
#define CMD_PARRY               398     // Aggiunto da GAia 2001
#define CMD_CHECKTYPOS          399
#define CMD_ADORE               400
#define CMD_AGREE               401
#define CMD_BLEED               402
#define CMD_BLINK               403
#define CMD_BLOW                404
#define CMD_BLAME               405
#define CMD_BARK                406
#define CMD_BHUG                407
#define CMD_BCHECK              408
#define CMD_BOAST               409
#define CMD_CHIDE               410
#define CMD_COMPLIMENT          411
#define CMD_CEYES               412
#define CMD_CEARS               413
#define CMD_CROSS               414
#define CMD_CONSOLE             415
#define CMD_CALM                416
#define CMD_COWER               417
#define CMD_CONFESS             418
#define CMD_DROOL               419
#define CMD_GRIT                420
#define CMD_GREET               421
#define CMD_GULP                422
#define CMD_GLOAT               423
#define CMD_GAZE                424
#define CMD_HUM                 425
#define CMD_HKISS               426
#define CMD_IGNORE              427
#define CMD_INTERRUPT           428
#define CMD_KNOCK               429
#define CMD_LISTEN              430
#define CMD_MUSE                431
#define CMD_PINCH               432
#define CMD_PRAISE              433
#define CMD_PLOT                434
#define CMD_PIE                 435
#define CMD_PLEADE              436
#define CMD_PANT                437
#define CMD_RUB                 438
#define CMD_ROLL                439
#define CMD_RECOIL              440
#define CMD_ROAR                441
#define CMD_RELAX               442
#define CMD_SNAP                443
#define CMD_STRUT               444
#define CMD_STROKE              445
#define CMD_STRETCH             446
#define CMD_SWAVE               447
#define CMD_SOB                 448
#define CMD_SCRATCH             449
#define CMD_SQUIRM              450
#define CMD_STRANGLE            451
#define CMD_SCOWL               452
#define CMD_SHUDDER             453
#define CMD_STRIP               454
#define CMD_SCOFF               455
#define CMD_SALUTE              456
#define CMD_SCOLD               457
#define CMD_STAGGER             458
#define CMD_TOSS                459
#define CMD_TWIRL               460
#define CMD_TOAST               461
#define CMD_TUG                 462
#define CMD_TOUCH               463
#define CMD_TREMBLE             464
#define CMD_TWITCH              465
#define CMD_WHIMPER             466
#define CMD_WHAP                467
#define CMD_WEDGE               468
#define CMD_APOLOGIZE           469
#define CMD_DESTROY             470
#define CMD_PERSONALIZE         471
#define CMD_INSERT_GEMS         472
#define CMD_ACHIEVEMENTS        473
#define CMD_REFUND              474
#define CMD_MERCY               475
#define CMD_FIND_ORIGINAL       476
//  free 477-479
#define CMD_DMANAGE             480
#define CMD_DRESTRICT           481
#define CMD_DLINK               482
#define CMD_DUNLINK             483
#define CMD_DLIST               484
#define CMD_DWHO                485
#define CMD_DGOSSIP             486
#define CMD_DTELL               487
#define CMD_DTHINK              488
#define CMD_SEND                489
#define CMD_MESSENGER           490
#define CMD_PROMPT              491
#define CMD_OOEDIT              492
#define CMD_WHOIS               493
#define CMD_OSAVE               494
#define CMD_DIG                 495
#define CMD_SCYTHE              496
#define CMD_CUT                 496
#define CMD_STATUS              497
#define CMD_SHOWSKILLS          498
#define CMD_RESETSKILLS         499
#define CMD_SETSKILLS           500
#define CMD_PERDONO             501 //FLYP 2003 Perdono
#define CMD_IMMOLATION	        502 //Flyp 20180128 --> canibalize per demoni
#define CMD_IMPTEST             503 //Alar, enables test mode
#define CMD_CHECKACHIELEVEL     504 //Montero check on achievement table
//  free 505-508
#define MAX_CMD_LIST            509
} // namespace Alarmud

#endif
