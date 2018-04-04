/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __INTERPRETER_HPP
#define __INTERPRETER_HPP
/***************************  System  include ************************************/
/***************************  Local    include ************************************/
namespace Alarmud {
#define STAT_MIN_VAL 7
#define STAT_MAX_SUM 29
#define MENU         "\n\rBenvenuto su Nebbie Arcane\n\r\n\r0) Esci da Nebbie Arcane.\n\r1) Inizia il gioco a Myst\n\r2) Inserisci la descrizione del personaggio.\n\r3) Leggi la storia di Nebbie Arcane.\n\r4) Cambia la password.\n\rC) Cancella questo personaggio! (Solo dal livello chump)\n\r\n\r   Fai la tua scelta: "
#define RACEHELP          "Dwarf:     (Nano). Basso. Minor movimento.Infravisione. Piu` robusto.\n\r           Meno agile. Riguadagno degli HP piu`veloce. Minor danno dai veleni.\n\r           Razzista\n\r\n\rMoon Elf:  (Elfodella luna). Piu` movimento. Riguadagno piu` veloce dei punti\n\rmana. Piu` abile, meno robusto. Scopre le porte segrete. Razzista.\n\rInfra-visione\n\r\n\rGold Elf:  (Elfo dorato) Come il Moon, ma piu` intelligentee meno magico.\n\r\n\rWild Elf:  (Elfo selvaggio) Come il Moon, ma piu` forte emeno intelligiente.\n\r\n\rDark Elf:  (Elfo delle ombre) Come il Moon ma piu`agile, con differenti limiti\n\r           di classe. Minore precisione neicombattimenti alla luce.\n\r\n\r           Puo' essere chierico-mago.Human:     (Umano) Tutte le statistiche nella media. Unica razza illimitata.\n\r             in tutte le classi. Meno razzista di tutti.\n\r           Maggiori possibilita` di selezione delle classi come ranger\n\r           multiclasse. Gli umani possono essere Barbari.\n\r\n\rGnome:     (Gnomo) Basso. Molto intelligente. Poco magico e minor movimento.\n\r           Infra-visione, Riguadagno piu` veloce dei punti Mana.\n\r\n\rHalf-Elf:  (Mezz'elfo) Infra-visione, Scopre le porte segrete. Buon\n\r           quadagno dei punti movimento e Mana. Grade selezione multi-classe.\n\r           Unica razza che puo` avere un Druid (Druido) multi-classe\n\r\n\rHalf-Orc:  (Mezzo Orco) Infra-visione. Molto robusto. Basso carisma.\n\r           Buon riguadagno dei punti movimento.\n\r\n\rHalf-Ogre: (Mezzo Orco) Infra-visione, Molto forte e robusto. Poco abile\n\r           e poco intelligente. Buon riguadagno degli HP. Grosso e pesante\n\r           Solo Cleric (Clerico) o Warrior (Guerriero).\n\r\n\rHalf-Giant:(Mezzo gigante) Infra-visione. Maggior forza e robustezza. Minor\n\r           intelligenza, saggezza e abilita`. Buon riguadagno degli HP.\n\r           Molto grosso. Solo Warrior, Barbarian e Psi.\n\r           I giganti hanno molti HP al livello 1.\n\r\n\rATTENZIONE, le razze seguenti vengono attaccate a vista dalle guardie\n\r            e iniziano il gioco NON nella capitale. Consigliate a giocatori\n\r            esperti\n\r\n\rDark dwarf:Come il dwarf ma puo' essere cleric/thief\n\r\n\nHalfling:  Molto basso. Molto agile ma debole. Minor movimento. Riguadagno\n\r           degli HP piu` rapido. Minor danno dai veleni. Riguadagno dei\n\r           punti movimento piu` rapido.Puo' essere warrior/psi.\n\r\n\rSea Elf:   (Elfo del mare) Come il moon ma meno robusto, meno forte meno agile.\n\r\n\rGoblin :   Piccoletti ma cattivi, possono essere barbarian.\n\r\n\rOrc    :   Come gli half-orc ma + grossi.\n\r\n\rTroll  :   Razza sperimentale .... provate e sappiateci dire.\n\r\n\rDeep_Gnome: Come lo gnomo ma evil. Puo' essere cleric/monk.\n\r\n\r[Batti INVIO] "
#define WELC_MESSG "\n\r\n\r Benvenuto su Nebbie Arcane. Buon divertimento.\n\r\n\r"
#define STORY     "\n\rNebbie Arcane nasce dalla divisione del nucleo originale degli implementor di Ombra.\n\rNebbie Arcane e' codificato da Alar, che al momento della divisione ha ribattezzatoil codice AlarMUD, dopo aver donato il codice di OmbraMUD a Shade. AlarMUD deriva dal BenemMud, ma le modifiche sono gia' numerose.\n\rIn particolare la crescita e la morte dei personaggi sono molto diverse.\n\rUsa NEWS, INFO per avere maggiori informazioni. \n\r.Ricorda: e` solo un esperimento ed un tentativo di divertirci e divertire.\n\rNon prendertela con qualcuno se qualcosa va storto. Non e` un servizio ma\n\rsolo il risultato di un gruppo di appassionati.\n\r\n\r[Batti INVIO] "
#define CLASS_HELP "Cleric:       (Ecclesiastico) Buona difesa. Incantesimi di cura\n\r\n\rDruid:        (Druido) Tipo d'aria aperta. Vari incatesimi. Limitato nella\n\r              scelta degli oggetti che puo` indossare ed usare.\n\r\n\rWarrior:      (Guerriero) Grosso, forte e stupido.\n\r\n\rMagic-user:   Usa la magia. Debole e gracile, ma furbo e molto potente ai\n\r              livelli piu` alti.\n\r\n\rThief:        (Ladro) Veloce, agile e furtivo. Un tipo poco raccomandabile.\n\r\n\rMonk:         (Frate) Maestro delle arti marziali. Solo classe singola.\n\r\n\rBarbarian:    (Barbaro) Un forte guerriero, abile segugio e grandi abilita` di\n\r              sopravvivenza. Maggior movimento. Riguadagno degli HP molto\n\r              veloce. Limitato uso della magia. Solo classe singola e solo\n\r              Humans (umani) ed Half-Giants (mezzi giganti) possono essere\n\r              Barbarians.\n\r\n\rPaladin:      (Paladino) Guerriero sacro, difensore delle Virtu`.\n\r              Buone abilita` di combattimento ed alcune abilita` da Cleric.\n\r\n\rRanger:       Cacciatore delle foreste. I rangers sono in simbiosi con la vita\n\r              della foresta. Un misto di abilita` dei Druidi e dei Guerrieri.\n\r\n\rPsionist:     Manipolatore del cervello. Usa la potenza della mente per\n\r              materializzare la sua volonta`.\n\r\n\rRICORDA! Personaggi con classe singola guadagnano piu` HP ad ogni livello\n\r         rispetto a quelli multi-classe.\n\r\n\r[Batti INVIO] "

#define RU_SORCERER "\n\rHai scelto la classe Magic user. Abbiamo due tipi di maghi qui.\n\rIl Mage (Mago) utilizza il Mana per la sua potenza.\n\rIl Sorcerer (Stregone) utilizza la sua memoria per praticare gli incantesimi.\n\rLo stregone deve sedersi e memorizzare l'incantesimo di sua scelta prima di\n\rpoterlo usare (recall). Si dice che gli stregoni siano piu` deboli ai bassi livellima, una volta\n\racquisita esperienza, possono essere una delle classi piu` potenti.\n\r\n\rBatti 'si' se vuoi essere uno Stregone. Batti return se vuoi essere un mago.\n\rVuoi essere uno stregone ? (si/no): "
#define NEWBIE_NOTE "Benvenuto a Nebbie Arcane. Di seguito ci sono alcune istruzioni per aiutarti\n\ra cominciare.\n\r\n\r 1) A noi piace che la gente si diverta con il nostro gioco. Ma nessuno e`\n\r    obbligato. Se ti piace sei il benvenuto, altrimenti non disturbare quelli\n\r    che si divertono.\n\r 2) I comandi per i novizi sono HELP, NEWS e COMMANDS. Utilizza HELP per avere\n\r    risposta alle tue domande. Se sei ancora confuso, chiedi in giro.\n\r 3) Per favore, niente insulti o bestemmie nei canali pubblici. Ossia i\n\r    comandi GOSSIP, SHOUT ed AUCTION. La punizione consiste nella rimozione\n\r    dell'abilita` di usare questi comandi.\n\r 4) Non tutti i canali pubblici funzionano attraverso tutto il mondo.\n\r    GOSSIP, AUCTION e TELL funzionano solo nella zona intorno al personaggio.\n\r    Ci sono speciali abilita` ed incantesimi per parlare al mondo intero.     \n\r 5) Il gioco e` una cosa seria :). Ti prego, quindi di usare un nome consono\n\r    all'ambiente in cui stai entrando. Nomi stupidi o troppo spiritosi\n\r    potrebbero rovinare l'atmosfera fantastica del gioco agli altri giocatori.\n\r    Se il nome non dovesse essere adeguato, aspettati la vista di un immortale\n\r    che ti chiedera` di cambiarlo.\n\r 6) Ricorda che noi cerchiamo di avvicinare il gioco di ruolo alla realta`\n\r    (non troppo per la verita` :). Quindi, cose come morire di fame od essere\n\r    troppo stanco per difendersi POSSONO accadere.\n\r"

extern int WizLock;
extern int Silence;
extern long SystemFlags;         /* used for making teleport/astral/stuff not work */
extern int plr_tick_count;


void AskRollConfirm( struct descriptor_data* d ) ;
void InterpretaRoll( struct descriptor_data* d, char* riga );
const char* OneArgumentNoFill( const char* argument, char* first_arg ) ;
void RollPrompt( struct descriptor_data* d ) ;
void ShowRollInstruction( struct descriptor_data* d ) ;
void ShowStatInstruction( struct descriptor_data* d ) ;
void ThreeArgumentInterpreter(char* pchArgument, char* pchFirstArg, char* pchSecondArg, char* pchThirdArg );
int _check_ass_name(char* name) ;
int parse_name(const char* arg, char* name) ;
void argument_interpreter(const char* argument,char* first_arg,char* second_arg ) ;
void assign_command_pointers () ;
void assign_nannies_pointers();
void check_affected(char* msg) ;
void command_interpreter( struct char_data* ch, const char* argument ) ;
int fill_word(const char* argument) ;
int find_name(char* name) ;
void half_chop(const char* string, char* arg1, char* arg2,size_t len1=99,size_t len2=99) ;
int is_abbrev(const char* arg1, const char* arg2) ;
int is_number( char* str ) ;
void nanny(struct descriptor_data* d, char* arg) ;
int old_search_block(const char* argument,int begin,int length,const char** list,int mode) ;
const char* one_argument(const char* argument, char* first_arg ) ;
void only_argument(const char* argument, char* dest);
int search_block(const char* arg, const char** list, bool exact) ;
void show_class_selection(struct descriptor_data* d, int r) ;
void show_race_choice(struct descriptor_data* d) ;
int special(struct char_data* ch, int cmd, const char* arg) ;
#endif // _INTERPRETER_HPP
} // namespace Alarmud

