/*$Id: head.c,v 1.2 2002/02/13 12:30:58 root Exp $
*/
/* Creazione testa */
#define SEVERED_HEAD       30   /* real number of the severed head base item */
      
/* for now I use the scraps number, should be fine */
void make_corpse(struct char_data *ch, int killedbytype)
{
   struct obj_data *corpse, *o, *cp;
   struct obj_data *money;        
   char buf[MAX_INPUT_LENGTH], 
   spec_desc[255]; /* used in describing the corpse */
   int r_num,i, ADeadBody=FALSE;
   
   
   
   
   
   /*   char *strdup(char *source); */
   
   struct obj_data *create_money( int amount );
   
   CREATE(corpse, struct obj_data, 1);
   clear_object(corpse);
   
   corpse->item_number = NOWHERE;
   corpse->in_room = NOWHERE;
   
   if (!IS_NPC(ch) || (!IsUndead(ch))) 
   {
      /* this is so we drop a severed head at the corpse, just for visual */
      if( GET_HIT(ch) < -50 && ( killedbytype == TYPE_SLASH || 
				killedbytype == TYPE_CLEAVE ) )
      {
	 if( ( r_num = real_object( SEVERED_HEAD ) ) >= 0 ) 
	 {
	    cp = read_object(r_num, REAL);
	    sprintf(buf,"head severed %s",corpse->name);
	    cp->name=strdup(buf);
	    sprintf( buf,"the severed head of %s",
		    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	    cp->short_description=strdup(buf);
	    cp->action_description=strdup(buf);
	    sprintf(buf,"%s is lying on the ground.",buf);
	    cp->description=strdup(buf);
	    
	    cp->obj_flags.type_flag = ITEM_CONTAINER;
	    cp->obj_flags.wear_flags = ITEM_TAKE;
	    cp->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
	    cp->affected[0].modifier=GET_RACE(ch);     /* race of corpse not used */
	    cp->affected[1].modifier=GetMaxLevel(ch);  /* level of corpsenot used */
	    cp->obj_flags.value[3] = 1; /* corpse identifyer */
	    if (IS_NPC(ch)) 
	    {
	       cp->obj_flags.timer= -3;        /*Ok, ora le teste sono permanenti */
	       cp->obj_flags.cost_per_day=1000;
	       cp->obj_flags.cost=0;
	       /* Devono pero` essere raccolte in
		* tempo..... 3 tick */
	    }
	    
	    else
	    cp->obj_flags.timer = MAX_PC_CORPSE_TIME; /*Non per i PC */
	    
	    obj_to_room(cp,ch->in_room);
	 } /* we got the numerb of the item... */
   
/* fine creazione testa */
