/*
 * flags.hpp
 *
 *  Created on: 24 feb 2018
 *      Author: giovanni
 */

#ifndef __FLAGS_HPP_
#define __FLAGS_HPP_

// Ok, questo è cevellotico, ma comodo per avere la certezza che i flag definiti qui vengono anche visualizzati al boot
// Quando viene incluso la prima volta, tutte le macro sono non definite e quindi le definisce, ignorando logDefineStatus.
// Alla seconda inclusione forzata da flag.cpp co#undef __FLAGS_HPP_ viene invece eseguira la parte con logDefineStatus
// e ridefiniso logDefineStatus in modo da fargli loggare il valore:
#ifndef logDefineStatus
extern void printFlags();
#define logDefineStatus(macroname) LOG4CXX_TRACE(::logger,::boost::format("LCONF %-30s: %s") % #macroname % (macroname?"ON":"off"))
#endif
#ifndef ACCESSI
#define ACCESSI			true
#else
logDefineStatus(ACCESSI)
#endif

#ifndef ALAR
#define ALAR				true
#else
logDefineStatus(ALAR)
#endif

#ifndef ALAR_RENT
#define ALAR_RENT			true
#else
logDefineStatus(ALAR_RENT)
#endif
/* se 1 il rent � normale */
#ifndef BLOCK_WRITE
#define BLOCK_WRITE		true
#else
logDefineStatus(BLOCK_WRITE)
#endif

#ifndef CHECK_RENT_INACTIVE
#define CHECK_RENT_INACTIVEtrue
#else
logDefineStatus(CHECK_RENT_INACTIVE)
#endif

#ifndef CLEAN_AT_BOOT
#define CLEAN_AT_BOOT		false
#else
logDefineStatus(CLEAN_AT_BOOT)
#endif

#ifndef DEATH_FIX
#define DEATH_FIX			true
#else
logDefineStatus(DEATH_FIX)
#endif

#ifndef DOFLEEFIGHTINGLD
#define DOFLEEFIGHTINGLD	true
#else
logDefineStatus(DOFLEEFIGHTINGLD)
#endif
/* auto flee se in combat va ld */
#ifndef EGO
#define EGO				false
#else
logDefineStatus(EGO)
#endif

#ifndef EGO_BLADE
#define EGO_BLADE			true
#else
logDefineStatus(EGO_BLADE)
#endif

#ifndef ENABLE_AUCTION
#define ENABLE_AUCTION		true
#else
logDefineStatus(ENABLE_AUCTION)
#endif

#ifndef EQPESANTE
#define EQPESANTE			true
#else
logDefineStatus(EQPESANTE)
#endif
/* L'eq pesa anche se indossato */
#ifndef FAST_TRACK
#define FAST_TRACK			true
#else
logDefineStatus(FAST_TRACK)
#endif

#ifndef HASH
#define HASH				false
#else
logDefineStatus(HASH)
#endif

#ifndef HEAVY_DEBUG
#define HEAVY_DEBUG		false
#else
logDefineStatus(HEAVY_DEBUG)
#endif

#ifndef IMPL_SECURITY
#define IMPL_SECURITY		true
#else
logDefineStatus(IMPL_SECURITY)
#endif

#ifndef LAG_MOBILES
#define LAG_MOBILES		true
#else
logDefineStatus(LAG_MOBILES)
#endif

#ifndef LEVEL_LOSS
#define LEVEL_LOSS			true
#else
logDefineStatus(LEVEL_LOSS)
#endif

#ifndef LIMITED_ITEMS
#define LIMITED_ITEMS		true
#else
logDefineStatus(LIMITED_ITEMS)
#endif

#ifndef LIMITEEQALRIENTRO
#define LIMITEEQALRIENTRO	false
#else
logDefineStatus(LIMITEEQALRIENTRO)
#endif
/* elimina tutto l'eq RARO in inventario al rientro nel mud */
#ifndef LINUX
#define LINUX				true
#else
logDefineStatus(LINUX)
#endif

#ifndef LOG
#define LOG				true
#else
logDefineStatus(LOG)
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG			true
#else
logDefineStatus(LOG_DEBUG)
#endif

#ifndef LOG_DEBUG1
#define LOG_DEBUG1			true
#else
logDefineStatus(LOG_DEBUG1)
#endif

#ifndef LOW_GOLD
#define LOW_GOLD			false
#else
logDefineStatus(LOW_GOLD)
#endif

#ifndef NETBSD
#define NETBSD				true
#else
logDefineStatus(NETBSD)
#endif

#ifndef NEW_ALIGN
#define NEW_ALIGN			true
#else
logDefineStatus(NEW_ALIGN)
#endif

#ifndef NEW_BASH
#define NEW_BASH			true
#else
logDefineStatus(NEW_BASH)
#endif

#ifndef NEW_CONNECT
#define NEW_CONNECT		true
#else
logDefineStatus(NEW_CONNECT)
#endif

#ifndef NEW_EQ_GAIN
#define NEW_EQ_GAIN		false
#else
logDefineStatus(NEW_EQ_GAIN)
#endif
/* la qualita' dell'equip influenza gli XP */
#ifndef NEW_GAIN
#define NEW_GAIN			true
#else
logDefineStatus(NEW_GAIN)
#endif

#ifndef NEW_RENT
#define NEW_RENT			false
#else
logDefineStatus(NEW_RENT)
#endif
/* se 1 Il rent non viene calcolato */
#ifndef NEW_ROLL
#define NEW_ROLL			true
#else
logDefineStatus(NEW_ROLL)
#endif

#ifndef NEW_EXP
#define NEW_EXP			false
#else
logDefineStatus(NEW_EXP)
#endif

#ifndef NEWER_EXP
#define NEWER_EXP			true
#else
logDefineStatus(NEWER_EXP)
#endif

#ifndef NICE_LIMITED
#define NICE_LIMITED		true
#else
logDefineStatus(NICE_LIMITED)
#endif

#ifndef NICE_MULTICLASS
#define NICE_MULTICLASS	true
#else
logDefineStatus(NICE_MULTICLASS)
#endif

#ifndef NICE_PKILL
#define NICE_PKILL			true
#else
logDefineStatus(NICE_PKILL)
#endif

#ifndef NO_REGISTER
#define NO_REGISTER		true
#else
logDefineStatus(NO_REGISTER)
#endif

#ifndef NODUPLICATES
#define NODUPLICATES		true
#else
logDefineStatus(NODUPLICATES)
#endif

#ifndef NOSCRAP
#define NOSCRAP			false
#else
logDefineStatus(NOSCRAP)
#endif

#ifndef NOTRACK
#define NOTRACK			false
#else
logDefineStatus(NOTRACK)
#endif

#ifndef OLD_EXP
#define OLD_EXP			false
#else
logDefineStatus(OLD_EXP)
#endif

#ifndef PREVENT_PKILL
#define PREVENT_PKILL		true
#else
logDefineStatus(PREVENT_PKILL)
#endif

#ifndef QUEST_GAIN
#define QUEST_GAIN			false
#else
logDefineStatus(QUEST_GAIN)
#endif

#ifndef SAVEWORLD
#define SAVEWORLD			false
#else
logDefineStatus(SAVEWORLD)
#endif

#ifndef SITELOCK
#define SITELOCK			true
#else
logDefineStatus(SITELOCK)
#endif

#ifndef SUSPENDREGISTER
#define SUSPENDREGISTER	true
#else
logDefineStatus(SUSPENDREGISTER)
#endif

#ifndef ZONE_COMM_ONLY
#define ZONE_COMM_ONLY		true
#else
logDefineStatus(ZONE_COMM_ONLY)
#endif



#endif /* __FLAGS_HPP_ */