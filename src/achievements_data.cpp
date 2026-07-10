/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "maximums.hpp"
#include "utils.hpp"

namespace Alarmud {

struct XpAchieTable RewardXp[ABS_MAX_LVL] = {
	//	lev_1_xp	lev_2_xp	lev_3_xp	lev_4_xp	lev_5_xp	lev_6_xp	lev_7_xp	lev_8_xp	lev_9_xp	lev_10_xp
	{		  1,		  1,		  1,		  1,		  1,		  1,		  1,		  1,		  1,		  1	},	//	 0
	{		399,		439,		479,		519,		559,		599,		639,		679,		719,		799	},	//	NOVIZIO
	{		850,		935,	   1020,	   1105,	   1190,	   1275,	   1360,	   1445,	   1530,	   1700	},	//	 2
	{	   1000,	   1100,	   1200,	   1300,	   1400,	   1500,	   1600,	   1700,	   1800,	   2000	},	//	 3
	{	   1250,	   1375,	   1500,	   1625,	   1750,	   1875,	   2000,	   2125,	   2250,	   2500	},	//	 4
	{	   1600,	   1760,	   1920,	   2080,	   2240,	   2400,	   2560,	   2720,	   2880,	   3200	},	//	 5
	{	   1900,	   2090,	   2280,	   2470,	   2660,	   2850,	   3040,	   3230,	   3420,	   3800	},	//	 6
	{	   2300,	   2530,	   2760,	   2990,	   3220,	   3450,	   3680,	   3910,	   4140,	   4600	},	//	 7
	{	   2800,	   3080,	   3360,	   3640,	   3920,	   4200,	   4480,	   4760,	   5040,	   5600	},	//	 8
	{	   3500,	   3850,	   4200,	   4550,	   4900,	   5250,	   5600,	   5950,	   6300,	   7000	},	//	 9
	{	   4500,	   4950,	   5400,	   5850,	   6300,	   6750,	   7200,	   7650,	   8100,	   9000	},	//	10
	{	   5500,	   6050,	   6600,	   7150,	   7700,	   8250,	   8800,	   9350,	   9900,	  11000	},	//	ALLIEVO
	{	   6500,	   7150,	   7800,	   8450,	   9100,	   9750,	  10400,	  11050,	  11700,	  13000	},	//	12
	{	   8000,	   8800,	   9600,	  10400,	  11200,	  12000,	  12800,	  13600,	  14400,	  16000	},	//	13
	{	   9500,	  10450,	  11400,	  12350,	  13300,	  14250,	  15200,	  16150,	  17100,	  19000	},	//	14
	{	  12500,	  13750,	  15000,	  16250,	  17500,	  18750,	  20000,	  21250,	  22500,	  25000	},	//	15
	{	  13500,	  14250,	  15750,	  17250,	  18000,	  19500,	  21000,	  22500,	  24000,	  27000	},	//	16
	{	  16650,	  17575,	  19425,	  21275,	  22200,	  24050,	  25900,	  27750,	  29600,	  33300	},	//	17
	{	  20700,	  21850,	  24150,	  26450,	  27600,	  29900,	  32200,	  34500,	  36800,	  41400	},	//	18
	{	  25200,	  26600,	  29400,	  32200,	  33600,	  36400,	  39200,	  42000,	  44800,	  50400	},	//	19
	{	  31050,	  32775,	  36225,	  39675,	  41400,	  44850,	  48300,	  51750,	  55200,	  62100	},	//	20
	{	  33600,	  35700,	  37800,	  39900,	  42000,	  46200,	  50400,	  54600,	  58800,	  67200	},	//	APPRENDISTA
	{	  42000,	  44625,	  47250,	  49875,	  52500,	  57750,	  63000,	  68250,	  73500,	  84000	},	//	22
	{	  51200,	  54400,	  57600,	  60800,	  64000,	  70400,	  76800,	  83200,	  89600,	 102400	},	//	23
	{	  63200,	  67150,	  71100,	  75050,	  79000,	  86900,	  94800,	 102700,	 110600,	 126400	},	//	24
	{	  76800,	  81600,	  86400,	  91200,	  96000,	 105600,	 115200,	 124800,	 134400,	 153600	},	//	25
	{	  84000,	  90000,	  96000,	 102000,	 108000,	 120000,	 132000,	 144000,	 156000,	 180000	},	//	26
	{	 105000,	 112500,	 120000,	 127500,	 135000,	 150000,	 165000,	 180000,	 195000,	 225000	},	//	27
	{	 126000,	 135000,	 144000,	 153000,	 162000,	 180000,	 198000,	 216000,	 234000,	 270000	},	//	28
	{	 154000,	 165000,	 176000,	 187000,	 198000,	 220000,	 242000,	 264000,	 286000,	 330000	},	//	29
	{	 165000,	 178750,	 192500,	 206250,	 220000,	 233750,	 255750,	 277750,	 302500,	 357500	},	//	30
	{	 170850,	 184250,	 199325,	 212725,	 226125,	 241200,	 264650,	 288100,	 314900,	 378550	},	//	INIZIATO
	{	 174300,	 190900,	 207500,	 221195,	 236550,	 253150,	 277220,	 302120,	 329925,	 394250	},	//	32
	{	 178500,	 196350,	 215220,	 230010,	 246330,	 265200,	 280500,	 296310,	 314160,	 410550	},	//	33
	{	 182700,	 202860,	 223650,	 239400,	 256410,	 277200,	 293895,	 310590,	 329490,	 427140	},	//	34
	{	 187110,	 208670,	 232540,	 249480,	 267575,	 290290,	 307230,	 325710,	 345730,	 446600	},	//	35
	{	 192080,	 214620,	 241570,	 259700,	 278810,	 303800,	 321440,	 341040,	 362110,	 466480	},	//	36
	{	 197225,	 220800,	 251275,	 270250,	 290375,	 317400,	 335800,	 356500,	 379500,	 486450	},	//	37
	{	 202860,	 227080,	 261800,	 281400,	 302400,	 331100,	 350700,	 372400,	 397600,	 508200	},	//	38
	{	 208620,	 233460,	 272700,	 292860,	 315000,	 345600,	 366480,	 389340,	 416700,	 531000	},	//	39
	{	 215000,	 240800,	 284875,	 306375,	 330025,	 363350,	 385710,	 410650,	 440750,	 559000	},	//	40
	{	 226800,	 253800,	 302940,	 326160,	 353700,	 390150,	 414450,	 442260,	 476550,	 604800	},	//	ESPERTO
	{	 247500,	 275880,	 330330,	 355740,	 387750,	 428340,	 455400,	 487740,	 528000,	 669900	},	//	42
	{	 279450,	 309825,	 369765,	 397710,	 434565,	 480330,	 511231,	 549585,	 599400,	 761400	},	//	43
	{	 324225,	 356895,	 422730,	 453915,	 496980,	 549351,	 586575,	 633600,	 696960,	 886050	},	//	44
	{	 387450,	 424350,	 496305,	 531360,	 581790,	 642675,	 688800,	 747840,	 830865,	1054417	},	//	45
	{	 490750,	 532275,	 610417,	 650055,	 709700,	 781425,	 838050,	 913550,	1022270,	1291050	},	//	MAESTRO
	{	 610500,	 656750,	 741850,	 787175,	 858400,	 943500,	1013337,	1109075,	1248750,	1572500	},	//	47
	{	 744250,	 796920,	 889665,	 943480,	1028782,	1130115,	1216219,	1336215,	1514835,	1905280	},	//	48
	{	 896000,	 959000,	1059800,	1122800,	1223600,	1344000,	1450400,	1596000,	1824200,	2296000	},	//	49
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	BARONE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	PRINCIPE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	IMMORTALE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	DIO_MINORE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	DIO
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	MAESTRO_DEGLI_DEI
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	CREATORE
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	QUESTMASTER
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	MAESTRO_DEL_CREATO
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	MAESTRO_DEI_CREATORI
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	IMMENSO
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},	//	65
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	},
	{	1250000,	1375000,	1537500,	1662500,	1837500,	2037500,	2231250,	2475000,	2837500,	5000000	}
};

struct RandomMaterialsTable MaterialName[5][100] = {
	//	organic 16
	{	//	female								neutral								4										5									key
		{	"d'ossa",							"d'ossa",							"d'ossa",								"d'ossa",							" ossa"						},
		{	"di legno",							"di legno",							"di legno",								"di legno",							" legno"					},
		{	"in pelle di drago",				"in pelle di drago",				"in pelle di drago",					"in pelle di drago",				" pelle drago"				},
		{	"ricoperta di conchiglie",			"ricoperto di conchiglie",			"ricoperte di conchiglie",				"ricoperti di conchiglie",			" conchiglie"				},
		{	"di legno di quercia",				"di legno di quercia",				"di legno di quercia",					"di legno di quercia",				" quercia"					},
		{	"di seta",							"di seta",							"di seta",								"di seta",							" seta"						},
		{	"di ebano",							"di ebano",							"di ebano",								"di ebano",							" ebano"					},
		{	"ricoperta da una strana corteccia","ricoperto da una strana corteccia","ricoperte da una strana corteccia",	"ricoperti da una strana corteccia"," corteccia"				},
		{	"di sangue condensato",				"di sangue condensato",				"di sangue condensato",					"di sangue condensato",				" sangue"					},
		{	"di ghiaccio",						"di ghiaccio",						"di ghiaccio",							"di ghiaccio",						" ghiaccio"					},
		{	"in pelle di lupo",					"in pelle di lupo",					"in pelle di lupo",						"in pelle di lupo",					" pelle lupo"				},
		{	"in pelle di serpente",				"in pelle di serpente",				"in pelle di serpente",					"in pelle di serpente",				" pelle serpente"   		},
		{	"di cuoio",							"di cuoio",							"di cuoio",								"di cuoio",							" cuoio"					},
		{	"di ebano nero",					"di ebano nero",					"di ebano nero",						"di ebano nero",					" ebano nero"				},
		{	"di pelle",							"di pelle",							"di pelle",								"di pelle",							" pelle"					},
		{	"di legno di frassino",				"di legno di frassino",				"di legno di frassino",					"di legno di frassino",				" frassiono"				}
	},
	//	metallic    17
	{	//	female								neutral								4										5									key
		{	"d'oro",							"d'oro",							"d'oro",								"d'oro",							" oro"						},
		{	"di bronzo",						"di bronzo",						"di bronzo",							"di bronzo",						" bronzo"					},
		{	"di rame",							"di rame",							"di rame",								"di rame",							" rame"						},
		{	"di argento",						"di argento",						"di argento",							"di argento",						" argento"					},
		{	"di ferro",							"di ferro",							"di ferro",								"di ferro",							" ferro"					},
		{	"di mithril",						"di mithril",						"di mithril",							"di mithril",						" mithril"					},
		{	"di platino",						"di platino",						"di platino",							"di platino",						" platino"					},
		{	"di vibranio",						"di vibranio",						"di vibranio",							"di vibranio",						" vibranio"					},
		{	"di metallo",						"di metallo",						"di metallo",							"di metallo",						" metallo"					},
		{	"di acciaio",						"di acciaio",						"di acciaio",							"di acciaio",						" acciaio"					},
		{	"di stagno",						"di stagno",						"di stagno",							"di stagno",						" stagno"					},
		{	"di adamantio",						"di adamantio",						"di adamantio",							"di adamantio",						" adamantio"				},
		{	"di titanio",						"di titanio",						"di titanio",							"di titanio",						" titanio"					},
		{	"di gromril",						"di gromril",						"di gromril",							"di gromril",						" gromril"					},
		{	"di warpietra",						"di warpietra",						"di warpietra",							"di warpietra",						" warpietra"				},
		{	"di saronite",						"di saronite",						"di saronite",							"di saronite",						" saronite"					},
		{	"di azerite",						"di azerite",						"di azerite",							"di azerite",						" azerite"					}
	},
	//	various 29
	{	//	female								neutral								4										5									key
		{	"antica",							"antico",							"antiche",								"antichi",							""							},
		{	"maledetta",						"maledetto",						"maledette",							"maledetti",						""							},
		{	"benedetta",						"benedetto",						"benedette",							"benedetti",						""							},
		{	"invisibile",						"invisibile",						"invisibili",							"invisibili",						""							},
		{	"sacra",							"sacro",							"sacre",								"sacri",							""							},
		{	"magica",							"magico",							"magiche",								"magici",							""							},
		{	"molto antica",						"molto antico",						"molto antiche",						"molto antichi",					""							},
		{	"malandata",						"malandato",						"malandate",							"malandati",						""							},
		{	"infuocata",						"infuocato",						"infuocate",							"infuocati",						""							},
		{	"rara",								"raro",								"rare",									"raroi",							""							},
		{	"rinforzata",						"rinforzato",						"rinforzate",							"rinforzati",						""							},
		{	"scheggiata",						"scheggiato",						"scheggiate",							"scheggiati",						""							},
		{	"con delle crepe",					"con delle crepe",					"con delle crepe",						"con delle crepe",					" crepe"					},
		{	"ammaccata",						"ammaccato",						"ammaccate",							"ammaccati",						""							},
		{	"insanguinata",						"insanguinato",						"insanguinate",							"insanguinati",						""							},
		{	"cesellata",						"cesellato",						"cesellate",							"cesellati",						""							},
		{	"ammantata d'Ombra",				"ammantato d'Ombra",				"ammantate d'Ombra",					"ammantati d'Ombra",				" ombra"					},
		{	"luminosa",							"luminoso",							"luminose",								"luminosi",							""							},
		{	"risplendete di luce",				"risplendete di luce",				"risplendeti di luce",					"risplendeti di luce",				" luce"						},
		{	"d'Ombra",							"d'Ombra",							"d'Ombra",								"d'Ombra",							" ombra"					},
		{	"intarsiata",						"intarsiato",						"intarsiate",							"intarsiati",						""							},
		{	"con venature metalliche",			"con venature metalliche",			"con venature metalliche",				"con venature metalliche",			" venature"					},
		{	"decorata",							"decorato",							"decorate",								"decorati",							""							},
		{	"molto ben lavorata",				"molto ben lavorato",				"molto ben lavorate",					"molto ben lavorati",				""							},
		{	"trasparente",						"trasparente",						"trasparenti",							"trasparenti",						""							},
		{	"leggendaria",						"leggendario",						"leggendarie",							"leggendari",						""							},
		{	"usurata",							"usurato",							"usurate",								"usurati",							""							},
		{	"di rara bellezza",					"di rara bellezza",					"di rara bellezza",						"di rara bellezza",					""							},
		{	"scintillante",						"scintillante",						"scintillanti",							"scintillanti",						""							}
	},
	//	colour 24
	{	//	female								neutral								4										5									key
		{	"$c0009rossa$c0007",				"$c0009rosso$c0007",				"$c0009rosse$c0007",					"$c0009rossi$c0007",				""							},
		{	"$c0008nera$c0007",					"$c0008nero$c0007",					"$c0008nere$c0007",						"$c0008neri$c0007",					""							},
		{	"$c0015bianca$c0007",				"$c0015bianco$c0007",				"$c0015bianche$c0007",					"$c0015bianchi$c0007",				""							},
		{	"$c0010verde$c0007",				"$c0010verde$c0007",				"$c0010verdi$c0007",					"$c0010verdi$c0007",				""							},
		{	"$c0011gialla$c0007",				"$c0011giallo$c0007",				"$c0011gialle$c0007",					"$c0011gialli$c0007",				""							},
		{	"$c0005viola$c0007",				"$c0005viola$c0007",				"$c0005viola$c0007",					"$c0005viola$c0007",				""							},
		{	"arancione",						"arancione",						"arancioni",							"arancioni",						""							},
		{	"$c0004blu$c0007",					"$c0004blu$c0007",					"$c0004blu$c0007",						"$c0004blu$c0007",					""							},
		{	"$c0012azzurra$c0007",				"$c0012azzurro$c0007",				"$c0012azzurre$c0007",					"$c0012azzurri$c0007",				""							},
		{	"$c0012in$c0005da$c0012co$c0007",	"$c0012in$c0005da$c0012co$c0007",	"$c0012in$c0005da$c0012co$c0007",		"$c0012in$c0005da$c0012co$c0007",	""							},
		{	"$c0003marrone$c0007",				"$c0003marrone$c0007",				"$c0003marroni$c0007",					"$c0003marroni$c0007",				""							},
		{	"$c0008nero pece$c0007",			"$c0008nero pece$c0007",			"$c0008nero pece$c0007",				"$c0008nero pece$c0007",			""							},
		{	"$c0011giallo canarino$c0007",		"$c0011giallo canarino$c0007",		"$c0011giallo canarino$c0007",			"$c0011giallo canarino$c0007",		""							},
		{	"$c0012blu intenso$c0007",			"$c0012blu intenso$c0007",			"$c0012blu intenso$c0007",				"$c0012blu intenso$c0007",			""							},
		{	"$c0004blu mare$c0007",				"$c0004blu mare$c0007",				"$c0004blu mare$c0007",					"$c0004blu mare$c0007",				""							},
		{	"$c0001amaranto$c0007",				"$c0001amaranto$c0007",				"$c0001amaranto$c0007",					"$c0001amaranto$c0007",				""							},
		{	"$c0009rosso fuoco$c0007",			"$c0009rosso fuoco$c0007",			"$c0009rosso fuoco$c0007",				"$c0009rosso fuoco$c0007",			""							},
		{	"$c0011dorata$c0007",				"$c0011dorato$c0007",				"$c0011dorate$c0007",					"$c0011dorati$c0007",				""							},
		{	"$c0015argentata$c0007",			"$c0015argentato$c0007",			"$c0015argentate$c0007",				"$c0015argentati$c0007",			""							},
		{	"$c0014celeste$c0007",				"$c0014celeste$c0007",				"$c0014celesti$c0007",					"$c0014celesti$c0007",				""							},
		{	"grigia",							"grigio",							"grigie",								"grigi",							""							},
		{	"bianco sporco",					"bianco sporco",					"bianco sporco",						"bianco sporco",					""							},
		{	"$c0013rosa$c0007",					"$c0013rosa$c0007",					"$c0013rosa$c0007",						"$c0013rosa$c0007",					""							},
		{	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014e$c0007",	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014e$c0007",	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014i$c0007",	"$c0014m$c0013u$c0012l$c0011t$c0010i$c0009c$c0010o$c0011l$c0012o$c0013r$c0014i$c0007",	""	}
	},
	//	mineral 15
	{	//	female								neutral								4										5									key
		{	"di roccia",						"di roccia",						"di roccia",							"di roccia",						" roccia"					},
		{	"di ossidiana",						"di ossidiana",						"di ossidiana",							"di ossidiana",						" ossidiana"				},
		{	"di argilla",						"di argilla",						"di argilla",							"di argilla",						" argilla"					},
		{	"di diamante",						"di diamante",						"di diamante",							"di diamante",						" diamante"					},
		{	"di cristallo",						"di cristallo",						"di cristallo",							"di cristallo",						" cristallo"				},
		{	"di vetro",							"di vetro",							"di vetro",								"di vetro",							" vetro"					},
		{	"di granito",						"di granito",						"di granito",							"di granito",						" granito"					},
		{	"di marmo",							"di marmo",							"di marmo",								"di marmo",							" marmo"					},
		{	"di quarzo",						"di quarzo",						"di quarzo",							"di quarzo",						" quarzo"					},
		{	"in resina",						"in resina",						"in resina",							"in resina",						" resina"					},
		{	"di basalto",						"di basalto",						"di basalto",							"di basalto",						" basalto"					},
		{	"di avorio",						"di avorio",						"di avorio",							"di avorio",						" avorio"					},
		{	"di kryptonite",					"di kryptonite",					"di kryptonite",						"di kryptonite",					" kryptonite"				},
		{	"d'ambra",							"d'ambra",							"d'ambra",								"d'ambra",							" ambra"					},
		{	"di bachelite",						"di bachelite",						"di bachelite",							"di bachelite",						" bachelite"				}
	}
};

struct RandomEquipTable EquipName[22][20] = {
						//	4 = plurale femminile		5 = plurale neutro
	{	//  finger 9					gender			key
		{	"un anello",				SEX_NEUTRAL,	"anello"				},
		{	"una fede",					SEX_FEMALE,		"fede"      			},
		{	"un anellino",				SEX_NEUTRAL,	"anellino"  			},
		{	"una fedina",				SEX_FEMALE,		"fedina"    			},
		{	"un cerchietto",			SEX_NEUTRAL,	"cerchietto"			},
		{	"un simbolo",				SEX_NEUTRAL,	"simbolo"   			},
		{	"un sigillo",				SEX_NEUTRAL,	"sigillo"   			},
		{	"un tirapugni",				SEX_NEUTRAL,	"tirapugni" 			},
		{	"una fascetta",				SEX_FEMALE,		"fascetta"  			}
	},
	{	//	wrist 9						gender			key
		{	"un bracciale",				SEX_NEUTRAL,	"bracciale"				},
		{	"un braccialetto",			SEX_NEUTRAL,	"braccialetto"			},
		{	"otto braccialetti",		5,				"otto braccialetti"		},
		{	"un ciondolo",				SEX_NEUTRAL,	"ciondolo"				},
		{	"una catenina",				SEX_FEMALE,		"catenina"				},
		{	"un cinturino",				SEX_NEUTRAL,	"cinturino"				},
		{	"delle manette",			4,				"manette"				},
		{	"una fascia",				SEX_FEMALE,		"fascia"				},
		{	"un cerchietto",			SEX_NEUTRAL,	"cerchietto"			}
	},
	{	//	ear 9						gender			key
		{	"un orecchino",				SEX_NEUTRAL,	"orecchino"				},
		{	"un pendente",				SEX_NEUTRAL,	"pendente"				},
		{	"un gioiello",				SEX_NEUTRAL,	"gioiello"				},
		{	"un monile",				SEX_NEUTRAL,	"monile"				},
		{	"una buccola",				SEX_FEMALE,		"buccola"				},
		{	"degli orecchini",			5,				"orecchini"				},
		{	"un anellino",				SEX_NEUTRAL,	"anellino"				},
		{	"un cerchio",				SEX_NEUTRAL,	"cerchio"				},
		{	"una gocciola",				SEX_FEMALE,		"gocciola"				}
	},
	{	//	neck 11						gender			key
		{	"una collana",				SEX_FEMALE,		"collana"				},
		{	"un ciondolo",				SEX_NEUTRAL,	"ciondolo"				},
		{	"una catenina",				SEX_FEMALE,		"catenina"				},
		{	"un amuleto",				SEX_NEUTRAL,	"amuleto"				},
		{	"un medaglione",			SEX_NEUTRAL,	"medaglione"			},
		{	"uno scarabeo",				SEX_NEUTRAL,	"scarabeo"				},
		{	"un pendaglio",				SEX_NEUTRAL,	"pendaglio"				},
		{	"un talismano",				SEX_NEUTRAL,	"talismano"				},
		{	"un monile",				SEX_NEUTRAL,	"monile"				},
		{	"un girocollo",				SEX_NEUTRAL,	"girocollo"				},
		{	"un collare",				SEX_NEUTRAL,	"collare"				}
	},
	{	//	eyes 11						gender			key
		{	"degli occhiali",			5,				"occhiali"				},
		{	"un monocolo",				SEX_NEUTRAL,	"monocolo"				},
		{	"una visiera",				SEX_FEMALE,		"visiera"				},
		{	"delle lenti",				4,				"lenti"					},
		{	"una lente",				SEX_FEMALE,		"lente"					},
		{	"una maschera",				SEX_FEMALE,		"maschera"				},
		{	"una mascherina",			SEX_FEMALE,		"mascherina"			},
		{	"un occhio",				SEX_NEUTRAL,	"occhio"				},
		{	"una benda",				SEX_FEMALE,		"benda"					},
		{	"una celata",				SEX_FEMALE,		"celata"				},
		{	"un visore",				SEX_NEUTRAL,	"visore"				}
	},
	{	//	hold 11						gender			key
		{	"un tomo",					SEX_NEUTRAL,	"tomo"					},
		{	"un corno",					SEX_NEUTRAL,	"corno"					},
		{	"un simbolo",				SEX_NEUTRAL,	"simbolo"				},
		{	"un grimorio",				SEX_NEUTRAL,	"grimorio"				},
		{	"una collanina",			SEX_FEMALE,		"collanina"				},
		{	"un rosario",				SEX_NEUTRAL,	"rosario"				},
		{	"una pietra",				SEX_FEMALE,		"pietra"				},
		{	"una perla",				SEX_FEMALE,		"perla"					},
		{	"una roccia",				SEX_FEMALE,		"roccia"				},
		{	"una gemma",				SEX_FEMALE,		"gemma"					},
		{	"un sigillo",				SEX_NEUTRAL,	"sigillo"				}
	},
	{	//	light 14					gender			key
		{	"una sfera di luce",		SEX_FEMALE,		"sfera luce"			},
		{	"un diamante brillante",	SEX_NEUTRAL,	"diamante brillante"	},
		{	"una fiamma eterna",		SEX_FEMALE,		"fiamma eterna"			},
		{	"una sfera danzante",		SEX_FEMALE,		"sfera danzante"		},
		{	"una torcia infuocata",		SEX_FEMALE,		"torcia infuocata"		},
		{	"una lampada antica",		SEX_FEMALE,		"lampada antica"		},
		{	"una lanterna da minatore",	SEX_FEMALE,		"lanterna minatore"		},
		{	"una lanterna antica",		SEX_FEMALE,		"lanterna antica"		},
		{	"una piccola stella",		SEX_FEMALE,		"stella piccola"		},
		{	"un frammento di stella",	SEX_NEUTRAL,	"stella frammento"		},
		{	"una gemma splendente",		SEX_FEMALE,		"gemma splendente"		},
		{	"una candela",				SEX_FEMALE,		"candela"				},
		{	"una fatina",				SEX_FEMALE,		"fatina"				},
		{	"un cerino",				SEX_NEUTRAL,	"cerino"				},
		{	"un fiammifero",			SEX_NEUTRAL,	"fiammifero"			}
	},
	{	//	back 10						gender			key
		{	"una borsa",				SEX_FEMALE,		"borsa"					},
		{	"uno zaino",				SEX_NEUTRAL,	"zaino"					},
		{	"una sacca",				SEX_FEMALE,		"sacca"					},
		{	"un forziere",				SEX_NEUTRAL,	"forziere"				},
		{	"un barile",				SEX_NEUTRAL,	"barile"				},
		{	"un cesto",					SEX_NEUTRAL,	"cesto"					},
		{	"una gerla",				SEX_FEMALE,		"gerla"					},
		{	"una cassapanca",			SEX_FEMALE,		"cassapanca"			},
		{	"una bisaccia",				SEX_FEMALE,		"bisaccia"				},
		{	"una cesta",				SEX_FEMALE,		"cesta"					}
	},
	{	//	about body 9				gender			key
		{	"un mantello",				SEX_NEUTRAL,	"mantello"				},
		{	"un manto",					SEX_NEUTRAL,	"manto"					},
		{	"una pelliccia",			SEX_FEMALE,		"pelliccia"				},
		{	"un vello",					SEX_NEUTRAL,	"vello"					},
		{	"una cappa",				SEX_FEMALE,		"cappa"					},
		{	"un tabarro",				SEX_NEUTRAL,	"tabarro"				},
		{	"un sari",					SEX_NEUTRAL,	"sari"					},
		{	"una pianeta",				SEX_FEMALE,		"pianeta"				},
		{	"un caftano",				SEX_NEUTRAL,	"caftano"				}
	},
	{	//	waist 9						gender			key
		{	"una cintura",				SEX_FEMALE,		"cintura"				},
		{	"un nastro",				SEX_NEUTRAL,	"nastro"				},
		{	"un cinturone",				SEX_NEUTRAL,	"cinturone"				},
		{	"una corda",				SEX_FEMALE,		"corda"					},
		{	"una liana",				SEX_FEMALE,		"liana"					},
		{	"un gonnellino",			SEX_NEUTRAL,	"gonnellino"			},
		{	"una fascia",				SEX_FEMALE,		"fascia"				},
		{	"una catena",				SEX_FEMALE,		"catena"				},
		{	"una fusciacca",			SEX_FEMALE,		"fusciacca"				}
	},
	{	//	feet 9						gender			key
		{	"degli stivali",			5,				"stivali"				},
		{	"un paio di stivali",		5,				"stivali paio"			},
		{	"delle scarpe",				4,				"scarpe"				},
		{	"un paio di scarpe",		4,				"scarpe paio"			},
		{	"dei calzari",				5,				"calzari"				},
		{	"degli anfibi",				5,				"anfibi"				},
		{	"dei sandali",				5,				"sandali"				},
		{	"degli scarponi",			5,				"scarponi"				},
		{	"un paio di scarponi",		5,				"scarponi paio"			}
	},
	{	//	hands 9						gender			key
		{	"un paio di guanti",		5,				"guanti paio"			},
		{	"dei guanti",				5,				"guanti"				},
		{	"dei guantoni",				5,				"guantoni"				},
		{	"delle fasce",				4,				"fasce"					},
		{	"delle placche",			4,				"placche"				},
		{	"delle protezioni",			4,				"protezioni"			},
		{	"dei gusci",				5,				"gusci"					},
		{	"degli uncini",				5,				"unicini"				},
		{	"degli artigli",			5,				"artigli"				}
	},
	{	//	body 10						gender			key
		{	"una armatura",				SEX_FEMALE,		"armatura"				},
		{	"una veste",				SEX_FEMALE,		"veste"					},
		{	"una corazza",				SEX_FEMALE,		"corazza"				},
		{	"una cotta",				SEX_FEMALE,		"cotta"					},
		{	"un corpetto",				SEX_NEUTRAL,	"corpetto"				},
		{	"un giustacuore",			SEX_NEUTRAL,	"giustacuore"			},
		{	"una uniforme",				SEX_FEMALE,		"uniforme"				},
		{	"un pettorale",				SEX_NEUTRAL,	"pettorale"				},
		{	"un saio",					SEX_NEUTRAL,	"saio"					},
		{	"un'armatura",				SEX_FEMALE,		"armatura"				},
		{	"un kimono",				SEX_NEUTRAL,	"kimono"				}
	},
	{	//	head 9						gender			key
		{	"un elmo",					SEX_NEUTRAL,	"elmo"					},
		{	"una corona",				SEX_FEMALE,		"corona"				},
		{	"una fascia",				SEX_FEMALE,		"fascia"				},
		{	"una bandana",				SEX_FEMALE,		"bandana"				},
		{	"una testa",				SEX_FEMALE,		"testa"					},
		{	"un cappello",				SEX_NEUTRAL,	"cappello"				},
		{	"un cappuccio",				SEX_NEUTRAL,	"cappuccio"				},
		{	"un diadema",				SEX_NEUTRAL,	"diadema"				},
		{	"un copricapo",				SEX_NEUTRAL,	"copricapo"				}
	},

	{	//	legs 9						gender			key
		{	"dei gambali",				5,				"gambali"				},
		{	"dei pantaloni",			5,				"pantaloni"				},
		{	"delle calze",				4,				"calze"					},
		{	"un paio di pantaloni",		5,				"pantaloni paio"		},
		{	"degli schinieri",			5,				"schinieri"				},
		{	"delle fasciature",			4,				"fasciature"			},
		{	"delle stecche",			4,				"stecche"				},
		{	"delle protezioni",			4,				"protezioni"			},
		{	"delle calotte",			4,				"calotte"				}
	},
	{	//	arms 9						gender			key
		{	"delle maniche",			4,				"maniche"				},
		{	"dei bracciali",			5,				"bracciali"				},
		{	"dei parabraccia",			5,				"parabraccia"			},
		{	"una cubitiera",			SEX_FEMALE,		"cubitiera"				},
		{	"delle lastre",				4,				"lastre"				},
		{	"un vambrace",				SEX_NEUTRAL,	"vambrace"				},
		{	"una protezione",			SEX_FEMALE,		"protezione"			},
		{	"delle bende",				4,				"bende"					},
		{	"un carapace",				SEX_NEUTRAL,	"carapace"				}
	},
	{	//	shield 14					gender			key
		{	"uno scudo",				SEX_NEUTRAL,	"scudo"					},
		{	"uno scutum",				SEX_NEUTRAL,	"scutum"				},
		{	"un pavese",				SEX_NEUTRAL,	"pavese"				},
		{	"uno scudo crociato",		SEX_NEUTRAL,	"scudo crociato"		},
		{	"uno scudo rotondo",		SEX_NEUTRAL,	"scudo rotondo"			},
		{	"uno scudo vichingo",		SEX_NEUTRAL,	"scudo vichingo"		},
		{	"uno scudo puntuto",		SEX_NEUTRAL,	"scudo puntuto"			},
		{	"un buckler",				SEX_NEUTRAL,	"buckler"				},
		{	"una lastra",				SEX_FEMALE,		"lastra"				},
		{	"uno scudo triangolare",	SEX_NEUTRAL,	"scudo triangolare"		},
		{	"uno scudo antisommossa",	SEX_NEUTRAL,	"scudo antisommossa"	},
		{	"una porta",				SEX_FEMALE,		"porta"					},
		{	"un kite",					SEX_NEUTRAL,	"kite"					},
		{	"uno scudo a rotella",		SEX_NEUTRAL,	"scudo rotella"			}
	},
	{	//	blunt weapon 15				gender			key
		{	"un martello",				SEX_NEUTRAL,	"martello"				},
		{	"una mazza",				SEX_FEMALE,		"mazza"					},
		{	"una stella del mattino",	SEX_FEMALE,		"stella mattino"		},
		{	"una tazza da te'",			SEX_FEMALE,		"tazza te"				},
		{	"uno scettro",				SEX_NEUTRAL,	"scettro"				},
		{	"un manganello",			SEX_NEUTRAL,	"manganello"			},
		{	"una clava",				SEX_FEMALE,		"clava"					},
		{	"una padella",				SEX_FEMALE,		"padella"				},
		{	"un tonfa",					SEX_NEUTRAL,	"tonfa"					},
		{	"un bastone",				SEX_NEUTRAL,	"bastone"				},
		{	"una mazza ferrata",		SEX_FEMALE,		"mazza ferrata"			},
		{	"un mazzafrusto",			SEX_NEUTRAL,	"mazzafrusto"			},
		{	"un nunchaku",				SEX_NEUTRAL,	"nunchaku"				},
		{	"un flagello",				SEX_NEUTRAL,	"flagello"				},
		{	"un martello da guerra",	SEX_NEUTRAL,	"martello guerra"		}
	},
	{	//	slash weapon 19				gender			key
		{	"una spada",				SEX_FEMALE,		"spada"					},
		{	"una scimitarra",			SEX_FEMALE,		"scimitarra"			},
		{	"una falce",				SEX_FEMALE,		"falce"					},
		{	"un'ascia",					SEX_FEMALE,		"ascia"					},
		{	"una spada corta",			SEX_FEMALE,		"spada corta"			},
		{	"un'azza",					SEX_FEMALE,		"azza"					},
		{	"uno spadone",				SEX_NEUTRAL,	"spadone"				},
		{	"un'alabarda",				SEX_FEMALE,		"alabarda"				},
		{	"una sciabola",				SEX_FEMALE,		"sciabola"				},
		{	"una claymore",				SEX_FEMALE,		"claymore"				},
		{	"una daga",					SEX_FEMALE,		"daga"					},
		{	"una spatha",				SEX_FEMALE,		"spatha"				},
		{	"un falcione",				SEX_NEUTRAL,	"falcione"				},
		{	"una flamberga",			SEX_FEMALE,		"flamberga"				},
		{	"un gladio",				SEX_NEUTRAL,	"gladio"				},
		{	"una katana",				SEX_FEMALE,		"katana"				},
		{	"un machete",				SEX_NEUTRAL,	"machete"				},
		{	"un tomahawk",				SEX_NEUTRAL,	"tomahawk"				},
		{	"un'ascia bipenne",			SEX_FEMALE,		"ascia bipenne"			}
	},
	{	//	pierce weapon 15			gender			key
		{	"un pugnale",				SEX_NEUTRAL,	"pugnale"				},
		{	"uno stocco",				SEX_NEUTRAL,	"stocco"				},
		{	"un fioretto",				SEX_NEUTRAL,	"fioretto"				},
		{	"un coltello",				SEX_NEUTRAL,	"coltello"				},
		{	"una lancia",				SEX_FEMALE,		"lancia"				},
		{	"un paio di forbici",		4,				"forbici paio"			},
		{	"uno stiletto",				SEX_NEUTRAL,	"stiletto"				},
		{	"un tridente",				SEX_NEUTRAL,	"tridente"				},
		{	"un punteruolo",			SEX_NEUTRAL,	"punteruolo"			},
		{	"una picca",				SEX_FEMALE,		"picca"					},
		{	"un dirk",					SEX_NEUTRAL,	"dirk"					},
		{	"un giavellotto",			SEX_NEUTRAL,	"giavellotto"			},
		{	"una partigiana",			SEX_FEMALE,		"partigiana"			},
		{	"uno spiedo",				SEX_NEUTRAL,	"spiedo"				},
		{	"un kris",					SEX_NEUTRAL,	"kris"					}
	},
	{	//	wand 9						gender			key
		{	"una bacchetta",			SEX_FEMALE,		"bacchetta"				},
		{	"un bastoncino",			SEX_NEUTRAL,	"bastoncino"			},
		{	"un rametto",				SEX_NEUTRAL,	"rametto"				},
		{	"una stecca",				SEX_FEMALE,		"stecca"				},
		{	"un corno",					SEX_NEUTRAL,	"corno"					},
		{	"un vincastro",				SEX_NEUTRAL,	"vincastro"				},
		{	"un cucchiaio",				SEX_NEUTRAL,	"cucchiaio"				},
		{	"una spatola",				SEX_FEMALE,		"spatola"				},
		{	"un rabdo",					SEX_NEUTRAL,	"rabdo"					}
	},
	{	//	staff 9						gender			key
		{	"un bastone",				SEX_NEUTRAL,	"bastone"				},
		{	"un'asta",					SEX_FEMALE,		"asta"					},
		{	"un pastorale",				SEX_NEUTRAL,	"pastorale"				},
		{	"uno scettro",				SEX_NEUTRAL,	"scettro"				},
		{	"un caduceo",				SEX_NEUTRAL,	"caduceo"				},
		{	"una verga",				SEX_FEMALE,		"verga"					},
		{	"un bacchio",				SEX_NEUTRAL,	"bucchio"				},
		{	"un alpenstock",			SEX_NEUTRAL,	"alpenstock"			},
		{	"una pertica",				SEX_FEMALE,		"pertica"				}
	}
};

struct ObjAchieTable RewardObj[3][20] = {
	{
		//	Casters
		{	//	Finger
			1374,				ITEM_JEWEL,
			APPLY_MANA_REGEN,	APPLY_MANA,			STAT_RANDOM,		ELEM_RESI_RANDOM,	APPLY_HIT,
			100,				75,					50,					25,					10
		},
		{	//	Wrist
			1375,				ITEM_JEWEL,
			APPLY_SPELLFAIL,	APPLY_MANA,			STAT_RANDOM,		APPLY_HIT,			APPLY_MANA_REGEN,
			100,				75,					50,					25,					10
		},
		{	//	Ear
			1376,				ITEM_JEWEL,
			APPLY_MANA,			APPLY_SPELLFAIL,	APPLY_MANA_REGEN,	OBJ_TELEPATHY,		STAT_RANDOM,
			100,				75,					50,					25,					10
		},
		{	//	Neck
			1377,				ITEM_JEWEL,
			APPLY_MANA,			APPLY_SPELLFAIL,	STAT_RANDOM,		APPLY_AC,			APPLY_MANA_REGEN,
			100,				75,					50,					25,					10
		},
		{	//	Eyes
			1378,				ITEM_JEWEL,
			OBJ_TRUE_SIGHT,		APPLY_MANA,			STAT_RANDOM,		ELEM_RESI_RANDOM,	APPLY_MANA_REGEN,
			100,				75,					50,					25,					10
		},
		{	//	Hold
			1379,				ITEM_JEWEL,
			ELEM_RESI_RANDOM,	APPLY_MANA,			APPLY_MANA_REGEN,	APPLY_SPELLFAIL,	OBJ_INVISIBILITY,
			100,				75,					50,					25,					10
		},
		{	//	Light
			1380,				ITEM_LIGHT,
			OBJ_SENCE_LIFE,		APPLY_MANA,			APPLY_MANA_REGEN,	APPLY_SPELLFAIL,	OBJ_LIFE_PROT,
			100,				75,					50,					25,					10
		},
		{	//	Back
			1381,				ITEM_CONTAINER,
			OBJ_ARTIFACT,		APPLY_MANA,			APPLY_MANA_REGEN,	STAT_RANDOM,		APPLY_HIT,
			100,				80,					50,					30,					10
		},
		{	//	About Body
			1382,				ITEM_ARMOR,
			APPLY_SPELLFAIL,	STAT_RANDOM,		APPLY_MANA_REGEN,	APPLY_HIT,			APPLY_MANA,
			100,				80,					50,					30,					10
		},
		{	//	Waist
			1383,				ITEM_ARMOR,
			APPLY_SPELLFAIL,	APPLY_MANA,			APPLY_MANA_REGEN,	STAT_RANDOM,		APPLY_HIT,
			100,				80,					50,					30,					10
		},
		{	//	Feet
			1384,				ITEM_ARMOR,
			APPLY_MOVE,			OBJ_FLY,			STAT_RANDOM,		APPLY_MOVE_REGEN,	OBJ_SNEAK,
			100,				80,					50,					30,					10
		},
		{	//	Hands
			1385,				ITEM_ARMOR,
			APPLY_MANA_REGEN,	APPLY_SPELLFAIL,	APPLY_HITROLL,		APPLY_MANA,			STAT_RANDOM,
			100,				80,					50,					30,					10
		},
		{	//	Body
			1386,				ITEM_ARMOR,
			APPLY_MANA,			APPLY_MANA_REGEN,	OBJ_RESI_SLASH,		APPLY_SPELLFAIL,	APPLY_HIT,
			100,				80,					60,					40,					20
		},
		{	//	Head
			1387,				ITEM_ARMOR,
			APPLY_MANA,			APPLY_AC,			OBJ_SPY,			APPLY_MANA_REGEN,	APPLY_HIT,
			100,				80,					60,					40,					20
		},
		{	//	Legs
			1388,				ITEM_ARMOR,
			APPLY_MANA_REGEN,	APPLY_MANA,			STAT_RANDOM,		APPLY_SPELLFAIL,	APPLY_AC,
			100,				80,					60,					40,					20
		},
		{	//	Arms
			1389,				ITEM_ARMOR,
			APPLY_HIT,			APPLY_MANA,			STAT_RANDOM,		APPLY_MANA_REGEN,	APPLY_SPELLFAIL,
			100,				80,					60,					40,					20
		},
		{	//	Shield
			1390,				ITEM_ARMOR,
			APPLY_HITROLL,		APPLY_MANA,			OBJ_RESI_PIERCE,	APPLY_MANA_REGEN,	APPLY_AC,
			100,				80,					60,					40,					20
		},
		{	//	Wield
			1391,				ITEM_WEAPON,
			APPLY_MANA,			APPLY_HITROLL,		APPLY_DAMROLL,		APPLY_SPELLFAIL,	APPLY_HITROLL,
			100,				70,					40,					20,					5
		},
		{	//	Wand
			1392,				ITEM_WAND,
			APPLY_MANA,			STAT_RANDOM,		APPLY_MANA_REGEN,	APPLY_SPELLFAIL,	APPLY_HIT,
			100,				70,					40,					20,					5
		},
		{	//	Staff
			1393,				ITEM_STAFF,
			APPLY_MANA,			STAT_RANDOM,		APPLY_HIT,			APPLY_MANA_REGEN,	APPLY_SPELLFAIL,
			100,				70,					40,					20,					5
		}
	},
	{
		//	Multi
		{	//	Finger
			1374,				ITEM_JEWEL,
			APPLY_SPELLFAIL,	APPLY_MANA,			APPLY_HITNDAM,		APPLY_HIT,			APPLY_HIT_REGEN,
			100,				75,					50,					25,					10
		},
		{	//	Wrist
			1375,				ITEM_JEWEL,
			APPLY_HIT,			APPLY_SPELLFAIL,	APPLY_MANA,			APPLY_MANA_REGEN,	APPLY_DAMROLL,
			100,				75,					50,					25,					10
		},
		{	//	Ear
			1376,				ITEM_JEWEL,
			APPLY_MANA,			APPLY_HIT_REGEN,	APPLY_HITROLL,		APPLY_HIT,			APPLY_SPELLFAIL,
			100,				75,					50,					25,					10
		},
		{	//	Neck
			1377,				ITEM_JEWEL,
			APPLY_AC,			APPLY_MANA_REGEN,	APPLY_HIT,			APPLY_HITNDAM,		APPLY_MANA,
			100,				75,					50,					25,					10
		},
		{	//	Eyes
			1378,				ITEM_JEWEL,
			APPLY_HIT,			OBJ_SENCE_LIFE,		APPLY_MANA,			OBJ_SPY,			APPLY_MANA_REGEN,
			100,				75,					50,					25,					10
		},
		{	//	Hold
			1379,				ITEM_JEWEL,
			APPLY_HIT,			STAT_RANDOM,		ELEM_RESI_RANDOM,	APPLY_DAMROLL,		OBJ_LIFE_PROT,
			100,				75,					50,					25,					10
		},
		{	//	Light
			1380,				ITEM_LIGHT,
			APPLY_HIT_REGEN,	APPLY_HIT,			APPLY_SPELLFAIL,	APPLY_HITNDAM,		STAT_RANDOM,
			100,				75,					50,					25,					10
		},
		{	//	Back
			1381,				ITEM_CONTAINER,
			OBJ_ARTIFACT,		APPLY_SPELLFAIL,	APPLY_MANA,			APPLY_HIT_REGEN,	APPLY_HIT,
			100,				80,					50,					30,     			10
		},
		{	//	About Body
			1382,				ITEM_ARMOR,
			APPLY_AC,			STAT_RANDOM,		ELEM_RESI_RANDOM,	APPLY_HIT,			APPLY_HITNDAM,
			100,				80,					50,					30,					10
		},
		{	//	Waist
			1383,				ITEM_ARMOR,
			APPLY_SPELLFAIL,	OBJ_FLY,			APPLY_MANA,			APPLY_MANA_REGEN,	APPLY_HITROLL,
			100,				80,					50,					30,					10
		},
		{	//	Feet
			1384,				ITEM_ARMOR,
			APPLY_MOVE,			APPLY_MOVE_REGEN,	STAT_RANDOM,		APPLY_HITROLL,		APPLY_AC,
			100,				80,					50,					30,					10
		},
		{	//	Hands
			1385,				ITEM_ARMOR,
			APPLY_HITROLL,		APPLY_MANA,			APPLY_HIT,			APPLY_AC,			APPLY_MANA_REGEN,
			100,				80,					50,					30,					10
		},
		{	//	Body
			1386,				ITEM_ARMOR,
			APPLY_HIT,			APPLY_AC,			OBJ_RESI_SLASH,		APPLY_MANA,			APPLY_HITNDAM,
			100,				80,					60,					40,					20
		},
		{	//	Head
			1387,				ITEM_ARMOR,
			APPLY_HIT,			ELEM_RESI_RANDOM,	OBJ_TRUE_SIGHT,		APPLY_MANA,			APPLY_MANA_REGEN,
			100,				80,					60,					40,					20
		},
		{	//	Legs
			1388,				ITEM_ARMOR,
			APPLY_MANA,			APPLY_HIT,			STAT_RANDOM,		APPLY_MANA_REGEN,	APPLY_HITNDAM,
			100,				80,					60,					40,					20
		},
		{	//	Arms
			1389,				ITEM_ARMOR,
			APPLY_HIT_REGEN,	ELEM_RESI_RANDOM,	APPLY_MANA_REGEN,	APPLY_SPELLFAIL,	APPLY_HITNDAM,
			100,				80,					60,					40,					20
		},
		{	//	Shield
			1390,				ITEM_ARMOR,
			APPLY_HIT,			OBJ_RESI_PIERCE,	APPLY_HITNDAM,		APPLY_SPELLFAIL,	APPLY_HIT_REGEN,
			100,				80,					60,					40,					20
		},
		{	//	Wield
			1391,				ITEM_WEAPON,
			APPLY_HITROLL,		APPLY_DAMROLL,		STAT_RANDOM,		APPLY_HITNDAM,		APPLY_HITROLL,
			100,				70,					40,					20,					5
		},
		{	//	Wand
			1392,				ITEM_WAND,
			APPLY_MANA_REGEN,	APPLY_SPELLFAIL,	APPLY_MANA,			STAT_RANDOM,		APPLY_HITROLL,
			100,				70,					40,					20,					5
		},
		{	//	Staff
			1393,				ITEM_STAFF,
			APPLY_MANA,			APPLY_HIT,			STAT_RANDOM,		APPLY_SPELLFAIL,	APPLY_HIT_REGEN,
			100,				70,					40,					20,					5
		}
	},
	{
		//	Melee
		{	//	Finger
			1374,				ITEM_JEWEL,
			APPLY_HITROLL,		APPLY_HIT,			APPLY_DAMROLL,		STAT_RANDOM,		APPLY_HIT_REGEN,
			100,				75,					50,					25,					10
		},
		{	//	Wrist
			1394,				ITEM_ARMOR,
			APPLY_HIT_REGEN,	APPLY_HITROLL,		STAT_RANDOM,		APPLY_HIT,			APPLY_MANA_REGEN,
			100,				75,					50,					25,     			10
		},
		{	//	Ear
			1376,				ITEM_JEWEL,
			APPLY_HITROLL,		APPLY_HIT_REGEN,	STAT_RANDOM,		ELEM_RESI_RANDOM,	APPLY_MANA,
			100,				75,					50,					25,					10
		},
		{	//	Neck
			1377,				ITEM_JEWEL,
			APPLY_HIT,			ELEM_RESI_RANDOM,	STAT_RANDOM,		APPLY_HITNDAM,		APPLY_SPELLFAIL,
			100,				75,					50,					25,					10
		},
		{	//	Eyes
			1395,				ITEM_ARMOR,
			OBJ_SENCE_LIFE,		APPLY_HIT,			APPLY_HIT_REGEN,	OBJ_SPY,			APPLY_MANA,
			100,				75,					50,					25,					10
		},
		{	//	Hold
			1379,				ITEM_JEWEL,
			APPLY_HITROLL,		OBJ_TELEPATHY,		APPLY_DAMROLL,		STAT_RANDOM,		APPLY_SPELLFAIL,
			100,				75,					50,					25,					10
		},
		{	//	Light
			1380,				ITEM_LIGHT,
			APPLY_HITROLL,		STAT_RANDOM,		APPLY_DAMROLL,		STAT_RANDOM,		OBJ_LIFE_PROT,
			100,				75,					50,					25,					10
		},
		{	//	Back
			1396,				ITEM_ARMOR,
			APPLY_HITROLL,		APPLY_AC,			APPLY_HIT_REGEN,	APPLY_DAMROLL,		APPLY_HIT,
			100,				80,					50,					30,					10
		},
		{	//	About Body
			1382,				ITEM_ARMOR,
			APPLY_AC,			APPLY_HITROLL,		APPLY_HIT_REGEN,	APPLY_HIT,			OBJ_INVISIBILITY,
			100,				80,					50,					30,					10
		},
		{	//	Waist
			1383,				ITEM_ARMOR,
			ELEM_RESI_RANDOM,	APPLY_HIT,			APPLY_HIT_REGEN,	APPLY_HITNDAM,		APPLY_SPELLFAIL,
			100,				80,					50,					30,					10
		},
		{	//	Feet
			1384,				ITEM_ARMOR,
			APPLY_MOVE,			APPLY_MOVE_REGEN,	OBJ_FLY,			STAT_RANDOM,		APPLY_HITNDAM,
			100,				80,					50,					30,					10
		},
		{	//	Hands
			1385,				ITEM_ARMOR,
			APPLY_HITNDAM,		APPLY_HIT_REGEN,	APPLY_HITROLL,		APPLY_HIT,			APPLY_DAMROLL,
			100,				80,					50,					30,					10
		},
		{	//	Body
			1386,				ITEM_ARMOR,
			APPLY_HIT,			OBJ_RESI_SLASH,		APPLY_AC,			APPLY_HITNDAM,		APPLY_HIT_REGEN,
			100,				80,					60,					40,					20
		},
		{	//	Head
			1387,				ITEM_ARMOR,
			OBJ_TRUE_SIGHT,		APPLY_HIT,			ELEM_RESI_RANDOM,	APPLY_HITROLL,		APPLY_DAMROLL,
			100,				80,					60,					40,					20
		},
		{	//	Legs
			1388,				ITEM_ARMOR,
			APPLY_HITROLL,		APPLY_HIT,			STAT_RANDOM,		APPLY_AC,			APPLY_HITNDAM,
			100,				80,					60,					40,					20
		},
		{	//	Arms
			1389,				ITEM_ARMOR,
			APPLY_HITNDAM,		APPLY_HIT,			STAT_RANDOM,		APPLY_AC,			APPLY_HIT_REGEN,
			100,				80,					60,					40,					20
		},
		{	//	Shield
			1390,				ITEM_ARMOR,
			APPLY_DAMROLL,		OBJ_RESI_PIERCE,	APPLY_AC,			APPLY_HITROLL,		APPLY_HIT_REGEN,
			100,				80,					60,					40,					20
		},
		{	//	Wield
			1391,				ITEM_WEAPON,
			APPLY_HITROLL,		APPLY_DAMROLL,		APPLY_HITROLL,		APPLY_DAMROLL,		APPLY_HITNDAM,
			100,				70,					40,					20,					5
		},
		{	//	Wand
			1392,				ITEM_WAND,
			0,					0,					0,					0,					0,
			100,				70,					40,					20,					5
		},
		{	//	Staff
			1393,				ITEM_STAFF,
			0,					0,					0,					0,					0,
			100,				70,					40,					20,					5
		}
	}
};

struct ObjBonusTable AchieBonus[MAX_CLASS][12] = {
	{	//	CLASS_MAGIC_USER
		{	 5,		 7,		10,		12,		20	},	//	Mana
		{	 3,		 5,		 5,		 5,		10	},	//	Mana Regen
		{	 5,		 5,		 7,		10,		12	},	//	Spellfail
		{	 5,		 5,		 5,		 6,		 8	},	//	Hit
		{	 5,		 5,		 5,		 5,		 5	},	//	Hit Regen
		{	 5,		 5,		 5,		 5,		10	},	//	Move
		{	 5,		 5,		 5,		 5,		 5	},	//	Move Regen
		{	 1,		 1,		 1,		 1,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 7,		 9,		10	},	//	Armor
		{	 1,		 2,		 2,		 3,		 4	}	//	AC
	},
	{	//	CLASS_CLERIC
		{	 5,		 7,		10,		12,		15	},	//	Mana
		{	 2,		 4,		 5,		 5,		 8	},	//	Mana Regen
		{	 5,		 5,		 5,		 8,		10	},	//	Spellfail
		{	 5,		 5,		 6,		 8,		10	},	//	Hit
		{	 5,		 5,		 5,		 6,		 8	},	//	Hit Regen
		{	 5,		 5,		 5,		 5,		10	},	//	Move
		{	 5,		 5,		 5,		 5,		 5	},	//	Move Regen
		{	 1,		 1,		 1,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 8,		10,		12	},	//	Armor
		{	 3,		 4,		 4,		 5,		 6	}	//	AC
	},
	{	//	CLASS_WARRIOR
		{	 5,		 5,		 5,		 5,		 5	},	//	Mana
		{	 1,		 1,		 1,		 1,		 2	},	//	Mana Regen
		{	 5,		 5,		 5,		 5,		 5	},	//	Spellfail
		{	 5,		 6,		 9,		12,		15	},	//	Hit
		{	 5,		 5,		 8,		12,		15	},	//	Hit Regen
		{	10,		10,		10,		15,		20	},	//	Move
		{	 5,		 5,		10,		12,		15	},	//	Move Regen
		{	 1,		 1,		 2,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 2	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 7,		10,		12,		15	},	//	Armor
		{	 6,		 7,		 8,		 9,		10	}	//	AC
	},
	{	//	CLASS_THIEF
		{	 5,		 5,		 5,		 5,		 5	},	//	Mana
		{	 1,		 1,		 1,		 1,		 2	},	//	Mana Regen
		{	 5,		 5,		 5,		 5,		 5	},	//	Spellfail
		{	 5,		 5,		 5,		 6,		 8	},	//	Hit
		{	 5,		 5,		 5,		 6,		 8	},	//	Hit Regen
		{	10,		10,		10,		15,		20	},	//	Move
		{	 5,		 5,		 8,		10,		10	},	//	Move Regen
		{	 1,		 1,		 2,		 2,		 3	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 2	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 5,		10,		10	},	//	Armor
		{	 3,		 3,		 3,		 4,		 5	}	//	AC
	},
	{	//	CLASS_DRUID
		{	 5,		 7,		10,		12,		15	},	//	Mana
		{	 2,		 4,		 5,		 5,		 8	},	//	Mana Regen
		{	 5,		 5,		 5,		 8,		10	},	//	Spellfail
		{	 5,		 5,		 6,		 8,		10	},	//	Hit
		{	 5,		 5,		 5,		 6,		 8	},	//	Hit Regen
		{	 5,		 5,		 5,		 5,		10	},	//	Move
		{	 5,		 5,		 5,		 5,		 5	},	//	Move Regen
		{	 1,		 1,		 1,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 7,		 9,		10	},	//	Armor
		{	 2,		 3,		 3,		 4,		 5	}	//	AC
	},
	{	//	CLASS_MONK
		{	 5,		 5,		 5,		 5,		 5	},	//	Mana
		{	 1,		 1,		 1,		 1,		 2	},	//	Mana Regen
		{	 5,		 5,		 5,		 5,		 5	},	//	Spellfail
		{	 5,		 5,		 7,		10,		12	},	//	Hit
		{	 5,		 5,		 7,		10,		12	},	//	Hit Regen
		{	10,		10,		10,		15,		20	},	//	Move
		{	 5,		 5,		10,		12,		15	},	//	Move Regen
		{	 1,		 1,		 2,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 2	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 8,		10,		12	},	//	Armor
		{	 3,		 4,		 4,		 5,		 6	}	//	AC
	},
	{	//	CLASS_BARBARIAN
		{	 0,		 0,		 0,		 0,		 0	},	//	Mana
		{	 0,		 0,		 0,		 0,		 0	},	//	Mana Regen
		{	 0,		 0,		 0,		 0,		 0	},	//	Spellfail
		{	 5,		 6,		 9,		12,		15	},	//	Hit
		{	 5,		 5,		 8,		12,		15	},	//	Hit Regen
		{	10,		10,		10,		15,		20	},	//	Move
		{	 5,		 5,		10,		12,		15	},	//	Move Regen
		{	 1,		 1,		 1,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 2	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 8,		10,		12	},	//	Armor
		{	 4,		 5,		 5,		 6,		 7	}	//	AC
	},
	{	//	CLASS_SORCERER
		{	 5,		 7,		10,		12,		20	},	//	Mana
		{	 3,		 5,		 5,		 5,		10	},	//	Mana Regen
		{	 5,		 5,		 7,		10,		12	},	//	Spellfail
		{	 5,		 5,		 5,		 6,		 8	},	//	Hit
		{	 5,		 5,		 5,		 5,		 5	},	//	Hit Regen
		{	 5,		 5,		 5,		 5,		10	},	//	Move
		{	 5,		 5,		 5,		 5,		 5	},	//	Move Regen
		{	 1,		 1,		 1,		 1,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 7,		 9,		10	},	//	Armor
		{	 1,		 2,		 2,		 3,		 4	}	//	AC
	},
	{	//	CLASS_PALADIN
		{	 5,		 5,		 5,		 8,		10	},	//	Mana
		{	 1,		 1,		 2,		 3,		 5	},	//	Mana Regen
		{	 5,		 5,		 5,		 5,		 5	},	//	Spellfail
		{	 5,		 5,		 7,		10,		12	},	//	Hit
		{	 5,		 5,		 6,		 8,		10	},	//	Hit Regen
		{	10,		10,		10,		10,		15	},	//	Move
		{	 5,		 5,		 5,		 5,		 5	},	//	Move Regen
		{	 1,		 1,		 2,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 2	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 7,		10,		12,		15	},	//	Armor
		{	 5,		 6,		 6,		 7,		 8	}	//	AC
	},
	{	//	CLASS_RANGER
		{	 5,		 5,		 5,		 5,		 5	},	//	Mana
		{	 1,		 1,		 2,		 3,		 5	},	//	Mana Regen
		{	 5,		 5,		 5,		 5,		 5	},	//	Spellfail
		{	 5,		 5,		 7,		10,		12	},	//	Hit
		{	 5,		 5,		 7,		10,		12	},	//	Hit Regen
		{	10,		10,		10,		10,		15	},	//	Move
		{	 5,		 5,		 5,		 5,		 5	},	//	Move Regen
		{	 1,		 1,		 2,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 2	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 8,		10,		10	},	//	Armor
		{	 4,		 5,		 5,		 6,		 7	}	//	AC
	},
	{	//	CLASS_PSI
		{	 5,		 7,		10,		12,		20	},	//	Mana
		{	 3,		 5,		 5,		 5,		10	},	//	Mana Regen
		{	 5,		 5,		 7,		10,		12	},	//	Spellfail
		{	 5,		 5,		 6,		 8,		12	},	//	Hit
		{	 5,		 5,		 6,		 8,		12	},	//	Hit Regen
		{	 5,		 5,		 5,		 5,		10	},	//	Move
		{	 5,		 5,		 8,		10,		10	},	//	Move Regen
		{	 1,		 1,		 1,		 2,		 2	},	//	Hitroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Damroll
		{	 1,		 1,		 1,		 1,		 1	},	//	Hit&Dam
		{	 5,		 5,		 7,		 9,		10	},	//	Armor
		{	 1,		 2,		 2,		 3,		 4	}	//	AC
	}
};

struct QuestRewardsTable QuestNebbie [MAX_QUEST_ACHIE] [100] = {
	//	Xarah
	{
		{	19755	},
		{	19780	},
		{	19779	},
		{	19778	},
		{	19777	},
		{	19756	},
		{	19781	},
		{	19782	},
		{	19783	},
		{	19784	},
		{	19757	},
		{	19785	},
		{	19786	},
		{	19787	},
		{	19788	},
		{	19758	},
		{	19789	},
		{	19790	},
		{	19759	},
		{	19791	},
		{	19792	},
		{	19760	},
		{	19793	},
		{	19794	},
		{	19795	},
		{	19796	},
		{	19761	},
		{	19699	},
		{	19698	},
		{	19697	},
		{	19696	},
		{	19763	},
		{	19797	},
		{	19798	},
		{	19766	},
		{	19695	},
		{	19694	},
		{	19767	},
		{	19693	},
		{	19692	},
		{	19768	},
		{	19691	},
		{	19690	},
		{	   -1	}
	},

	//	Nilmys
	{
		{	 9019	},
		{	 9018	},
		{	 9020	},
		{	 9021	},
		{	 9098	},
		{	 9099	},
		{	 9005	},
		{	 9006	},
		{	 9007	},
		{	 9009	},
		{	 9097	},
		{	 9087	},
		{	 9015	},
		{	 9016	},
		{	 9017	},
		{	 9014	},
		{	 9095	},
		{	 9094	},
		{	 9010	},
		{	 9011	},
		{	 9012	},
		{	 9013	},
		{	 9090	},
		{	 9091	},
		{	 9029	},
		{	 9031	},
		{	 9030	},
		{	 9028	},
		{	 9092	},
		{	 9093	},
		{	 9023	},
		{	 9025	},
		{	 9024	},
		{	 9026	},
		{	 9088	},
		{	 9089	},
		{	   -1	}
	}
};

struct MobQuestAchie QuestMobAchie[MAX_QUEST_ACHIE] = {
	//	Xarah
	{	 8,	19709,	19719,	19605,	19712,	19710,	19724,	19602,	19933,		0,		0	},

	//	Nilmys
	{	 9,	 9007,	 9020,	 9021,	 9047,	 9015,	 9014,	 9018,	 8916,	 8923,		0	}
};

struct MercySystem QuestNumber[MAX_QUEST_ACHIE] = {
	//  Xarah
	{	11,	1,		31,	2,		46,	3,		51,	3,		56,	4,		61,		"Xarah"		},

	//  Nilmys
	{	16,	1,		56,	2,		61,	3,		66,	3,		71,	4,		75,		"Nilmys"	}
};

struct ClassAchieTable AchievementsList[MAX_ACHIE_CLASSES][MAX_ACHIE_TYPE] = {
	//	CLASS_ACHIE
	{
		{
			0,	/* achie_number */									"",	/*	achie_string1 */					"",	// achie_string2
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,	/* classe */										0,	/*	grado_diff */						0	//n_livelli
		},
		{
			ACHIE_MAGE_1,											"portale fatto",							"portali fatti",
			"Non sono sicuro dove porti...",						1,
			"Un taxi signore?",										50,
			"Scarpe? A cosa servono?",								300,
			"Un piccolo mondo!",									1000,
			"Signore dei Portali",									10000,
			"Stargate",												50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_MAGIC_USER,										LEV_BEGINNER,								6
		},
		{
			ACHIE_MAGE_2,											"raggio disintegrante lanciato",			"raggi disintegranti lanciati",
			"Ops, non volevo",										1,
			"Quasi quasi ci prendo gusto",							50,
			"Sei sicuro di volermi provocare?",						300,
			"Nemico? Quale nemico?",								1000,
			"Disintegratore",										10000,
			"Cannone al plasma!",									50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_MAGIC_USER,										LEV_VERY_EASY,								6
		},
		{
			ACHIE_MAGE_3,											"incantesimo di invisibilita' lanciato",	"incantesimi di invisibilita' lanciati",
			"Cosa mi e' successo?!?",								1,
			"Guardami adesso!",										50,
			"Ti giuro che non ci sono!",							300,
			"Sono dietro di te",									1000,
			"Mr. Nessuno",											10000,
			"Dicono che sia esistito ma nessuno l'ha mai visto",	50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_MAGIC_USER,										LEV_VERY_EASY,								6
		},
		{
			ACHIE_CLERIC_1,											"incantesimo di guarigione lanciato",		"incantesimi di guarigione lanciati",
			"Mi sembra di ricordare si faccia cosi'",				1,
			"Ne ho visti tanti messi peggio",						50,
			"Vai sereno tank!",										300,
			"Chi trova un chierico trova un tesoro",				1000,
			"Lasciatemi passare, sono IL medico!",					10000,
			"Con la sola imposizione delle mani...",				50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_CLERIC,											LEV_BEGINNER,								6
		},
		{
			ACHIE_CLERIC_2,											"resurrezione fatta",						"resurrezioni fatte",
			"Ci posso provare",										1,
			"Ci penso io!",											50,
			"Morte non ti temo",									300,
			"Immortalis",											1000,
			"Nemesi di Thanatos",									10000,
			"Arcangelo!",											50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_CLERIC,											LEV_HARD,									6
		},
		{
			ACHIE_CLERIC_3,											"incantesimo di santuario lanciato",		"incantesimi di santuario lanciati",
			"Cosi' dovrebbe andare meglio",							1,
			"Se mi lasci non ti accendo",							50,
			"Inserisco la spina",									300,
			"Ti copro io, tranquillo!",								1000,
			"Portatore di Luce",									10000,
			"Vi illumino di immenso",								50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_CLERIC,											LEV_BEGINNER,								6
		},
		{
			ACHIE_WARRIOR_1,										"avversario buttato a terra",				"avversari buttati a terra",
			"Scusa, sono inciampato",								1,
			"Attaccabrighe",										50,
			"Bullo",												300,
			"Bulldozer",											1000,
			"Demolitore",											10000,
			"Schiacciasassi",										50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_WARRIOR,											LEV_BEGINNER,								6
		},
		{
			ACHIE_WARRIOR_2,										"atterramento fallito",						"atterramenti falliti",
			"Buccia di banana",										1,
			"Zoppo",												50,
			"Ubriacone",											300,
			"Maldestro",											1000,
			"Il guerriero non e' il mio mestiere",					10000,
			"Goofy!",												50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_WARRIOR,											LEV_BEGINNER,								6
		},
		{
			ACHIE_WARRIOR_3,										"compagno salvato",							"compagni salvati",
			"Dici a me?",											1,
			"Ti guardo le spalle",									50,
			"Impiccione",											300,
			"Salva donzelle",										1000,
			"Risolvi problemi",										10000,
			"Darei la vita per i miei compagni",					50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_WARRIOR,											LEV_EASY,									6
		},
		{
			ACHIE_THIEF_1,											"avversario pugnalato alle spalle",			"avversari pugnalati alle spalle",
			"Non sono stato io!",									1,
			"Ci prendo gusto",										50,
			"Toh, un puntaspilli",									300,
			"Non darmi mai le spalle",								1000,
			"Punitore",												10000,
			"Approfittatore",										50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_THIEF,											LEV_BEGINNER,								6
		},
		{
			ACHIE_THIEF_2,											"avversario ucciso con un colpo",			"avversari uccisi con un colpo",
			"Era gia' morto!",										1,
			"Ora lo zittisco",										50,
			"Ne uccide piu' lo stab della spada",					300,
			"Fratello morte",										1000,
			"Assassino",											10000,
			"Natural Born Killer",									50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_THIEF,											LEV_VERY_EASY,								6
		},
		{
			ACHIE_THIEF_3,											"avversario derubato",						"avversari derubati",
			"Ti e' caduto qualcosa?",								1,
			"Povero e' chi il povero fa",							50,
			"Robin Hood!",											300,
			"Me lo hai dato tu, giuro!",							1000,
			"Lupin III",											10000,
			"Rubo ai ricchi... per arricchirmi",					50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_THIEF,											LEV_NORMAL,									6
		},
		{
			ACHIE_DRUID_1,											"massa di insetti evocata",					"masse di insetti evocate",
			"Mi sto per sentire male",								1,
			"Ho mangiato pesante oggi, state attenti!",				50,
			"Forse non c'e' bisogno di entrare...",					300,
			"Sta a vedere!",										1000,
			"Morte Strisciante",									10000,
			"Signore delle mosche",									50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_DRUID,											LEV_NORMAL,									6
		},
		{
			ACHIE_DRUID_2,											"metamorfosi riuscita",						"metamorfosi riuscite",
			"Cosa... mi... sta... succedendo...",					1,
			"Mi sto imbufalendo",									50,
			"Ci vorrebbe la forza di un orso!",						300,
			"Sei sicuro fossi io?",									1000,
			"Doppleganger",											10000,
			"Philip Dick ero io...",								50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_DRUID,											LEV_EASY,									6
		},
		{
			ACHIE_DRUID_3,											"reincarnazione fatta",						"reincarnazioni fatte",
			"Cosa ho combinato? Scusa, non volevo!",				1,
			"Non vorrei essere nei tuoi panni!",					50,
			"Conosci la fiaba del principe ranocchio?",				300,
			"Vorresti essere qualcun altro, vero?",					1000,
			"Maestro Manipolatore",									10000,
			"Il limite e' solo la mente...",						50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_DRUID,											LEV_HARD,									6
		},
		{
			ACHIE_MONK_1,											"avversario disarmato",						"avversari disarmati",
			"Non volevo, scusami :-)",								1,
			"Ti e' caduta l'arma?",									50,
			"Vuoi vedere che tela faccio volare?",					300,
			"Arma? Quale arma?",									1000,
			"Io e te, senza armi, ora, qui!",						10000,
			"Quello e' un coltello? Quest... ah no",				50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_MONK,												LEV_BEGINNER,								6
		},
		{
			ACHIE_MONK_2,											"palmo vibrante",							"palmi vibranti",
			"Volevo solo stringerti la mano!",						1,
			"Tirami il dito!",										50,
			"One Inch Punch",										300,
			"Ti spiezzo con un dito",								1000,
			"Nirvana",												10000,
			"Potrei ucciderti usando solo questo pollice",			50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_MONK,												LEV_HARD,									6
		},
		{
			ACHIE_MONK_3,											"nemico accecato",							"nemici accecati",
			"Ops, non l'ho fatto apposta...",						1,
			"Ops, l'ho fatto di nuovo...",							50,
			"Aspetta, hai qualcosa nell'occhio!",					300,
			"Guarda qui e sorridi",									1000,
			"Tu, non puoi, vedermi!",								10000,
			"Hey, ma lo sai che hai dei begl'occhi?",				50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_MONK,												LEV_VERY_EASY,								6
		},
		{
			ACHIE_BARBARIAN_1,										"volta in berserk",							"volte in berserk",
			"Mi sono un po' innervosito",							1,
			"Ora mi arrabbio sul serio!",							50,
			"Statemi lontano adesso",								300,
			"Prova a ripeterlo, se hai il coraggio!",				1000,
			"Furia Rossa",											10000,
			"Conan sono io!",										50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_BARBARIAN,										LEV_BEGINNER,								6
		},
		{
			ACHIE_BARBARIAN_2,										"pugnalata alle spalle evitata",			"pugnalate alle spalle evitate",
			"Che cosa e' stato?",									1,
			"Andra' meglio la prossima volta",						50,
			"Ne sei proprio sicuro?",								300,
			"Cosa pensavi di fare?",								1000,
			"Occhi di Beholder",									10000,
			"Pensi che sia nato ieri?",								50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_BARBARIAN,										LEV_EASY,									6
		},
		{
			ACHIE_BARBARIAN_3,										"medicazione effettuata",					"medicazioni effettuate",
			"Aspetta che ci soffio sopra!",							1,
			"Ora si che va meglio!",								50,
			"Dammi un pezzo della tua maglia!",						300,
			"E' solo un graffio...",								1000,
			"Maestro Sciamano",										10000,
			"Ho insegnato io a Candy Candy come si benda",			50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_BARBARIAN,										LEV_VERY_EASY,								6
		},
		{
			ACHIE_SORCERER_1,										"portale fatto",							"portali fatti",
			"Non sono sicuro dove porti...",						1,
			"Un taxi signore?",										50,
			"Scarpe? A cosa servono?",								300,
			"Un piccolo mondo!",									1000,
			"Signore dei Portali",									10000,
			"Stargate",												50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_SORCERER,											LEV_BEGINNER,								6
		},
		{
			ACHIE_SORCERER_2,										"raggio disintegrante lanciato",			"raggi disintegranti lanciati",
			"Ops, non volevo",										1,
			"Quasi quasi ci prendo gusto",							50,
			"Sei sicuro di volermi provocare?",						300,
			"Nemico? Quale nemico?",								1000,
			"Disintegratore",										10000,
			"Cannone al plasma!",									50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_SORCERER,											LEV_VERY_EASY,								6
		},
		{
			ACHIE_SORCERER_3,										"incantesimo di invisibilita' lanciato",	"incantesimi di invisibilita' lanciati",
			"Cosa mi e' successo?!?",								1,
			"Guardami adesso!",										50,
			"Ti giuro che non ci sono!",							300,
			"Sono dietro di te",									1000,
			"Mr. Nessuno",											10000,
			"Dicono che sia esistito ma nessuno l'ha mai visto",	50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_SORCERER,											LEV_VERY_EASY,								6
		},
		{
			ACHIE_PALADIN_1,										"benedizione invocata",						"benedizioni invocate",
			"Che cos'e' questa luce?",								1,
			"Hai visto la luce!",									50,
			"Gli Dei sono dalla mia parte",							300,
			"Sono il prescelto!",									1000,
			"L'Eletto",												10000,
			"Io sono Leggenda",										50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_PALADIN,											LEV_EASY,									6
		},
		{
			ACHIE_PALADIN_2,										"grido di guerra lanciato",					"grida di guerra lanciati",
			"Volevo solo chiamarti...",								1,
			"Posso sussurrarti una cosa all'orecchio?",				50,
			"Huj, Huj, Hajra'",										300,
			"Forse ho esagerato con la birra",						1000,
			"Furia di Guerra",										10000,
			"Non e' il ruggito di un drago quello?",				50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_PALADIN,											LEV_VERY_EASY,								6
		},
		{
			ACHIE_PALADIN_3,										"consacrazione",							"consacrazioni",
			"Aiuto, le mie mani brillano!",							1,
			"Ho le mani d'oro io",									50,
			"Serve un massaggio?",									300,
			"Non ti muovere e fatti toccare!",						1000,
			"Io porto la Luce nel Mondo",							10000,
			"Tu fidati, io ti tocchero' soltato... nell'anima!",	50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_PALADIN,											LEV_EASY,									6
		},
		{
			ACHIE_RANGER_1,											"pelle ottenuta scuoiando",					"pelli ottenute scuoiando",
			"Povera bestia, mi viene da piangere",					1,
			"Mi sa che posso farci qualcosa di utile",				50,
			"Dammi una lama e ti concio per le feste!",				300,
			"Faccio solo capolavori",								1000,
			"Mastro Conciatore",									10000,
			"Gran maestro delle pelli",								50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_RANGER,											LEV_VERY_EASY,								6
		},
		{
			ACHIE_RANGER_2,											"evocazione di animali",					"evocazioni di animali",
			"Cosa vuole da me questa creatura?",					1,
			"Ho un amico che puo' aiutarci",						50,
			"Il ranger che sussurrava ai cavalli",					300,
			"Lasciate che gli animali vengano da me",				1000,
			"Animal Man",											10000,
			"San Francesco scansati",								50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_RANGER,											LEV_NORMAL,									6
		},
		{
			ACHIE_RANGER_3,											"razione ottenuta",							"razioni ottenute",
			"Povera creatura, mi viene da piangere",				1,
			"Pero', sai che non vengono male cotte?",				50,
			"Facciamo una o due bistecche, siora?",					300,
			"Ora ti faccio vedere come ricavarne delle bistecche",	1000,
			"Mastro Macellaio",										10000,
			"Non spreco nemmeno un grammo",							50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_RANGER,											LEV_BEGINNER,								6
		},
		{
			ACHIE_PSI_1,											"porta dimensionale aperta",				"porte dimensionali aperte",
			"Ricordavo fosse piu' lontano",							1,
			"Taxi? No grazie!",										50,
			"Basta pensarlo e sei li'!",							300,
			"A cosa servono i maghi se hai uno psi?",				1000,
			"Il Mondo nelle Mie Mani",								10000,
			"Beam me up!",											50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_PSI,												LEV_BEGINNER,								6
		},
		{
			ACHIE_PSI_2,											"cervello spappolato",						"cervelli spappolati",
			"Dici che gli ho fatto male?",							1,
			"Stasera cervello alla coque!",							50,
			"E' un cervello od un uovo?",							300,
			"Master Blaster",										1000,
			"Mind Melter",											10000,
			"Sono nella tua mente... paura?",						50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_PSI,												LEV_BEGINNER,								6
		},
		{
			ACHIE_PSI_3,											"scarica adrenalinica",						"scariche adrenaliniche",
			"Provala anche solo una volta...",						1,
			"Forse ho qualcosa che puo' piacerti",					50,
			"Meglio di una benedizione, vero?",						300,
			"Ho solo roba buona, io!",								1000,
			"Signore dei Cartelli Nebbiosi",						10000,
			"Pablo Escobar",										50000,
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			CLASS_PSI,												LEV_VERY_EASY,								6
		}
	},

	//  BOSSKILL_ACHIE
	{
		{	//	 0
			ACHIE_EVANGELINE,										"Signora delle Bambole uccisa",				"Evangeline uccise",
			"Ah, non era una Barbie?",								1,
			"Le bambole mi inquietano",								10,
			"Non sono qui a pettinare bambole!",					100,
			"Mi piacciono solo quelle gonfiabili...",				500,
			"Master of Puppets",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_CHAMPION,								5
		},
		{	//	 1
			ACHIE_SALOMONE,											"Re Salomone ucciso",						"Re Salomone uccisi",
			"Ah, c'era un piano di sopra?",							1,
			"Interessante questo scettro",							10,
			"Io pulisco Apo sul serio!",							100,
			"Al di sopra dei Cavalieri ci sono io!",				500,
			"Distruttore dell'Equilibrio",							5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		},
		{	//	 2
			ACHIE_THION,											"Dio delle Ombre ucciso",					"Thion uccisi",
			"Ma non potevamo farci i fatti nostri?",				1,
			"The Lord of the Earrings",								10,
			"Ammazza (che) Dio",									100,
			"Lucertole, spettri ed ombre... tutto qui?",			500,
			"Colui che spazza via l'Ombra",							5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	 3
			ACHIE_WARLOCK,											"Warlock ucciso",							"Warlock uccisi",
			"E questo da dove esce?",								1,
			"Piu' lo spezzetti piu' torna",							10,
			"Scoperchia bare",										100,
			"Pacificatore di anime",								500,
			"Castiga dinastie",										5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	 4
			ACHIE_STANISLAV,										"Spettro di Stanislav ucciso",				"Spettri di Stanislav uccisi",
			"Ma io non volevo pestarlo!",							1,
			"Sembrava un vecchietto a modo...",						10,
			"Misericordioso",										100,
			"Scaccia spettri",										500,
			"Sovrano di Nilmys",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	 5
			ACHIE_SAURON,											"Oscuro Signore ucciso",					"Sauron uccisi",
			"Ma io non ci volevo venire...",						1,
			"Attenti al pozzo!",									10,
			"AAA anello cercasi",									100,
			"Mandami contro anche i tuoi Nazgul!",					500,
			"Signore di Mordor",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		},
		{	//	 6
			ACHIE_XAGELON,											"Terrore Divino ucciso",					"Xagelon uccisi",
			"Ma vedi, sembrava cosi' piccolo...",					1,
			"Agli Dei piace piccolo",								10,
			"Botte piccola, vino buono",							100,
			"Fattela con quelli alla tua altezza",					500,
			"Signore di Bofgorak",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_CHAMPION,								5
		},
		{	//	 7
			ACHIE_SLAVALOUS,										"Slavalous ucciso",							"Slavalous uccisi",
			"Che diavoletto!",										1,
			"Medaglia medaglia medaglia!",							10,
			"Il suonatore di campane",								100,
			"L'esorcista",											500,
			"Signore dei Diavoli",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	 8
			ACHIE_MORPHAIL,											"Sacerdote della Morte ucciso",				"Morphail uccisi",
			"Il Bevitore di Sangue",								1,
			"Araldo della Morte",									10,
			"L'uomo del Fiume",										100,
			"Il Signore del Fiume",									500,
			"Distruttore di filatteri",								5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	 9
			ACHIE_NARIS,											"Demone della Morte ucciso",				"Naris uccisi",
			"Non era esattamente un Imp",							1,
			"Il demone incompreso",									10,
			"Demone? Vediamo cosa sa fare!",						100,
			"Un demone al giorno toglie il paladino di torno",		500,
			"Signore dei Nove Gironi",								5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	10
			ACHIE_TSUCHIGUMO,										"Tsuchigumo ucciso",						"Tsuchigumo uccisi",
			"Aracnofobia",											1,
			"Occhio che morde!",									10,
			"Brutto e peloso",										100,
			"Il disinfestatore",									500,
			"Altro che Minamoto!",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_CHAMPION,								5
		},
		{	//	11
			ACHIE_THANATOS,											"Thanatos ucciso",							"Thanatos uccisi",
			"L'eredita' di Shelin",									1,
			"Che i morti restino morti",							10,
			"Eroe delle Nebbie!",									100,
			"Cavaliere delle Nebbie",								500,
			"Signore delle Nebbie",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_GOD_MODE,								5
		},
		{	//	12
			ACHIE_STRAHD,											"Signore di Ravenloft ucciso",				"Strahd uccisi",
			"Ho qui con me un paletto di frassino",					1,
			"Ammazza pipistrelli",									10,
			"Ammazza non morti",									100,
			"Ammazza Vampiri",										500,
			"Van Helsing",											5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_IMPERIAL,								5
		},
		{	//	13
			ACHIE_SHELOB,											"Shelob uccisa",							"Shelob uccise",
			"Toglietemi le ragnatele da dosso!",					1,
			"Questo si che e' un ragno!",							10,
			"Se ci portassi mia moglie qui...",						100,
			"E' solo un ragnetto!",									500,
			"Stermina Aracnidi",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		},
		{	//	14
			ACHIE_ROC,												"Roc ucciso",								"Roc uccisi",
			"E' un aereo? E' un uccello? No! E' un Roc!",			1,
			"L'avidita' puo' essere letale",						10,
			"Prova ad ingoiarmi",									100,
			"Sterminatore di grandi Uccelli!",						500,
			"Signore dei Roc",										5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	15
			ACHIE_LAZLO,											"Capitano di Nilmys ucciso",				"Lazlo uccisi",
			"Ahia! Ma questo fa male!",								1,
			"Non saro' io di vedetta!",								10,
			"Punitore di soldati",									100,
			"Celata di stagno",										500,
			"Capitano della Guardia",								5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		},
		{	//	16
			ACHIE_HYPNOS,											"Signore dei Sogni ucciso",					"Hypnos uccisi",
			"Salve, soffro di insonnia, mi puo' aiutare?",			1,
			"Ho fatto un brutto sogno",								10,
			"Se mi vedi chiudere gli occhi scuotimi!",				100,
			"Anche tu fai sempre tardi la notte?",					500,
			"? ? ? ? ? ? ? ? ? ? ? ? ? ?",							5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		},
		{	//	17
			ACHIE_DEMI_LICH,										"Demi Lich ucciso",							"Demi Lich uccisi",
			"Fa freddo o sono io?",									1,
			"Per Alar, sei sciupatissimo! Mangia un po'!",			10,
			"Era un lick o un lich?",								100,
			"Distruttore di non morti",								500,
			"Signore dei Lich",										5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	18
			ACHIE_BALROG,											"Balrog ucciso",							"Balrog uccisi",
			"Ma questo mi fa paura!",								1,
			"Frustino infuocato eh... mmm",							10,
			"Tamburi nella notte",									100,
			"Quando scavi troppo a fondo...",						500,
			"Signore di Moria",										5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		},
		{	//	19
			ACHIE_FIDEX_GATOR,										"Fidex Gator ucciso",						"Fodex Gator uccisi",
			"Che bello! Dite che morde?",							1,
			"Aspettate, io ci so fare con i cani",					10,
			"Dog sitter",											100,
			"Che bel cagnolino! Lo vuoi un croccantino?",			500,
			"Addestratore di cani",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	20
			ACHIE_DRAGGSYLIA,										"Draggsylia uccisa",						"Draggsylia uccise",
			"Tranquilli, non soffia questa!",						1,
			"Io vi sostengo e, se riesco, meno qualcosa",			10,
			"Guardate, un drago!",									100,
			"Una lucertola allo spiedo?",							500,
			"Dominatore di Draghi",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_CHAMPION,								5
		},
		{	//	21
			ACHIE_ARKHAT,											"Arkhat ucciso",							"Arkhat uccisi",
			"Va giu' in un colpo!",									1,
			"Hey, ha provato a mangiarmi!",							10,
			"Ma che bel circolo sanguinoleto!",						100,
			"Ti mangio io se non stai attento!",					500,
			"Signore delle Tenebre",								5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_TORMENT,								5
		},
		{	//	22
			ACHIE_GONHAG,											"Gonhag ucciso",							"Gonhag uccisi",
			"Whisky! Vieni qui bello!",								1,
			"Ho dimenticato il guinzaglio, lo avete?",				10,
			"Hey, non mi sbavare cosi'!",							100,
			"A cuccia su, da bravo",								500,
			"Dominatore di Draghi",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_IMPERIAL,								5
		},
		{	//	23
			ACHIE_BARBALBERO,										"Barbalbero ucciso",						"Barbalbero uccisi",
			"Che bel bastoncino!",									1,
			"Con questo facciamo un bel falo'!",					10,
			"Ho sempre odiato la natura!",							100,
			"Ora mi faccio un bell'arco",							500,
			"Signore di Fangorn",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	24
			ACHIE_ORRORE_DETURPANTE,								"Orrore Deturpante ucciso",					"Orrori Deturpanti uccisi",
			"Sono tutto sporco di fango",							1,
			"Ma questo non si fa schifo da solo?",					10,
			"Qui ora ci coltivo dei mirtilli...",					100,
			"Quanto puzza questo...",								500,
			"Signore della Fenice",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	25
			ACHIE_TIMNUS,											"Timnus ucciso",							"Timus uccisi",
			"Ma era un Dio?",										1,
			"Pensavo fosse uno gnomo...",							10,
			"Avevo bisogno di orecchie per la mia collana",			100,
			"Questi elfi si credono di essere sempre i piu' fighi",	500,
			"Signore degli Elfi",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	26
			ACHIE_SHERAZADE,										"Sherazade uccisa",							"Sherazade uccise",
			"Volevo solo portarti a cena",							1,
			"Hey, ma lo sai che hai degli occhi bellissimi!",		10,
			"Avrei fatto di tutto per te!",							100,
			"Prendi il mio cuore... o prendero' io il tuo!",		500,
			"Signore dell'Harem",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EASY,									5
		},
		{	//	27
			ACHIE_DRAGO_DUNA_NERA,									"Drago delle Dune ucciso",					"Draghi delle Dune uccisi",
			"Ah, ma non era un mucchietto di sabbia?",				1,
			"Voglio fare un bel castello di sabbia",				10,
			"Avete delle formine con voi?",							100,
			"Spaleremo tutto con dei bulldozer",					500,
			"Signore delle Dune",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_EXPERT,									5
		},
		{	//	28
			ACHIE_INHEP,											"Inhep ucciso",								"Inhep uccisi",
			"Ma questo e' proprio grosso!",							1,
			"Non mi fai paura, gigantone",							10,
			"Sono piu' piccolo ma faccio piu' male!",				100,
			"Piu' sono grossi piu' fanno rumore quando cadono",		500,
			"Signore del Monolito",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_CHAMPION,								5
		},
		{	//	29
			ACHIE_ARTEFATTO,										"Costrutto Atlantideo ucciso",				"Artefatti uccisi",
			"Ops, ho rotto il giocattolino",						1,
			"Fermi tutti, lo rimonto io",							10,
			"Mi piace un sacco giocare con i lego",					100,
			"Lo smonto e lo rimonto quanto mi pare e piace",		500,
			"Signore dei Costrutti",								5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_HARD,									5
		}
	},

	//	QUEST_ACHIE
	{
		{	//	 0
			ACHIE_QUEST_XARAH,										"Xarah completata",							"Xarah completate",
			"Conquistatore di Torri",								1,
			"Signore dei Protettori",								30,
			"Maestro dei Sortilegi",								150,
			"Re delle Lucertole",									1000,
			"Signore delle Ombre",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_CHAMPION,								5
		},
		{	//	 1
			ACHIE_QUEST_NILMYS,										"Nilmys completata",						"Nilmys completate",
			"Oscuro Signore",										1,
			"Colui che cammina nella Nebbia",						30,
			"Distruttore di Circoli",								150,
			"Sterminatore dell'Ombra",								1000,
			"Portatore di Luce",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_IMPERIAL,								5
		}
	},

	//	OTHER_ACHIE
	{
		{	//	 0
			ACHIE_PKILL_WIN,										"Pkill effettuato",							"Pkill effettuati",
			"Ben fatto!",											1,
			"Sei un professionista",								50,
			"Ma allora sei infame",									500,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	 1
			ACHIE_PKILL_LOSS,										"morte in Pkill",							"morti in Pkill",
			"Ehi, cosa ti ho fatto?",								1,
			"Ma perche' sempre a me?",								50,
			"Calimero!",											500,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	 2
			ACHIE_DEATH,											"morte",									"morti",
			"E' solo un graffio",									10,
			"E' una ferita leggera",								100,
			"Barcollo ma non mollo",								1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	 3
			ACHIE_JUNK,												"oggetto distrutto",						"oggetti distrutti",
			"Non gettare la carta!",								10,
			"Il riciclo e' il futuro",								1000,
			"Ti prego, fermati!",									100000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	 4
			ACHIE_ASTRAL,											"incantesimo di cammino astrale",			"incantesimi di cammino astrale",
			"Noi ce ne andiamo",									100,
			"Il mondo a portata di stagno",							1000,
			"Il piano astrale e' il mio salotto",					100000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	 5
			ACHIE_REPAIR,											"oggetto riparato",							"oggetti riparati",
			"Togliamo i graffi",									100,
			"Mai pensato ad un'assicurazione?",						1000,
			"Il fabbro ti ama",										10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	 6
			ACHIE_MINING,											"colpo di piccone a segno",					"colpi di piccone a segno",
			"Toh, un piccone",										100,
			"Se brilla e' buono",									1000,
			"Andiam, andiam, andiamo a lavorar...",					10000,
			"Moria e' casa mia!",									100000,
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								4
		},
		{	//	 7
			ACHIE_BAR,												"lingotto forgiato",						"lingotti forgiati",
			"Verso la ricchezza",									1000,
			"Non e' tutto oro quello che luccica",					10000,
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								2
		},
		{	//	 8
			ACHIE_GOLDENBAR,										"lingotto d'oro forgiato",					"lingotti d'oro forgiati",
			"Verso la ricchezza",									5000,
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								1
		},
		{	//	 9
			ACHIE_WEAPONSMITH,										"arma forgiata",							"armi forgiate",
			"E' arrivato l'arrotino",								50,
			"Ammazza che mazza",									100,
			"Allievo di Gamil Zirak",								1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	10
			ACHIE_QUEST_COMPLETE,									"quest completata in totale",				"quest completate in totale",
			"Investigatore",										10,
			"Philip Marlowe",										100,
			"Sherlock Holmes",										1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	11
			ACHIE_QUEST_FAILED,										"quest fallita in totale",					"quest fallite in totale",
			"Scusi signor mob, un'informazione!",					10,
			"Eppure la sapevo...",									100,
			"King of Epic Fail",									1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	12
			ACHIE_QUEST_HUNT_COMPLETE,								"quest di caccia completata",				"quest di caccia completate",
			"Cacciatore in ascesa",									10,
			"Boba Fett",											100,
			"Cacciatore di Taglie",									1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	13
			ACHIE_QUEST_HUNT_FAILED,								"quest di caccia fallita",					"quest di caccia fallite",
			"Ma dove diavolo era?",									10,
			"Ehm, era troppo grosso per me...",						100,
			"Forse e' meglio farseli amici",						1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	14
			ACHIE_QUEST_RESCUE_COMPLETE,							"quest di salvataggio completata",			"quest di salvataggio completate",
			"Boy scout",											10,
			"Salva principesse",									100,
			"S.W.A.T.",												1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	15
			ACHIE_QUEST_RESCUE_FAILED,								"quest di salvataggio fallita",				"quest di salvataggio fallite",
			"Scusami, avevo da fare...",							10,
			"Ti giuro che stavo arrivando...",						100,
			"Per chi mi avete preso, per un chierichetto?",			1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	16	non attiva, futura implementazione
			ACHIE_QUEST_RESEARCH_COMPLETE,							"quest di ricerca completata",				"quest di ricerca completate",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	17	non attiva, futura implementazione
			ACHIE_QUEST_RESEARCH_FAILED,							"quest di ricerca fallita",					"quest di ricerca fallite",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	18	non attiva, futura implementazione
			ACHIE_QUEST_DELIVERY_COMPLETE,							"quest di consegna completata",				"quest di consegna completate",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	19	non attiva, futura implementazione
			ACHIE_QUEST_DELIVERY_FAILED,							"quest di consegna fallita",				"quest di consegna fallite",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	20	non attiva, serve per il conteggio
			ACHIE_QUEST_TOTAL,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	21	non attiva, serve per il conteggio
			ACHIE_QUEST_HUNT_TOTAL,									"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	22	non attiva, serve per il conteggio
			ACHIE_QUEST_RESCUE_TOTAL,								"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	23	non attiva, serve per il conteggio
			ACHIE_QUEST_RESEARCH_TOTAL,								"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	24	non attiva, serve per il conteggio
			ACHIE_QUEST_DELIVERY_TOTAL,								"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	25
			ACHIE_PROCAREA_TOTAL,									"dimensione completata",					"dimensioni completate",
			"Oltre la soglia",										1,
			"Cacciatore di mondi",									10,
			"Araldo delle dimensioni",								100,
			"Signore del varco",									500,
			"Custode degli abissi",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	26
			ACHIE_PROCAREA_SOLO,									"clear in solitaria della dimensione",		"clear in solitaria delle dimensioni",
			"Da solo nel buio",										1,
			"Solitudine consapevole",								10,
			"Uno contro il piano",									100,
			"Eremita dimensionale",									500,
			"Il lupo e la nebbia",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	27
			ACHIE_PROCAREA_GROUP,									"clear di gruppo della dimensione",			"clear di gruppo delle dimensioni",
			"Insieme si vince",										1,
			"Falange dimensionale",									10,
			"Legione del varco",									100,
			"Compagnia degli audaci",								500,
			"Esercito effimero",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	28
			ACHIE_PROCAREA_DEATH,									"morte nelle Dimensioni Effimere",			"morti nelle Dimensioni Effimere",
			"Prima caduta oltre il velo",							1,
			"Abituato alla nebbia",									10,
			"Anime perdute",										100,
			"Eterno ritorno",										500,
			"Catena di sventure",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	29
			ACHIE_PROCAREA_KILL_BOSS,								"boss ucciso nelle dimensioni",				"boss uccisi nelle dimensioni",
			"Colpo al cuore",										1,
			"Cacciatore di guardiani",								10,
			"Flagello dei titani",									100,
			"Nemesi dei custodi",									500,
			"Mietitore dimensionale",								5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	30
			ACHIE_PROCAREA_KILL_MOB,								"nemico ucciso nelle dimensioni",			"nemici uccisi nelle dimensioni",
			"Scacciato il branco",									25,
			"Falange spezzata",										100,
			"Stirpe annientata",									500,
			"Piaga del piano",										2000,
			"Annientatore cosmico",									10000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		},
		{	//	31
			ACHIE_PROCAREA_KILL_TRAP,								"trappola disinnescata",					"trappole disinnescate",
			"Occhio vigile",										1,
			"Niente sorprese",										10,
			"Untoccabile",											100,
			"Maestro dei tranelli",									500,
			"Nebbia non inganna",									5000,
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_NORMAL,									5
		}
	},

	//	RACESLAYER_ACHIE	classe -1 = achievement non attivo
	{
		{	//	 0
			RACE_HALFBREED,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	 1
			RACE_HUMAN,												"umano ucciso",								"umani uccisi",
			"Serial Killer",										1000,
			"Misantropo",											10000,
			"L'Apocalisse sono io!",								300000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	 2
			RACE_ELVEN, 											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	 3
			RACE_DWARF, 											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	 4
			RACE_HALFLING,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	 5
			RACE_GNOME,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	 6
			RACE_REPTILE,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0   //n_livelli
		},
		{	//	 7
			RACE_SPECIAL,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	 8
			RACE_LYCANTH,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	 9
			RACE_DRAGON,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	10
			RACE_UNDEAD,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	11
			RACE_ORC,   											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	12
			RACE_INSECT,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	13
			RACE_ARACHNID,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	14
			RACE_DINOSAUR,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	15
			RACE_FISH,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	16
			RACE_BIRD,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	17
			RACE_GIANT,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	18
			RACE_PREDATOR,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	19
			RACE_PARASITE,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	20
			RACE_SLIME,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	21
			RACE_DEMON,												"demone ucciso",							"demoni uccisi",
			"Per un mondo buono!",									100,
			"Il Signore della Loggia Bianca",						1000,
			"Asmodeus e' il mio scendiletto",						5000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	22
			RACE_SNAKE,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	23
			RACE_HERBIV,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	24
			RACE_TREE,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	25
			RACE_VEGGIE,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	26
			RACE_ELEMENT,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	27
			RACE_PLANAR,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	28
			RACE_DEVIL,												"diavolo ucciso",							"diavoli uccisi",
			"Odore di Zolfo",										100,
			"Diavoli? Perche' ce ne sono ancora?",					1000,
			"Araldo del Bene",										5000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	29
			RACE_GHOST,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	30
			RACE_GOBLIN,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	31
			RACE_TROLL,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	32
			RACE_VEGMAN,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	33
			RACE_MFLAYER,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	34
			RACE_PRIMATE,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	35
			RACE_ENFAN,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	36
			RACE_DARK_ELF,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	37
			RACE_GOLEM,												"golem ucciso",								"golem uccisi",
			"E' solo un oggetto!",									10,
			"Cio' che e' vivo non si muova!",						1000,
			"Disgregatore",											10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	38
			RACE_SKEXIE,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	39
			RACE_TROGMAN,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	40
			RACE_PATRYN,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	41
			RACE_LABRAT,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	42
			RACE_SARTAN,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	43
			RACE_TYTAN,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	44
			RACE_SMURF,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	45
			RACE_ROO,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	46
			RACE_HORSE,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	47
			RACE_DRAAGDIM,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	48
			RACE_ASTRAL,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	49
			RACE_GOD,												"Dio ucciso",								"Dei uccisi",
			"Non temo nessuno",										10,
			"Ribelliamoci agli Dei!",								100,
			"Godslayer",											1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	50
			RACE_GIANT_HILL,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	51
			RACE_GIANT_FROST,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	52
			RACE_GIANT_FIRE,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	53
			RACE_GIANT_CLOUD,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	54
			RACE_GIANT_STORM,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	55
			RACE_GIANT_STONE,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	56
			RACE_DRAGON_RED,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	57
			RACE_DRAGON_BLACK,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	58
			RACE_DRAGON_GREEN,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	59
			RACE_DRAGON_WHITE,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	60
			RACE_DRAGON_BLUE,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	61
			RACE_DRAGON_SILVER,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	62
			RACE_DRAGON_GOLD,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	63
			RACE_DRAGON_BRONZE,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	64
			RACE_DRAGON_COPPER,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	65
			RACE_DRAGON_BRASS,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	66
			RACE_UNDEAD_VAMPIRE,									"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	67
			RACE_UNDEAD_LICH,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	68
			RACE_UNDEAD_WIGHT,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	69
			RACE_UNDEAD_GHAST,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	70
			RACE_UNDEAD_SPECTRE,									"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	71
			RACE_UNDEAD_ZOMBIE,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	72
			RACE_UNDEAD_SKELETON,									"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	73
			RACE_UNDEAD_GHOUL,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	74
			RACE_HALF_ELVEN,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	75
			RACE_HALF_OGRE,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	76
			RACE_HALF_ORC,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	77
			RACE_HALF_GIANT,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	78
			RACE_LIZARDMAN,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	79
			RACE_DARK_DWARF,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	80
			RACE_DEEP_GNOME,										"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	81
			RACE_GNOLL,												"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	82
			RACE_GOLD_ELF,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	83
			RACE_WILD_ELF,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	84
			RACE_SEA_ELF,											"",											"",
			"",	/* lvl1 */											0,	//	lvl1_val
			"",	/* lvl2 */											0,	//	lvl2_val
			"",	/* lvl3 */											0,	//	lvl3_val
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			-1,											 			0,	/*	grado_diff */						0	//	n_livelli
		},
		{	//	85
			GROUP_INSECTOID,										"insettoide ucciso",						"insettoidi uccisi",
			"Autan",												100,
			"Ti schiaccio come un...",								1000,
			"Mister Raid",											10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	86
			GROUP_ANIMAL,											"animale ucciso",							"animali uccisi",
			"Carnivoro",											10,
			"Amo le carneficine",									1000,
			"Animal Killer",										10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	87
			GROUP_VEGGIE,											"vegetale ucciso",							"vegetali uccisi",
			"Solo un'insalatina leggera",							10,
			"Ma lo sai che la carne fa male?",						1000,
			"Mr. Vegan",											10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	88
			GROUP_SPECIALS,											"mostro esotico ucciso",					"mostri esotici uccisi",
			"Speciale? No, ordinario!",								10,
			"Specializzato",										100,
			"Special One",											1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	89
			GROUP_DARKRACES,										"abitante del sottosuolo ucciso",			"abitanti del sottosuolo uccisi",
			"Io porto la luce!",									10,
			"Paint it White",										100,
			"Ritorna nell'ombra!",									1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	90
			GROUP_DRAKES,											"drago ucciso",								"draghi uccisi",
			"Soffia Soffia!",										100,
			"Flagello di Draghi",									1000,
			"Un mondo di scaglie",									10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	91
			GROUP_GIANTS,											"gigante ucciso",							"giganti uccisi",
			"Mira sempre alle ginocchia!",							100,
			"Attento che cade!",									1000,
			"Chiamami Jack",										5000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	92
			GROUP_RACEHALFBREED,									"mezzosangue ucciso",						"mezzosangue uccisi",
			"Scegli da che parte stare!",							10,
			"Ne' carne ne' pesce",									100,
			"Scusa ma sono xenofobo",								10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	93
			GROUP_PLANAR,											"creatura planare uccisa",					"creature planari uccise",
			"Ognuno a casa sua",									10,
			"Killing Planner",										100,
			"Tu... non puoi... passare",							1000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	94
			GROUP_UNDEAD,											"non morto ucciso",							"non morti uccisi",
			"Spaccaossa",											10,
			"Tritaossa",											1000,
			"Drain nun te temo!",									10000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	95
			GROUP_GREENSKIN,										"pelleverde ucciso",						"pelleverde uccisi",
			"Sono brutti e puzzano!",								1000,
			"Muoiono in un niente",									5000,
			"Flagello dei pelleverde",								20000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		},
		{	//	96
			GROUP_HUMANOID,											"umanoide ucciso",							"umanoidi uccisi",
			"Se non e' umano non conta",							500,
			"Ma due hobbit valgono uno o due?",						2500,
			"Sterminiamoli tutti",									15000,
			"",	/* lvl4 */											0,	//	lvl4_val
			"",	/* lvl5 */											0,	//	lvl5_val
			"",	/* lvl6 */											0,	//	lvl6_val
			"",	/* lvl7 */											0,	//	lvl7_val
			"",	/* lvl8 */											0,	//	lvl8_val
			"",	/* lvl9 */											0,	//	lvl9_val
			"",	/* lvl10 */											0,	//	lvl10_val
			0,														LEV_BEGINNER,								3
		}
	}
};
} // namespace Alarmud
