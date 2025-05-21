//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <map>
#include <FlexLexer.h>
#include "tokeniser.h"
#include <cstring>

using namespace std;

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND ,WTFM};
enum TYPES {INTEGER, BOOLEAN, DOUBLE, CHAR};

TOKEN current;				// Current token


FlexLexer* lexer = new yyFlexLexer; // This is the flex tokeniser
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

	
map<string, enum TYPES> DeclaredVariables;	
unsigned long TagNumber=0;

bool IsDeclared(const char *id){
	return DeclaredVariables.find(id)!=DeclaredVariables.end();
}


void Error(string s){
	cerr << "Ligne n°"<<lexer->lineno()<<", lu : '"<<lexer->YYText()<<"'("<<current<<"), mais ";
	cerr<< s << endl;
	exit(-1);
}

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"
	

TYPES Identifier(void){
	cout << "\tpush "<<lexer->YYText()<<endl;
	TYPES type = DeclaredVariables[lexer->YYText()];
	current=(TOKEN) lexer->yylex();
	return type;
}

TYPES Number(void){
	
	cout <<"\tpush $"<<atoi(lexer->YYText())<<endl;
	current=(TOKEN) lexer->yylex();

	return INTEGER;
}

TYPES Flottant(void){
	double d;					// 64-bit float
	unsigned int *i;			// pointer to a 32 bit unsigned int 

	// took that code from teacher
	d=atof(lexer->YYText());
	i=(unsigned int *) &d; // i points to the const double
	//cout <<"\tpush $"<<*i<<"\t# Conversion of "<<d<<endl;
	// Is equivalent to : 
	cout <<"\tsubq $8,%rsp\t\t\t# allocate 8 bytes on stack's top"<<endl;
	cout <<"\tmovl	$"<<*i<<", (%rsp)\t# Conversion of "<<d<<" (32 bit high part)"<<endl;
	cout <<"\tmovl	$"<<*(i+1)<<", 4(%rsp)\t# Conversion of "<<d<<" (32 bit low part)"<<endl;
	current=(TOKEN) lexer->yylex();
	return DOUBLE;
}


enum TYPES CharConst(void){
	cout<<"\tmovq $0, %rax"<<endl;
	cout<<"\tmovb $"<<lexer->YYText()<<",%al"<<endl;
	cout<<"\tpush %rax\t# push a 64-bit version of "<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();
	return CHAR;
}


TYPES Expression(void);			// Called by Term() and calls Term()

TYPES Factor(void){
	if(current==RPARENT){
		current=(TOKEN) lexer->yylex();
		TYPES type = Expression();
		if(current!=LPARENT)
			Error("')' était attendu");		// ")" expected
		else
			current=(TOKEN) lexer->yylex();
		return type;
	}
	else 
		if (current==NUMBER)
			return Number();
		else if (current==FLOTTANT)
			return Flottant();
		else if(current==ID)
			return Identifier();
		else if(current==CHARCONST)	
			return CharConst();
			else
				Error("'(' ou chiffre ou lettre ou charconst attendue");

}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator(void){
	OPMUL opmul;
	if(strcmp(lexer->YYText(),"*")==0)
		opmul=MUL;
	else if(strcmp(lexer->YYText(),"/")==0)
		opmul=DIV;
	else if(strcmp(lexer->YYText(),"%")==0)
		opmul=MOD;
	else if(strcmp(lexer->YYText(),"&&")==0)
		opmul=AND;
	else opmul=WTFM;
	current=(TOKEN) lexer->yylex();
	return opmul;
}

// Term := Factor {MultiplicativeOperator Factor}
TYPES Term(void){
	OPMUL mulop;

	TYPES type = Factor();

	while(current==MULOP){
		mulop=MultiplicativeOperator();		// Save operator in local variable
		TYPES type2 = Factor();
		if (type != type2){
			Error("The two factor doesn't have the same type !!");
		}
		if (type == INTEGER){
			cout << "\tpop %rbx"<<endl;	// get first operand
			cout << "\tpop %rax"<<endl;	// get second operand
			switch(mulop){
				case AND:
					cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
					cout << "\tpush %rax\t# AND"<<endl;	// store result
					break;
				case MUL:
					cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
					cout << "\tpush %rax\t# MUL"<<endl;	// store result
					break;
				case DIV:
					cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
					cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
					cout << "\tpush %rax\t# DIV"<<endl;		// store result
					break;
				case MOD:
					cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
					cout << "\tdiv %rbx"<<endl;			// remainder goes to %rdx
					cout << "\tpush %rdx\t# MOD"<<endl;		// store result
					break;
				default:
					Error("opérateur multiplicatif attendu");
			}
		}
		else if (type == DOUBLE){
			cout << "\tfldl (%rsp) #empile first into float pile " << endl;
			cout << "\taddq $8, %rsp #depilement int pile " << endl;
			cout << "\tfldl (%rsp) #empile second into float pile " << endl;
			cout << "\taddq $8, %rsp #depilement int pile " << endl;

			switch(mulop){
				case AND:
					cout << "\tfmulp %st(0),%st(1) # st0 && st1 , stockage dans st1 et depilement de st1 pcq p a la fin" << endl;
					break;
				case MUL:
					cout << "\tfmulp %st(0),%st(1) # st0 * st1 , stockage dans st1 et depilement de st1 pcq p a la fin" << endl;
					break;
				case DIV:
					cout << "\tfdivp %st(0),%st(1) # st0 / st1 , stockage dans st1 et depilement de st1 pcq p a la fin" << endl; // quotient goes to %rax
					break;

				case MOD:
					cout << "\tfdivp %st(0),%st(1) # st0 % st1 , stockage dans st1 et depilement de st1 pcq p a la fin" << endl; // rest is in rdx
					break;

				default:
					Error("opérateur multiplicatif attendu");
			}
			cout << "\tsubq $8, %rsp    # Empilement d'une valeur non initialisée" << endl;
			cout << "\tfstpl (%rsp)     # copie %st(0) au sommet de la pile générale et dépile la pile de registres %st(X)" << endl;
		}
	}
	return type;
}

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator(void){
	OPADD opadd;
	if(strcmp(lexer->YYText(),"+")==0)
		opadd=ADD;
	else if(strcmp(lexer->YYText(),"-")==0)
		opadd=SUB;
	else if(strcmp(lexer->YYText(),"||")==0)
		opadd=OR;
	else opadd=WTFA;
	current=(TOKEN) lexer->yylex();
	return opadd;
}

// SimpleExpression := Term {AdditiveOperator Term}
TYPES SimpleExpression(void){
	OPADD adop;
	
	TYPES type = Term();
	while(current==ADDOP){
		adop=AdditiveOperator();		// Save operator in local variable
		TYPES type2 = Term();

		if (type != type2){
			Error("Type différenet des terms de l'expression");
		}
		if (type == INTEGER){
			cout << "\tpop %rbx"<<endl;	// get first operand
			cout << "\tpop %rax"<<endl;	// get second operand
			switch(adop){
				case OR:
					cout << "\taddq	%rbx, %rax\t# OR"<<endl;// operand1 OR operand2
					break;			
				case ADD:
					cout << "\taddq	%rbx, %rax\t# ADD"<<endl;	// add both operands
					break;			
				case SUB:	
					cout << "\tsubq	%rbx, %rax\t# SUB"<<endl;	// substract both operands
					break;
				default:
					Error("opérateur additif inconnu");
			}
			cout << "\tpush %rax"<<endl;			// store result
		}

		if (type == DOUBLE){
			cout << "\tfldl (%rsp) #empile first into float pile " << endl;
			cout << "\taddq $8, %rsp #depilement int pile " << endl;
			cout << "\tfldl (%rsp) #empile second into float pile " << endl;
			cout << "\taddq $8, %rsp #depilement int pile " << endl;
			switch (adop){

				case OR:
					cout<<"\tfaddp	%st(0),%st(1) # st0 + st1 , stockage dans st1 et depilement de st1 pcq p a la fin" << endl;
					break;

				case ADD :
					cout<<"\tfaddp	%st(0),%st(1) # st0 + st1 , stockage dans st1 et depilement de st1 pcq p a la fin" << endl;
					break;
				
				case SUB:
					cout<<"\tfsubp	%st(0),%st(1) # st0 + st1 , stockage dans st1 et depilement de st1 pcq p a la fin" << endl;
					break;
				default:
					Error("opérateur additif inconnu");

			}
			cout << "\tsubq $8, %rsp    # Empilement d'une valeur non initialisée" << endl;
			cout << "\tfstpl (%rsp)     # copie %st(0) au sommet de la pile générale et dépile la pile de registres %st(X)" << endl;
		}
	}
	return type;

}

/*
// DeclarationPart := "[" Ident {"," Ident} "]"
void DeclarationPart(void){
	if(current!=RBRACKET)
		Error("caractère '[' attendu");
	
	cout << "\t.align 8"<<endl;
	
	current=(TOKEN) lexer->yylex();
	if(current!=ID)
		Error("Un identificater était attendu");
	cout << lexer->YYText() << ":\t.quad 0"<<endl;
	DeclaredVariables[lexer->YYText()]= INTEGER; // OR BOOLEAN
	current=(TOKEN) lexer->yylex();
	while(current==COMMA){
		current=(TOKEN) lexer->yylex();
		if(current!=ID)
			Error("Un identificateur était attendu");
		cout << lexer->YYText() << ":\t.quad 0"<<endl;
		DeclaredVariables.insert({lexer->YYText(), INTEGER}); // OR BOOLEAN
		current=(TOKEN) lexer->yylex();
	}
	if(current!=LBRACKET)
		Error("caractère ']' attendu");
	current=(TOKEN) lexer->yylex();
}
*/

//  VarDeclaration := Ident {"," Ident} ":" Type
void VarDeclaration(void){

	set<string> varset;
	current=(TOKEN) lexer->yylex();
	if(current!=ID)
		Error("Un identificater était attendu");
	string variable = lexer->YYText();
	varset.insert(variable);
	current=(TOKEN) lexer->yylex();
	while (current==COMMA){
		current=(TOKEN) lexer->yylex();
		if(current!=ID){
			Error("Un identificateur était attendu");
		}
		variable = lexer->YYText();
		varset.insert(variable);
		current=(TOKEN) lexer->yylex();
	}

	

	if (current != COLON){
		Error("Lors d'une déclaration, un : est attendu après l'identificateur");
	}
	current=(TOKEN) lexer->yylex();

	if (strcmp(lexer->YYText(), "BOOLEAN")==0){
		for (string var : varset){
			cout << var << ":\t.quad 0"<<endl; // .boolean 0
			DeclaredVariables[var] = BOOLEAN;
		}
		
	}
	
	else if (strcmp(lexer->YYText(), "INTEGER")==0){
		for (string var : varset){
			cout << var << ":\t.quad 0"<<endl;
			DeclaredVariables[var] = INTEGER;
		}
	}

	else if (strcmp(lexer->YYText(), "DOUBLE")==0){
		for (string var : varset){
			cout << var << ":\t.double 0.0"<<endl;
			DeclaredVariables[var] = DOUBLE;
		}
	}

	else if (strcmp(lexer->YYText(), "CHAR")==0){
		for (string var : varset){
			cout << var << ":\t.byte 0"<<endl;
			DeclaredVariables[var] = CHAR;
		}
	}
}




// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
void VarDeclarationPart(void){
	
	VarDeclaration();


	current=(TOKEN) lexer->yylex();
	while (current == SEMICOLON){
		VarDeclaration();
		current=(TOKEN) lexer->yylex();
	}
	
	
	if (current != DOT){
		Error("'.' attendu à la fin d'un déclaration de variable ");
	}


}




// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator(void){
	OPREL oprel;
	if(strcmp(lexer->YYText(),"==")==0)
		oprel=EQU;
	else if(strcmp(lexer->YYText(),"!=")==0)
		oprel=DIFF;
	else if(strcmp(lexer->YYText(),"<")==0)
		oprel=INF;
	else if(strcmp(lexer->YYText(),">")==0)
		oprel=SUP;
	else if(strcmp(lexer->YYText(),"<=")==0)
		oprel=INFE;
	else if(strcmp(lexer->YYText(),">=")==0)
		oprel=SUPE;
	else oprel=WTFR;
	current=(TOKEN) lexer->yylex();
	return oprel;
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
TYPES Expression(void){
	OPREL oprel;
	unsigned long long tag;
	TYPES type = SimpleExpression();
	if(current==RELOP){
		tag=++TagNumber;
		oprel=RelationalOperator();
		TYPES type2 = SimpleExpression();
		if (type != type2){
			Error("PAs le même type entre les expressions");
		}
		if (type == DOUBLE){
			cout << "\tfldl (%rsp) #empile first into float pile " << endl;
			cout << "\taddq $8, %rsp #depilement int pile " << endl;
			cout << "\tfldl (%rsp) #empile second into float pile " << endl;
			cout << "\taddq $8, %rsp #depilement int pile " << endl;
			cout<<"\tfcomip %st(1)\t\t# comparato st0 st1	"<<endl;


			cout<<"\tfaddp %st(1)\t# pop nothing"<<endl;

		}
		else {
			cout << "\tpop %rax"<<endl;
			cout << "\tpop %rbx"<<endl;
			cout << "\tcmpq %rax, %rbx"<<endl;
		}
		
		switch(oprel){
			case EQU:
				cout << "\tje Vrai"<<++TagNumber<<"\t# If equal"<<endl;
				break;
			case DIFF:
				cout << "\tjne Vrai"<<++TagNumber<<"\t# If different"<<endl;
				break;
			case SUPE:
				cout << "\tjae Vrai"<<++TagNumber<<"\t# If above or equal"<<endl;
				break;
			case INFE:
				cout << "\tjbe Vrai"<<++TagNumber<<"\t# If below or equal"<<endl;
				break;
			case INF:
				cout << "\tjb Vrai"<<++TagNumber<<"\t# If below"<<endl;
				break;
			case SUP:
				cout << "\tja Vrai"<<++TagNumber<<"\t# If above"<<endl;
				break;
			default:
				Error("Opérateur de comparaison inconnu");
		}
		cout << "\tpush $0\t\t# False"<<endl;
		cout << "\tjmp Suite"<<TagNumber<<endl;
		cout << "Vrai"<<TagNumber<<":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;	
		cout << "Suite"<<TagNumber<<":"<<endl;

		return BOOLEAN;
	}
	return type ;
}

void Statement();

// AssignementStatement := Identifier ":=" Expression
TYPES AssignementStatement(void){
	string variable;
	
	variable = lexer->YYText();
	if(!IsDeclared(lexer->YYText())){
		cerr << "Erreur : Variable '"<<lexer->YYText()<<"' non déclarée"<<endl;
		exit(-1);
	}
	Identifier(); // ? ?

	
	TYPES type = DeclaredVariables[variable];

	if(current!=ASSIGN)
		Error("caractères ':=' attendus");
	current=(TOKEN) lexer->yylex();
	TYPES type2 = Expression();


	if (type != type2){
		Error("L'identifiant et l'expression non pas le même type");
	}
	// was missing that part when i looked at the correctin tp7
	if(type==CHAR){
		cout << "\tpop %rax"<<endl;
		cout << "\tmovb %al,"<<variable<<endl;
	}
	else {
	// ---
		cout << "\tpop "<<variable<<endl;
		cout << "\taddq $8, %rsp\t\t\t# pop nothing"<<endl;
		
	}
	return type;
	
}

// "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement(){
	/*
	// doesnt work like a if else
	// will do the if and if there is a else will do it 

	
	current=(TOKEN) lexer->yylex();
	TYPES type = Expression();
	if (type != BOOLEAN){
		Error("Expression type in IfStatement must be Boolean");
	}

	// current=(TOKEN) lexer->yylex();

	if (current!=KEYBOARD || strcmp(lexer->YYText(), "THEN") != 0)
		Error("THEN attendu après IF");
	
	current=(TOKEN) lexer->yylex();
	Statement();

	current=(TOKEN) lexer->yylex();
	
	if (current==KEYBOARD && strcmp(lexer->YYText(), "ELSE") == 0)
		current=(TOKEN) lexer->yylex();
		Statement();
	*/
	unsigned long long tag=TagNumber++;
	current=(TOKEN) lexer->yylex();

	if(Expression()!=BOOLEAN)
		Error("le type de l'expression doit être BOOLEAN");

	cout<<"\tpop %rax\t# Get the result of expression"<<endl;
	cout<<"\tcmpq $0, %rax"<<endl;
	cout<<"\tje Else"<<tag<<"\t# if FALSE, jump to Else"<<tag<<endl;
	if(current!=KEYBOARD||strcmp(lexer->YYText(),"THEN")!=0)
		Error("mot-clé 'THEN' attendu");
	current=(TOKEN) lexer->yylex();
	Statement();

	cout<<"\tjmp Next"<<tag<<"\t# Do not execute the else statement"<<endl;
	cout<<"Else"<<tag<<":"<<endl; // Might be the same effective adress than Next:

	if(current==KEYBOARD&&strcmp(lexer->YYText(),"ELSE")==0){
		current=(TOKEN) lexer->yylex();
		Statement();
	}

	cout<<"Next"<<tag<<":"<<endl;

}


// "WHILE" Expression "DO" Statement

void WhileStatement(){
	/*
	current=(TOKEN) lexer->yylex();
	TYPES type = Expression();
	if (type != BOOLEAN){
		Error("Expression type WhileStatement must be Boolean");
	}

	current=(TOKEN) lexer->yylex();
	

	if (current!=KEYBOARD || strcmp(lexer->YYText(), "DO") != 0){
		Error("DO Attendu après WHILE");
	}

	Statement();
	*/
	unsigned long long tag=TagNumber++;
	cout<<"While"<<tag<<":"<<endl;
	current=(TOKEN) lexer->yylex();

	if(Expression()!=BOOLEAN)
		Error("expression booléene attendue");

	cout<<"\tpop %rax\t# Get the result of expression"<<endl;
	cout<<"\tcmpq $0, %rax"<<endl;
	cout<<"\tje EndWhile"<<tag<<"\t# if FALSE, jump out of the loop"<<tag<<endl;

	if(current!=KEYBOARD||strcmp(lexer->YYText(), "DO")!=0)
		Error("mot-clé DO attendu");

	current=(TOKEN) lexer->yylex();
	Statement();

	cout<<"\tjmp While"<<tag<<endl;
	cout<<"EndWhile"<<tag<<":"<<endl;

}

// "FOR" AssignementStatement "To" Expression "DO" Statement
// ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
void ForStatement(){

	/*
	current=(TOKEN) lexer->yylex();
	
	AssignementStatement();

	current=(TOKEN) lexer->yylex();

	if (current!=KEYBOARD || strcmp(lexer->YYText(), "To") != 0){
		Error("To Attendu après FOR");
	}

	Expression();

	if (current!=KEYBOARD || strcmp(lexer->YYText(), "DO") != 0){
		Error("DO Attendu après WHILE");
	}

	Statement();

	*/
	unsigned long long tag=TagNumber;
	current=(TOKEN) lexer->yylex();
	
	if (current != ID){
		Error("ID attendu");
	}
	string variable = lexer->YYText();

	TYPES type = AssignementStatement();

	

	string c = lexer->YYText();

	if (c == "To"){
		
		current=(TOKEN) lexer->yylex();
		string nbExpression= lexer->YYText();
		
		TYPES type2 = Expression();
		if (type != type2){
			Error("La variable et l'expression doivent avoir le même type dans un for !");
		}
	
		
		cout << "Loop" << tag << ":" << endl;
		cout << "\tpop %rax \t #get the result of expression"<<endl;
		
		//cout << "\tmovq $" << variable << ", %rax \t #putting " << variable << " in rax for ddd so i can look whats in f " << endl;
		//cout << "\tcmpq $" << nbExpression << ", f"<<endl;
		cout << "\tcmpq %rax, f"<<endl;
		
		

		cout << "\tjae EndLoop" << tag << " #jump if equal we out of the loop" <<endl;
		cout << "\tpush %rax" << endl;

		if(current!=KEYBOARD || strcmp(lexer->YYText(), "DO")!=0){
			Error("mot-clé DO attendu");
		}
		current = (TOKEN) lexer->yylex();
		Statement();

		// +1 at variable 

		cout << "\taddq $1, f \t #adding one to f" << endl;
		


		cout << "\tjmp Loop"<< tag << endl;
		
		
		/*
		current=(TOKEN) lexer->yylex();
		TYPES type2 = Expression();
		if (type != type2){
			Error("La variable et l'expression doivent avoir le même type dans un for !");
		}
	

		cout << "Loop" << tag << ":" << endl;
		cout << "\tpop (%rbx) \t #get the result of expression"<<endl;
		
		cout << "\tmovq $" << variable << ", %rax \t #putting " << variable << " in rax for cmp" << endl;
		cout << "\tcmpq %rbx, %rax"<<endl;
		

		cout << "\tje EndLoop" << tag << " #jump if equal we out of the loop" <<endl;
		cout << "\tpush (%rbx)" << endl;

		if(current!=KEYBOARD || strcmp(lexer->YYText(), "DO")!=0){
			Error("mot-clé DO attendu");
		}
		current = (TOKEN) lexer->yylex();
		Statement();

		// +1 at variable 

		cout << "\taddq $1, f \t #adding one to rax" << endl;
		


		cout << "\tjmp Loop"<< tag << endl;
		*/
	}

	else if (c == "DOWNTO"){

		current=(TOKEN)lexer->yylex();

		TYPES type2 = Expression();
		if (type != type2){
			Error("La variable et l'expression doivent avoir le même type dans un for !");
		}

		cout << "Loop" << tag << ":" << endl;
		cout << "\tpop %rax \t #get the result of expression"<<endl;

		cout << "\tcmpq %rax, f"<<endl;
		
		cout << "\tpush %rax" << endl;

		cout << "\tjbe EndLoop" << tag << " #jump if equal we out of the loop" <<endl;

		if(current!=KEYBOARD || strcmp(lexer->YYText(), "DO")!=0){
			Error("mot-clé DO attendu");
		}
		current = (TOKEN) lexer->yylex();
		Statement();


		// -1 at variable 

		cout << "\tsubq $1, f \t # sub one to f" << endl;
		cout << "\tjmp Loop"<< tag << endl;
	}
	else {
		Error("mot clé down or to expected ngl");
	}

	cout << "EndLoop" << tag << ":" << endl;
	

}


// "BEGIN" Statement { ";" Statement } "END"

void BlockStatement(){

	current = (TOKEN) lexer->yylex();
	Statement();

	

	while (strcmp(lexer->YYText(), ";") == 0){
		current=(TOKEN) lexer->yylex(); // pass the ; 
		
		Statement();
		
	}

	
	

	if (current != KEYBOARD || strcmp(lexer->YYText(), "END")){
		Error("END Attendu a la fin d'un block commenceant par BEGIN");
	}
	current=(TOKEN) lexer->yylex();
}	
// DisplayStatement := "DISPLAY" Expression

void DisplayStatement(){
	unsigned long long tag=++TagNumber;
	current = (TOKEN) lexer->yylex();
	TYPES type = Expression();
	if (type == INTEGER){
		cout << "\tpop %rsi\t# The value to be displayed"<<endl;
		cout << "\tmovq $FormatString1, %rdi\t# \"%llu\\n\""<<endl;
		cout << "\tmovl	$0, %eax"<<endl;
		cout << "\tcall	printf@PLT"<<endl;

	}
	else if (type == DOUBLE){
		/*
		Tentative
		cout << "\tfldl (%rsp) 					#empile first into float pile " << endl;
		cout << "\taddq $8, %rsp 				#depilement int pile " << endl;
		cout << "\tfstpl (%rsp)                  # The value to be displayed" << endl;
		cout << "\tmovq $FormatString2, %rdi    # %f\n" << endl;
		cout << "\tmovl    $1, %edi" << endl;
		cout << "\tmovl    $0, %eax" << endl;
		cout << "\tcall    __printf_chk@PLT" << endl;
		*/
		
		cout << "\tmovsd	(%rsp), %xmm0\t\t# &stack top -> %xmm0"<<endl;
		cout << "\tsubq	$16, %rsp\t\t# allocation for 3 additional doubles"<<endl;
		cout << "\tmovsd %xmm0, 8(%rsp)"<<endl;
		
		/*
		cout << "\tfldl (%rsp) 					#empile first into float pile " << endl;
		cout << "\tsubq $16, %rsp 				#depilement int pile " << endl;
		cout << "\tfstpl (%rsp)                  # The value to be displayed" << endl;
		cout << "\tmovq $FormatString2, %rsi    # %f\n" << endl;
		cout << "\tmovl    $1, %edi" << endl;
		cout << "\tmovl    $0, %eax" << endl; 
		cout << "\tcall    __printf_chk@PLT" << endl;
		*/
		
		cout << "\tmovq $FormatString2, %rdi\t# \"%lf\\n\""<<endl;
		cout << "\tmovq	$1, %rax"<<endl;
		cout << "\tcall	printf"<<endl;
		cout << "nop"<<endl;
		cout << "\taddq $24, %rsp\t\t\t# pop nothing"<<endl;
		 

	}

	else if (type == CHAR){
		/*
		cout << "\tpop %rdx                     # The value to be displayed" << endl;
		cout << "\tmovq $FormatString3, %rsi    # %c\n" << endl;
		cout << "\tmovl    $1, %edi" << endl;
		cout << "\tmovl    $0, %eax" << endl;
		cout << "\tcall    __printf_chk@PLT" << endl;
		*/

		cout<<"\tpop %rsi\t\t\t# get character in the 8 lowest bits of %si"<<endl;
		cout << "\tmovq $FormatString3, %rdi\t# \"%c\\n\""<<endl;
		cout << "\tmovl	$0, %eax"<<endl;
		cout << "\tcall	printf@PLT"<<endl;

	}
	// was missing that part when looked tp7 correction 
	else if (type == BOOLEAN){
			cout << "\tpop %rdx\t# Zero : False, non-zero : true"<<endl;
			cout << "\tcmpq $0, %rdx"<<endl;
			cout << "\tje False"<<tag<<endl;
			cout << "\tmovq $TrueString, %rdi\t# \"TRUE\\n\""<<endl;
			cout << "\tjmp Next"<<tag<<endl;
			cout << "False"<<tag<<":"<<endl;
			cout << "\tmovq $FalseString, %rdi\t# \"FALSE\\n\""<<endl;
			cout << "Next"<<tag<<":"<<endl;
			cout << "\tcall	puts@PLT"<<endl;
	}
	// ----

}

// CaseStatement := "CASE" Expression "OF" CaseListElement {";" CaseListElement} "END"
// CaseListElement := CaseLabel {"," CaseLabel} ":" [Statement | empty ]
// CaseLabel := Constant 
void CaseStatement(void){

	current = (TOKEN) lexer->yylex();
	unsigned long long tag=TagNumber++;

	TYPES type = Expression();

	cout << "\tpop %rax \t #get the result of expression"<<endl;


	if (current != KEYBOARD || strcmp(lexer->YYText(), "OF")!=0){
		Error("mot clée OF attendu");
	}


	CaseListElement(); 


}

void RepeatStatement(void){
	// todo 
}
// Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DisplayStatement
void Statement(void){

	// current = (TOKEN) lexer->yylex();
	
	// look if id and not :=
	// if :=
	// if (strcmp(lexer->YYText(),":=") == 0){
	// 	AssignementStatement();
	// }
	
	
	if (current == ID){
		AssignementStatement();
		
	}

	//else if IF
	

	else if (strcmp(lexer->YYText(),"IF") == 0){
	
		IfStatement();
		
	}

	
	// else if WHILE

	else if (strcmp(lexer->YYText(),"WHILE") == 0){
	
		WhileStatement();
		
	}

	// else if FOR

	else if (strcmp(lexer->YYText(),"FOR") == 0){
		
		ForStatement();
		
	}
	// else if BLOCK

	else if (strcmp(lexer->YYText(),"BEGIN") == 0){
	
		BlockStatement();
		
	}

	else if (strcmp(lexer->YYText(), "CASE") == 0){
		CaseStatement();
	}

	// else Display

	else if (strcmp(lexer->YYText(),"DISPLAY") == 0){
		
		DisplayStatement();
		
	}
	else {
		cout << "Statement Problem" << endl;
		cout << lexer->YYText() << endl;
	}
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.align 8"<<endl;	// Alignement on addresses that are a multiple of 8 (64 bits = 8 bytes)
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	
	current=(TOKEN) lexer->yylex();
	Statement();
	while(current==SEMICOLON){
		
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	if(current!=DOT)
		Error("caractère '.' attendu");
	current=(TOKEN) lexer->yylex();
}

// Program := VarDeclarationPart StatementPart
// Program := [DeclarationPart] StatementPart
void Program(void){
	if (current==KEYBOARD && strcmp(lexer->YYText(), "VAR") == 0){
		VarDeclarationPart();
	}
	StatementPart();
}

int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	cout << "\t.data"<<endl;
	cout << "\tFormatString1:    .string \"%llu\"    # used by printf to display 64-bit unsigned integers"<< endl;
	cout << "\tFormatString2:    .string \"%lf\"    # used by printf to display 64-bit float"<< endl;
	cout << "\tFormatString3:    .string \"%c\"    # used by printf to display 64-bit char"<< endl;
	cout << "TrueString:\t.string \"TRUE\"\t# used by printf to display the boolean value TRUE"<<endl; 
	cout << "FalseString:\t.string \"FALSE\"\t# used by printf to display the boolean value FALSE"<<endl; 

	// Let's proceed to the analysis and code production
	current=(TOKEN) lexer->yylex();
	Program();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(current!=FEOF){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}
		
			





