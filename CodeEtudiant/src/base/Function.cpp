#include <Function.h>

Function::Function(){
  _head = NULL;
  _end = NULL;
  BB_computed = false;
  BB_pred_succ = false;
  dom_computed = false;
}

Function::~Function(){}

void Function::set_head(Line *head){
  _head = head;
}

void Function::set_end(Line *end){
  _end = end;
}

Line* Function::get_head(){
  return _head;
}

Basic_block* Function::get_firstBB(){
   return _myBB.front();
}

Line* Function::get_end(){
  return _end;
}
void Function::display(){
  cout<<"Begin Function"<<endl;
  Line* element = _head;

  if(element == _end)
    cout << _head->get_content() <<endl;

  while(element != _end){
    cout << element->get_content() <<endl;

    if(element->get_next()==_end){
      cout << element->get_next()->get_content() <<endl;
      break;
    }
    else element = element->get_next();

    }
  cout<<"End Function\n\n"<<endl;

}

int Function::size(){
  Line* element = _head;
  int lenght=0;
  while(element != _end)
    {
      lenght++;
      if (element->get_next()==_end)
	break;
      else
	element = element->get_next();
    }
  return lenght;
}


void Function::restitution(string const filename){

  Line* element = _head;
  ofstream monflux(filename.c_str(), ios::app);

  if(monflux){
    monflux<<"Begin"<<endl;
    if(element == _end)
      monflux << _head->get_content() <<endl;
    while(element != _end)
      {
	if(element->isInst() ||
	   element->isDirective())
	  monflux<<"\t";

	monflux << element->get_content() ;

	if(element->get_content().compare("nop"))
	  monflux<<endl;

	if(element->get_next()==_end){
	  if(element->get_next()->isInst() ||
	     element->get_next()->isDirective())
	    monflux<<"\t";
	  monflux << element->get_next()->get_content()<<endl;
	  break;
	}
	else element = element->get_next();

      }
    monflux<<"End\n\n"<<endl;

  }

  else {
    cout<<"Error cannot open the file"<<endl;
  }

  monflux.close();
}

void Function::comput_label(){
  Line* element = _head;

  if(element == _end && element->isLabel())
    _list_lab.push_back(getLabel(element));
  while(element != _end)
    {

      if(element->isLabel())
	_list_lab.push_back(getLabel(element));

      if(element->get_next()==_end){
	if(element->isLabel())
	  _list_lab.push_back(getLabel(element));
	break;
      }
      else element = element->get_next();

    }

}

int Function::nbr_label(){
  return _list_lab.size();

}

Label* Function::get_label(int index){

  list<Label*>::iterator it;
  it=_list_lab.begin();

  int size=(int) _list_lab.size();
  if(index< size){
    for (int i=0; i<index;i++ ) it++;
    return *it;
  }
  else cout<<"Error get_label : index is bigger than the size of the list"<<endl;

  return _list_lab.back();
}

Basic_block *Function::find_label_BB(OPLabel* label){
  //Basic_block *BB = new Basic_block();
   int size=(int)_myBB.size();
   string str;
   for(int i=0; i<size; i++){
      if(get_BB(i)->is_labeled()){

	 str=get_BB(i)->get_head()->get_content();
	 if(!str.compare(0, (str.size()-1),label->get_op())){
	    return get_BB(i);
	 }
      }
  }
  return NULL;
}


/* ajoute nouveau BB � la liste de BB de la fonction en le creant */
/* debut est l'entente, fin la derniere ligne du BB, br vaut NULL si le BB ne finit pas par un saut sinon contient la ligne du saut, index est le num�ro du BB */

void Function::add_BB(Line *debut, Line* fin, Line *br, int index){
   Basic_block *b=new Basic_block();
   b->set_head(debut);
   b->set_end(fin);
   b->set_index(index);
   b->set_branch(br);
   _myBB.push_back(b);
}


//Calcule la liste des blocs de base : il faut d�limiter les BB, en parcourant la liste des lignes (qui contiennent des directives, des labels ou des instructions) � partir de la premiere de la fonction, il faut s'arreter � chaque branchement (et prendre en compte le delayed slot qui appartient au meme BB, c'est l'instruction qui suit tout branchement) ou � chaque label (on estime que tout label est utilis� par un saut et donc correspond bien � une ent�te de BB).

// Pour cr�er un bloc il est conseiller d'utiliser la fonction addBB ci-dessus qui cr�e un BB et l'ajoute � la liste des BB de la fonction
void Function::comput_basic_block(){
  Line *debut, *current, *prev; // debut contient NULL ou une ent�te, current la ligne en cours de traitement, prev la ligne pr�c�dent current
   bool verbose = true;

   int bb_num = 0; // numerotation des BB dans l'ordre
   Line *l = NULL;
   Instruction *i = NULL;
   int k=0;

   cout << "comput BB" <<endl;
   if (verbose){
   cout<<" head:"<< _head->get_content()<<endl;
   cout<<" tail:"<< _end->get_content()<<endl;
   }
   if (BB_computed) return; // NE PAS TOUCHER, �vite de recalculer si d�j� fait

   current = _head->get_next()->get_next();
   while(current->isDirective()){
     current = current->get_next();
   }

   cout<< current->get_content()<<endl;

   debut = _head->get_next();

   while(current != _end->get_next()){ // il faut traiter la derniere ligne donc il faut s'arr�ter � la suivante!

    if(current->isInst()){
      if(debut==NULL){
        debut = current;
      }
      i = dynamic_cast <Instruction *>(current);
      if(i->is_branch()){
        current = current->get_next();
        add_BB(debut, current, current->get_prev(), k++);
        debut = NULL;
      }
    }
    if(current->isLabel() ){
      if(debut != NULL){
        add_BB(debut, current->get_prev(), NULL, k++);
      }
      debut = current;
    }

    current = current->get_next();

   }

   if (verbose)
     cout<<"end comput Basic Block"<<endl;

   BB_computed = true;
   return;
}

int Function::nbr_BB(){
   return _myBB.size();
}

Basic_block *Function::get_BB(int index){

  list<Basic_block*>::iterator it;
  it=_myBB.begin();
  int size=(int)_myBB.size();

  if(index< size){
    for (int i=0; i<index;i++ ) it++;
    return *it;
  }
  else
    return NULL;
}

list<Basic_block*>::iterator Function::bb_list_begin(){
   return _myBB.begin();
}

list<Basic_block*>::iterator Function::bb_list_end(){
   return _myBB.end();
}

/* comput_pred_succ calcule les successeurs et pr�d�cesseur des BB, pour cela il faut commencer par les successeurs */
/* et it�rer sur tous les BB d'une fonction */
/* il faut determiner si un BB a un ou deux successeurs : d�pend de la pr�sence d'un saut pr�sent ou non � la fin */
/* pas de saut ou saut incontionnel ou appel de fonction : 1 successeur (lequel ?)*/
/* branchement conditionnel : 2 successeurs */
/* le dernier bloc n'a pas de successeurs - celui qui se termine par jr R31 */
/* les sauts indirects n'ont pas de successeur */

void Function::comput_succ_pred_BB(){

  int nbBB = _myBB.size(); // nombre de BB dans la function

  // on peut r�cup�rer un BB de la fonction avec la m�thode getBB(num du BB) ou tous les BB un a un car les BB ont des num�ros entre 0 et nbBB-1.
  Basic_block *current;
  Instruction *instr;
  Basic_block *succ=NULL;
  if (BB_pred_succ) return; // on ne le fait qu'une fois

  comput_label();

  int i = 0;
  for(i = 0; i < nbBB; i++){
  	current = get_BB(i);
    instr = dynamic_cast <Instruction *>(current->get_branch());
    if(instr){
      if(instr->is_cond_branch()){
        current->set_link_succ_pred(find_label_BB(instr->get_op_label()));
        current->set_link_succ_pred(get_BB(i+1));
      }else if(instr->is_call()){
        current->set_link_succ_pred(get_BB(i+1));
      }else if(!instr->is_indirect_branch()){
      	current->set_link_succ_pred(find_label_BB(instr->get_op_label()));
      }
    }else{
      //pas de saut
      current->set_link_succ_pred(get_BB(i+1));
    }





  }

  // ne pas toucher ci-dessous
  BB_pred_succ = true;
  return;
}

void Function::compute_dom(){
  int nbBB = _myBB.size(); // nombre de BB dans la function

  // on peut r�cup�rer les BB de la fonction avec la m�thode get_BB(num du BB) pour tous les num�ros de BB entre 0 et nbBB-1.

  list<Basic_block*> workinglist; // liste de travail
  Basic_block *current, *bb, *pred;
  bool change = false;

  if (dom_computed) return; // on ne le fait qu'une fois
  comput_succ_pred_BB();   // on a besoin d'avoir calcul� les blocs pr�d�cesseurs et successeurs avant de calculer les dominants

  //on met tous les blocs sans predecesseur dans la liste
  for(int i=0 ; i<nbBB; i++){
    current = get_BB(i);
    if(current->get_nb_pred()==0){
      workinglist.push_front(current);
    }
  }


    while(workinglist.size()!=0){

      current = workinglist.front();
      workinglist.remove(current);

      //le block se domine lui-meme
      if(  !current->Domin[current->get_index()]){
        current->Domin[current->get_index()]=true;
        change = true;
      }

      //si le block a un seul predecesseur, il est domine par ce predecesseur et par tous ceux qui dominent ce predecesseur
      if(current->get_nb_pred()==1){
        if(!current->Domin[current->get_predecessor(0)->get_index()]){
          current->Domin[current->get_predecessor(0)->get_index()]=true;
          for(int j=0; j<nbBB; j++){
            if(current->get_predecessor(0)->Domin[j])
              current->Domin[j] = true ;
          }
          change = true;
        }
      }
      else{
          //si le bloc a plusieurs predecesseurs
          Basic_block* p1, *p2;
          bool noDomination;
          bool domCommuns[nbBB];
          int cpt=0;

          //on veut recuperer les blocs dominateurs en commun de tous les predecesseurs du bloc, car ils dominent le bloc
          for(int i=0; i<nbBB; i++){
            domCommuns[i]=true;
          }

          //on regarde si un ou plusieurs des predecesseurs du bloc domine tous les autres predecesseurs du blocs, alors il domine le bloc
          for(int i=0; i<current->get_nb_pred()-1; i++){

            p1 = current->get_predecessor(i);

            for(int k=0; k<nbBB; k++){
              if(!p1->Domin[k])
                domCommuns[k]=false;
            }

            noDomination = false;
            for(int j=1; j<current->get_nb_pred(); j++){

                p2 = current->get_predecessor(j);

                if(!p2->Domin[p1->get_index()]){
                  noDomination=true;
                  cpt++;
                  break;
                }

            }
            if( !noDomination){
              if(!current->Domin[p1->get_index()]){
                
                for(int j=0; j<nbBB; j++){
                  current->Domin[j] = p1->Domin[j];
                }
                change = true;
              }

            }
          }

          if(cpt==current->get_nb_pred()-1 && current->get_nb_pred()!=0){
            for(int k=0; k<nbBB; k++){
              if(domCommuns[k]){
                current->Domin[k]=true;
              }
            }
          }

      }

      //s'il y a eu au moins un changement, on doit re-evaluer les successeurs de ce bloc
      if(change){
        if(current->get_successor1()!=NULL)
          workinglist.push_front(current->get_successor1());
        if(current->get_successor2()!=NULL)
          workinglist.push_front(current->get_successor2());

        change=false;
      }

  }





  // affichage du resultat
  list<Basic_block*>::iterator it; // iterateur
  it=_myBB.begin();

  for (int j=0; j< nbBB; j++){
    current = *it;
    cout << "Dominants pour BB" << current -> get_index() << " : ";
    for (int i = 0; i< nbr_BB(); i++){
      if (current->Domin[i]) cout << " BB" << i  ;
    }
    cout << endl;
    it++;
  }
  dom_computed = true;
  return;
}



void Function::compute_live_var(){

  list<Basic_block*> workinglist;
  Basic_block *current, *bb, *pred;
  bool change = false;
  int nbBB= (int) _myBB.size();
  vector<bool> oldtmpLIN(NB_REG, false);
  vector<bool> oldtmpLOUT(NB_REG, false);
  int nb_succ = 0;

  Basic_block *succ1, *succ2;
  /* A REMPLIR avec algo vu en cours et en TD*/
 /* algorithme it�ratif qui part des blocs sans successeur, ne pas oublier que lorsque l'on sort d'une fonction le registre $2 contient le r�sultat (il est donc vivant), le registre pointeur de pile ($29) est aussi vivant ! */

//recherche de blocs sans successeur
for(int i = 0; i < nbBB; i++){
  current = get_BB(i);
  if(current->get_nb_succ() == 0){
    workinglist.push_back(current);
  }
}


// tant que la liste de travail n'est pas vide
while(!workinglist.empty()){
  current = workinglist.front();
  workinglist.pop_front();
  //cout << "traitement bloc "<< current->get_index()<< endl;


  //on copie les anciennes valeurs pour verifier s'il y a eu un changement
  for(int i = 0; i < NB_REG; i++){
    oldtmpLIN[i] = current->LiveIn[i];
    oldtmpLOUT[i] = current->LiveOut[i];
  }

  nb_succ = current->get_nb_succ();

  //on clean LiveOut
  for(int i = 0; i < NB_REG; i++) current->LiveOut[i] = false;


  //suivant le nombre de successeur
  switch(nb_succ){
    case 0:
      //si aucun alors c'est une fin de fonction
      //donc LiveOut[31] = LiveOut[2] = true
      current->LiveOut[2] = true;
      current->LiveOut[29] = true;
      break;
    case 1:
      succ1 = current->get_successor1();
      for(int i = 0; i < NB_REG; i++){
        current->LiveOut[i] = succ1->LiveIn[i];
      }
      break;
    case 2:
      succ1 = current->get_successor1();
      for(int i = 0; i < NB_REG; i++){
        current->LiveOut[i] = succ1->LiveIn[i];
      }

      succ2 = current->get_successor2();
      for(int i = 0; i < NB_REG; i++){
        if(succ2->LiveIn[i]){
          current->LiveOut[i] = true;
        }
      }
      break;
  }


  //si il y a eu une difference alors il y a eu un changement
  for(int i = 0; i < NB_REG; i++){
    if(oldtmpLOUT[i] != current->LiveOut[i]){
      change = true;
      break;
    }
  }


  //on clean LiveIn
  for(int i = 0; i < NB_REG; i++) current->LiveIn[i] = false;

  //LiveIn = Use U (LiveOut \ DEF)
  for(int i = 0; i < NB_REG; i++){
    current->LiveIn[i] = current->Use[i];
    if(current->LiveOut[i] && !current->Def[i]){
      current->LiveIn[i] = true;
    }
  }

  //on regarde s'il y a changement
  if(!change){
    for(int i = 0; i < NB_REG; i++){
      if(oldtmpLIN[i] != current->LiveIn[i]){
        change = true;
        break;
      }
    }
  }

  //s'il y a eu changement alors on ajoute les predecesseurs dans la liste de
  //travail
  if(change){
    int nb_pred = current->get_nb_pred();
    for(int i = 0; i < nb_pred; i++){
      workinglist.push_back(current->get_predecessor(i));
    }
  }
  change = false;

  /*
  cout << "LIN: ";
  for(int i = 0; i < NB_REG; i++){
    if(current->LiveIn[i]) cout << "$" << i << " ";
  }
  cout << endl << "LOUT: ";
  for(int i = 0; i < NB_REG; i++){
    if(current->LiveOut[i]) cout << "$" << i << " ";
  }
  cout << endl;*/
}



  // Affichage du resultat
  list<Basic_block*>::iterator it;
  it=_myBB.begin();
  for (int j = 0 ; j < nbBB ; j++){
    bb = *it;
    cout << "********* bb " << bb->get_index() << "***********" << endl;
    cout << "LIVE_OUT : " ;
    for(int i = 0; i < NB_REG; i++){
      if (bb->LiveOut[i]){
	cout << "$"<< i << " ";
      }
    }
    cout << endl;
    cout << "LIVE_IN :  " ;
    for(int i = 0 ; i < NB_REG ; i++){
      if (bb->LiveIn[i]){
	cout << "$"<< i << " ";
      }}
    cout << endl;
    it++;
  }
  return;
 }



/* en implementant la fonction test de la classe BB, permet de tester des choses sur tous les blocs de base d'une fonction par exemple l'affichage de tous les BB d'une fonction ou l'affichage des succ/pred des BBs comme c'est le cas -- voir la classe Basic_block et la m�thode test */

void Function::test(){
  int size=(int)_myBB.size();
   for(int i=0;i<size; i++){
    get_BB(i)->test();
  }
   return;
}
