// symboltable.c

#include <stdio.h>
#include "jalLexer.h"
#include "jalParser.h"

#include "symboltable.h"

Symbol *SymbolTail= NULL;  // points to most recent symbol
Symbol *SymbolHead = NULL; // points to oldest symbol

// static prototypes
static void append_node(Symbol *lnode);
static void remove_node(Symbol *lnode);
static void insert_node(Symbol *lnode, Symbol *after); 

static Symbol *AddSymbol(void);                           


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Symbol *NewSymbolFunction(void)
{  Symbol *s;
   SymbolFunction *f;
   
   s = AddSymbol();
   s->Name = NULL; //CreateName(Name);
   s->Type = S_FUNCTION;
            
   f = malloc(sizeof(SymbolFunction));
   s->details = f;
   
   f->ReturnType = L_VOID;   
//   f->NrOfParams = 0;        
   f->Param      = NULL;

   return s;
}


// add paramter record to function-def
SymbolParam *SymbolFunctionAddParam(SymbolFunction *f, int TokenType)
{  SymbolParam *p;
   
   // create record   
   p = malloc(sizeof(SymbolParam));

   // add to list
   if(f->Param == NULL) {
      // first in list
      f->Param = p;
   } else {
      // assing to end of list
      SymbolParam *x = f->Param;
      while (x->next != NULL) x = x->next;
      x->next = p;
   }              

   // init    
   p->Type = TokenType; 
   p->Name = NULL;
   p->CallMethod = 'v'; // default call by value

   return p;
}

//-----------------------------------------------------------------------------
// AddSymbol - malloc memory for Symbol stuct & add to chain
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static Symbol *AddSymbol()
{  Symbol *s;

   #ifdef DEBUG 
   printf("//AddSymbol\n");
   #endif          
   s = malloc(sizeof(Symbol));
   if (s == NULL) {
      printf("Out of memory error\n");
      exit(1);
   }

   append_node(s);

	s->Name = NULL;
	s->Type = 0;    // function, procedure, variable, constant

   return s;
}

//-----------------------------------------------------------------------------
// CreateName - malloc memory, copy name and return pointer
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
char *CreateName(char *Name)
{  char *n;
   
   n = malloc(strlen(Name) + 1);
   strcpy(n, Name);
   
   return n;
}           

//-----------------------------------------------------------------------------
// SymbolParamSetName -
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SymbolParamSetName(SymbolParam *p, char *Name)
{
   p->Name = CreateName(Name);   
}
  

  
Symbol *GetSymbolPointer  (char *SymbolName)
{  Symbol *s;
   
   for(s = SymbolHead; s != NULL; s = s->Next) {
      if (strcmp(SymbolName, s->Name) != 0) continue;

      // match
      return s;
   }       
   
   return NULL;
}
  
void DumpSymbol(Symbol *s)
{  int i;
      
   printf("//Symbol name: '%s', Type: %d\n", s->Name, s->Type);
   switch (s->Type) {
      case S_FUNCTION : {
         SymbolFunction *f = s->details;
         printf("//   Function returns %s (%d)\n", VarTypeString(f->ReturnType), f->ReturnType);         
         SymbolParam *p = f->Param;
         for (;;) {
            if (p != NULL) break;
            printf("//   param Name: '%s', Type: %s (%d), CallBy: %c\n",
                  p->Name, jalParserTokenNames[p->Type], p->Type, p->CallMethod);
            p = p->next;    
         }
         break;
      }        
      case S_PVAR : {
         break;
      }
      default : {
         printf("//    Unknown type\n");
         break;
      }
   }                 
}                        



void DumpSymbolTable()
{  Symbol *s;
      
   for(s = SymbolHead; s != NULL; s = s->Next) {
      if (s== NULL) break;
      printf("DumpSymbolTable %x\n", s);
      DumpSymbol(s);
   }
}

// /* destroy the dll list */
// while(head != NULL)
//  remove_node(head);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void append_node(Symbol *lnode) {
   if(SymbolHead == NULL) {
      SymbolHead = lnode;
      lnode->Prev = NULL;
   } else {
      SymbolTail->Next = lnode;
      lnode->Prev = SymbolTail;
   }

   SymbolTail = lnode;
   lnode->Next = NULL;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void insert_node(Symbol *lnode, Symbol *after) 
{
   lnode->Next = after->Next;
   lnode->Prev = after;

   if(after->Next != NULL) {
      after->Next->Prev = lnode;
   } else {
     SymbolTail = lnode;
   }
   after->Next = lnode;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void remove_node(Symbol *lnode) 
{
   if(lnode->Prev == NULL) {
      SymbolHead = lnode->Next;
   } else {
      lnode->Prev->Next = lnode->Next;
   }
   if(lnode->Next == NULL) {
      SymbolTail = lnode->Prev;
   } else {
      lnode->Next->Prev = lnode->Prev;
   }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SymbolPrintPvarTable()
{  Symbol *s;
   Pvar   *v;

   printf("\n\n   // Pseudo Var table\n");   

   for(s = SymbolHead; s != NULL; s = s->Next) {
      if (s->Type != S_PVAR) continue;
      v = s->details;   

      if ((v->put != 0) | (v->get != 0)) {
         printf("   const ByCall __%s = { (void *)&%s, (void *)&%s, &%s, %d, %d, %d};\n",
            s->Name, v->put, v->get, v->data, v->size, v->p1, v->p2); 
      }
   }              
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static Pvar *AddPvar(char *Name)
{  Symbol *s;
   Pvar *p;
//   static int ID = 0;
   
   #ifdef DEBUG
   printf("// AddPvar Name: %s\n", Name);
   #endif
   
   s = AddSymbol();
   s->Name = CreateName(Name);
   s->Type = S_PVAR;

   p = malloc(sizeof(Pvar));
   s->details = p;
   
//   p->ID    = ID++;
   
   p->put      = NULL;   
   p->get      = NULL;   
   p->data     = NULL;   
   p->size     = 1;
   p->p1       = 0;
   p->p2       = 0;

   return p;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Pvar *SymbolGetPvar(char *SymbolName)
{  Symbol *s;
   Pvar *p;

   for(s = SymbolHead; s != NULL; s = s->Next) {
      if (strcmp(SymbolName, s->Name) != 0) continue;

      // name match                            
      if (s->Type == S_PVAR) {
         p = s->details;
         return p;
      } else {
         printf("// GetPvar: name found with non-pvar type\n");
      }
   }          
   return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Pvar *SymbolGetOrAddPvar(char *Name)
{  Pvar *p;

   #ifdef DEBUG
   printf("// SymbolGetOrAddPvar Name: %s\n", Name);
   #endif
   
   p = SymbolGetPvar(Name);
   
   if (p == NULL) {
      p = AddPvar(Name);
//      printf("SymbolGetOrAddPvar new %d\n", p);
   } else {
//      printf("SymbolGetOrAddPvar found %d\n", p);
   }
      
   return p;   
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SymbolPvarAdd_PutName(char *BaseName, char *PutName)
{  Pvar *p;

   p = SymbolGetOrAddPvar(BaseName);
   p->put = CreateName(PutName);
   
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SymbolPvarAdd_GetName(char *BaseName, char *GetName)
{  Pvar *p;

   p = SymbolGetOrAddPvar(BaseName);
   p->get = CreateName(GetName);
   
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SymbolPvarAdd_DataName(char *BaseName, char *DataName)
{  Pvar *p;
//   printf("SymbolPvarAdd_DataName Base: %s, Data: %s\n", BaseName, DataName);

   p = SymbolGetOrAddPvar(BaseName);

   p->data =CreateName(DataName);
   
}
