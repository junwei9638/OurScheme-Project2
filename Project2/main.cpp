# include <ctype.h>
# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <cctype>
# include <iostream>
# include <iomanip>
# include <string>
# include <vector>
# include <sstream>

using namespace std;

struct Token {
	string tokenName;
	int tokenTypeNum;
	int tokenColumn;
	int tokenLine;
	int functionType;
}; // TokenType

struct Define {
	string symbolName;
	Define *Binding;
}; // Define Symbol


struct TokenTree {
	Token *leftToken;
	Token *rightToken;
	TokenTree *leftNode;
	TokenTree *rightNode;
	TokenTree *backNode;
}; // TokenType

enum FunctionType {
  CONS, LIST,
  FUNC_QUOTE,
  DEFINE,
  CAR, CDR,
  IS_ATOM, IS_PAIR, IS_LIST, IS_NULL, IS_INT, IS_REAL, IS_NUM, IS_STR, IS_BOOL, IS_SYMBOL,
  PLUS, MINUS, DIV, MULT,
  NOT, AND, OR,
  GREATER, GREATEREQUAL, LESS, LESSEQUAL, EQUAL,
  STR_APPEND, IS_STR_GREATER, IS_STR_LESS, IS_STR_EQUAL,
  IS_EQV, IS_EQUAL,
  BEGIN,
  IF, COND,
  CLEAR_ENV
}; // FunctionType

enum TokenType {
	INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, LEFTPAREN, RIGHTPAREN
}; // TokenType

enum Error {
	LEFT_ERROR, RIGHT_ERROR, CLOSE_ERROR, EOF_ERROR, NO_ERROR, NOT_S_EXP_ERROR
}; // Error

vector<Token> gTokens;
vector<Define> gDefindSymbols;
TokenTree *gTreeRoot = NULL;
TokenTree *gCurrentNode = NULL;

int gLine = 1;
int gColumn = 0;

bool gIsEnd = false;
int gAtomType = 0;

string gErrorMsgName = "\0";
int gErrorMsgType = NO_ERROR;
int gErrorLine = 0;
int gErrorColumn = 0;

// ------------------JudgeMent Function--------------------- //
bool ExitDetect() {

	int nilExit = -1;
	int exitNil = -1;
	string tokenString = "\0";

	for ( int i = 0 ; i < gTokens.size() ; i++ )
		tokenString += gTokens[i].tokenName;

	nilExit = (int) tokenString.find("(nil.exit)");
	exitNil = (int) tokenString.find("(exit.nil)");

	if ( tokenString == "(exit)" || nilExit != -1 || exitNil != -1 ) {
		gIsEnd = true;
		return true;
	} // if


	return false;
} // ExitDetect()

bool IsAtom( int typeNum ) {
	if ( typeNum == SYMBOL || typeNum == INT ||
			 typeNum == FLOAT || typeNum == STRING ||
			 typeNum == NIL || typeNum == T )
		return true;

	else return false;
} // IsAtom()

bool IsFunction( string tokenName ) {
	if ( tokenName == "cons" || tokenName == "list" || tokenName == "quote" || tokenName == "define" ||
			 tokenName == "car" || tokenName == "cdr" || tokenName == "atom?" || tokenName == "pair?" ||
			 tokenName == "list?" || tokenName == "null?" || tokenName == "integer?" || tokenName == "real?" ||
			 tokenName == "number?" || tokenName == "string?" || tokenName == "boolean?" || tokenName == "symbol?" ||
			 tokenName == "+" || tokenName == "-" || tokenName == "*" || tokenName == "/" || tokenName == "not" ||
			 tokenName == "and" || tokenName == "or" || tokenName == ">" || tokenName == ">=" || tokenName == "<" ||
			 tokenName == "<=" || tokenName == "=" || tokenName == "string-append" || tokenName == "string>?" ||
			 tokenName == "string<?" || tokenName == "string=?" || tokenName == "eqv?" || tokenName == "equal?" ||
			 tokenName == "begin" || tokenName == "if" || tokenName == "cond" || tokenName == "clean-environment" )
		return true ;
	else return false ;
} // IsFunction()

bool CheckParameter( TokenTree * CurrentNode, string tokenName ) {
	if ( tokenName == "cons" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL && CurrentNode->rightNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "quote" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "define" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL && CurrentNode->rightNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "car" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "atom?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "pair?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "null?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "integer?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "real?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "number?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "string?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "boolean?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "symbol?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "+" ) {
		if ( CurrentNode->rightNode == NULL && CurrentNode->rightNode->rightNode != NULL )
			return false ;
		else return true ;
	} // if

	else if ( tokenName == "-" ) {
		if ( CurrentNode->rightNode == NULL && CurrentNode->rightNode->rightNode != NULL )
			return false ;
		else return true ;
	} // if

	else if ( tokenName == "*" ) {
		if ( CurrentNode->rightNode == NULL && CurrentNode->rightNode->rightNode != NULL )
			return false ;
		else return true ;
	} // if

	else if ( tokenName == "/" ) {
		if ( CurrentNode->rightNode == NULL && CurrentNode->rightNode->rightNode != NULL )
			return false ;
		else return true ;
	} // if

	else if ( tokenName == "not" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "and" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "or" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == ">" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == ">=" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "<" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "<=" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "=" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "string-append" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "string>?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "string<?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "string=?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "eqv?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "equal?" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL && CurrentNode->rightNode->rightNode == NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "begin" ) {
		if ( CurrentNode->rightNode != NULL )
			return true ;
		else return false ;
	} // if

	else if ( tokenName == "if" ) {
		if ( CurrentNode->rightNode != NULL && CurrentNode->rightNode->rightNode != NULL ) {
			if ( CurrentNode->rightNode->rightNode->rightNode != NULL ) {
				if ( CurrentNode->rightNode->rightNode->rightNode->rightNode != NULL )
					return false;
				else return true;
			} // if

			return true;
		} // if

		else return false ;
	} // if

	else if ( tokenName == "cond" ) {
		if ( CurrentNode->rightNode != NULL ) return true ;
		else return false ;
	} // if

	else if ( tokenName == "clear-environment" ) {
		if ( CurrentNode->rightNode != NULL ) return false ;
		else return true ;
	} // if

  return false ;
} // CheckParameter()
// ------------------Setting Function--------------------- //
void SetFunctionType( int & functionType ,string tokenName ) {
	if ( tokenName == "cons" ) functionType = CONS ;
	else if ( tokenName == "list" ) functionType = LIST ;
	else if ( tokenName == "quote" ) functionType = FUNC_QUOTE ;
	else if ( tokenName == "define" ) functionType = DEFINE ;
	else if ( tokenName == "car" ) functionType = CAR;
	else if ( tokenName == "cdr" ) functionType = CDR;
	else if ( tokenName == "atom?" ) functionType = IS_ATOM ;
	else if ( tokenName == "pair?" ) functionType = IS_PAIR ;
	else if ( tokenName == "list?" ) functionType = IS_LIST ;
	else if ( tokenName == "null?" ) functionType = IS_NULL ;
	else if ( tokenName == "integer?" ) functionType = IS_INT ;
	else if ( tokenName == "real?" ) functionType = IS_REAL ;
	else if ( tokenName == "number?" ) functionType = IS_NUM ;
	else if ( tokenName == "string?" ) functionType = IS_STR ;
	else if ( tokenName == "boolean?" ) functionType = IS_BOOL ;
	else if ( tokenName == "symbol?" ) functionType = IS_SYMBOL ;
	else if ( tokenName == "+" ) functionType = PLUS;
	else if ( tokenName == "-" ) functionType = MINUS ;
	else if ( tokenName == "*" ) functionType = MULT ;
	else if ( tokenName == "/" ) functionType = DIV ;
	else if ( tokenName == "not" ) functionType = NOT ;
	else if ( tokenName == "and" ) functionType = AND ;
	else if ( tokenName == "or" ) functionType = OR ;
	else if ( tokenName == ">" ) functionType = GREATER ;
	else if ( tokenName == ">=" ) functionType = GREATEREQUAL ;
	else if ( tokenName == "<" ) functionType = LESS ;
	else if ( tokenName == "<=" ) functionType = LESSEQUAL ;
	else if ( tokenName == "=" ) functionType = EQUAL ;
	else if ( tokenName == "string-append" ) functionType = STR_APPEND ;
	else if ( tokenName == "string>?" ) functionType = IS_STR_GREATER;
	else if ( tokenName == "string<?" ) functionType = IS_STR_LESS ;
	else if ( tokenName == "string=?" ) functionType = IS_STR_EQUAL ;
	else if ( tokenName == "eqv?" ) functionType = IS_EQV ;
	else if ( tokenName == "equal?" ) functionType = IS_EQUAL ;
	else if ( tokenName == "begin" ) functionType = BEGIN ;
	else if ( tokenName == "if" ) functionType = IF ;
	else if ( tokenName == "cond" ) functionType = COND ;
	else if ( tokenName == "clear-environment" ) functionType = CLEAR_ENV ;

} // SetFunctionType()

void InitialLineColumn() {
	gLine = 1;
	gColumn = 0;
} // InitialLineColumn()

void ClearSpaceAndOneLine() {               // read space and "ONE" Line
	char peek = cin.peek();
	InitialLineColumn();

	while ( peek == ' ' || peek == '\t' ) {
		cin.get();
		gColumn++;
		peek = cin.peek();
	} // while

	if ( peek == '\n' ) {                    // if == endl -> initial line and column
		InitialLineColumn();                  // else record the line and column
		cin.get();
	} // if

	if ( peek == ';' ) {                     // comment case
		while ( peek != '\n' && peek != EOF) {
			cin.get();
			peek = cin.peek();
		} // while

		if ( peek == '\n' ) cin.get();
		InitialLineColumn();                  // get endl
	} // if

} // ClearSpaceAndOneLine()

void ClearInput() {
	char peek = cin.peek();
	while ( peek != '\n' && peek != EOF) {
		cin.get();
		peek = cin.peek();
	} // while

	if ( peek == '\n' ) cin.get();
} // ClearInput()

void SetErrorMsg( int errorType, string errorToken, int line, int column ) {
	gErrorMsgType = errorType;
	gErrorMsgName = errorToken;
	gErrorLine = line;
	gErrorColumn = column;
} // SetErrorMsg()

void GlobalVariableReset() {
	gTreeRoot = NULL;
	gCurrentNode = NULL;
	gTokens.clear();
	gErrorLine = 1;
	gErrorColumn = 0;
	gErrorMsgType = NO_ERROR;
	gErrorMsgName = "\0";
	gAtomType = 0;
} // GlobalVariableReset()



// ------------------Get Token--------------------- //

string StringProcess() {
	string inputString = "\0";
	bool closeQuote = false;
	char ch = cin.get();
	gColumn++;
	inputString += ch;
	char peek = cin.peek();
	gAtomType = STRING;

	while ( closeQuote == false && peek != '\n' && peek != EOF) {

		ch = cin.get();
		gColumn++;
		peek = cin.peek();

		if ( ch == '"' ) closeQuote = true;

		if ( ch == '\\' && peek == '\\' ) {              // double slash
			cin.get();
			gColumn++;
			peek = cin.peek();
			inputString += '\\';
		} // if

		else if ( ch == '\\' && peek == 'n' ) {           // \n case
			cin.get();
			gColumn++;
			peek = cin.peek();
			inputString += '\n';
		} // if

		else if ( ch == '\\' && peek == 't' ) {          // \t case
			cin.get();
			gColumn++;
			peek = cin.peek();
			inputString += '\t';
		} // if

		else if ( ch == '\\' && peek == '"' ) {          // \" case
			cin.get();
			gColumn++;
			peek = cin.peek();
			inputString += '"';
		} // if

		else inputString += ch;

	} // while


	if ( closeQuote == false ) {
		if ( peek == EOF)
			SetErrorMsg(EOF_ERROR, "\"", 0, 0);
		else if ( peek == '\n' )
			SetErrorMsg(CLOSE_ERROR, "\"", gLine, gColumn + 1);
		return "\0";
	} // if                                        // no closing quote

	return inputString;
} // StringProcess()

string NumProcess( string atomExp ) {

	if ( atomExp[0] == '+' ) atomExp = atomExp.substr(1, atomExp.length() - 1);  // '+' case
	if ( atomExp[0] == '.' ) atomExp = "0" + atomExp;                             // .xxx case
	if ( atomExp[0] == '-' && atomExp[1] == '.' ) atomExp.insert(1, "0");        // -.xxx case

	int dot = (int) atomExp.find('.');
	if ( dot != atomExp.npos ) gAtomType = FLOAT;
	else gAtomType = INT;
	return atomExp;
} // NumProcess()

string AtomAnalyze( string atomExp ) {
	bool isNum = false;
	bool isSymbol = false;
	int signBit = 0;
	int digitBit = 0;
	int dotBit = 0;
	if ( atomExp == "#t" || atomExp == "t" ) {
		atomExp = "#t";
		gAtomType = T;
	} // if

	else if ( atomExp == "nil" || atomExp == "#f" ) {
		atomExp = "nil";
		gAtomType = NIL;
	} // if
	else if ( atomExp == "'" ) {
		atomExp = "'";
		gAtomType = QUOTE;
	} // if
	else if ( atomExp == "." ) {
		atomExp = ".";
		gAtomType = DOT;
	} // if

	else {

		for ( int i = 0 ; i < atomExp.length() ; i++ ) {
			if (((int) atomExp[i] >= 48 && (int) atomExp[i] <= 57) ||
					atomExp[i] == '+' || atomExp[i] == '-' || atomExp[i] == '.' ) {
				if ( atomExp[i] == '+' || atomExp[i] == '-' ) signBit++;
				if ((int) atomExp[i] >= 48 && (int) atomExp[i] <= 57 ) digitBit++;
				if ( atomExp[i] == '.' ) dotBit++;
				isNum = true;
			} // if

			else isSymbol = true;
		} // for                                                           // Num Judge

		if ( signBit > 1 || digitBit == 0 || dotBit > 1 ) isNum = false;

		gAtomType = SYMBOL;
		if ( isNum && !isSymbol ) atomExp = NumProcess(atomExp);
	} // else


	return atomExp;
} // AtomAnalyze()

string GetAtom() {
	string atomExp = "\0";
	char ch = '\0';
	char peek = cin.peek();
	while ( peek != '\n' && peek != '"' &&
					peek != ' ' && peek != ';' &&
					peek != '(' && peek != ')' &&
					peek != '\0' && peek != EOF &&
					peek != '\'' && peek != '\t' &&
					peek != '\r' ) {
		ch = cin.get();
		gColumn++;
		atomExp = atomExp + ch;
		peek = cin.peek();
	} // while

	atomExp = AtomAnalyze(atomExp);

	return atomExp;
} // GetAtom()

char GetChar() {
	char peek = '\0';
	peek = cin.peek();
	while ( peek == ' ' || peek == '\t' || peek == '\n' || peek == ';' ) {
		if ( peek == ';' ) {
			while ( peek != '\n' && peek != EOF) {
				cin.get();
				gColumn++;
				peek = cin.peek();
			} // while
		} // if

		else if ( peek == '\n' ) {
			cin.get();
			gLine++;
			gColumn = 0;
		} // if

		else if ( peek == ' ' || peek == '\t' ) {
			cin.get();
			gColumn++;
		} // if


		peek = cin.peek();
	} // while

	return peek;
} // GetChar()

bool GetToken() {
	Token token;
	char peek = GetChar();

	token.tokenName = "\0";
	token.tokenTypeNum = 0;
	token.tokenColumn = 0;
	token.tokenLine = 0;

	if ( peek == EOF) {
		SetErrorMsg(EOF_ERROR, " ", 0, 0);
		gIsEnd = true;
		return false;
	} // if

	else {

		if ( peek == '(' ) {
			token.tokenName = cin.get();
			gColumn++;
			token.tokenColumn = gColumn;
			token.tokenLine = gLine;
			peek = GetChar();

			if ( peek == ')' ) {
				token.tokenName = "nil";
				cin.get();
				gColumn++;
				gAtomType = NIL;
			} // if                                                   // () case

			else
				gAtomType = LEFTPAREN;
		} // if                                         // left praen

		else if ( peek == ')' ) {
			token.tokenName = cin.get();
			gColumn++;
			token.tokenColumn = gColumn;
			token.tokenLine = gLine;
			gAtomType = RIGHTPAREN;
		} // if             // right paren

		else if ( peek == '"' ) {
			token.tokenColumn = gColumn + 1;
			token.tokenLine = gLine;
			token.tokenName = StringProcess();
			if ( token.tokenName == "\0" ) return false;
		} // if                                        // string


		else if ( peek == '\'' ) {
			token.tokenName = cin.get();
			gColumn++;
			token.tokenColumn = gColumn;
			token.tokenLine = gLine;
			gAtomType = QUOTE;
		} // if

		else {
			token.tokenColumn = gColumn + 1;
			token.tokenLine = gLine;
			token.tokenName = GetAtom();
			int notSymbol = (int) token.tokenName.find('"');
			if ( notSymbol != token.tokenName.npos ) {
				SetErrorMsg(CLOSE_ERROR, "\"", 0, 0);
				return false;
			} // if
		} // else

		token.tokenTypeNum = gAtomType;
		gTokens.push_back(token);

		return true;
	} // else

} // GetToken()

// ------------------Tree Build--------------------- //


void InitialNode() {
	gCurrentNode->leftNode = NULL;
	gCurrentNode->leftToken = NULL;
	gCurrentNode->rightNode = NULL;
	gCurrentNode->rightToken = NULL;
} // InitialNode()

void InsertAtomToTree() {
	if ( gTreeRoot != NULL ) {
		if ( gCurrentNode->leftToken == NULL ) {                              // left token null
			if ( gCurrentNode->leftNode == NULL ) {                             // left node null -> insert left
				gCurrentNode->leftToken = new Token;
				gCurrentNode->leftToken->tokenName = gTokens.back().tokenName;
				gCurrentNode->leftToken->tokenTypeNum = gTokens.back().tokenTypeNum;
			} // if

			else if ( gCurrentNode->leftNode != NULL ) {                        // left node !null
				while ( gCurrentNode->rightNode != NULL )                         // find right node null
					gCurrentNode = gCurrentNode->backNode;

				if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {                 // if !dot-> create right node
					gCurrentNode->rightNode = new TokenTree;                       // and insert to left token
					gCurrentNode->rightNode->backNode = gCurrentNode;
					gCurrentNode = gCurrentNode->rightNode;
					InitialNode();
					gCurrentNode->leftToken = new Token;
					gCurrentNode->leftToken->tokenName = gTokens.back().tokenName;
					gCurrentNode->leftToken->tokenTypeNum = gTokens.back().tokenTypeNum;
				} // if

				else if ( gTokens[gTokens.size() - 2].tokenTypeNum == DOT ) {            // insert right token
					gCurrentNode->rightToken = new Token;
					gCurrentNode->rightToken->tokenName = gTokens.back().tokenName;
					gCurrentNode->rightToken->tokenTypeNum = gTokens.back().tokenTypeNum;
				} // if
			} // if
		} // if

		else if ( gCurrentNode->leftToken != NULL ) {                       // left token !null
			while ( gCurrentNode->rightNode != NULL )                        // find right node null
				gCurrentNode = gCurrentNode->backNode;

			if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {                 // if !dot-> create right node
				gCurrentNode->rightNode = new TokenTree;                       // and insert to left token
				gCurrentNode->rightNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->rightNode;
				InitialNode();
				gCurrentNode->leftToken = new Token;
				gCurrentNode->leftToken->tokenName = gTokens.back().tokenName;
				gCurrentNode->leftToken->tokenTypeNum = gTokens.back().tokenTypeNum;
			} // if

			else if ( gTokens[gTokens.size() - 2].tokenTypeNum == DOT ) {            // if == dot-> insert right token
				gCurrentNode->rightToken = new Token;
				gCurrentNode->rightToken->tokenName = gTokens.back().tokenName;
				gCurrentNode->rightToken->tokenTypeNum = gTokens.back().tokenTypeNum;
			} // if
		} // if
	} // if
} // InsertAtomToTree()

void BuildTree() {
	if ( gTreeRoot == NULL ) {
		gTreeRoot = new TokenTree;
		gCurrentNode = gTreeRoot;
		InitialNode();
	} // if

	else {
		if ( gCurrentNode->leftToken == NULL ) {                // left token null
			if ( gCurrentNode->leftNode == NULL ) {               // left node null-> create node
				gCurrentNode->leftNode = new TokenTree;
				gCurrentNode->leftNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->leftNode;
				InitialNode();
			} // if

			else if ( gCurrentNode->leftNode != NULL ) {          // left node !null
				while ( gCurrentNode->rightNode != NULL )           // find right node null-> create node
					gCurrentNode = gCurrentNode->backNode;

				gCurrentNode->rightNode = new TokenTree;
				gCurrentNode->rightNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->rightNode;
				InitialNode();

				if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {    // if !dot-> create left node
					gCurrentNode->leftNode = new TokenTree;
					gCurrentNode->leftNode->backNode = gCurrentNode;
					gCurrentNode = gCurrentNode->leftNode;
					InitialNode();
				} // if

			} // if
		} // if

		else if ( gCurrentNode->leftToken != NULL ) {         // left token !null
			while ( gCurrentNode->rightNode != NULL )           // find right node null-> create node
				gCurrentNode = gCurrentNode->backNode;

			gCurrentNode->rightNode = new TokenTree;                       // and create right node
			gCurrentNode->rightNode->backNode = gCurrentNode;
			gCurrentNode = gCurrentNode->rightNode;
			InitialNode();

			if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {                 // if !dot-> create left node
				gCurrentNode->leftNode = new TokenTree;
				gCurrentNode->leftNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->leftNode;
				InitialNode();
			} // if

		} // if
	} // else


} // BuildTree()


bool SyntaxChecker() {
	if ( IsAtom(gTokens.back().tokenTypeNum)) {
		// cout << "Atom " ;

		if ( gTreeRoot != NULL ) InsertAtomToTree();
		return true;
	} // if


	else if ( gTokens.back().tokenTypeNum == LEFTPAREN ) {

		BuildTree();                                                 // // create node

		if ( !GetToken()) return false;

		if ( SyntaxChecker()) {
			if ( !GetToken()) return false;

			while ( SyntaxChecker()) {
				if ( !GetToken()) return false;
			} // while

			if ( gErrorMsgType != NOT_S_EXP_ERROR ) return false;

			if ( gTokens.back().tokenTypeNum == DOT ) {
				// cout << "Dot " ;
				if ( GetToken()) {
					if ( SyntaxChecker()) {
						if ( !GetToken()) return false;
					} // if  syntax checker

					else {
						if ( gErrorMsgType == NOT_S_EXP_ERROR ) {
							SetErrorMsg(LEFT_ERROR, gTokens.back().tokenName,
													gTokens.back().tokenLine, gTokens.back().tokenColumn);
						} // if

						return false;

					} // else
				} // if: GetToken()

				else return false;
			} // if

			if ( gTokens.back().tokenTypeNum == RIGHTPAREN ) {
				gCurrentNode = gCurrentNode->backNode;
				return true;
			} // if
			else {
				SetErrorMsg(RIGHT_ERROR, gTokens.back().tokenName,
										gTokens.back().tokenLine, gTokens.back().tokenColumn);
				return false;
			} // else
		} // if

		else {
			if ( gErrorMsgType == NOT_S_EXP_ERROR ) {
				SetErrorMsg(LEFT_ERROR, gTokens.back().tokenName,
										gTokens.back().tokenLine, gTokens.back().tokenColumn);
			} // if

			return false;
		} // else

	} // if

	else if ( gTokens.back().tokenTypeNum == QUOTE ) {
		// cout << "Quote " ;
		Token temp;
		gTokens.pop_back();
		temp.tokenName = "(";
		temp.tokenTypeNum = LEFTPAREN;
		gTokens.push_back(temp);
		BuildTree();
		temp.tokenName = "quote";
		temp.tokenTypeNum = QUOTE;
		gTokens.push_back(temp);
		InsertAtomToTree() ;

		if ( GetToken()) {
			if ( SyntaxChecker()) {
				temp.tokenName = ")";
				temp.tokenTypeNum = RIGHTPAREN;
				gTokens.push_back(temp);
				return true;
			} // if :push right paren
			else {
				SetErrorMsg(LEFT_ERROR, gTokens.back().tokenName,
										gTokens.back().tokenLine, gTokens.back().tokenColumn);
				return false;
			} // else
		} // if

		else return false;
	} // if

	SetErrorMsg(NOT_S_EXP_ERROR, gTokens.back().tokenName,
							gTokens.back().tokenLine, gTokens.back().tokenColumn);
	return false;

} // SyntaxChecker()

// ------------------Print Function--------------------- //

void PrintAtom( int i ) {
	if ( gTokens[i].tokenTypeNum == FLOAT ) {
		cout << fixed << setprecision(3)
				 << round(atof(gTokens[i].tokenName.c_str()) * 1000) / 1000 << endl;
	} // if
	else cout << gTokens[i].tokenName << endl;
} // PrintAtom()

bool NeedPrint( int i, int printParenNum ) {
	if ( i > 0 ) {
		if ( gTokens[i].tokenTypeNum == LEFTPAREN && gTokens[i - 1].tokenTypeNum == DOT )       // . ( case
			return false;

		else if ( gTokens[i].tokenTypeNum == DOT ) {                                     // atom . Nil case
			if ( gTokens[i + 1].tokenTypeNum == NIL ) return false;
			if ( !IsAtom(gTokens[i + 1].tokenTypeNum)) return false;
		} // if

		else if ( gTokens[i].tokenTypeNum == NIL && gTokens[i - 1].tokenTypeNum == DOT )         // atom . Nil case
			return false;

		else if ( gTokens[i].tokenTypeNum == NIL && gTokens[i - 1].tokenTypeNum == DOT )         // atom . Nil case
			return false;

		else if ( gTokens[i].tokenTypeNum == RIGHTPAREN ) {
			if ( printParenNum > 1 ) return true;
			else return false;
		} // if


	} // if

	return true;

} // NeedPrint()

void PrintSpace( int printParenNum ) {
	for ( int i = 0 ; i < printParenNum ; i++ )
		cout << "  ";
} // PrintSpace()

void PrintSExp() {
	int printParenNum = 0;
	Token temp;
	bool lineReturn = false;

	for ( int i = 0 ; i < gTokens.size() ; i++ ) {
		if ( gTokens.size() == 1 ) PrintAtom(i);

		else {
			if ( NeedPrint(i, printParenNum)) {

				if ( gTokens[i].tokenTypeNum == RIGHTPAREN ) printParenNum--;

				if ( lineReturn ) {
					PrintSpace(printParenNum);
					lineReturn = false;
				} // if

				if ( gTokens[i].tokenTypeNum == QUOTE ) {
					cout << gTokens[i].tokenName << endl;
					lineReturn = true;
				} // if

				else if ( gTokens[i].tokenTypeNum == LEFTPAREN ) {
					cout << gTokens[i].tokenName << " ";
					printParenNum++;
				} // if

				else if ( gTokens[i].tokenTypeNum == RIGHTPAREN ) {
					cout << gTokens[i].tokenName << endl;
					lineReturn = true;
				} // if

				else if ( gTokens[i].tokenTypeNum == DOT ) {
					cout << gTokens[i].tokenName << endl;
					lineReturn = true;
				} // if


				else if ( IsAtom(gTokens[i].tokenTypeNum)) {
					PrintAtom(i);
					lineReturn = true;
				} // if

			} // if
		} // else

		if ( i == (gTokens.size() - 1) && printParenNum > 0 ) {
			while ( printParenNum > 0 ) {
				printParenNum--;
				PrintSpace(printParenNum);
				cout << ")" << endl;
			} // while
		} // if
	} // for

	cout << endl;
} // PrintSExp()

void PrintErrorMessage() {
	if ( gErrorMsgType == LEFT_ERROR || gErrorMsgType == NOT_S_EXP_ERROR )
		cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
				 << gErrorLine << " Column " << gErrorColumn << " is >>" << gErrorMsgName << "<<" << endl << endl;
	else if ( gErrorMsgType == RIGHT_ERROR )
		cout << "ERROR (unexpected token) : ')' expected when token at Line "
				 << gErrorLine << " Column " << gErrorColumn << " is >>" << gErrorMsgName << "<<" << endl << endl;
	else if ( gErrorMsgType == CLOSE_ERROR )
		cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line "
				 << gErrorLine << " Column " << gErrorColumn << endl << endl;
	else if ( gErrorMsgType == EOF_ERROR )
		cout << "ERROR (no more input) : END-OF-FILE encountered";
} // PrintErrorMessage()

void PrintEvaluateError() {
	cout << "ERROR " << endl ;
} // PrintEvaluateError()

void PrintDefinition( int index ) {
  
}  // PrintDefinition()

// ------------------Functional Function--------------------- //

bool Cons( TokenTree* CurrentNode ) {
  return true ;
}  // Cons()

bool List( TokenTree* CurrentNode ) {
  return true ;
} // List()

bool Quote( TokenTree* CurrentNode ) {
  return true ;
} // Quote

bool Define( TokenTree* CurrentNode ) {
  return true ;
} // Define()

bool Car( TokenTree* CurrentNode ) {
  return true ;
} // Car()

bool Cdr( TokenTree* CurrentNode ) {
  return true ;
} // Cdr()

bool Is_Atom( TokenTree* CurrentNode ) {
  return true ;
} // Is_Atom

bool Is_Pair( TokenTree* CurrentNode ) {
  return true ;
} // Is_Pair()

bool Is_List( TokenTree* CurrentNode ) {
  return true ;
} // Is_List

bool Is_Null( TokenTree* CurrentNode ) {
  return true ;
} // Is_Null

bool Is_Int( TokenTree* CurrentNode ) {
  return true ;
} // Is_Int()

bool Is_Real( TokenTree* CurrentNode ) {
  return true ;
} // Is_Real()

bool Is_Num( TokenTree* CurrentNode ) {
  return true ;
} // Is_Num()

bool Is_Str( TokenTree* CurrentNode ) {
  return true ;
} // Is_Str()

bool Is_Bool( TokenTree* CurrentNode ) {
  return true ;
} // Is_Bool

bool Is_Symbol( TokenTree* CurrentNode ) {
  return true ;
} // Is_Symbol()

bool Plus( TokenTree* CurrentNode ) {
  return true ;
} // Plus()

bool Minus( TokenTree* CurrentNode ) {
  return true ;
} // Minus

bool Div( TokenTree* CurrentNode ){
  return true ;
} // Div()

bool Mult( TokenTree* CurrentNode ) {
  return true ;
} // Mult()

bool Not( TokenTree* CurrentNode ) {
  return true ;
} // Not()

bool And( TokenTree* CurrentNode ) {
  return true ;
} // And()

bool Or( TokenTree* CurrentNode ) {
  return true ;
} // Or()

bool Greater( TokenTree* CurrentNode ) {
  return true ;
} // Greater()

bool GreaterEqual( TokenTree* CurrentNode ) {
  return true ;
} // GreaterEqual()

bool Less( TokenTree* CurrentNode ) {
  return true ;
} // Less()

bool LessEqual( TokenTree* CurrentNode ) {
  return true ;
} // LessEqual()

bool Equal( TokenTree* CurrentNode ) {
  return true ;
} // Equal()

bool Str_Append( TokenTree* CurrentNode ) {
  return true ;
} // Str_Append()

bool Is_Str_Greater(  TokenTree* CurrentNode) {
  return true ;
} // Is_Str_Greater()

bool Is_Str_Less( TokenTree* CurrentNode ) {
  return true ;
} // Is_Str_Less()

bool Is_Str_Equal( TokenTree* CurrentNode ) {
  return true ;
} // Is_Str_Equal()

bool Is_Eqv( TokenTree* CurrentNode ) {
  return true ;
} // Is_Equal()

bool Is_Equal( TokenTree* CurrentNode ) {
  return true ;
} // Is_Equal()

bool Begin( TokenTree* CurrentNode ) {
  return true ;
} // Begin()

bool If(  TokenTree* CurrentNode) {
  return true ;
} // If()

bool Cond( TokenTree* CurrentNode ) {
  return true ;
} // Cond()

bool Clear_Env( ){
  gDefindSymbols.clear() ;
  return true;
} // Clear_Env()

bool FunctionJudge( TokenTree* CurrentNode, string tokenName  ) {
  if ( tokenName == "cons" ) return Cons( CurrentNode ) ;
  else if ( tokenName == "list" ) return List( CurrentNode ) ;
  else if ( tokenName == "quote" ) return Quote( CurrentNode ) ;
  else if ( tokenName == "define" ) return Define( CurrentNode ) ;
  else if ( tokenName == "car" ) return Car( CurrentNode ) ;
  else if ( tokenName == "cdr" ) return Cdr( CurrentNode ) ;
  else if ( tokenName == "atom?" ) return Is_Atom( CurrentNode ) ;
  else if ( tokenName == "pair?" ) return Is_Pair( CurrentNode ) ;
  else if ( tokenName == "list?" ) return Is_List( CurrentNode ) ;
  else if ( tokenName == "null?" ) return Is_Null( CurrentNode ) ;
  else if ( tokenName == "integer?" ) return Is_Int( CurrentNode ) ;
  else if ( tokenName == "real?" ) return Is_Real( CurrentNode ) ;
  else if ( tokenName == "number?" ) return Is_Num( CurrentNode ) ;
  else if ( tokenName == "string?" ) return Is_Str( CurrentNode ) ;
  else if ( tokenName == "boolean?" ) return Is_Bool( CurrentNode ) ;
  else if ( tokenName == "symbol?" ) return
  else if ( tokenName == "+" ) return
  else if ( tokenName == "-" ) return
  else if ( tokenName == "*" ) return
  else if ( tokenName == "/" ) return
  else if ( tokenName == "not" )  return
  else if ( tokenName == "and" ) return
  else if ( tokenName == "or" ) return
  else if ( tokenName == ">" ) return
  else if ( tokenName == ">=" ) return
  else if ( tokenName == "<" ) return
  else if ( tokenName == "<=" ) return
  else if ( tokenName == "=" ) return
  else if ( tokenName == "string-append" ) return
  else if ( tokenName == "string>?" ) return
  else if ( tokenName == "string<?" ) return
  else if ( tokenName == "string=?" ) return
  else if ( tokenName == "eqv?" ) return
  else if ( tokenName == "equal?" ) return
  else if ( tokenName == "begin" ) return
  else if ( tokenName == "if" ) return
  else if ( tokenName == "cond" ) return
  else if ( tokenName == "clear-environment" ) return Clear_Env() ;
} // FunctionJudge()



// ------------------Evaluate Function--------------------- //

bool PostOrderTraversal( TokenTree * CurrentNode ) {
	if ( CurrentNode != NULL) {
    PostOrderTraversal( CurrentNode->leftNode ) ;
    PostOrderTraversal( CurrentNode->rightNode ) ;
		
    if ( CurrentNode->leftToken != NULL ) {
			if ( IsFunction( CurrentNode->leftToken->tokenName ) ) {
				if ( CheckParameter( CurrentNode,CurrentNode->leftToken->tokenName )  )
          FunctionJudge( CurrentNode,CurrentNode->leftToken->tokenName ) ;
					//cout << CurrentNode->leftToken->tokenName << endl;
				else {
					cout << "nono parameter" << endl;
					return false;
				} // else
			} // if
		} // if
	} // if

	return true;

} // Evaluate()

bool EvaluateSExp(){
  gCurrentNode = gTreeRoot ;
  if ( gCurrentNode == NULL ) {
    for ( int i = 0 ; i < gDefindSymbols.size() ; i++ ) {
      if( gTokens[0].tokenName == gDefindSymbols[i].symbolName ) PrintDefinition(i) ;
    } // for
    return false ;
  } // if
  
  else if( PostOrderTraversal( gCurrentNode ) ) return true;
  else return false ;
} // EvaluateSExp()



// ------------------Main Function--------------------- //

int main() {
	cout << "Welcome to OurScheme!" << endl << endl;
	char uTestNum = '\0';
	while ( uTestNum != '\n' ) {
		uTestNum = cin.get();
	} // while


	do {
		cout << "> ";
		if ( GetToken()) {
			if ( SyntaxChecker()) {
				if ( !ExitDetect()) {
					if ( EvaluateSExp() ) PrintSExp();
					else PrintEvaluateError();
					ClearSpaceAndOneLine();
				} // if
			} // if

			else {
				PrintErrorMessage();
				ClearInput();
				InitialLineColumn();
			} // else
		} // if

		else {
			PrintErrorMessage();
			ClearInput();
			InitialLineColumn();
		} // else

		GlobalVariableReset();
	} while ( !gIsEnd );


	cout << endl << "Thanks for using OurScheme!" << endl;
	return 0;

} // main()
