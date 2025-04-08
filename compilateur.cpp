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

using namespace std;

// char current;				// Current car	
// char nextcar;

// void ReadChar(void){		// Read character and skip spaces until 
// 				// non space character is read
// 	while(cin.get(current) && (current==' '||current=='\t'||current=='\n'))
// 	   	cin.get(current);
// }

char current, lookedAhead;                // Current char    
int NLookedAhead=0;

void ReadChar(void){
    if(NLookedAhead>0){
        current=lookedAhead;    // Char has already been read
        NLookedAhead--;
    }
    else
        // Read character and skip spaces until 
        // non space character is read
        while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
}

void LookAhead(void){
    while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
    NLookedAhead++;
}


void Error(string s){
	cerr<< s << endl;
	exit(-1);
}

// ArithmeticExpression := Term {AdditiveOperator Term}
// Term := Digit | "(" ArithmeticExpression ")"

// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"


// AdditiveOperator := "+" | "-"
// AdditiveOperator := "+" | "-" | "||"
	
void AdditiveOperator(void){
	if((current=='+')||(current=='-'))
		ReadChar();
	else if (current=='|'){
		ReadChar();
		if (current=='|'){
			ReadChar();
		}
		else {
			Error("'|' attendu après un '|'");
		}
	}
	else
		Error("Opérateur additif attendu");	   // Additive operator expected
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"

void MultiplicativeOperator(void){

	if ((current=='*') || (current=='/') || (current=='%')){
		ReadChar();
	}
	else if (current=='&'){
		ReadChar();
		if (current=='&'){
			ReadChar();
		}
		else{
			Error("'&' Attendu après un '&'");
		}
	}
	else {
		Error("Opérateur multiplicatif attendu");
	}
}

// <OpérateurRelationnel> ::= '=' | '<>' | '<' | '<=' | '>=' | '>' 
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  

void RelationalOperator(void){

	LookAhead();

	if ((lookedAhead == '=')){
		if((current=='<')||(current=='>') || (current=='=') || (current=='!')){
			ReadChar();
			ReadChar();
		}
		else {
			Error("RelationalOperator attendu");
		}
	}
	else {
		if((current=='<')||(current=='>')){
			ReadChar();
		}
		else if (current=='='){
			Error("second '=' attendu");
		}
		else if (current=='!'){
			Error("'=' attendu après '!'");
		}
	}
		
}

// Factor := Number | Letter | "(" Expression ")"| "!" Factor

void Factor(void){
	if((current>='0')||(current<='9')){
		Number();
	}
	else if ((current>='a') || (current<='z')){
		Letter();
	}
	if (current=='('){
		ReadChar();
		Expression();

		if (current != ')'){
			Error("')' était attendu");
		}
	}
	if (current =='!'){
		ReadChar();
		Factor();
	}
}

// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"

void Digit(void){
	if((current<'0')||(current>'9')) // code ASCII
		Error("Chiffre attendu");		   // Digit expected
	else{
		cout << "\tpush $"<<current<<endl;
		ReadChar();
	}
}

//Number := Digit{Digit}

void Number(void){

	if((current<'0')||(current>'9')) // code ASCII
		Error("Chiffre attendu");		   // Digit expected
	else{
		string nombre;
		while((current>='0'||current<='9')){
			nombre += current;
			ReadChar();
		}
		long nb = std::stol(nombre)
		cout << "\tpush $"<<nb<<endl;
	}
}

// Letter := "a"|...|"z"

void Letter(void){
	if ((current<'a') || (current>'z')){
		Error("Lettre attendu");
	}
	else{
		cout << "\tpush "<<current<<endl;
		ReadChar();
	}
}


void ArithmeticExpression(void);			// Called by Term() and calls Term()


// Term := Digit | "(" ArithmeticExpression ")"
// Term := Factor {MultiplicativeOperator Factor}

void Term(void){

	char adop;
	Factor();

	while((current=='*') || (current=='/') || (current=='%') || (current=='&')){
		adop=current;		
		MultiplicativeOperator();
		Term();
		cout << "\tpop %rbx \t#pop first ope"<<endl;	// get first operand
		cout << "\tpop %rax \t#pop second ope"<<endl;

		switch(adop){
			case '*':
			case '&':
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax"<<endl;	// store result
				break;
			case '/':
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
				cout << "\tpush %rax"<<endl;		// store result
				break;
			case '%':
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// remainder goes to %rdx
				cout << "\tpush %rdx"<<endl;		// store result
				break;
			default:
				Error("opérateur mul attendu");
		}

	}
}


// ArithmeticExpression := Term {AdditiveOperator Term}*
// SimpleExpression := Term {AdditiveOperator Term}

void SimpleExpression(void){
	char adop;
	Term();
	while(current=='+'||current=='-'||current=='|'){
		adop=current;		// Save operator in local variable
		AdditiveOperator();
		Term();
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		if(adop=='+')
			cout << "\taddq	%rbx, %rax"<<endl;	// add both operands
		else
			cout << "\tsubq	%rbx, %rax"<<endl;	// substract both operands
		cout << "\tpush %rax"<<endl;			// store result
	}

}




// <expression> ::= <ExpressionArithmétique> | <ExpressionArithmétique> <OpérateurRelationnel> <ExpressionArithmétique> 
// Expression := SimpleExpression [RelationalOperator SimpleExpression]

void expression(void){
	
	Term():

}

int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;

	// Let's proceed to the analysis and code production
	ReadChar();
	ArithmeticExpression();
	ReadChar();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(cin.get(current)){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}
		
			





