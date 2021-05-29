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
# include <algorithm>
# include <string.h>
# include <exception>

using namespace std;

struct Token {
	string tokenName;
	int tokenTypeNum;
	int tokenColumn;
	int tokenLine;
}; // TokenType

struct TokenTree {
  string tokenName ;
  int tokenType ;
	TokenTree *leftNode;
	TokenTree *rightNode;
  bool needToBePrimitive;
  bool fromQuote ;
}; // TokenType

struct DefineSymbol {
	string symbolName;
	TokenTree * binding;
}; // DefineSymbol Symbol


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
	INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, LEFTPAREN, RIGHTPAREN, NO_TYPE
}; // TokenType

enum Error {
	LEFT_ERROR, RIGHT_ERROR, CLOSE_ERROR, EOF_ERROR, NO_ERROR, NOT_S_EXP_ERROR,
  PARAMETER_NUM_ERROR, PARAMETER_TYPE_ERROR, UNBOND_ERROR, NO_APPLY_ERROR,
  NO_RETURN_VAL_ERROR, DIVISION_BY_ZERO_ERROR, FORMAT_ERROR, NON_LIST_ERROR,
  LEVEL_OF_DEFINE
}; // Error

vector<Token> gTokens;
vector<DefineSymbol> gDefineSymbols;

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

class Exception : public exception {
 public:
	int mErrorType ;
  TokenTree* mErrorNode ;

  Exception( int type, TokenTree * errorNode ) {
		mErrorType = type ;
    mErrorNode = errorNode ;
	}  // Exception()
  
}; // ErrorHandling class

class Project {
public:
// ------------------Setting Function--------------------- //
  void InitialLineColumn() {
		gLine = 1;
		gColumn = 0;
	} // InitialLineColumn()

	void InitialNode( TokenTree *currentNode ) {
		currentNode->leftNode = NULL;
		currentNode->rightNode = NULL;
    currentNode->fromQuote = false ;
    currentNode->tokenType = NO_TYPE ;
    currentNode->tokenName = "\0" ;
	} // InitialNode()

	void InitialToken( Token *token ) {
		token->tokenColumn = 0;
		token->tokenLine = 0;
		token->tokenName = "\0";
		token->tokenTypeNum = NO_TYPE;
	} // InitialToken()

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

	void GlobalVariableInitial() {
		gTreeRoot = NULL;
		gCurrentNode = NULL;
		gTokens.clear();
		gErrorLine = 1;
		gErrorColumn = 0;
		gErrorMsgType = NO_ERROR;
		gErrorMsgName = "\0";
		gAtomType = 0;
	} // GlobalVariableReset()


	void InitialDefineStruct( DefineSymbol  define ) {
		define.symbolName = "\0";
		define.binding = NULL;
	} // InitialDefineStruct()

	DefineSymbol GetDefineSymbol( string tokenName ) {
		DefineSymbol temp ;
		InitialDefineStruct( temp );
		for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
			if ( tokenName == gDefineSymbols[i].symbolName ) {
				temp.symbolName = gDefineSymbols[i].symbolName;
				temp.binding = gDefineSymbols[i].binding;
			} // if
		} // for

		return temp;
	} // GetDefineSymbol()

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

	bool AtomJudge( int typeNum ) {
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
				 tokenName == "number?" || tokenName == "string?" || tokenName == "boolean?" ||
				 tokenName == "symbol?" ||
				 tokenName == "+" || tokenName == "-" || tokenName == "*" || tokenName == "/" || tokenName == "not" ||
				 tokenName == "and" || tokenName == "or" || tokenName == ">" || tokenName == ">=" ||
				 tokenName == "<" ||
				 tokenName == "<=" || tokenName == "=" || tokenName == "string-append" || tokenName == "string>?" ||
				 tokenName == "string<?" || tokenName == "string=?" || tokenName == "eqv?" || tokenName == "equal?" ||
				 tokenName == "begin" || tokenName == "if" || tokenName == "cond" ||
				 tokenName == "clean-environment" )
			return true;
		else return false;
	} // IsFunction()

	bool CheckDefinition( string tokenName ) {
		for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
			if ( tokenName == gDefineSymbols[i].symbolName ) return true;
		} // for

		return false;
	} // CheckDefinition()


  void CheckNonList( TokenTree* currentNode ) {
    TokenTree* walkNode = currentNode ;
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      if ( walkNode->tokenName != "\0" ) {
        cout << "ERROR (non-list) : ";
        throw Exception( NON_LIST_ERROR, currentNode ) ;
      } // if : nonlist
    } // while : check right token
    
  } // CheckNonList()
  
  void CheckParameterNum( TokenTree* currentNode, int needNum, string functionName ) {
    int num = 0 ;
    TokenTree* walkNode = currentNode ;
    while ( walkNode->rightNode  ) {
      walkNode = walkNode->rightNode ;
      num++ ;
    } // while
    
    if ( num != needNum ) {
      if ( functionName == "define" ) {
        cout << "ERROR (DEFINE format) : " ;
        throw Exception( PARAMETER_NUM_ERROR, currentNode ) ;
      } // if : define format
      
      else if ( functionName == "+" || functionName == "-" || functionName == "*" || functionName == "/" ||
           functionName == "and" || functionName == "or" || functionName == ">" || functionName == ">=" ||
           functionName == "<" || functionName == "<=" || functionName == "=" ||
           functionName == "string-append" || functionName == "string>?" || functionName == "string<?" ||
           functionName == "string=?" ) {
        if ( num < needNum ) {
          cout << "ERROR (incorrect number of arguments) : " + functionName ;
          throw Exception( PARAMETER_NUM_ERROR, NULL ) ;
        } // if : not >= 2
      } // if : define format
      
      else {
        cout << "ERROR (incorrect number of arguments) : " + functionName ;
        throw Exception( PARAMETER_NUM_ERROR, NULL ) ;
      } // else
    } // if : throw exception
    
  } // CheckParameterNum( )

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
			if ( peek == EOF )
				SetErrorMsg( EOF_ERROR, "\"",  0, 0);
			else if ( peek == '\n' )
				SetErrorMsg( CLOSE_ERROR, "\"",  gLine, gColumn + 1);
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
		else if ( atomExp == "'" || atomExp == "quote" ) {
			gAtomType = QUOTE;
		} // if
    
		else if ( atomExp == "." ) {
			atomExp = ".";
			gAtomType = DOT;
		} // if

		else if ( atomExp == "quote" ) gAtomType = QUOTE;

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

		if ( peek == EOF ) {
			SetErrorMsg(EOF_ERROR, "\0",  0, 0);
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

// ------------------ReadSExp--------------------- //


	bool SyntaxChecker() {
		if ( AtomJudge(gTokens.back().tokenTypeNum) || gTokens.back().tokenName == "quote" ) {
			return true;
		} // if

		else if ( gTokens.back().tokenTypeNum == LEFTPAREN ) {

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

		else if ( gTokens.back().tokenTypeNum == QUOTE && gTokens.back().tokenName == "'" ) {
			// cout << "Quote " ;
			Token temp;
			gTokens.pop_back();
			temp.tokenName = "(";
			temp.tokenTypeNum = LEFTPAREN;
			gTokens.push_back(temp);
			temp.tokenName = "quote";
			temp.tokenTypeNum = QUOTE;
			gTokens.push_back(temp);

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


	void CreateTree( TokenTree *currentNode, vector<Token> &Tokens, bool isRight ) {
		if ( AtomJudge( Tokens.front().tokenTypeNum ) || Tokens.front().tokenTypeNum == QUOTE ) {
			if ( isRight ) {
				currentNode->rightNode = new TokenTree;
				InitialNode( currentNode->rightNode );
				currentNode->rightNode->tokenName = Tokens.front().tokenName;
				currentNode->rightNode->tokenType = Tokens.front().tokenTypeNum;
				Tokens.erase(Tokens.begin());
			} // if : isRight

			else {
        currentNode->leftNode = new TokenTree;
        InitialNode( currentNode->leftNode );
        currentNode->leftNode->tokenName = Tokens.front().tokenName;
        currentNode->leftNode->tokenType = Tokens.front().tokenTypeNum;
        Tokens.erase(Tokens.begin());
			} // if : isLeft
		} // if


		else if ( Tokens.front().tokenTypeNum == LEFTPAREN ) {
			if ( isRight ) {
				currentNode->rightNode = new TokenTree;
				InitialNode(currentNode->rightNode);
				Tokens.erase(Tokens.begin());
				CreateTree(currentNode->rightNode, Tokens, false);
				currentNode = currentNode->rightNode;
			} // if : isRight

			else {
				currentNode->leftNode = new TokenTree;
				currentNode->leftNode->needToBePrimitive = true;
				InitialNode( currentNode->leftNode );
				Tokens.erase(Tokens.begin());
				CreateTree( currentNode->leftNode, Tokens, false );
				currentNode = currentNode->leftNode;
			} // else : is left


			while ( Tokens.front().tokenTypeNum == LEFTPAREN || Tokens.front().tokenTypeNum == QUOTE ||
							AtomJudge(Tokens.front().tokenTypeNum)) {
				currentNode->rightNode = new TokenTree;
				InitialNode(currentNode->rightNode);
				CreateTree(currentNode->rightNode, Tokens, false);
				currentNode = currentNode->rightNode;
			} // while


			if ( Tokens.front().tokenTypeNum == DOT ) {
				Tokens.erase(Tokens.begin());
				CreateTree(currentNode, Tokens, true);
			} // if : Dot

			if ( Tokens.front().tokenTypeNum == RIGHTPAREN ) {
				Tokens.erase(Tokens.begin());
				return;
			} // if : right paren

		} // if : left paren


	} // CreateTree()


	bool ReadSExp() {
		if ( SyntaxChecker()) {
			vector<Token> tokens = gTokens;
			gTreeRoot = new TokenTree;
			gCurrentNode = gTreeRoot;
			InitialNode(gTreeRoot);

			CreateTree( gTreeRoot , tokens, false );
      
      gTreeRoot = gTreeRoot->leftNode ;
      gCurrentNode = gTreeRoot;

			return true;
		} // if

		else return false;

	} // ReadSExp()


// ------------------Print Function--------------------- //


	void PrintSExpTree( TokenTree *currentNode, bool isRightNode, int &layer ) {
		static bool lineReturn = false;
		if ( !isRightNode && currentNode->tokenName == "\0" ) {
			cout << "( ";
			lineReturn = false;
			layer++;
		} // if


		if ( lineReturn && currentNode->tokenName == "\0" ) {
			for ( int i = 0 ; i < layer ; i++ ) cout << "  ";
		} // if

		if ( currentNode->tokenName != "\0" ) {
      if ( !isRightNode ) {
        cout << currentNode->tokenName << endl;
        lineReturn = true;
      } // if : leftNode
      
      else if ( currentNode->tokenType != NIL ) {
        for ( int i = 0 ; i < layer ; i++ ) cout << "  ";
        cout << "." << endl;
        for ( int i = 0 ; i < layer ; i++ ) cout << "  ";
        cout << currentNode->tokenName << endl;
      } // else
		} // if

		if ( currentNode->leftNode ) PrintSExpTree( currentNode->leftNode, false, layer);

		if ( currentNode->rightNode ) PrintSExpTree( currentNode->rightNode, true, layer);

		if ( layer > 1 && currentNode->tokenName == "\0" &&
        ( currentNode->rightNode == NULL || ( currentNode->leftNode->tokenName != "\0" && currentNode->rightNode->tokenName != "\0" ) ) ) {
			lineReturn = true;
			layer--;
			for ( int i = 0 ; i < layer ; i++ ) cout << "  ";
			cout << ")" << endl;
		} // if : print right paren
    
	} // PrintSExpTree()


	void PrintFunctionMsg() {
		int layer = 0;
		if ( gTreeRoot->tokenName != "\0" ) {
      if ( gTreeRoot->tokenType == FLOAT ) {
        cout << fixed << setprecision(3)
             << round(atof( gTreeRoot->tokenName.c_str()) * 1000) / 1000 << endl;
      } // if : float print
      else cout << gTreeRoot->tokenName << endl;
    } // if : only one result

    else if ( !gTreeRoot->leftNode && !gTreeRoot->rightNode ) {
      cout << "nil" << endl;
    } // if : only one node and has no tokenName
    
    else {
      PrintSExpTree( gTreeRoot, false, layer );
      for ( int i = 0 ; i < layer ; i++ ) cout << ")" << endl;
    } // else
	} // PrintFunctionMsg()


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

  void PrintEvaluateErrorMessage( int errorType, TokenTree* errorNode ) {
    int layer = 0 ;
    if ( errorNode != NULL ) {
      PrintSExpTree( errorNode, false, layer );
      for ( int i = 0 ; i < layer ; i++ ) cout << ")" << endl;
    } // if : have to print tree
    
    else cout << endl ;
  } // PrintEvaluateErrorMessage()
  
// ------------------Functional Function--------------------- //

	TokenTree* Cons( TokenTree* currentNode ) {
    CheckParameterNum( currentNode, 2, "cons" ) ;
    TokenTree* resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    resultNode->leftNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    resultNode->rightNode = EvaluateSExp( currentNode->rightNode->rightNode->leftNode ) ;
    return resultNode ;

	}  // Cons()
  
  TokenTree* Quote( TokenTree* currentNode ) {
    currentNode->rightNode->leftNode->fromQuote = true ;
    return currentNode->rightNode->leftNode;
  } // Quote

	TokenTree* Define( TokenTree* currentNode ) {
		TokenTree* resultNode = new TokenTree ;
		DefineSymbol defined  ;
		InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "define" ) ;
		if ( currentNode->rightNode->leftNode->tokenType != SYMBOL || IsFunction(currentNode->rightNode->leftNode->tokenName ) ) {
			cout << "ERROR (DEFINE format) : " ;
			throw Exception( FORMAT_ERROR, currentNode ) ;
		} // if : first token is not symbol or is a function

		if ( currentNode != gTreeRoot ) {
			cout <<  "ERROR (level of DEFINE)" ;
			throw Exception( FORMAT_ERROR, NULL ) ;
		} // if : first token is not symbol or is a function

		defined.symbolName = currentNode->rightNode->leftNode->tokenName ;
		defined.binding = EvaluateSExp( currentNode->rightNode->rightNode->leftNode ) ;

    gDefineSymbols.push_back( defined ) ;

    resultNode->tokenName = currentNode->rightNode->leftNode->tokenName + " defined" ;
		return resultNode ;
	} // Define()

  TokenTree* List( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* resultWalkNode = resultNode ;
    InitialNode( resultNode ) ;
    
    if ( currentNode->rightNode ) {
      currentNode = currentNode->rightNode ;
      resultWalkNode->leftNode = EvaluateSExp( currentNode->leftNode ) ;
    } // if : connect to left node
    
    while( currentNode->rightNode ) {
      currentNode = currentNode->rightNode ;
      resultWalkNode->rightNode = new TokenTree ;
      InitialNode( resultWalkNode->rightNode ) ;
      resultWalkNode->rightNode->leftNode = EvaluateSExp( currentNode->leftNode ) ;
      resultWalkNode = resultWalkNode->rightNode ;
    } // while
    
    return resultNode ;

	} // List()

  TokenTree* Car( TokenTree *currentNode ) {
    TokenTree * resultNode = NULL;
    TokenTree * walkNode = NULL;
    CheckParameterNum( currentNode, 1, "car" ) ;
    
    walkNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    if ( walkNode->tokenName != "\0" ) {
      cout << "ERROR (car with incorrect argument type) : " + walkNode->tokenName ;
      throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : parameter type
    
    if ( walkNode->rightNode )
      resultNode = walkNode->leftNode  ;
    
    else {
      while( walkNode->leftNode && walkNode->leftNode->tokenName == "\0" )
        walkNode = walkNode->leftNode ;
      if ( !walkNode->rightNode && walkNode->leftNode ) resultNode = walkNode->leftNode ;
      else resultNode = walkNode ;
    } // else
    
    return resultNode ;
	} // Car()

  TokenTree* Cdr( TokenTree *currentNode ) {
    TokenTree * resultNode = NULL;
    TokenTree * walkNode = NULL;
    CheckParameterNum( currentNode, 1, "cdr" ) ;
    
    walkNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    if ( walkNode->tokenName != "\0" ) {
      cout << "ERROR (cdr with incorrect argument type) : " + walkNode->tokenName ;
      throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : parameter type
    
    if ( walkNode->rightNode )
      resultNode = walkNode->rightNode  ;
    
    else {
      resultNode = new TokenTree ;
      InitialNode( resultNode ) ;
    } // else
    
    return resultNode ;
	
	} // Cdr()

  TokenTree* Is_Atom( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "atom?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( AtomJudge( judgeNode->tokenType ) && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Atom

  TokenTree* Is_Pair( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "pair?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenName == "\0" ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is pair
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not pair
    
    return resultNode ;
	} // Is_Pair()

  TokenTree* Is_List( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode = NULL ;
    bool isList = true;
    CheckParameterNum( currentNode, 1, "list?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    walkNode = judgeNode ;
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      if ( walkNode->tokenName != "\0" ) isList = false ;
    } // while : check right token
    
    if ( judgeNode && isList ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_List

  TokenTree* Is_Null( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "null?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenType == NIL && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Null

  TokenTree* Is_Int( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "integer?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenType == INT && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Int()

  TokenTree* Is_Real( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "real?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenType == INT && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Real()

  TokenTree* Is_Num( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "number?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Num()

  TokenTree* Is_Str( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "string?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Str()

  TokenTree* Is_Bool( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "boolean?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( ( judgeNode->tokenType == T || judgeNode->tokenType == NIL ) && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Bool

	TokenTree* Is_Symbol( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "number?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenType == SYMBOL ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
	} // Is_Symbol()

	TokenTree* Plus( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
		int resultInt = 0;
		float inputNum = 0.0;
		float resultFloat = 0.0;
		bool isFloat = false;
		stringstream sstream;
		TokenTree *walkNode = currentNode;
    TokenTree *judgeNode = NULL;
    InitialNode( resultNode ) ;
    CheckParameterNum( currentNode, 2, "+" ) ;
    
		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;

      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        inputNum = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
        resultFloat = inputNum + resultFloat;
        if ( judgeNode->tokenType == FLOAT ) isFloat = true;
      } // if : int or float
      
      else {
        cout << "ERROR (+ with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // else : throw Exception
      

		} // while: walkNode go to right node

		if ( isFloat ) {
			sstream << resultFloat;
			string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
		} // if : float result
		else {
			resultInt = (int) resultFloat;
			sstream << resultInt;
			string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = INT ;
		} // else : int result

    return resultNode ;
	} // Plus()

  TokenTree* Minus( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    int resultInt = 0;
    float inputNum = 0.0;
    float resultFloat = 0.0;
    bool isFloat = false;
    bool firstNum = true ;
    stringstream sstream;
    TokenTree *walkNode = currentNode;
    TokenTree *judgeNode = NULL;
    InitialNode( resultNode ) ;
    CheckParameterNum( currentNode, 2, "-" ) ;
    
    while ( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode;

      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        if ( firstNum ) {
          resultFloat = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
          firstNum = false ;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // if : first num
          
        else {
          inputNum = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
          resultFloat = resultFloat - inputNum;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // else : not first num
      } // if : int or float
      
      else {
        cout << "ERROR (- with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // else : throw Exception
      
    } // while: walkNode go to right node

    if ( isFloat ) {
      sstream << resultFloat;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
    } // if : float result
    else {
      resultInt = (int) resultFloat;
      sstream << resultInt;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = INT ;
    } // else : int result

    return resultNode ;
	} // Minus

  TokenTree* Div( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    int resultInt = 0;
    float inputNum = 0.0;
    float resultFloat = 0.0;
    bool isFloat = false;
    bool firstNum = true ;
    stringstream sstream;
    TokenTree *walkNode = currentNode;
    TokenTree *judgeNode = NULL;
    InitialNode( resultNode ) ;
    CheckParameterNum( currentNode, 2, "/" ) ;
    
    while ( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode;

      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        if ( firstNum ) {
          resultFloat = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
          firstNum = false ;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // if : first num
          
        else {
          inputNum = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
          
          if ( inputNum == 0.0 ) {
            cout << "ERROR (division by zero) : /" ;
            throw Exception( DIVISION_BY_ZERO_ERROR, NULL ) ;
          } // if : division zero
          
          resultFloat = resultFloat / inputNum;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // else : not first num
      } // if : int or float
      
      else {
        cout << "ERROR (/ with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // else : throw Exception
      
    } // while: walkNode go to right node

    if ( isFloat ) {
      sstream << resultFloat;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
    } // if : float result
    else {
      resultInt = (int) resultFloat;
      sstream << resultInt;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = INT ;
    } // else : int result

    return resultNode ;
	} // Div()

  TokenTree* Mult( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    int resultInt = 0;
    float inputNum = 0.0;
    float resultFloat = 1.0;
    bool isFloat = false;
    stringstream sstream;
    TokenTree *walkNode = currentNode;
    TokenTree *judgeNode = NULL;
    InitialNode( resultNode ) ;
    CheckParameterNum( currentNode, 2, "*" ) ;
    
    while ( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode;

      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        inputNum = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
        resultFloat = inputNum * resultFloat;
        if ( judgeNode->tokenType == FLOAT ) isFloat = true;
      } // if
      
      else {
        cout << "ERROR (* with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // else : throw Exception
      

    } // while: walkNode go to right node

    if ( isFloat ) {
      sstream << resultFloat;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
    } // if : float result
    else {
      resultInt = (int) resultFloat;
      sstream << resultInt;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = INT ;
    } // else : int result

    return resultNode ;
	} // Mult()

  TokenTree* Not( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 1, "not" ) ;
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenName == "nil" && !judgeNode->fromQuote ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T;
    } // if
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL;
    } // else

    return resultNode;
	} // Not()

  TokenTree* And( TokenTree *currentNode ) {
		TokenTree* walkNode = currentNode;
    TokenTree* judgeNode = NULL;
    
    CheckParameterNum( currentNode, 2, "and" ) ;
    
    walkNode = walkNode->rightNode;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    while ( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == NIL ) return judgeNode;
    } // while
      
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    return judgeNode;
	} // And()

	TokenTree* Or( TokenTree *currentNode ) {
    TokenTree* walkNode = currentNode;
    TokenTree* judgeNode = NULL;
    
    CheckParameterNum( currentNode, 2, "or" ) ;
    
    while ( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType != NIL ) return judgeNode;
    } // while
      
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    return judgeNode;
	} // Or()

	TokenTree* Greater( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
		float compare1 = 0.0;
		float compare2 = 0.0;
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, ">" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
      compare1 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (> with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        compare2 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
        if ( compare1 <= compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (> with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // Greater()

  TokenTree* GreaterEqual( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
    float compare1 = 0.0;
    float compare2 = 0.0;
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, ">=" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
      compare1 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (>= with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        compare2 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
        if ( compare1 < compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (>= with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // GreaterEqual()

  TokenTree* Less( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
    float compare1 = 0.0;
    float compare2 = 0.0;
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "<" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
      compare1 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (< with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        compare2 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
        if ( compare1 >= compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (< with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // Less()

  TokenTree* LessEqual( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
    float compare1 = 0.0;
    float compare2 = 0.0;
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "<=" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
      compare1 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (<= with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        compare2 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
        if ( compare1 > compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (<= with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // LessEqual()

  TokenTree* Equal( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
    float compare1 = 0.0;
    float compare2 = 0.0;
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "=" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
      compare1 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (= with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote ) {
        compare2 = round(atof( judgeNode->tokenName.c_str()) * 1000) / 1000;
        if ( compare1 != compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (= with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // Equal()

  TokenTree* Str_Append( TokenTree *currentNode ) {
		string resultTokenName = "\0";
    TokenTree* resultNode = new TokenTree ;
		TokenTree* walkNode = currentNode;
    TokenTree* judgeNode = NULL;
    InitialNode( resultNode ) ;
    CheckParameterNum( currentNode, 2, "string-append" ) ;
		
    while ( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
        resultTokenName = resultTokenName + judgeNode->tokenName ;
      } // if : string
      
      else {
        cout << "ERROR (string-append with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node

		resultTokenName.erase(remove(resultTokenName.begin(), resultTokenName.end(), '"'), resultTokenName.end());
		resultTokenName = "\"" + resultTokenName;
		resultTokenName = resultTokenName + "\"";
    
    resultNode->tokenName = resultTokenName ;
    resultNode->tokenType = STRING ;
    
    return  resultNode ;
	} // Str_Append()

  TokenTree* Is_Str_Greater( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
    string compare1 = "\0";
    string compare2 = "\0";
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "string>?" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
      compare1 = judgeNode->tokenName ;
    } // if : string
    
    else {
      cout << "ERROR (string>? with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
        compare2 = judgeNode->tokenName ;
        if ( strcmp(compare1.c_str(), compare2.c_str()) <= 0 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : string
      
      else {
        cout << "ERROR (string>? with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // Is_Str_Greater()

  TokenTree* Is_Str_Less( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
    string compare1 = "\0";
    string compare2 = "\0";
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "string<?" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
      compare1 = judgeNode->tokenName ;
    } // if : string
    
    else {
      cout << "ERROR (string<? with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
        compare2 = judgeNode->tokenName ;
        if ( strcmp(compare1.c_str(), compare2.c_str()) >= 0 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (string<? with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // Is_Str_Less()

  TokenTree* Is_Str_Equal( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* judgeNode = NULL ;
    TokenTree* walkNode  = currentNode ;
    string compare1 = "\0";
    string compare2 = "\0";
    bool isNil = false ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "string=?" ) ;
    
    walkNode = walkNode->rightNode ;
    judgeNode = EvaluateSExp( walkNode->leftNode ) ;
    if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
      compare1 = judgeNode->tokenName ;
    } // if : string
    
    else {
      cout << "ERROR (string=? with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
      else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
    } // if : throw exception
    
    while( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == STRING && !judgeNode->fromQuote ) {
        compare2 = judgeNode->tokenName ;
        if ( strcmp(compare1.c_str(), compare2.c_str()) != 0 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (string=? with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) throw Exception( PARAMETER_TYPE_ERROR, judgeNode ) ;
        else throw Exception( PARAMETER_TYPE_ERROR, NULL ) ;
      } // if : throw exception
    } // while : walk every node
    
    if ( isNil ) {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // if : nil
    
    else {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // else : true

    return resultNode ;
	} // Is_Str_Equal()

  /*TokenTree* Is_Eqv( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Is_Equal()

  TokenTree* Is_Equal( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Is_Equal()

  TokenTree* Begin( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Begin()

  TokenTree* If( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // If()

  TokenTree* Cond( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Cond()

  TokenTree* Clean_Env( TokenTree *currentNode ) {
		gDefineSymbols.clear();
	} // Clear_Env()*/

	TokenTree* FindCorrespondFunction( TokenTree *currentNode, string tokenName ) {
		if ( tokenName == "cons" ) return Cons( currentNode );
    else if ( tokenName == "quote" ) return Quote( currentNode );
		else if ( tokenName == "define" ) return Define(currentNode);
		else if ( tokenName == "list" ) return List(currentNode);
		else if ( tokenName == "car" ) return Car(currentNode);
		else if ( tokenName == "cdr" ) return Cdr(currentNode);
		else if ( tokenName == "atom?" ) return Is_Atom(currentNode);
		else if ( tokenName == "pair?" ) return Is_Pair(currentNode);
		else if ( tokenName == "list?" ) return Is_List(currentNode);
		else if ( tokenName == "null?" ) return Is_Null(currentNode);
		else if ( tokenName == "integer?" ) return Is_Int(currentNode);
		else if ( tokenName == "real?" ) return Is_Real(currentNode);
		else if ( tokenName == "number?" ) return Is_Num(currentNode);
		else if ( tokenName == "string?" ) return Is_Str(currentNode);
		else if ( tokenName == "boolean?" ) return Is_Bool(currentNode);
		else if ( tokenName == "symbol?" ) return Is_Symbol(currentNode);
    else if ( tokenName == "+" ) return Plus(currentNode);
		else if ( tokenName == "-" ) return Minus(currentNode);
		else if ( tokenName == "*" ) return Mult(currentNode);
		else if ( tokenName == "/" ) return Div(currentNode);
		else if ( tokenName == "not" ) return Not(currentNode);
		else if ( tokenName == "and" ) return And(currentNode);
		else if ( tokenName == "or" ) return Or(currentNode);
		else if ( tokenName == ">" ) return Greater(currentNode);
		else if ( tokenName == ">=" ) return GreaterEqual(currentNode);
		else if ( tokenName == "<" ) return Less(currentNode);
		else if ( tokenName == "<=" ) return LessEqual(currentNode);
		else if ( tokenName == "=" ) return Equal(currentNode);
		else if ( tokenName == "string-append" ) return Str_Append(currentNode);
		else if ( tokenName == "string>?" ) return Is_Str_Greater(currentNode);
		else if ( tokenName == "string<?" ) return Is_Str_Less(currentNode);
		else if ( tokenName == "string=?" ) return Is_Str_Equal(currentNode);
		/*else if ( tokenName == "eqv?" ) return Is_Eqv(currentNode);
		else if ( tokenName == "equal?" ) return Is_Equal(currentNode);
		else if ( tokenName == "begin" ) return Begin(currentNode);
		else if ( tokenName == "if" ) return If(currentNode);
		else if ( tokenName == "cond" ) return Cond(currentNode);
		else if ( tokenName == "clean-environment" ) return Clean_Env(currentNode);*/
    return NULL ;
	} // FindCorrespondFunction()



// ------------------Evaluate Function--------------------- //


	TokenTree * EvaluateSExp( TokenTree *currentNode ) {
    if ( currentNode->tokenName != "\0" ) {
      if ( currentNode->tokenType == SYMBOL && !currentNode->fromQuote ) {
        if ( CheckDefinition( currentNode->tokenName ) ) {
          DefineSymbol defined = GetDefineSymbol( currentNode->tokenName ) ;
          currentNode = defined.binding ;
        } // if : is in definition
        
        else {
          cout << "ERROR (unbound symbol) : " + currentNode->tokenName ;
          throw Exception( UNBOND_ERROR, NULL ) ;
        } // else : throw exception
        
      } // if : if SYMBOL->check definition
    } // if : left token
    
    else {
      CheckNonList( currentNode ) ;

      if ( currentNode->needToBePrimitive == true ) {
      	if ( currentNode->leftNode->tokenType == SYMBOL ) {
					if ( CheckDefinition( currentNode->leftNode->tokenName ) ) {
						DefineSymbol defined = GetDefineSymbol( currentNode->leftNode->tokenName ) ;
						if ( defined.binding->tokenName != "\0" ) {
							currentNode->leftNode = defined.binding ;
						} // if : define is a token

						else {
							cout << "ERROR (attempt to apply non-function) : " ;
							throw Exception( NO_APPLY_ERROR, defined.binding ) ;
						} // else : define is a node-> error
					} // if : is in definition

					else {
            if ( !IsFunction( currentNode->leftNode->tokenName ) ) {
              cout << "ERROR (unbound symbol) : " + currentNode->leftNode->tokenName ;
              throw Exception( UNBOND_ERROR, NULL ) ;
            } // if
					} // else : throw exception
      	} // if : if is SYMBOL　

        if ( IsFunction( currentNode->leftNode->tokenName ) ) {
          currentNode = FindCorrespondFunction( currentNode, currentNode->leftNode->tokenName ) ;
        } // if : check is Function
        
        else {
          cout << "ERROR (attempt to apply non-function) : " + currentNode->leftNode->tokenName ;
          throw Exception( NO_APPLY_ERROR, NULL ) ;
        } // else : no apply error
      } // if
      
      
    } // else left node

    return currentNode;
	} // EvaluateSExp()

}; // Project

// ------------------Main Function--------------------- //

int main() {
	cout << "Welcome to OurScheme!" << endl << endl;
	char uTestNum = '\0';
	while ( uTestNum != '\n' ) {
		uTestNum = cin.get();
	} // while

	Project project ;

	project.GlobalVariableInitial() ;

	do {
		cout << "> ";
		if ( project.GetToken()) {
			if ( project.ReadSExp() ) {
				if ( !project.ExitDetect()) {
          
					try {
						gTreeRoot = project.EvaluateSExp( gTreeRoot ) ;
						project.PrintFunctionMsg() ;
					} // try
					catch( Exception e ) {
						project.PrintEvaluateErrorMessage( e.mErrorType, e.mErrorNode ) ;
					} // catch
          
					project.ClearSpaceAndOneLine();
				} // if
			} // if

			else {
				project.PrintErrorMessage();
				project.ClearInput();
				project.InitialLineColumn();
			} // else
		} // if

		else {
			project.PrintErrorMessage();
			project.ClearInput();
			project.InitialLineColumn();
		} // else

		project.GlobalVariableInitial();
	} while ( !gIsEnd );

 
	cout << endl << "Thanks for using OurScheme!" << endl;
	return 0;

} // main()
