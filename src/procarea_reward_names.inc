/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* Nomi unici premio istanza: keyword, short_descr, description per variante. */
struct ProcRewardNameEntry {
	const char* keywords;
	const char* short_descr;
	const char* description;
};

static constexpr ProcRewardNameEntry kProcRewardLightNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"candela velo bruma sottile",
		"una candela del velo brumoso",
		"Una candela giace abbandonata qui, la sua fiamma e' piccola e incerta, come chiusa in una bolla di nebbia.",
	},
			{
		"lanterna runa sussurrante",
		"una lanterna dalle rune sussurranti",
		"Una lanterna e' stata lasciata cadere, i simboli sul vetro mutano al minimo soffio d'aria.",
	},
			{
		"lanterna runa sussurrante",
		"una lanterna dalle rune sussurranti",
		"Una lanterna riposa sul pavimento, i simboli sul vetro mutano al minimo soffio d'aria.",
	},
			{
		"torcia nebbia argentata",
		"una torcia di nebbia argentata",
		"Una torcia e' stata dimenticata in un angolo, il suo fuoco e' bianco e freddo al tatto, come ghiaccio antico.",
	},
			{
		"torcia nebbia argentata",
		"una torcia di nebbia argentata",
		"Una torcia giace smarrita sulla pietra, il suo fuoco e' bianco e freddo al tatto, come ghiaccio antico.",
	},
			{
		"globo luce varco oscuro",
		"un globo di luce del varco oscuro",
		"Un globo e' caduto qui, semidimenticato, una sfera nera trattiene un nucleo violaceo.",
	},
			{
		"globo luce varco oscuro",
		"un globo di luce del varco oscuro",
		"Un globo giace abbandonato qui, una sfera nera trattiene un nucleo violaceo.",
	},
			{
		"fiaccola cenere custode",
		"una fiaccola di cenere del custode",
		"Una fiaccola e' stata lasciata cadere, braci grigie si rigenerano sul bastone.",
	},
			{
		"fiaccola cenere custode",
		"una fiaccola di cenere del custode",
		"Una fiaccola riposa sul pavimento, braci grigie si rigenerano sul bastone.",
	},
			{
		"stella eclisse morente",
		"una stella dell'eclisse morente",
		"Una stella e' stata dimenticata in un angolo, una scheggia di cielo spento orbita lentamente intorno ad esso.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardFingerNames[PROCAREA_TEMPLATE_BANDS * 2] = {
			{
		"anello bruma sigillo infranto",
		"l'anello del sigillo infranto",
		"Un anello giace smarrito sulla pietra, metallo opaco, inciso con una runa spezzata a meta'.",
	},
			{
		"anello bruma filo speranza",
		"l'anello del filo di speranza",
		"Un anello e' caduto qui, semidimenticato, un filo d'argento avvolge l'anello come nebbia solida come nebbia solida.",
	},
			{
		"anello runa eco svanita",
		"l'anello dell'eco svanita",
		"Un anello giace abbandonato qui, la pietra al centro pulsa una volta, poi tace.",
	},
			{
		"anello runa verso dimenticato",
		"l'anello del verso dimenticato",
		"Un anello e' stato lasciato cadere, rune minuscole corrono sul bordo esterno.",
	},
			{
		"anello runa eco svanita",
		"l'anello dell'eco svanita",
		"Un anello riposa sul pavimento, la pietra al centro pulsa una volta, poi tace.",
	},
			{
		"anello runa verso dimenticato",
		"l'anello del verso dimenticato",
		"Un anello e' stato dimenticato in un angolo, rune minuscole corrono sul bordo esterno.",
	},
			{
		"anello nebbia luna specchiata",
		"l'anello della luna specchiata",
		"Un anello giace smarrito sulla pietra, il gioiello riflette la nebbia anche al coperto.",
	},
			{
		"anello nebbia stella caduta",
		"l'anello della stella caduta",
		"Un anello e' caduto qui, semidimenticato, un puntino argenteo brilla al centro del cerchio.",
	},
			{
		"anello nebbia luna specchiata",
		"l'anello della luna specchiata",
		"Un anello giace abbandonato qui, il gioiello riflette la nebbia anche al coperto.",
	},
			{
		"anello nebbia stella caduta",
		"l'anello della stella caduta",
		"Un anello e' stato lasciato cadere, un puntino argenteo brilla al centro del cerchio.",
	},
			{
		"anello varco catena spezzata",
		"l'anello della catena spezzata",
		"Un anello riposa sul pavimento, due anelli intrecciati si aprono come una serratura rotta.",
	},
			{
		"anello varco chiave senza porta",
		"l'anello della chiave senza porta",
		"Un anello e' stato dimenticato in un angolo, la sua gemma e' nera e assorbe la luce circostante.",
	},
			{
		"anello varco catena spezzata",
		"l'anello della catena spezzata",
		"Un anello giace smarrito sulla pietra, due anelli intrecciati si aprono come una serratura rotta.",
	},
			{
		"anello varco chiave senza porta",
		"l'anello della chiave senza porta",
		"Un anello e' caduto qui, semidimenticato, la sua gemma e' nera e assorbe la luce circostante.",
	},
			{
		"anello custode giuramento antico",
		"l'anello del giuramento antico",
		"Un anello giace abbandonato qui, inciso con una croce obliqua consumata.",
	},
			{
		"anello custode ultimo respiro",
		"l'anello dell'ultimo respiro",
		"Un anello e' stato lasciato cadere, il suo metallo e' scuro e caldo, come appena caduto da una mano.",
	},
			{
		"anello custode giuramento antico",
		"l'anello del giuramento antico",
		"Un anello riposa sul pavimento, inciso con una croce obliqua consumata.",
	},
			{
		"anello custode ultimo respiro",
		"l'anello dell'ultimo respiro",
		"Un anello e' stato dimenticato in un angolo, il suo metallo e' scuro e caldo, come appena caduto da una mano.",
	},
			{
		"anello eclisse orbita nera",
		"l'anello dell'orbita nera",
		"Un anello giace smarrito sulla pietra, un cerchio di ossidiana orbita intorno ad esso.",
	},
			{
		"anello eclisse sole ingoiato",
		"l'anello del sole ingoiato",
		"Un anello e' caduto qui, semidimenticato, al centro brilla un alone dorato intrappolato nel nero.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardNeckNames[PROCAREA_TEMPLATE_BANDS * 2] = {
			{
		"collare bruma filo nebbia",
		"il collare a filo di nebbia",
		"Un collare giace abbandonato qui, un filo di perle opache forma un cerchio stretto.",
	},
			{
		"amuleto bruma sussurro lontano",
		"l'amuleto del sussurro lontano",
		"Un amuleto e' stato lasciato cadere, una catena lunga tintinna senza vento.",
	},
			{
		"collare runa verso spezzato",
		"il collare al verso spezzato",
		"Un collare riposa sul pavimento, rune incise su piastre sottili si scontrano al movimento.",
	},
			{
		"amuleto runa eco dimenticata",
		"l'amuleto dell'eco dimenticata",
		"Un amuleto e' stato dimenticato in un angolo, un cristallo opaco pende lungo.",
	},
			{
		"collare runa verso spezzato",
		"il collare al verso spezzato",
		"Un collare giace smarrito sulla pietra, rune incise su piastre sottili si scontrano al movimento.",
	},
			{
		"amuleto runa eco dimenticata",
		"l'amuleto dell'eco dimenticata",
		"Un amuleto e' caduto qui, semidimenticato, un cristallo opaco pende lungo.",
	},
			{
		"collare nebbia argento gelido",
		"il collare d'argento gelido",
		"Un collare giace abbandonato qui, maglie sottili riflettono la Piazza anche lontano da Myst.",
	},
			{
		"amuleto nebbia stella darkstar",
		"l'amuleto della stella DarkStar",
		"Un amuleto e' stato lasciato cadere, una stella a otto punte di nebbia solida pende come nebbia solida.",
	},
			{
		"collare nebbia argento gelido",
		"il collare d'argento gelido",
		"Un collare riposa sul pavimento, maglie sottili riflettono la Piazza anche lontano da Myst.",
	},
			{
		"amuleto nebbia stella darkstar",
		"l'amuleto della stella DarkStar",
		"Un amuleto e' stato dimenticato in un angolo, una stella a otto punte di nebbia solida pende come nebbia solida.",
	},
			{
		"collare varco catena oscura",
		"il collare a catena oscura",
		"Un collare giace smarrito sulla pietra, anelli neri si intrecciano nella catena senza peso.",
	},
			{
		"amuleto varco croce obliqua",
		"l'amuleto della croce obliqua",
		"Un amuleto e' caduto qui, semidimenticato, la croce di DarkStar e' incisa su una piastra consumata.",
	},
			{
		"collare varco catena oscura",
		"il collare a catena oscura",
		"Un collare giace abbandonato qui, anelli neri si intrecciano nella catena senza peso.",
	},
			{
		"amuleto varco croce obliqua",
		"l'amuleto della croce obliqua",
		"Un amuleto e' stato lasciato cadere, la croce di DarkStar e' incisa su una piastra consumata.",
	},
			{
		"collare custode cenere viva",
		"il collare di cenere viva",
		"Un collare riposa sul pavimento, ceneri rosse si rigenerano tra gli anelli del metallo.",
	},
			{
		"amuleto custode cuore spezzato",
		"l'amuleto del cuore spezzato",
		"Un amuleto e' stato dimenticato in un angolo, due meta' di gemma si cercano senza unirsi.",
	},
			{
		"collare custode cenere viva",
		"il collare di cenere viva",
		"Un collare giace smarrito sulla pietra, ceneri rosse si rigenerano tra gli anelli del metallo.",
	},
			{
		"amuleto custode cuore spezzato",
		"l'amuleto del cuore spezzato",
		"Un amuleto e' caduto qui, semidimenticato, due meta' di gemma si cercano senza unirsi.",
	},
			{
		"collare eclisse anello nero",
		"il collare all'anello nero",
		"Un collare giace abbandonato qui, un disco eclittico forma un cerchio come un'eclisse fermata nel tempo.",
	},
			{
		"amuleto eclisse sole prigioniero",
		"l'amuleto del sole prigioniero",
		"Un amuleto e' stato lasciato cadere, un globo dorato e' intrappolato in vetro nero.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardBodyNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"corazza bruma velo cristallino",
		"la corazza del velo cristallino",
		"Una corazza riposa sul pavimento, piastre di bruma indurita scivolano l'una sull'altra.",
	},
			{
		"giubba runa tessuto instabile",
		"la giubba dal tessuto instabile",
		"Una giubba e' stata dimenticata in un angolo, rune ricamate mutano al minimo movimento.",
	},
			{
		"giubba runa tessuto instabile",
		"la giubba dal tessuto instabile",
		"Una giubba giace smarrita sulla pietra, rune ricamate mutano al minimo movimento.",
	},
			{
		"corpetto nebbia argento tessuto",
		"un corpetto nebbioso d'argento",
		"Un corpetto e' caduto qui, semidimenticato, filamenti argentei attraversano la stoffa.",
	},
			{
		"corpetto nebbia argento tessuto",
		"un corpetto nebbioso d'argento",
		"Un corpetto giace abbandonato qui, filamenti argentei attraversano la stoffa.",
	},
			{
		"corazza varco piastra oscura",
		"la corazza dalla piastra oscura",
		"Una corazza e' stata lasciata cadere, lamiera nera incisa con la croce obliqua di DarkStar.",
	},
			{
		"corazza varco piastra oscura",
		"la corazza dalla piastra oscura",
		"Una corazza riposa sul pavimento, lamiera nera incisa con la croce obliqua di DarkStar.",
	},
			{
		"usbergo custode sala finale",
		"l'usbergo della sala finale",
		"Un usbergo e' stato dimenticato in un angolo, sigilli fusi nella maglia ricordano l'ultimo guardiano.",
	},
			{
		"usbergo custode sala finale",
		"l'usbergo della sala finale",
		"Un usbergo giace smarrito sulla pietra, sigilli fusi nella maglia ricordano l'ultimo guardiano.",
	},
			{
		"corazza eclisse carapace vuoto",
		"la corazza dal carapace vuoto",
		"Una corazza e' caduta qui, semidimenticata, la parte del petto e' protetta da una scocca che non riflette nulla.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardHeadNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"elmo bruma corona nebbia",
		"l'elmo a corona di nebbia",
		"Un elmo giace abbandonato qui, una corona di vapori solidi corre intorno alla calotta.",
	},
			{
		"celata runa visiera sussurrante",
		"la celata dalla visiera sussurrante",
		"Una celata e' stata lasciata cadere, rune sulla visiera mutano al minimo soffio d'aria.",
	},
			{
		"celata runa visiera sussurrante",
		"la celata dalla visiera sussurrante",
		"Una celata riposa sul pavimento, rune sulla visiera mutano al minimo soffio d'aria.",
	},
			{
		"bacinetto nebbia cresta argentata",
		"il bacinetto dalla cresta argentata",
		"Un bacinetto e' stato dimenticato in un angolo, una cresta di nebbia solida corre lungo la calotta.",
	},
			{
		"bacinetto nebbia cresta argentata",
		"il bacinetto dalla cresta argentata",
		"Un bacinetto giace smarrito sulla pietra, una cresta di nebbia solida corre lungo la calotta.",
	},
			{
		"elmo varco maschera oscura",
		"l'elmo con maschera oscura",
		"Un elmo e' caduto qui, semidimenticato, la sua maschera integrata e' liscia e nera come il varco stesso.",
	},
			{
		"elmo varco maschera oscura",
		"l'elmo con maschera oscura",
		"Un elmo giace abbandonato qui, la sua maschera integrata e' liscia e nera come il varco stesso.",
	},
			{
		"elmo custode visiera spezzata",
		"l'elmo dalla visiera spezzata",
		"Un elmo e' stato lasciato cadere, una crepa obliqua attraversa il metallo come un colpo ricordato.",
	},
			{
		"elmo custode visiera spezzata",
		"l'elmo dalla visiera spezzata",
		"Un elmo riposa sul pavimento, una crepa obliqua attraversa il metallo come un colpo ricordato.",
	},
			{
		"corona eclisse sole negato",
		"la corona del sole negato",
		"Una corona e' stata dimenticata in un angolo, punte nere orbitano il cerchio come un'eclisse ferma.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardLegsNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"gambali bruma passo leggero",
		"i gambali dal passo leggero",
		"I gambali giacciono smarriti sulla pietra, la stoffa e' cosi' leggera che sembra fatta di velo.",
	},
			{
		"schinieri runa passo sigillato",
		"gli schinieri al passo sigillato",
		"Gli schinieri sono caduti qui, semidimenticati, rune sulle ginocchiere brillano.",
	},
			{
		"schinieri runa passo sigillato",
		"gli schinieri al passo sigillato",
		"Gli schinieri giacciono abbandonati qui, rune sulle ginocchiere brillano.",
	},
			{
		"gambali nebbia striscia argentata",
		"i gambali dalla striscia argentata",
		"I gambali sono stati lasciati cadere, strisce d'argento corrono sui gambali.",
	},
			{
		"gambali nebbia striscia argentata",
		"i gambali dalla striscia argentata",
		"I gambali riposano sul pavimento, strisce d'argento corrono sui gambali.",
	},
			{
		"schinieri varco passo profondo",
		"gli schinieri al passo profondo",
		"Gli schinieri sono stati dimenticati in un angolo, piastre scure coprono ginocchia e stinchi.",
	},
			{
		"schinieri varco passo profondo",
		"gli schinieri al passo profondo",
		"Gli schinieri giacciono smarriti sulla pietra, piastre scure coprono ginocchia e stinchi.",
	},
			{
		"gambali custode passo ultimo",
		"i gambali dell'ultimo passo",
		"I gambali sono caduti qui, semidimenticati, consumati ai bordi, rinforzati dove serviva al custode.",
	},
			{
		"gambali custode passo ultimo",
		"i gambali dell'ultimo passo",
		"I gambali giacciono abbandonati qui, consumati ai bordi, rinforzati dove serviva al custode.",
	},
			{
		"schinieri eclisse passo negato",
		"gli schinieri al passo negato",
		"Gli schinieri sono stati lasciati cadere, il metallo nero frena ogni movimento brusco.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardFeetNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"calzari bruma passo silente",
		"i calzari dal passo silente",
		"I calzari riposano sul pavimento, suole di nebbia compressa non fanno rumore.",
	},
			{
		"stivali runa impronta cancellata",
		"gli stivali dall'impronta cancellata",
		"Gli stivali sono stati dimenticati in un angolo, rune sotto la suola cancellano le tracce appena posate.",
	},
			{
		"stivali runa impronta cancellata",
		"gli stivali dall'impronta cancellata",
		"Gli stivali giacciono smarriti sulla pietra, rune sotto la suola cancellano le tracce appena posate.",
	},
			{
		"calzari nebbia suola argentata",
		"i calzari dalla suola argentata",
		"I calzari sono caduti qui, semidimenticati, l'argento sotto le suole riflette stanze lontane.",
	},
			{
		"calzari nebbia suola argentata",
		"i calzari dalla suola argentata",
		"I calzari giacciono abbandonati qui, l'argento sotto le suole riflette stanze lontane.",
	},
			{
		"stivali varco passo profondo",
		"gli stivali dal passo profondo",
		"Gli stivali sono stati lasciati cadere, talloni rinforzati e suola nera assorbono ogni urto.",
	},
			{
		"stivali varco passo profondo",
		"gli stivali dal passo profondo",
		"Gli stivali riposano sul pavimento, talloni rinforzati e suola nera assorbono ogni urto.",
	},
			{
		"calzari custode impronta finale",
		"i calzari dell'impronta finale",
		"I calzari sono stati dimenticati in un angolo, consumati davanti, intatti dietro; dentro c'e' ancora cenere fine.",
	},
			{
		"calzari custode impronta finale",
		"i calzari dell'impronta finale",
		"I calzari giacciono smarriti sulla pietra, consumati davanti, intatti dietro; dentro c'e' ancora cenere fine.",
	},
			{
		"stivali eclisse passo sospeso",
		"gli stivali dal passo sospeso",
		"Gli stivali sono caduti qui, semidimenticati, le suole non sembrano lasciare impronta.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardHandsNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"guanti bruma presa leggera",
		"i guanti dalla presa leggera",
		"I guanti giacciono abbandonati qui, dita avvolte in velo solido, afferrano senza stringere.",
	},
			{
		"guanti runa dita incise",
		"i guanti dalle dita incise",
		"I guanti sono stati lasciati cadere, ogni dito porta un simbolo che si accende al tocco.",
	},
			{
		"guanti runa dita incise",
		"i guanti dalle dita incise",
		"I guanti riposano sul pavimento, ogni dito porta un simbolo che si accende al tocco.",
	},
			{
		"guanti nebbia palmo argentato",
		"i guanti dal palmo argentato",
		"I guanti sono stati dimenticati in un angolo, i palmi sono riflettenti come specchi di nebbia.",
	},
			{
		"guanti nebbia palmo argentato",
		"i guanti dal palmo argentato",
		"I guanti giacciono smarriti sulla pietra, i palmi sono riflettenti come specchi di nebbia.",
	},
			{
		"guanti varco presa oscura",
		"i guanti dalla presa oscura",
		"I guanti sono caduti qui, semidimenticati, cuoio nero e rinforzi ai nodi delle dita.",
	},
			{
		"guanti varco presa oscura",
		"i guanti dalla presa oscura",
		"I guanti giacciono abbandonati qui, cuoio nero e rinforzi ai nodi delle dita.",
	},
			{
		"guanti custode presa spezzata",
		"i guanti dalla presa spezzata",
		"I guanti sono stati lasciati cadere, rinforzi consumati sulle nocche, intatti sul palmo interno.",
	},
			{
		"guanti custode presa spezzata",
		"i guanti dalla presa spezzata",
		"I guanti riposano sul pavimento, rinforzi consumati sulle nocche, intatti sul palmo interno.",
	},
			{
		"guanti eclisse presa negata",
		"i guanti dalla presa negata",
		"I guanti sono stati dimenticati in un angolo, le dita sembrano chiuse sul vuoto piu' che la lama.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardArmsNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"avambracci bruma rivo gelido",
		"gli avambracci al rivo gelido",
		"Gli avambracci giacciono smarriti sulla pietra, strisce di bruma corrono lungo le piastre come acqua immobile.",
	},
			{
		"parabracci runa arco inciso",
		"i parabracci all'arco inciso",
		"I parabracci sono caduti qui, semidimenticati, rune disegnano archi concentrici che pulsano debolmente.",
	},
			{
		"parabracci runa arco inciso",
		"i parabracci all'arco inciso",
		"I parabracci giacciono abbandonati qui, rune disegnano archi concentrici che pulsano debolmente.",
	},
			{
		"avambracci nebbia lamina argentata",
		"gli avambracci alla lamina argentata",
		"Gli avambracci sono stati lasciati cadere, lamine argentee si sovrappongono come nebbia stratificata.",
	},
			{
		"avambracci nebbia lamina argentata",
		"gli avambracci alla lamina argentata",
		"Gli avambracci riposano sul pavimento, lamine argentee si sovrappongono come nebbia stratificata.",
	},
			{
		"parabracci varco piastra nera",
		"i parabracci alla piastra nera",
		"I parabracci sono stati dimenticati in un angolo, piastre scure coprono avambraccio e gomito.",
	},
			{
		"parabracci varco piastra nera",
		"i parabracci alla piastra nera",
		"I parabracci giacciono smarriti sulla pietra, piastre scure coprono avambraccio e gomito.",
	},
			{
		"avambracci custode arco spezzato",
		"gli avambracci all'arco spezzato",
		"Gli avambracci sono caduti qui, semidimenticati, una crepa obliqua attraversa il metallo in obliquo sul metallo.",
	},
			{
		"avambracci custode arco spezzato",
		"gli avambracci all'arco spezzato",
		"Gli avambracci giacciono abbandonati qui, una crepa obliqua attraversa il metallo in obliquo sul metallo.",
	},
			{
		"parabracci eclisse arco negato",
		"i parabracci all'arco negato",
		"I parabracci sono stati lasciati cadere, il metallo non brilla e non arrugginisce.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardAboutNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"mantello bruma velo errante",
		"il mantello del velo errante",
		"Un mantello riposa sul pavimento, la stoffa e' nebbia tessuta, si muove anche senza vento.",
	},
			{
		"mantello runa bordo sussurrante",
		"il mantello dal bordo sussurrante",
		"Un mantello e' stato dimenticato in un angolo, il bordo e' ricamato di rune che mutano al minimo movimento.",
	},
			{
		"mantello runa bordo sussurrante",
		"il mantello dal bordo sussurrante",
		"Un mantello giace smarrito sulla pietra, il bordo e' ricamato di rune che mutano al minimo movimento.",
	},
			{
		"mantello nebbia coda argentata",
		"il mantello dalla coda argentata",
		"Un mantello e' caduto qui, semidimenticato, strisce d'argento corrono sulla stoffa.",
	},
			{
		"mantello nebbia coda argentata",
		"il mantello dalla coda argentata",
		"Un mantello giace abbandonato qui, strisce d'argento corrono sulla stoffa.",
	},
			{
		"mantello varco ombra profonda",
		"il mantello dall'ombra profonda",
		"Un mantello e' stato lasciato cadere, nero assoluto, pesante solo sotto uno sguardo fisso.",
	},
			{
		"mantello varco ombra profonda",
		"il mantello dall'ombra profonda",
		"Un mantello riposa sul pavimento, nero assoluto, pesante solo sotto uno sguardo fisso.",
	},
			{
		"mantello custode coda spezzata",
		"il mantello dalla coda spezzata",
		"Un mantello e' stato dimenticato in un angolo, l'orlo e' bruciato e rammendato con filo di sigillo.",
	},
			{
		"mantello custode coda spezzata",
		"il mantello dalla coda spezzata",
		"Un mantello giace smarrito sulla pietra, l'orlo e' bruciato e rammendato con filo di sigillo.",
	},
			{
		"mantello eclisse coda negata",
		"il mantello dalla coda negata",
		"Un mantello e' caduto qui, semidimenticato, disteso non proietta ombra, la assorbe.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardWaistNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"cintura bruma nodo nebbia",
		"la cintura al nodo di nebbia",
		"Una cintura giace abbandonata qui, un nodo di vapori solidi tiene su la cintura.",
	},
			{
		"cintura runa fibbia incisa",
		"la cintura alla fibbia incisa",
		"Una cintura e' stata lasciata cadere, la fibbia e' una runa tridimensionale che gira lentamente.",
	},
			{
		"cintura runa fibbia incisa",
		"la cintura alla fibbia incisa",
		"Una cintura riposa sul pavimento, la fibbia e' una runa tridimensionale che gira lentamente.",
	},
			{
		"cintura nebbia catena argentata",
		"la cintura alla catena argentata",
		"Una cintura e' stata dimenticata in un angolo, maglie sottili d'argento scintillano come bruma al sole.",
	},
			{
		"cintura nebbia catena argentata",
		"la cintura alla catena argentata",
		"Una cintura giace smarrita sulla pietra, maglie sottili d'argento scintillano come bruma al sole.",
	},
			{
		"cintura varco fibbia oscura",
		"la cintura alla fibbia oscura",
		"Una cintura e' caduta qui, semidimenticata, fibbia nera a forma di varco obliquo.",
	},
			{
		"cintura varco fibbia oscura",
		"la cintura alla fibbia oscura",
		"Una cintura giace abbandonata qui, fibbia nera a forma di varco obliquo.",
	},
			{
		"cintura custode nodo spezzato",
		"la cintura al nodo spezzato",
		"Una cintura e' stata lasciata cadere, rammendata dove il custode cadde in ginocchio.",
	},
			{
		"cintura custode nodo spezzato",
		"la cintura al nodo spezzato",
		"Una cintura riposa sul pavimento, rammendata dove il custode cadde in ginocchio.",
	},
			{
		"cintura eclisse fibbia negata",
		"la cintura alla fibbia negata",
		"Una cintura e' stata dimenticata in un angolo, la fibbia non ha buco ne' perno e si chiude da sola.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardWristNames[PROCAREA_TEMPLATE_BANDS * 2] = {
			{
		"bracciale bruma cerchio sinistro",
		"il bracciale al cerchio sinistro",
		"Un bracciale giace smarrito sulla pietra, nebbia chiusa in un cerchio opaco.",
	},
			{
		"bracciale bruma spirale destro",
		"il bracciale a spirale destro",
		"Un bracciale e' caduto qui, semidimenticato, una spirale d'argento sulla fascia destra sembra allargarsi al minimo scossone.",
	},
			{
		"bracciale runa arco sinistro",
		"il bracciale ad arco sinistro",
		"Un bracciale giace abbandonato qui, rune minuscole formano un arco sulla fascia sinistra.",
	},
			{
		"bracciale runa spirale destro",
		"il bracciale a spirale destro",
		"Un bracciale e' stato lasciato cadere, la spirale runica corre verso l'interno.",
	},
			{
		"bracciale runa arco sinistro",
		"il bracciale ad arco sinistro",
		"Un bracciale riposa sul pavimento, rune minuscole formano un arco sulla fascia sinistra.",
	},
			{
		"bracciale runa spirale destro",
		"il bracciale a spirale destro",
		"Un bracciale e' stato dimenticato in un angolo, la spirale runica corre verso l'interno.",
	},
			{
		"bracciale nebbia luna sinistro",
		"il bracciale alla luna sinistro",
		"Un bracciale giace smarrito sulla pietra, una mezzaluna di nebbia solida chiude un cerchio stretto.",
	},
			{
		"bracciale nebbia stella destro",
		"il bracciale alla stella destro",
		"Un bracciale e' caduto qui, semidimenticato, una stella a otto punte di bruma.",
	},
			{
		"bracciale nebbia luna sinistro",
		"il bracciale alla luna sinistro",
		"Un bracciale giace abbandonato qui, una mezzaluna di nebbia solida chiude un cerchio stretto.",
	},
			{
		"bracciale nebbia stella destro",
		"il bracciale alla stella destro",
		"Un bracciale e' stato lasciato cadere, una stella a otto punte di bruma.",
	},
			{
		"bracciale varco catena sinistro",
		"il bracciale a catena sinistro",
		"Un bracciale riposa sul pavimento, tre anelli neri tintinnano senza suono.",
	},
			{
		"bracciale varco sigillo destro",
		"il bracciale al sigillo destro",
		"Un bracciale e' stato dimenticato in un angolo, un sigillo obliquo si chiude da solo.",
	},
			{
		"bracciale varco catena sinistro",
		"il bracciale a catena sinistro",
		"Un bracciale giace smarrito sulla pietra, tre anelli neri tintinnano senza suono.",
	},
			{
		"bracciale varco sigillo destro",
		"il bracciale al sigillo destro",
		"Un bracciale e' caduto qui, semidimenticato, un sigillo obliquo si chiude da solo.",
	},
			{
		"bracciale custode ferro sinistro",
		"il bracciale di ferro sinistro",
		"Un bracciale giace abbandonato qui, ferro consumato, ancora caldo al tatto.",
	},
			{
		"bracciale custode cenere destro",
		"il bracciale di cenere destro",
		"Un bracciale e' stato lasciato cadere, cenere rossa si rigenera sul metallo.",
	},
			{
		"bracciale custode ferro sinistro",
		"il bracciale di ferro sinistro",
		"Un bracciale riposa sul pavimento, ferro consumato, ancora caldo al tatto.",
	},
			{
		"bracciale custode cenere destro",
		"il bracciale di cenere destro",
		"Un bracciale e' stato dimenticato in un angolo, cenere rossa si rigenera sul metallo.",
	},
			{
		"bracciale eclisse orbita sinistro",
		"il bracciale all'orbita sinistro",
		"Un bracciale giace smarrito sulla pietra, un anello nero orbita il cerchio senza toccarlo.",
	},
			{
		"bracciale eclisse sole destro",
		"il bracciale al sole destro",
		"Un bracciale e' caduto qui, semidimenticato, un sole miniatura e' intrappolato nel metallo.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardWeaponNames[PROCAREA_TEMPLATE_BANDS * 3] = {
			{
		"lama velo bruma spezzato",
		"la lama del velo spezzato",
		"Una lama giace abbandonata qui, il filo e' sottile e opaco come nebbia congelata.",
	},
			{
		"stiletto bruma ago nebbia",
		"lo stiletto ago di nebbia",
		"Uno stiletto e' stato lasciato cadere, sottile come un ago di bruma, entra dove l'armatura esita.",
	},
			{
		"mazza bruma nodo contuso",
		"la mazza al nodo contuso",
		"Una mazza riposa sul pavimento, un nodo di nebbia solida capovolge l'equilibrio del colpo.",
	},
			{
		"lama runa verso interrotto",
		"la lama al verso interrotto",
		"Una lama e' stata dimenticata in un angolo, rune sul pomo si spengono ad ogni fendente.",
	},
			{
		"stiletto runa punto dimenticato",
		"lo stiletto al punto dimenticato",
		"Uno stiletto giace smarrito sulla pietra, la punta e' un simbolo runico piegato in acciaio.",
	},
			{
		"mazza runa testa sigillata",
		"la mazza alla testa sigillata",
		"Una mazza e' caduta qui, semidimenticata, la testa e' un sigillo rotondo che non si apre mai.",
	},
			{
		"lama runa verso interrotto",
		"la lama al verso interrotto",
		"Una lama giace abbandonata qui, rune sul pomo si spengono ad ogni fendente.",
	},
			{
		"stiletto runa punto dimenticato",
		"lo stiletto al punto dimenticato",
		"Uno stiletto e' stato lasciato cadere, la punta e' un simbolo runico piegato in acciaio.",
	},
			{
		"mazza runa testa sigillata",
		"la mazza alla testa sigillata",
		"Una mazza riposa sul pavimento, la testa e' un sigillo rotondo che non si apre mai.",
	},
			{
		"lama nebbia filo argentato",
		"la lama dal filo argentato",
		"Una lama e' stata dimenticata in un angolo, il filo riflette la Piazza anche lontano da Myst.",
	},
			{
		"stiletto nebbia stella darkstar",
		"lo stiletto a stella DarkStar",
		"Uno stiletto giace smarrito sulla pietra, la guardia e' una stella a otto punte di nebbia solida.",
	},
			{
		"mazza nebbia testa lunare",
		"la mazza dalla testa lunare",
		"Una mazza e' caduta qui, semidimenticata, una mezzaluna di metallo nebbioso capovolge il peso.",
	},
			{
		"lama nebbia filo argentato",
		"la lama dal filo argentato",
		"Una lama giace abbandonata qui, il filo riflette la Piazza anche lontano da Myst.",
	},
			{
		"stiletto nebbia stella darkstar",
		"lo stiletto a stella DarkStar",
		"Uno stiletto e' stato lasciato cadere, la guardia e' una stella a otto punte di nebbia solida.",
	},
			{
		"mazza nebbia testa lunare",
		"la mazza dalla testa lunare",
		"Una mazza riposa sul pavimento, una mezzaluna di metallo nebbioso capovolge il peso.",
	},
			{
		"lama varco filo oscuro",
		"la lama dal filo oscuro",
		"Una lama e' stata dimenticata in un angolo, nera, senza riflesso, incisa con la croce obliqua.",
	},
			{
		"stiletto varco ago profondo",
		"lo stiletto ago profondo",
		"Uno stiletto giace smarrito sulla pietra, l'ago e' piu' lungo del manico, entra e non chiede permesso.",
	},
			{
		"mazza varco testa caduta",
		"la mazza dalla testa caduta",
		"Una mazza e' caduta qui, semidimenticata, testa squadrata e nera, pesante come una pietra del varco.",
	},
			{
		"lama varco filo oscuro",
		"la lama dal filo oscuro",
		"Una lama giace abbandonata qui, nera, senza riflesso, incisa con la croce obliqua.",
	},
			{
		"stiletto varco ago profondo",
		"lo stiletto ago profondo",
		"Uno stiletto e' stato lasciato cadere, l'ago e' piu' lungo del manico, entra e non chiede permesso.",
	},
			{
		"mazza varco testa caduta",
		"la mazza dalla testa caduta",
		"Una mazza riposa sul pavimento, testa squadrata e nera, pesante come una pietra del varco.",
	},
			{
		"lama custode filo spezzato",
		"la lama dal filo spezzato",
		"Una lama e' stata dimenticata in un angolo, una crepa corre lungo il filo senza renderlo meno tagliente.",
	},
			{
		"stiletto custode ago finale",
		"lo stiletto ago finale",
		"Uno stiletto giace smarrito sulla pietra, sottile, consumato, ancora affilato dove serviva.",
	},
			{
		"mazza custode testa scheggiata",
		"la mazza dalla testa scheggiata",
		"Una mazza e' caduta qui, semidimenticata, la testa e' scheggiata ma bilanciata sul manico.",
	},
			{
		"lama custode filo spezzato",
		"la lama dal filo spezzato",
		"Una lama giace abbandonata qui, una crepa corre lungo il filo senza renderlo meno tagliente.",
	},
			{
		"stiletto custode ago finale",
		"lo stiletto ago finale",
		"Uno stiletto e' stato lasciato cadere, sottile, consumato, ancora affilato dove serviva.",
	},
			{
		"mazza custode testa scheggiata",
		"la mazza dalla testa scheggiata",
		"Una mazza riposa sul pavimento, la testa e' scheggiata ma bilanciata sul manico.",
	},
			{
		"lama eclisse filo negato",
		"la lama dal filo negato",
		"Una lama e' stata dimenticata in un angolo, il filo non brilla e non arrugginisce, e' assenza affilata.",
	},
			{
		"stiletto eclisse ago prigioniero",
		"lo stiletto ago prigioniero",
		"Uno stiletto giace smarrito sulla pietra, un sole miniatura e' intrappolato nella guardia.",
	},
			{
		"mazza eclisse testa ingoiata",
		"la mazza dalla testa ingoiata",
		"Una mazza e' caduta qui, semidimenticata, la testa e' un disco nero che assorbe l'impatto visivo.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardHoldNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"focus bruma sfera opaca",
		"il focus a sfera opaca",
		"Un focus giace abbandonato qui, una sfera di nebbia compressa vibra sul pavimento.",
	},
			{
		"focus runa prisma instabile",
		"il focus a prisma instabile",
		"Un focus e' stato lasciato cadere, facce runiche ruotano senza motore visibile.",
	},
			{
		"focus runa prisma instabile",
		"il focus a prisma instabile",
		"Un focus riposa sul pavimento, facce runiche ruotano senza motore visibile.",
	},
			{
		"focus nebbia cristallo darkstar",
		"il focus a cristallo DarkStar",
		"Un focus e' stato dimenticato in un angolo, un cristallo a stella cattura la luce e la restituisce argentata.",
	},
			{
		"focus nebbia cristallo darkstar",
		"il focus a cristallo DarkStar",
		"Un focus giace smarrito sulla pietra, un cristallo a stella cattura la luce e la restituisce argentata.",
	},
			{
		"focus varco sfera profonda",
		"il focus a sfera profonda",
		"Un focus e' caduto qui, semidimenticato, nera, lucida, profonda oltre il diametro visibile.",
	},
			{
		"focus varco sfera profonda",
		"il focus a sfera profonda",
		"Un focus giace abbandonato qui, nera, lucida, profonda oltre il diametro visibile.",
	},
			{
		"focus custode scheggia sigillo",
		"il focus con la scheggia del sigillo",
		"Un focus e' stato lasciato cadere, un frammento di sigillo della sala finale pulsa debolmente.",
	},
			{
		"focus custode scheggia sigillo",
		"il focus con la scheggia del sigillo",
		"Un focus riposa sul pavimento, un frammento di sigillo della sala finale pulsa debolmente.",
	},
			{
		"focus eclisse sole catturato",
		"il focus al sole catturato",
		"Un focus e' stato dimenticato in un angolo, un sole miniatura e' intrappolato nel vetro nero.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardEarNames[PROCAREA_TEMPLATE_BANDS * 2] = {
			{
		"orecchino bruma campana sinistro",
		"l'orecchino a campana sinistro",
		"Un orecchino giace smarrito sulla pietra, una campanella di nebbia tintinna senza vento.",
	},
			{
		"orecchino bruma goccia destro",
		"l'orecchino a goccia destro",
		"Un orecchino e' caduto qui, semidimenticato, una goccia opaca scivola e torna al centro.",
	},
			{
		"orecchino runa spirale sinistro",
		"l'orecchino a spirale sinistro",
		"Un orecchino giace abbandonato qui, una spirale runica ruota lentamente.",
	},
			{
		"orecchino runa stella destro",
		"l'orecchino a stella destro",
		"Un orecchino e' stato lasciato cadere, una stella runica brilla quando la magia passa vicino.",
	},
			{
		"orecchino runa spirale sinistro",
		"l'orecchino a spirale sinistro",
		"Un orecchino riposa sul pavimento, una spirale runica ruota lentamente.",
	},
			{
		"orecchino runa stella destro",
		"l'orecchino a stella destro",
		"Un orecchino e' stato dimenticato in un angolo, una stella runica brilla quando la magia passa vicino.",
	},
			{
		"orecchino nebbia luna sinistro",
		"l'orecchino alla luna sinistro",
		"Un orecchino giace smarrito sulla pietra, una mezzaluna d'argento cattura suoni lontani.",
	},
			{
		"orecchino nebbia stella destro",
		"l'orecchino alla stella destro",
		"Un orecchino e' caduto qui, semidimenticato, una stella a otto punte scintilla come bruma al sole.",
	},
			{
		"orecchino nebbia luna sinistro",
		"l'orecchino alla luna sinistro",
		"Un orecchino giace abbandonato qui, una mezzaluna d'argento cattura suoni lontani.",
	},
			{
		"orecchino nebbia stella destro",
		"l'orecchino alla stella destro",
		"Un orecchino e' stato lasciato cadere, una stella a otto punte scintilla come bruma al sole.",
	},
			{
		"orecchino varco uncino sinistro",
		"l'orecchino all'uncino sinistro",
		"Un orecchino riposa sul pavimento, un uncino nero non graffia mai.",
	},
			{
		"orecchino varco sigillo destro",
		"l'orecchino al sigillo destro",
		"Un orecchino e' stato dimenticato in un angolo, un sigillo obliquo si chiude da solo.",
	},
			{
		"orecchino varco uncino sinistro",
		"l'orecchino all'uncino sinistro",
		"Un orecchino giace smarrito sulla pietra, un uncino nero non graffia mai.",
	},
			{
		"orecchino varco sigillo destro",
		"l'orecchino al sigillo destro",
		"Un orecchino e' caduto qui, semidimenticato, un sigillo obliquo si chiude da solo.",
	},
			{
		"orecchino custode piastra sinistro",
		"l'orecchino alla piastra sinistro",
		"Un orecchino giace abbandonato qui, una piastra consumata vibra al minimo scossone.",
	},
			{
		"orecchino custode cenere destro",
		"l'orecchino di cenere destro",
		"Un orecchino e' stato lasciato cadere, cenere rossa si rigenera sul gioiello.",
	},
			{
		"orecchino custode piastra sinistro",
		"l'orecchino alla piastra sinistro",
		"Un orecchino riposa sul pavimento, una piastra consumata vibra al minimo scossone.",
	},
			{
		"orecchino custode cenere destro",
		"l'orecchino di cenere destro",
		"Un orecchino e' stato dimenticato in un angolo, cenere rossa si rigenera sul gioiello.",
	},
			{
		"orecchino eclisse disco sinistro",
		"l'orecchino al disco sinistro",
		"Un orecchino giace smarrito sulla pietra, un disco nero assorbe rumori superflui.",
	},
			{
		"orecchino eclisse sole destro",
		"l'orecchino al sole destro",
		"Un orecchino e' caduto qui, semidimenticato, un sole miniatura e' intrappolato nel metallo.",
	},
};

static constexpr ProcRewardNameEntry kProcRewardEyesNames[PROCAREA_TEMPLATE_BANDS] = {
			{
		"lente bruma velo sottile",
		"le lenti del velo sottile",
		"Le lenti giacciono abbandonate qui, cristalli opachi filtrano la nebbia fitta.",
	},
			{
		"lente runa doppio verso",
		"le lenti al doppio verso",
		"Le lenti sono state lasciate cadere, rune sui vetri creano doppie immagini utili.",
	},
			{
		"lente runa doppio verso",
		"le lenti al doppio verso",
		"Le lenti riposano sul pavimento, rune sui vetri creano doppie immagini utili.",
	},
			{
		"lente nebbia stella darkstar",
		"le lenti a stella DarkStar",
		"Le lenti sono state dimenticate in un angolo, una stella a otto punte e' incisa su ogni vetro.",
	},
			{
		"lente nebbia stella darkstar",
		"le lenti a stella DarkStar",
		"Le lenti giacciono smarrite sulla pietra, una stella a otto punte e' incisa su ogni vetro.",
	},
			{
		"lente varco doppio buio",
		"le lenti al doppio buio",
		"Le lenti sono cadute qui, semidimenticate, vetri neri filtrano la luce fino al minimo necessario.",
	},
			{
		"lente varco doppio buio",
		"le lenti al doppio buio",
		"Le lenti giacciono abbandonate qui, vetri neri filtrano la luce fino al minimo necessario.",
	},
			{
		"lente custode scheggia vista",
		"le lenti con la scheggia della vista",
		"Le lenti sono state lasciate cadere, una scheggia del sigillo del custode e' incastonata nella montatura.",
	},
			{
		"lente custode scheggia vista",
		"le lenti con la scheggia della vista",
		"Le lenti riposano sul pavimento, una scheggia del sigillo del custode e' incastonata nella montatura.",
	},
			{
		"lente eclisse sole negato",
		"le lenti al sole negato",
		"Le lenti sono state dimenticate in un angolo, i vetri assorbono il bagliore e lasciano solo contorni.",
	},
};

[[nodiscard]] static const ProcRewardNameEntry*
procarea_reward_name_for(const ProcRewardGearSlotDef& def, int variant) {
	switch(def.slot) {
	case ProcRewardGearSlot::Light:
		return &kProcRewardLightNames[variant];
	case ProcRewardGearSlot::Finger:
		return &kProcRewardFingerNames[variant];
	case ProcRewardGearSlot::Neck:
		return &kProcRewardNeckNames[variant];
	case ProcRewardGearSlot::Body:
		return &kProcRewardBodyNames[variant];
	case ProcRewardGearSlot::Head:
		return &kProcRewardHeadNames[variant];
	case ProcRewardGearSlot::Legs:
		return &kProcRewardLegsNames[variant];
	case ProcRewardGearSlot::Feet:
		return &kProcRewardFeetNames[variant];
	case ProcRewardGearSlot::Hands:
		return &kProcRewardHandsNames[variant];
	case ProcRewardGearSlot::Arms:
		return &kProcRewardArmsNames[variant];
	case ProcRewardGearSlot::About:
		return &kProcRewardAboutNames[variant];
	case ProcRewardGearSlot::Waist:
		return &kProcRewardWaistNames[variant];
	case ProcRewardGearSlot::Wrist:
		return &kProcRewardWristNames[variant];
	case ProcRewardGearSlot::Wield:
		return &kProcRewardWeaponNames[variant];
	case ProcRewardGearSlot::Hold:
		return &kProcRewardHoldNames[variant];
	case ProcRewardGearSlot::Ear:
		return &kProcRewardEarNames[variant];
	case ProcRewardGearSlot::Eyes:
		return &kProcRewardEyesNames[variant];
	default:
		return nullptr;
	}
}
