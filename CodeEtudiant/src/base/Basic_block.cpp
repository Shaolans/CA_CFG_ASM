#include <Basic_block.h>


//static
void Basic_block::show_dependances(Instruction *i1, Instruction *i2){

   if(i1->is_dep_RAW1(i2))
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": RAW1"<<endl;
   if(i1->is_dep_RAW2(i2))
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": RAW2"<<endl;

   if(i1->is_dep_WAR(i2))
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": WAR"<<endl;

   if(i1->is_dep_WAW(i2))
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": WAW"<<endl;

   if(i1->is_dep_MEM(i2))
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": MEM"<<endl;

}

Basic_block::Basic_block():
  Use(vector<bool>(NB_REG, false)),
  LiveIn(vector<bool>(NB_REG, false)),
  LiveOut(vector<bool>(NB_REG, false)),
  Def(vector<bool>(NB_REG, false)),
  DefLiveOut(vector<int>(NB_REG, -1)),
  Domin(vector<bool>(NB_MAX_BB, false)){
   _head = NULL;
   _end = NULL;
   _branch = NULL;
   _index = 0;
   _nb_instr = 0;
   _firstInst=NULL;
   _lastInst=NULL;
   dep_done = false;
   use_def_done = false;
}


Basic_block::~Basic_block(){}


void Basic_block::set_index(int i){
   _index = i;
}

int Basic_block::get_index(){
   return _index;
}

void Basic_block::set_head(Line *head){
   _head = head;
}

void Basic_block::set_end(Line *end){
   _end = end;
}

Line* Basic_block::get_head(){
   return _head;
}

Line* Basic_block::get_end(){
   return _end;
}

void Basic_block::set_successor1(Basic_block *BB){
   _succ.push_front(BB);
}

Basic_block *Basic_block::get_successor1(){
   if (_succ.size()>0)
      return _succ.front();
   else
      return NULL;
}

void Basic_block::set_successor2(Basic_block *BB){
   _succ.push_back(BB);
}

Basic_block *Basic_block::get_successor2(){
   if (_succ.size()> 1)
      return _succ.back();
   else
      return NULL;
}

void Basic_block::set_predecessor(Basic_block *BB){
   _pred.push_back(BB);
}

Basic_block *Basic_block::get_predecessor(int index){

   list<Basic_block*>::iterator it;
   it=_pred.begin();
   int size=(int)_pred.size();
   if(index< size){
      for (int i=0; i<index; i++, it++);
      return *it;
   }
   else cout<<"Error: index is bigger than the size of the list"<<endl;
   return _pred.back();

}

int Basic_block::get_nb_succ(){
   return _succ.size();
}

int Basic_block::get_nb_pred(){
   return _pred.size();
}

void Basic_block::set_branch(Line* br){
   _branch=br;
}

Line* Basic_block::get_branch(){
   return _branch;
}

void Basic_block::display(){
   cout<<"Begin BB"<<endl;
   Line* element = _head;
   int i=0;
   if(element == _end)
      cout << _head->get_content() <<endl;

   while(element != _end->get_next()){
      if(element->isInst()){
	 cout<<"i"<<i<<" ";
	 i++;
      }
      if(!element->isDirective())
	 cout <<element->get_content() <<endl;

      element = element->get_next();
   }
   cout<<"End BB"<<endl;
}

string Basic_block::get_content(){
   string rt = "";
   Line* element = _head;


   while(element != _end->get_next()){
     if(element->isInst()){
	rt = rt + element->get_content() + "\\l" ;
     }
     else if(element->isLabel())
       rt = rt + element->get_content() + "\\l" ;

      element = element->get_next();
   }

   return rt ;
}

int Basic_block::size(){
   Line* element = _head;
   int lenght=0;
   while(element != _end){
      lenght++;
      if(element->get_next()==_end)
	 break;
      else
	 element = element->get_next();
   }
   return lenght;
}


void Basic_block::restitution(string const filename){
   Line* element = _head;
   ofstream monflux(filename.c_str(), ios::app);
   if(monflux){
      monflux<<"Begin BB"<<endl;
      if(element == _end)
	monflux << _head->get_content() <<endl;
      while(element != _end)
      {
	 if(element->isInst())
	    monflux<<"\t";
	 if(!element->isDirective())
	    monflux << element->get_content()<<endl ;

	 if(element->get_next()==_end){
	    if(element->get_next()->isInst())
	       monflux<<"\t";
	    if(!element->isDirective())
	       monflux << element->get_next()->get_content()<<endl;
	    break;
	 }
	 else element = element->get_next();
      }
      monflux<<"End BB\n\n"<<endl;
   }
   else {
      cout<<"Error cannot open the file"<<endl;
   }
   monflux.close();

}

bool Basic_block::is_labeled(){
   if (_head->isLabel()){
      return true;
   }
   else return false;
}

int Basic_block::get_nb_inst(){
   if (_nb_instr == 0)
      link_instructions();
   return _nb_instr;

}

Instruction* Basic_block::get_instruction_at_index(int index){
   Instruction *inst;

   if(index >= get_nb_inst()){
      return NULL;
   }

   inst=get_first_instruction();

   for(int i=0; i<index; i++, inst=inst->get_next());

   return inst;
}

Line* Basic_block::get_first_line_instruction(){

   Line *current = _head;
   while(!current->isInst()){
      current=current->get_next();
      if(current==_end && !current->isInst())
	 return NULL;
   }
   return current;
}

Instruction* Basic_block::get_first_instruction(){
  if(_firstInst==NULL){
      _firstInst= getInst(this->get_first_line_instruction());
      this->link_instructions();
  }
   return _firstInst;
}

Instruction* Basic_block::get_last_instruction(){
   if(_lastInst==NULL)
      this->link_instructions();
   return _lastInst;
}


/* link_instructions  num�rote les instructions du bloc */
/* remplit le champ nb d'instructions du bloc (_nb_instr) */
/* remplit le champ derniere instruction du bloc (_lastInst) */
void Basic_block::link_instructions(){

   int index=0;
   Line *current, *next;
   current=get_first_line_instruction();
   next=current->get_next();

   Instruction *i1 = getInst(current);

   i1->set_index(index);
   index++;
   Instruction *i2;

//Calcul des successeurs
   while(current != _end){

      while(!next->isInst()){
	 next=next->get_next();
	 if(next==_end){
	    if(next->isInst())
	       break;
	    else{
	       _lastInst = i1;
	       _nb_instr = index;
	       return;
	    }
	 }
      }

      i2 = getInst(next);
      i2->set_index(index);
      index++;
      i1->set_link_succ_pred(i2);

      i1=i2;
      current=next;
      next=next->get_next();
   }
   _lastInst = i1;
   _nb_instr = index;
}

bool Basic_block::is_delayed_slot(Instruction *i){
   if (get_branch()== NULL)
      return false;
   int j = (getInst(get_branch()))->get_index();
   return (j < i-> get_index());

}

/* set_link_succ_pred : ajoute succ comme successeur de this et ajoute this comme pr�d�cesseur de succ
 */

void Basic_block::set_link_succ_pred(Basic_block* succ){
  _succ.push_back(succ);
  succ->set_predecessor(this);
}

/* add_dep_link ajoute la d�pendance avec pred � la liste des dependances pr�c�desseurs de succ */
/* ajoute la dependance avec succ � la liste des d�pendances successeurs de pred */

/* dep est une structure de donn�es contenant une instruction et  un type de d�pendance */

void add_dep_link(Instruction *pred, Instruction* succ, t_Dep type){
   dep *d;
   d=(dep*)malloc(sizeof(dep));
   d->inst=succ;
   d->type=type;
   pred->add_succ_dep(d);

   d=(dep*)malloc(sizeof(dep));
   d->inst=pred;
   d->type=type;
   succ->add_pred_dep(d);
}


/* calcul des d�pendances entre les instructions dans le bloc  */
/* une instruction a au plus 1 reg dest et 2 reg sources */
/* Attention le reg source peut �tre 2 fois le m�me */
/* Utiliser les m�thodes is_dep_RAW1, is_dep_RAW2, is_dep_WAR, is_dep_WAW, is_dep_MEM pour d�terminer les d�pendances */

/* ne pas oublier les d�pendances de controle avec le branchement s'il y en a un */

/* utiliser la fonction add_dep_link ci-dessus qui ajoute � la liste des d�pendances pred et succ une dependance entre 2 instructions */


void Basic_block::comput_pred_succ_dep(){


   link_instructions();
   if (dep_done) return;
   Instruction *current;
   Instruction *itmp;
   int rawTab[64] ; // ecriture
   list<int> warTab[64] ; // lecture
   int dest, source1, source2;
   int i=0;
   list<Instruction *> lastMemInst ;
   list<Instruction *>:: iterator iteInst;
   Instruction * tmpMemInst;
   bool hasDep[get_nb_inst()] = {false};

   current = get_first_instruction();

   Instruction * derniereInst ;

   if(get_last_instruction()->is_mem() ){
	   derniereInst = get_last_instruction()->get_next();
	}
	else{
		derniereInst = get_last_instruction();
	}


   for(int k=0; k<64; k++){
      rawTab[k]=-1;
   }

   while( current != derniereInst ){

     t_Inst type = current->get_type();

		source1 = -1;
		source2=-1;
		dest=-1;

      if(current->get_reg_src1()){

          source1 = current->get_reg_src1()->get_reg();

          if(rawTab[source1] != -1 ){
            //cout << "Dependance RAW entre " << rawTab[source1] << " et " << i << "\n";
            add_dep_link(get_instruction_at_index(rawTab[source1]),current,RAW);
			hasDep[rawTab[source1]] = true;
          }

          warTab[source1].push_front(i);

      }

      if(current->get_reg_src2()){
          source2 = current->get_reg_src2()->get_reg();

          if(rawTab[source2] != -1 ){
            //cout << "Dependance RAW entre " << rawTab[source2] << " et " << i << "\n";
            add_dep_link(get_instruction_at_index(rawTab[source2]), current, RAW);
            hasDep[rawTab[source2]] = true;
          }

          warTab[source2].push_front(i);

      }

      if(current->get_reg_dst()){

        dest = current->get_reg_dst()->get_reg();

		//on vide la liste
        while(!warTab[dest].empty() ){

			//On pop
			int k = warTab[dest].front();
			warTab[dest].remove(k);

			if(k!=i){
				//cout << "Dependance WAR entre " << k << " et " << i << "\n";
				add_dep_link(get_instruction_at_index(k),current,WAR);
				hasDep[k] = true;
			}
        }

        if(rawTab[dest] != -1){

          //cout << "Dependance WAW entre " << rawTab[dest] << " et " << i << "\n";
          add_dep_link(get_instruction_at_index(rawTab[dest]), current,WAW);
          hasDep[rawTab[dest]] = true;
        }

        rawTab[dest]=i;

      }

      if(current->is_mem()){

			for(iteInst = lastMemInst.begin(); iteInst!=lastMemInst.end(); ++iteInst){

				tmpMemInst = *iteInst;

				if (current->is_dep_MEM(tmpMemInst)){
					add_dep_link(tmpMemInst,current, MEMDEP);
					//cout << "Dependance MEM entre " << tmpMemInst->get_index() << " et " << i << "\n";
					hasDep[tmpMemInst->get_index()]=true;
				}

			}
			lastMemInst.push_front(current);
	    }

     i++;
     current = current->get_next();

   }

   if(current){
	   current = current->get_prev();

	   if(current->is_branch()){

		   for(i=0; i<get_nb_inst()-2; i++){
			   if(!hasDep[i]){
				   add_dep_link(get_instruction_at_index(i),current, CONTROL);
				   //cout << "Dependance CONTROL entre " << i << " et " << get_nb_inst()-1 << "\n";
			   }
		   }
	   }
	}

   // NE PAS ENLEVER : cette fonction ne doit �tre appel�e qu'une seule fois
   dep_done = true;
  return;
}

void Basic_block::reset_pred_succ_dep(){

  Instruction *ic=get_first_instruction();
  while (ic){
    ic -> reset_pred_succ_dep();
    ic = ic -> get_next();
  }
  dep_done = false;
  return;
}

/* calcul le nb de cycles pour executer le BB, on suppose qu'une instruction peut sortir du pipeline � chaque cycle, il faut trouver les cycles de gel induit par les d�pendances */

int Basic_block::nb_cycles(){

  Instruction *ic=get_first_instruction();

   /* tableau ci-dessous utile pour savoir pour chaque instruction quand elle sort pour en d�duire les cycles qu'elle peut induire avec les instructions qui en d�pendent, initialisation � -1  */

   vector<int> inst_cycle(get_nb_inst());
   for (int i=0; i< get_nb_inst(); i++ ){
     inst_cycle[i] = -1;
   }
   comput_pred_succ_dep();

   inst_cycle[0] = 1;

   Instruction *tmp_inst;
   Instruction *inst_calc;
   int max = 0;
   int i = 0;
   int j = 0;
   int tmp = 0;
   int tmp_delai = 0;
   for(i = 1; i < get_nb_inst(); i++){
   	tmp_inst = get_instruction_at_index(i);
   	max = inst_cycle[i-1] + 1;
   	for(j = 0; j < tmp_inst->get_nb_pred(); j++){
      inst_calc = tmp_inst->get_pred_dep(j)->inst;
   		tmp_delai = delai(inst_calc->get_type(), tmp_inst->get_type());
   		if(tmp_delai != -1){
   			tmp = inst_cycle[inst_calc->get_index()] + tmp_delai;
   			if(tmp > max){
   				max = tmp;
   			}
   		}
   	}
   	inst_cycle[i] = max;
    //cout << "i" << i << " cycle " << max << endl;
   }

  return inst_cycle[get_nb_inst()-1];
}

/*
calcule DEF et USE pour l'analyse de registre vivant
� la fin on doit avoir
 USE[i] vaut 1 si $i est utilis� dans le bloc avant d'�tre potentiellement d�fini dans le bloc
 DEF[i] vaut 1 si $i est d�fini dans le bloc
ne pas oublier les conventions d'appel : les registres $4, $5, $6, $7 peuvent contenir des param�tres (du 1er au 4eme les autres sont sur la pile) avant un appel de fonctions, au retour d'une fonction $2 a �t� �crit car il contient la valeur de retour (sauf si on rend void). Un appel de fonction (call) �crit aussi l'adresse de retour dans $31.

******************/

void Basic_block::compute_use_def(void){
  Instruction * inst = get_first_instruction();
  if (use_def_done) return;
  OPRegister *dst;
  OPRegister *src1;
  OPRegister *src2;
  int tmpreg;
  while(inst){
    src1 = inst->get_reg_src1();
    if(src1){
      tmpreg = src1->get_reg();
      if(!Def[tmpreg] && !Use[tmpreg]){
        Use[tmpreg] = true;
      }
    }

    src2 = inst->get_reg_src2();
    if(src2){
      tmpreg = src2->get_reg();
      if(!Def[tmpreg] && !Use[tmpreg]){
        Use[tmpreg] = true;
      }
    }

    dst = inst->get_reg_dst();
    if(dst){
      tmpreg = dst->get_reg();
      if(!Def[tmpreg]){
        Def[tmpreg] = true;
      }
    }

    inst = inst->get_next();

  }

  inst = get_instruction_at_index(get_nb_inst() - 2);
  if(inst->is_call()){
    Use[4] = true;
    Use[5] = true;
    Use[6] = true;
    Def[2] = true;
    Def[31] = true;
  }

#ifdef DEBUG
  cout << "****** BB " << get_index() << "************" << endl;
  cout << "USE : " ;
  for(int i=0; i<NB_REG; i++){
      if (Use[i])
      cout << "$"<< i << " ";
  }
  cout << endl;
  cout << "DEF : " ;
  for(int i=0; i<NB_REG; i++){
      if (Def[i])
	cout << "$"<< i << " ";
    }
  cout << endl;
#endif

    return;
}

/**** compute_def_liveout
� la fin de la fonction on doit avoir
DefLiveOut[i] vaut l'index de l'instruction du bloc qui d�finit $i si $i vivant en sortie seulement
Si $i est d�fini plusieurs fois c'est l'instruction avec l'index le plus grand
*****/
void Basic_block::compute_def_liveout(){

  Instruction * inst = get_first_instruction();
  OPRegister *dst;
  int reg;

  for(int i = 0; i < get_nb_inst(); i++){
    dst = get_instruction_at_index(i)->get_reg_dst();
    if(dst){
      reg = dst->get_reg();
      if(LiveOut[reg]){
        DefLiveOut[reg] = i;
      }
    }
  }


#ifdef DEBUG
  cout << "DEF LIVE OUT: " << endl;
  for(int i=0; i<NB_REG; i++){
      if (DefLiveOut[i] != -1)
	cout << "$"<< i << " definit par i" << DefLiveOut[i] << endl;
    }

#endif

}



/**** renomme les registres renommables : ceux qui sont d�finis et utilis�s dans le bloc et dont la d�finition n'est pas vivante en sortie
Utilise comme registres disponibles ceux dont le num�ro est dans la liste param�tre
*****/
void Basic_block::reg_rename(list<int> *frees){
  Instruction * inst = get_first_instruction();
  int newr;
  compute_def_liveout();


  Instruction *current;
  dep *tmp;
  OPRegister *dst;
  OPRegister *tmpreg1;
  OPRegister *tmpreg2;
  int reg;
  int nb_inst = get_nb_inst();
  list<int> lfree(frees->begin(), frees->end());

  //parcours toutes les instructions
  for(int i = 0; i < nb_inst; i++){
    current = get_instruction_at_index(i);
    dst = current->get_reg_dst();

    //s'il y a une definition de registre
    if(dst){
      reg = dst->get_reg();

      //si elle n'est pas vivante en sortie ou du moins n'est pas la derniere
      //definition en sortie
      if(DefLiveOut[reg] != i){
        if(lfree.empty()) return;
        newr = lfree.front();
        lfree.pop_front();

        //on cherche toutes les dependances RAW jusqu'à trouver une nouvelle
        //definition ou alors a la fin du BB
        for(int j = 0; j < current->get_nb_succ(); j++){
          tmp = current->get_succ_dep(j);

          if(tmp->type == RAW){
            tmpreg1 = tmp->inst->get_reg_src1();
            tmpreg2 = tmp->inst->get_reg_src2();

            if(tmpreg1 && tmpreg1->get_reg() == reg){
              //modifier le src1 de tmp
              tmpreg1->set_reg(newr);
            }

            if(tmpreg2 && tmpreg2->get_reg() == reg){
              //modifier le src2 de tmp
              tmpreg2->set_reg(newr);
            }

          }

        }
        //modifier l'instruction current a la nouvelle definition newr
        dst->set_reg(newr);

      }
    }

  }

}


/**** renomme les registres renommables : ceux qui sont d�finis et utilis�s dans le bloc et dont la d�finition n'est pas vivante en sortie
Utilise comme registres disponibles ceux dont le num�ro est dans la liste param�tre
*****/
void Basic_block::reg_rename(){
  Instruction * inst = get_first_instruction();
  int newr;
  list<int> *frees, lfree;

  for(int i = 0; i < NB_REG; i++){
    if(i != 0 && i != 26 && i != 27 && i != 28 && i != 29 && i != 30 && i != 31){
      if(!LiveIn[i] && !Def[i]) lfree.push_back(i);
    }
  }
  reg_rename(&lfree);


  /* A REMPLIR */
}

void Basic_block::apply_scheduling(list <Node_dfg*> *new_order){
   list <Node_dfg*>::iterator it=new_order->begin();
   Instruction *inst=(*it)->get_instruction();
   Line *n=_head, *prevn=NULL;
   Line *end_next = _end->get_next();
   if(!n){
      cout<<"wrong bb : cannot apply"<<endl;
      return;
   }

   while(!n->isInst()){
     prevn=n;
     n=n->get_next();
     if(n==_end){
       cout<<"wrong bb : cannot apply"<<endl;
       return;
     }
   }

   //y'a des instructions, on sait pas si c'est le bon BB, mais on va supposer que oui
   inst->set_index(0);
   inst->set_prev(NULL);
   _firstInst = inst;
   n = inst;

   if(prevn){
     prevn->set_next(n);
     n->set_prev(prevn);
   }
   else{
     set_head(n);
   }

   int i;
   it++;
   for(i=1; it!=new_order->end(); it++, i++){

     inst->set_link_succ_pred((*it)->get_instruction());

     inst=(*it)->get_instruction();
     inst->set_index(i);
     prevn = n;
     n = inst;
     prevn->set_next(n);
     n->set_prev(prevn);
     }
   inst->set_next(NULL);
   _lastInst = inst;
   set_end(n);
   n->set_next(end_next);
   return;
}

/* permet de tester des choses sur un bloc de base, par exemple la construction d'un DFG, � venir ... l� ne fait rien qu'afficher le BB */
void Basic_block::test(){
   cout << "test du BB " << get_index() << endl;
   display();
   cout << "nb de successeur : " << get_nb_succ() << endl;
   int nbsucc = get_nb_succ() ;
   if (nbsucc >= 1 && get_successor1())
      cout << "succ1 : " << get_successor1()-> get_index();
   if (nbsucc >= 2 && get_successor2())
      cout << " succ2 : " << get_successor2()-> get_index();
   cout << endl << "nb de predecesseurs : " << get_nb_pred() << endl ;

   int size=(int)_pred.size();
   for (int i = 0; i < size; i++){
      if (get_predecessor(i) != NULL)
	 cout << "pred "<< i <<  " : " << get_predecessor(i)-> get_index() << "; ";
   }
   cout << endl;
}
