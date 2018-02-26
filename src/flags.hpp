/*
 * flags.hpp
 *
 *  Created on: 24 feb 2018
 *      Author: giovanni
 */

#ifndef __FLAGS_HPP_
#define __FLAGS_HPP_
// COnfiguration constants
#define ACCESSI					true
#define ALAR					true
#define ALAR_RENT  		 		true		/* se 1 il rent � normale */
#define CHECK_RENT_INACTIVE 	true
#define CLEAN_AT_BOOT 			false
#define DEATH_FIX       		true
#define DOFLEEFIGHTINGLD 		true  /* auto flee se in combat va ld */
#define EGO             		false
#define EGO_BLADE       		true
#define ENABLE_AUCTION			true
#define EQPESANTE 				true  /* L'eq pesa anche se indossato */
#define FAST_TRACK      		true
#define HEAVY_DEBUG				false
#define IMPL_SECURITY			true
#define LAG_MOBILES     		true
#define LEVEL_LOSS      		true
#define LIMITED_ITEMS   		true
#define LIMITEEQALRIENTRO 		false /* elimina tutto l'eq RARO in inventario al rientro nel mud */
#define LINUX					true
#define LOG						true
#define LOG_DEBUG				true
#define LOG_DEBUG1				true
#define LOW_GOLD        		false
#define NETBSD					true
#define NEW_ALIGN 				true
#define NEW_BASH        		true
#define NEW_CONNECT     		true
#define NEW_EQ_GAIN     		false /* la qualita' dell'equip influenza gli XP */
#define NEW_GAIN        		true
#define NEW_RENT  		 		false    /* se 1 Il rent non viene calcolato */
#define NEW_ROLL        		true
#define NEWER_EXP       		true
#define NICE_LIMITED 			true
#define NICE_MULTICLASS 		true
#define NICE_PKILL      		true
#define NO_REGISTER				true
#define NODUPLICATES    		true
#define NOSCRAP         		false
#define PREVENT_PKILL   		true
#define SAVEWORLD				false
#define SITELOCK        		true
#define SUSPENDREGISTER 		true
#define ZONE_COMM_ONLY  		true

#define LOG_DEFINE(def) LOG_DBG(boost::format("%35s: %s") << #def << (def?"ON":"OFF"))

#define EQINDEX 600.0

#define MAX_ALIAS 10
#define BEG_OF_TIME 827100000
#define PRINCEEXP 400000000





#endif /* __FLAGS_HPP_ */
