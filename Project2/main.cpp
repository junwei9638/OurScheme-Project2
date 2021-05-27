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
	TokenTree *backNode;
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
string gErrorFunctionName = "\0" ;
TokenTree* gErrorBinding = NULL ;
int gErrorMsgType = NO_ERROR;
int gErrorLine = 0;
int gErrorColumn = 0;

class Exception : public exception {
 public:
	int mErrorType ;
	const char* mErrorMsg ;
  TokenTree* mErrorNode ;

  Exception( int type, const char* errorMsg, TokenTree * errorNode ) {
		mErrorType = type ;
    mErrorMsg = errorMsg ;
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
		gErrorBinding = NULL;
		gAtomType = 0;
	} // GlobalVariableReset()


	void InitialDefineStruct( DefineSymbol * define ) {
		define->symbolName = "\0";
		define->binding = NULL;
	} // InitialDefineStruct()

	DefineSymbol* GetDefineSymbol( string tokenName ) {
		DefineSymbol* temp = new DefineSymbol;
		InitialDefineStruct( temp );
		for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
			if ( tokenName == gDefineSymbols[i].symbolName ) {
				temp->symbolName = gDefineSymbols[i].symbolName;
				temp->binding = gDefineSymbols[i].binding;
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
    
    while ( currentNode->rightNode ) {
      currentNode = currentNode->rightNode ;
      if ( currentNode->tokenName != "\0" ) {
        string errorMsg = "ERROR (non-list) : ";
        throw Exception( NON_LIST_ERROR, errorMsg.c_str(), currentNode ) ;
      } // if : nonlist
    } // while : check right token
    
  } // CheckNonList()
  
  void CheckParameterNum( TokenTree* currentNode, int needNum, string functionName ) {
    int num = 0 ;
    while ( currentNode->rightNode  ) {
      currentNode = currentNode->rightNode ;
      num++ ;
    } // while
    
    if ( num != needNum ) {
      string errorMsg = "ERROR (incorrect number of arguments) : " + functionName ;
      throw Exception( PARAMETER_NUM_ERROR, errorMsg.c_str(), NULL ) ;
    } // if : throw exception
    
  } // CheckParameterNum( )

	/*bool CheckParameter( TokenTree *currentNode, string tokenName ) {
		if ( tokenName == "cons" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL &&
					 currentNode->rightNode->rightNode->rightNode == NULL ) {

				if ( currentNode->rightNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // non list check

				if ( currentNode->rightNode->leftToken != NULL ) {
					if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
						if ( !CheckDefinition(currentNode->rightNode->leftToken->tokenName)) {
							if ( !currentNode->rightNode->leftToken->fromQuote ) {
								SetErrorMsg(UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0);
								return false;
							} // if : not from quote Symbol
						} // if : not in definition
					} // if
				} // 1st node type check

				if ( currentNode->rightNode->rightNode->leftToken != NULL ) {
					if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
						if ( !CheckDefinition(currentNode->rightNode->rightNode->leftToken->tokenName)) {
							if ( !currentNode->rightNode->rightNode->leftToken->fromQuote ) {
								SetErrorMsg(UNBOND_ERROR, currentNode->rightNode->rightNode->leftToken->tokenName, "\0", NULL,
														0, 0);
								return false;
							} // if : not from quote Symbol
						} // if : not in definition
					} // if
				} // 2nd node type check

				return true;
			} // if : num check

			else {

				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode && currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else : num of parameter error
		} // if : cons

		else if ( tokenName == "list" ) {
			TokenTree *walkNode = currentNode;
			bool inDefinition = false;
			while ( walkNode->rightNode != NULL ) {
				walkNode = walkNode->rightNode;
				if ( walkNode->leftToken != NULL ) {
					if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
						for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
							if ( walkNode->leftToken->tokenName == gDefineSymbols[i].symbolName )
								inDefinition = true;
						} // for

						if ( !inDefinition ) {
							if ( !walkNode->leftToken->fromQuote ) {
								SetErrorMsg(UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0);
								return false;
							} // if : not from quote Symbol
						} // if : not in definition -> return false

						inDefinition = false;
					} //if : left token is not null

					if ( walkNode->rightToken != NULL ) {
						SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
						return false;
					} // if : ( list 3 . 4 ) case
				} // if : check symbol in define set

			} // while : check right node type


			return true;
		} // if : list

		else if ( tokenName == "quote" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
				if ( currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // non list check
				return true;
			} // if : num check

			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else : num error
		} // if : quote

		else if ( tokenName == "define" ) {
			if ( currentNode != gTreeRoot->leftNode ) {
				SetErrorMsg(LEVEL_OF_DEFINE, "\0", "\0", NULL, 0, 0);
				return false;
			} // if : error -> level of define

			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL &&
					 currentNode->rightNode->rightNode->rightNode == NULL ) {
				if ( currentNode->rightNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode->leftToken != NULL ) {
					if ( currentNode->rightNode->leftToken->tokenTypeNum != SYMBOL ) {
						SetErrorMsg(FORMAT_ERROR, "\0", "\0", currentNode, 0, 0);
						return false;
					} // if : type error

					else if ( IsFunction(currentNode->rightNode->leftToken->tokenName)) {
						SetErrorMsg(FORMAT_ERROR, "\0", "\0", currentNode, 0, 0);
						return false;
					} // else : Symbol is a function
				} // if : first parameter

				if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
					if ( !currentNode->rightNode->rightNode->leftToken->fromQuote ) {
						if ( !CheckDefinition(currentNode->rightNode->rightNode->leftToken->tokenName) &&
								 !IsFunction(currentNode->rightNode->rightNode->leftToken->tokenName)) {
							SetErrorMsg(UNBOND_ERROR, currentNode->rightNode->rightNode->leftToken->tokenName, "\0", NULL,
													0, 0);
							return false;
						} // if : not in definition
					} // if not from quote
				} // if : second definition

				else if ( currentNode->rightNode->leftNode != NULL ) {
					SetErrorMsg(FORMAT_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // else : type error -> qoute or dot-piar behind define

				return true;
			} // if : parameter num check

			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode && currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(FORMAT_ERROR, "\0", "\0", currentNode, 0, 0);
				return false;
			} // else : Num error
		} // if : define

		else if ( tokenName == "car" || tokenName == "cdr" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {

				if ( currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : not list

				if ( currentNode->rightNode->leftToken != NULL ) {
					if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
						if ( !currentNode->rightNode->leftToken->fromQuote ) {
							if ( !CheckDefinition(currentNode->rightNode->leftToken->tokenName)) {
								SetErrorMsg(UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0);
								return false;
							} // if : error-> not in definition

							else {
								DefineSymbol temp = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
								if ( temp.binding == NULL ) {
									SetErrorMsg(PARAMETER_TYPE_ERROR, temp.definedName, tokenName, NULL, 0, 0);
									return false;
								} // if  : in definition but not node
							} // else : in definition
						} // if : not from quote

						else {
							SetErrorMsg(PARAMETER_TYPE_ERROR, currentNode->rightNode->leftToken->tokenName, tokenName, NULL,
													0, 0);
							return false;
						} // else : from quote
					} // if : symbol type

					else {
						SetErrorMsg(PARAMETER_TYPE_ERROR, currentNode->rightNode->leftToken->tokenName, tokenName, NULL,
												0, 0);
						return false;
					} // else : not symbol
				} // if : Check symbol in definition


				return true;
			} // if : num check

			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else : num error
		} // if : car, cdr


		else if ( tokenName == "atom?" || tokenName == "pair?" || tokenName == "list?" || tokenName == "null?" ||
							tokenName == "integer?" || tokenName == "real?" || tokenName == "number?" ||
							tokenName == "string?" ||
							tokenName == "boolean?" || tokenName == "symbol?" || tokenName == "not" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {

				if ( currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode->leftToken != NULL ) {
					if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
						if ( !CheckDefinition(currentNode->rightNode->leftToken->tokenName)) {
							if ( !currentNode->rightNode->leftToken->fromQuote ) {
								SetErrorMsg(UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0);
								return false;
							} // if : not from quote Symbol
						} // if : Unbond error
					} // if : symbol->check definition
				} // if : check token type

				return true;
			} // if : num check


			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else
		} // if : atom?, pair?, list?, null?, integer?, real?, number?, string?, boolean?, symbol?, not


		else if ( tokenName == "+" || tokenName == "-" || tokenName == "*" || tokenName == ">" ||
							tokenName == ">=" || tokenName == "<" || tokenName == "<=" || tokenName == "=" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL ) {
				TokenTree *walkNode = currentNode;
				while ( walkNode->rightNode != NULL ) {
					walkNode = walkNode->rightNode;
					if ( walkNode->rightToken != NULL ) {
						SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
						return false;
					} // if : non list check

					if ( walkNode->leftNode != NULL ) {
						SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", tokenName, walkNode->leftNode, 0, 0);
						return false;
					} // if : left node is a node


					if ( walkNode->leftToken != NULL ) {
						if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
							if ( !walkNode->leftToken->fromQuote ) {
								if ( !CheckDefinition(walkNode->leftToken->tokenName)) {
									SetErrorMsg(UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0);
									return false;
								} // if : not in definition

								else {
									DefineSymbol temp = GetDefineSymbol(walkNode->leftToken->tokenName);
									if ( temp.binding != NULL ) {
										SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", tokenName, currentNode, 0, 0);
										return false;
									} // if  : in definition but node

									if ( temp.tokenTypeNum != INT && temp.tokenTypeNum != FLOAT ) {
										SetErrorMsg(PARAMETER_TYPE_ERROR, temp.definedName, tokenName, NULL, 0, 0);
										return false;
									} // if : in definition but not in float
								} // else : in definition
							} // if : not from quote

							else {
								SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
								return false;
							} // from quote but symbol
						} // if : symbol

						else if ( walkNode->leftToken->tokenTypeNum != INT &&
											walkNode->leftToken->tokenTypeNum != FLOAT ) {
							SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
							return false;
						} // if : not int or float

						else {
							if ((walkNode->leftToken->tokenTypeNum == INT || walkNode->leftToken->tokenTypeNum == FLOAT) &&
									walkNode->leftToken->fromQuote ) {
								SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
								return false;
							} // if : int or float but from quote
						} // else :  int or float
					} // if : left token null

				} // while : check every right node

				return true;
			} // if : num check

			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode != NULL && currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else : num check false
		} // if : +, -, *, >, >=, <, <=, =


		else if ( tokenName == "/" ) {
			float zero = 0.000;
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL ) {
				TokenTree *walkNode = currentNode;

				walkNode = walkNode->rightNode;
				if ( walkNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( walkNode->leftNode != NULL ) {
					SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", tokenName, currentNode, 0, 0);
					return false;
				} // if : walkNode->leftNode


				if ( walkNode->leftToken != NULL ) {
					if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
						if ( !walkNode->leftToken->fromQuote ) {
							if ( !CheckDefinition(walkNode->leftToken->tokenName)) {
								SetErrorMsg(UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0);
								return false;
							} // if : not in definition

							else {
								DefineSymbol temp = GetDefineSymbol(walkNode->leftToken->tokenName);
								if ( temp.binding != NULL ) {
									SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", tokenName, currentNode, 0, 0);
									return false;
								} // if  : in definition but node

								if ( temp.tokenTypeNum != INT && temp.tokenTypeNum != FLOAT ) {
									SetErrorMsg(PARAMETER_TYPE_ERROR, temp.definedName, tokenName, NULL, 0, 0);
									return false;
								} // if : in definition but not in float
							} // else : in definition
						} // if : not from quote

						else {
							SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
							return false;
						} // from quote but symbol
					} // if : symbol

					else if ( walkNode->leftToken->tokenTypeNum != INT && walkNode->leftToken->tokenTypeNum != FLOAT ) {
						SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
						return false;
					} // if : not int or float

					else {
						if ((walkNode->leftToken->tokenTypeNum == INT || walkNode->leftToken->tokenTypeNum == FLOAT) &&
								walkNode->leftToken->fromQuote ) {
							SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
							return false;
						} // if : int or float but from quote
					} // else :  int or float
				} // if : left token null

				while ( walkNode->rightNode != NULL ) {
					walkNode = walkNode->rightNode;
					if ( walkNode->rightToken != NULL ) {
						SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
						return false;
					} // if : non list check

					if ( walkNode->leftNode != NULL ) {
						SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", "-", currentNode, 0, 0);
						return false;
					} // if : walkNode->leftNode


					if ( walkNode->leftToken != NULL ) {
						if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
							if ( !walkNode->leftToken->fromQuote ) {
								if ( !CheckDefinition(walkNode->leftToken->tokenName)) {
									SetErrorMsg(UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0);
									return false;
								} // if : not in definition

								else {
									DefineSymbol temp = GetDefineSymbol(walkNode->leftToken->tokenName);
									if ( temp.binding != NULL ) {
										SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", "/", currentNode, 0, 0);
										return false;
									} // if  : in definition but node

									if ( temp.tokenTypeNum != INT && temp.tokenTypeNum != FLOAT ) {
										SetErrorMsg(PARAMETER_TYPE_ERROR, temp.definedName, "/", NULL, 0, 0);
										return false;
									} // if : in definition but not in float

									else {
										if ((round(atof(temp.definedName.c_str()) * 1000) / 1000) == zero ) {
											SetErrorMsg(DIVISION_BY_ZERO_ERROR, "\0", "\0", NULL, 0, 0);
											return false;
										} // if : defined division is zero
									} // else : function result is a token -> int or float

								} // else : in definition
							} // if : not from quote

							else {
								SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
								return false;
							} // from quote but symbol
						} // if : symbol

						else {
							if ( walkNode->leftToken->tokenTypeNum != INT && walkNode->leftToken->tokenTypeNum != FLOAT ) {
								SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
								return false;
							} // if : function result is a token -> int or float

							else if ( walkNode->leftToken->tokenTypeNum == INT ||
												walkNode->leftToken->tokenTypeNum == FLOAT ) {
								if ((round(atof(walkNode->leftToken->tokenName.c_str()) * 1000) / 1000) == zero ) {
									SetErrorMsg(DIVISION_BY_ZERO_ERROR, "\0", "\0", NULL, 0, 0);
									return false;
								} // if : division is zero
							} // if : function result is a token -> int or float


							if ((walkNode->leftToken->tokenTypeNum == INT || walkNode->leftToken->tokenTypeNum == FLOAT) &&
									walkNode->leftToken->fromQuote ) {
								SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
								return false;
							} // if : int or float but from quote

						} // else : left token -> int or float

					} // if : leftside is a Token

				} // while : check every right node

				return true;
			} // if : num check

			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode != NULL && currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else : num check false
		} // if : /


		else if ( tokenName == "and" || tokenName == "or" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL ) {
				TokenTree *walkNode = currentNode;
				while ( walkNode->rightNode != NULL ) {
					walkNode = walkNode->rightNode;
					if ( walkNode->rightToken != NULL ) {
						SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
						return false;
					} // if : non list check

					if ( walkNode->leftToken != NULL ) {
						if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
							if ( !CheckDefinition(walkNode->leftToken->tokenName)) {
								if ( !walkNode->leftToken->fromQuote ) {
									SetErrorMsg(UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0);
									return false;
								} // if : not from quote Symbol
							} // if : not in definition
						} // if : Check token in definition
					} // if : leftside is a Token

				} // while : check every right node

				return true;
			} // if : num check

			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode != NULL && currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else : num check false
		} // if : and , or


		else if ( tokenName == "string-append" || tokenName == "string>?" || tokenName == "string<?" ||
							tokenName == "string=?" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL ) {
				TokenTree *walkNode = currentNode;
				while ( walkNode->rightNode != NULL ) {
					walkNode = walkNode->rightNode;
					if ( walkNode->rightToken != NULL ) {
						SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
						return false;
					} // if : non list check

					if ( walkNode->leftNode != NULL ) {
						SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", tokenName, walkNode->leftNode, 0, 0);
						return false;
					} // if : left node is a node


					if ( walkNode->leftToken != NULL ) {
						if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
							if ( !walkNode->leftToken->fromQuote ) {
								if ( !CheckDefinition(walkNode->leftToken->tokenName)) {
									SetErrorMsg(UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0);
									return false;
								} // if : not in definition

								else {
									DefineSymbol temp = GetDefineSymbol(walkNode->leftToken->tokenName);
									if ( temp.binding != NULL ) {
										SetErrorMsg(PARAMETER_TYPE_ERROR, "\0", tokenName, currentNode, 0, 0);
										return false;
									} // if  : in definition but node

									if ( temp.tokenTypeNum != STRING ) {
										SetErrorMsg(PARAMETER_TYPE_ERROR, temp.definedName, tokenName, NULL, 0, 0);
										return false;
									} // if : in definition but not in float
								} // else : in definition
							} // if : not from quote

							else {
								SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
								return false;
							} // from quote but symbol
						} // if : symbol

						else if ( walkNode->leftToken->tokenTypeNum != STRING ) {
							SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
							return false;
						} // if : not int or float

						else {
							if ( walkNode->leftToken->tokenTypeNum == STRING && walkNode->leftToken->fromQuote ) {
								SetErrorMsg(PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, tokenName, NULL, 0, 0);
								return false;
							} // if : int or float but from quote
						} // else :  int or float
					} // if : left token null

				} // while : check every right node

				return true;
			} // if : num check

			else {
				if ( currentNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				if ( currentNode->rightNode != NULL && currentNode->rightNode->rightToken != NULL ) {
					SetErrorMsg(NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0);
					return false;
				} // if : non list check

				SetErrorMsg(PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
				return false;
			} // else : num check false
		} // if : string-append, string>?, string<?, string=?

		else if ( tokenName == "eqv?" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL &&
					 currentNode->rightNode->rightNode == NULL )
				return true;
			else return false;
		} // if

		else if ( tokenName == "equal?" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL &&
					 currentNode->rightNode->rightNode == NULL )
				return true;
			else return false;
		} // if

		else if ( tokenName == "begin" ) {
			if ( currentNode->rightNode != NULL )
				return true;
			else return false;
		} // if

		else if ( tokenName == "if" ) {
			if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL ) {
				if ( currentNode->rightNode->rightNode->rightNode != NULL ) {
					if ( currentNode->rightNode->rightNode->rightNode->rightNode != NULL )
						return false;
					else return true;
				} // if

				return true;
			} // if

			else return false;
		} // if

		else if ( tokenName == "cond" ) {
			if ( currentNode->rightNode != NULL ) return true;
			else return false;
		} // if

		else if ( tokenName == "clean-environment" ) {
			if ( currentNode->rightNode != NULL ) return false;
			else return true;
		} // if

		return false;
	} // CheckParameter()*/


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
				currentNode->rightNode->backNode = currentNode;
				InitialNode(currentNode->rightNode);
				Tokens.erase(Tokens.begin());
				CreateTree(currentNode->rightNode, Tokens, false);
				currentNode = currentNode->rightNode;
			} // if : isRight

			else {
				currentNode->leftNode = new TokenTree;
				currentNode->leftNode->backNode = currentNode;
				currentNode->leftNode->needToBePrimitive = true;
				InitialNode( currentNode->leftNode );
				Tokens.erase(Tokens.begin());
				CreateTree( currentNode->leftNode, Tokens, false );
				currentNode = currentNode->leftNode;
			} // else : is left


			while ( Tokens.front().tokenTypeNum == LEFTPAREN || Tokens.front().tokenTypeNum == QUOTE ||
							AtomJudge(Tokens.front().tokenTypeNum)) {
				currentNode->rightNode = new TokenTree;
				currentNode->rightNode->backNode = currentNode;
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
			gTreeRoot->backNode = NULL;
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

    else {
      PrintSExpTree( gTreeRoot, false, layer );
      for ( int i = 0 ; i < layer ; i++ ) cout << ")" << endl;
    } // else


	} // PrintFunctionMsg()


	void PrintErrorMessage() {
		int layer = 0;
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

		else if ( gErrorMsgType == PARAMETER_NUM_ERROR )
			cout << "ERROR (incorrect number of arguments) : " << gErrorMsgName << endl;

		else if ( gErrorMsgType == NO_APPLY_ERROR ) {
			if ( gErrorBinding == NULL )
				cout << "ERROR (attempt to apply non-function) : " << gErrorMsgName << endl;
			else {
				cout << "ERROR (attempt to apply non-function) : ";
				PrintSExpTree(gErrorBinding, false, layer);
				for ( int i = 0 ; i < layer ; i++ ) cout << ")" << endl;
			} // else : not null error
		} // if

		else if ( gErrorMsgType == UNBOND_ERROR )
			cout << "ERROR (unbound symbol) : " << gErrorMsgName << endl;

		else if ( gErrorMsgType == PARAMETER_TYPE_ERROR ) {
			if ( gErrorBinding == NULL )
				cout << "ERROR (" << gErrorFunctionName << " with incorrect argument type) : " << gErrorMsgName
						 << endl;
			else {
				cout << "ERROR (" << gErrorFunctionName << " with incorrect argument type) : ";
				PrintSExpTree(gErrorBinding, false, layer);
				for ( int i = 0 ; i < layer ; i++ ) cout << ")" << endl;
			} // else : not null error
		} // if : parameter type

		else if ( gErrorMsgType == NON_LIST_ERROR ) {
			cout << "ERROR (non-list) : ";
			PrintSExpTree(gErrorBinding, false, layer);
			for ( int i = 0 ; i < layer ; i++ ) cout << ")" << endl;
		} // if

		else if ( gErrorMsgType == FORMAT_ERROR ) {
			cout << "ERROR (DEFINE format) : ";
			PrintSExpTree(gErrorBinding, false, layer);
			for ( int i = 0 ; i < layer ; i++ ) cout << ")" << endl;
		} // if

		else if ( gErrorMsgType == DIVISION_BY_ZERO_ERROR )
			cout << "ERROR (division by zero) : /" << endl;

		else if ( gErrorMsgType == LEVEL_OF_DEFINE )
			cout << "ERROR (level of DEFINE)" << endl;
	} // PrintErrorMessage()

  void PrintEvaluateErrorMessage( int errorType, string errorMsg, TokenTree* errorNode ) {
    int layer = 0 ;
    cout << errorMsg ;
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
		DefineSymbol* defined = NULL ;
		InitialNode( resultNode ) ;
		if ( currentNode->rightNode->leftNode->tokenType != SYMBOL || IsFunction(currentNode->rightNode->leftNode->tokenName ) ) {
			string errorMsg = "ERROR (DEFINE format) : " ;
			throw Exception( FORMAT_ERROR, errorMsg.c_str(), currentNode ) ;
		} // if : first token is not symbol or is a function

		defined->symbolName = currentNode->rightNode->leftNode->tokenName ;
		defined->binding = EvaluateSExp( currentNode->rightNode->rightNode->leftNode ) ;

    gDefineSymbols.push_back()
    
		return resultNode ;
	} // Define()

	/*void List( TokenTree *currentNode ) {
		TokenTree *resultRootNode = NULL;
		TokenTree *resultWalkNode = NULL;
		TokenTree *walkNode = currentNode;

		if ( walkNode->rightNode != NULL ) {                        // root node
			resultRootNode = new TokenTree;
			resultRootNode->backNode = NULL;
			InitialNode(resultRootNode);
			resultWalkNode = resultRootNode;

			walkNode = walkNode->rightNode;
			if ( walkNode->leftToken != NULL ) {
				if ( CheckDefinition(walkNode->leftToken->tokenName)) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					if ( defined.binding != NULL )     // definition is a node
						resultWalkNode->leftNode = defined.binding;
					else {
						resultWalkNode->leftToken = new Token;
						resultWalkNode->leftToken->tokenName = defined.definedName;
						resultWalkNode->leftToken->tokenTypeNum = defined.tokenTypeNum;
					} // else : definition is a token
				} // if : find if in definition

				else resultWalkNode->leftToken = walkNode->leftToken;
			} // if : get token and check definition

			if ( walkNode->leftNode != NULL ) resultWalkNode->leftNode = walkNode->leftNode;
		} // if

		while ( walkNode->rightNode != NULL ) {                      // right node
			resultWalkNode->rightNode = new TokenTree;
			resultWalkNode = resultWalkNode->rightNode;
			InitialNode(resultWalkNode);

			walkNode = walkNode->rightNode;

			if ( walkNode->leftToken != NULL ) {
				if ( CheckDefinition(walkNode->leftToken->tokenName)) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					if ( defined.binding != NULL )     // definition is a node
						resultWalkNode->leftNode = defined.binding;
					else {
						resultWalkNode->leftToken = new Token;
						resultWalkNode->leftToken->tokenName = defined.definedName;
						resultWalkNode->leftToken->tokenTypeNum = defined.tokenTypeNum;
					} // else : definition is a token
				} // if : find if in definition

				else resultWalkNode->leftToken = walkNode->leftToken;
			} // if : get token and check definition

			if ( walkNode->leftNode != NULL ) resultWalkNode->leftNode = walkNode->leftNode;
		} // while

		//----------------connect to tree-------------------//
		ResultConnectToTree(currentNode, resultRootNode, "\0", 0, false);

	} // List()

	void Car( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		TokenTree *resultBinding = NULL;
		TokenTree *walkNode;

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				walkNode = defined.binding;

				if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
					if ( walkNode->leftToken != NULL ) {
						resultTokenName = walkNode->leftToken->tokenName;
						resultTokenType = walkNode->leftToken->tokenTypeNum;
					} // if : left side token
					else if ( walkNode->leftNode != NULL )
						resultBinding = walkNode->leftNode;
				} // if : right side has something -> get left side

				else if ( walkNode->rightToken == NULL && walkNode->rightNode == NULL ) {
					while ( walkNode->leftNode != NULL )
						walkNode = walkNode->leftNode;
					if ( walkNode->leftToken != NULL ) {
						resultTokenName = walkNode->leftToken->tokenName;
						resultTokenType = walkNode->leftToken->tokenTypeNum;
					} // if : find left token
				} // if : right side has nothing -> get first token of left side

			} // if : one token but symbol -> find definition
		} // if : only one token

		else if ( currentNode->rightNode->leftNode != NULL ) {
			walkNode = currentNode->rightNode->leftNode;
			if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
				if ( walkNode->leftToken != NULL ) {
					resultTokenName = walkNode->leftToken->tokenName;
					resultTokenType = walkNode->leftToken->tokenTypeNum;
				} // if : get left side token

				else if ( walkNode->leftNode != NULL ) resultBinding = walkNode->leftNode;
			} // if : right side has something -> get left side

			else if ( walkNode->rightToken == NULL && walkNode->rightNode == NULL ) {
				while ( walkNode->leftNode != NULL ) walkNode = walkNode->leftNode;

				if ( walkNode->leftToken != NULL ) {
					resultTokenName = walkNode->leftToken->tokenName;
					resultTokenType = walkNode->leftToken->tokenTypeNum;
				} // if : find left token
			} // if : right side has nothing -> get first token of left side
		} // if : leftnode

		ResultConnectToTree(currentNode, resultBinding, resultTokenName, resultTokenType, false);

	} // Car()

	void Cdr( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		TokenTree *resultBinding = NULL;
		TokenTree *walkNode;

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				walkNode = defined.binding;

				if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
					if ( walkNode->rightToken != NULL ) {
						resultTokenName = walkNode->rightToken->tokenName;
						resultTokenType = walkNode->rightToken->tokenTypeNum;
					} // if : get right token in left side
					else if ( walkNode->rightNode != NULL )
						resultBinding = walkNode->rightNode;
				} // if : right side has something -> get right side

			} // if : right token is a symbol -> check definition
		} // if : check left token

		else if ( currentNode->rightNode->leftNode != NULL ) {
			walkNode = currentNode->rightNode->leftNode;
			if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
				if ( walkNode->rightToken != NULL ) {
					resultTokenName = walkNode->rightToken->tokenName;
					resultTokenType = walkNode->rightToken->tokenTypeNum;
				} // if : get right token in left side
				else if ( walkNode->rightNode != NULL )
					resultBinding = walkNode->rightNode;
			} // if : right side has something -> get right side
		} // if : check right node if left side


		ResultConnectToTree(currentNode, resultBinding, resultTokenName, resultTokenType, false);
	} // Cdr()

	void Is_Atom( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> nil


		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else {
					if ( AtomJudge(defined.tokenTypeNum)) {
						resultTokenName = "#t";
						resultTokenType = T;
					} // if : Check if is atom
					else {
						resultTokenName = "#t";
						resultTokenType = T;
					} // else
				} // else : definition is a token

			} // if : left token is a symbol

			else if ( AtomJudge(currentNode->rightNode->leftToken->tokenTypeNum)) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : Check if is atom

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else : not atom
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Atom

	void Is_Pair( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "#t";
			resultTokenType = T;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // if : definition is a binding

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : definition is a token

			} // if : definition check
			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else : not symbol -> not definition
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Pair()

	void Is_List( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			if ( FindRightToken(currentNode->rightNode->leftNode)) {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // if : find right token

			else {
				resultTokenName = "#t";
				resultTokenType = T;
			} // else : no right token

		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					if ( FindRightToken(defined.binding)) {
						resultTokenName = "nil";
						resultTokenType = NIL;
					} // if : find right token

					else {
						resultTokenName = "#t";
						resultTokenType = T;
					} // else : no right token
				} // if : definition is a binding

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : definition is a token

			} // if : definition check
			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_List

	void Is_Null( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else if ( defined.tokenTypeNum == NIL ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // else : definition is a token

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : definition check

			else if ( currentNode->rightNode->leftToken->tokenTypeNum == NIL ) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : 2nd token

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Null

	void Is_Int( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else if ( defined.tokenTypeNum == INT ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // else : definition is a token

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : definition check

			else if ( currentNode->rightNode->leftToken->tokenTypeNum == INT ) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : 2nd token

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Int()

	void Is_Real( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else if ( defined.tokenTypeNum == INT || defined.tokenTypeNum == FLOAT ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // else : definition is a token

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : definition check

			else if ( currentNode->rightNode->leftToken->tokenTypeNum == INT ||
								currentNode->rightNode->leftToken->tokenTypeNum == FLOAT ) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : 2nd token

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else :
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Real()

	void Is_Num( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else if ( defined.tokenTypeNum == INT || defined.tokenTypeNum == FLOAT ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // else : definition is a token

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : definition check

			else if ( currentNode->rightNode->leftToken->tokenTypeNum == INT ||
								currentNode->rightNode->leftToken->tokenTypeNum == FLOAT ) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : 2nd token

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Num()

	void Is_Str( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else if ( defined.tokenTypeNum == STRING ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // else : definition is a token

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : definition check

			else if ( currentNode->rightNode->leftToken->tokenTypeNum == STRING ) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : 2nd token

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Str()

	void Is_Bool( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else if ( defined.tokenTypeNum == T || defined.tokenTypeNum == NIL ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // else : definition is a token

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : definition check

			else if ( currentNode->rightNode->leftToken->tokenTypeNum == NIL ||
								currentNode->rightNode->leftToken->tokenTypeNum == T ) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : 2nd token

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Bool

	void Is_Symbol( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				if ( currentNode->rightNode->leftToken->fromQuote ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // if : from Quote

				else if ( CheckDefinition(currentNode->rightNode->leftToken->tokenName)) {
					DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
					if ( defined.tokenTypeNum == SYMBOL ) {
						resultTokenName = "#t";
						resultTokenType = T;
					} // if : in definition and defined is
					else {
						resultTokenName = "nil";
						resultTokenType = NIL;
					} // else : in deifinition but not symbol
				} // else : not nil result

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : is symbol

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else : not symbol & not nil
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Symbol()

	void Plus( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		int resultInt = 0;
		float inputNum = 0.0;
		float resultFloat = 0.0;
		bool isFloat = false;
		stringstream sstream;
		TokenTree *walkNode = currentNode;

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;

			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					inputNum = round(atof(defined.definedName.c_str()) * 1000) / 1000;
				} // if : define type

				else inputNum = round(atof(walkNode->leftToken->tokenName.c_str()) * 1000) / 1000;

				resultFloat = inputNum + resultFloat;

				if ( walkNode->leftToken->tokenTypeNum == FLOAT ) isFloat = true;
			} // if : leftNode -> function result

		} // while: walkNode go to right node

		if ( isFloat ) {
			sstream << resultFloat;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = FLOAT;
		} // if : float result
		else {
			resultInt = (int) resultFloat;
			sstream << resultInt;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = INT;
		} // else : int result

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Plus()

	void Minus( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		int resultInt = 0;
		float inputNum = 0.0;
		float resultFloat = 0.0;
		bool isFloat = false;
		stringstream sstream;
		TokenTree *walkNode = currentNode;

		walkNode = walkNode->rightNode;

		if ( walkNode->leftToken != NULL ) {
			if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
				resultFloat = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define type

			else resultFloat = round(atof(walkNode->leftToken->tokenName.c_str()) * 1000) / 1000;

			if ( walkNode->leftToken->tokenTypeNum == FLOAT ) {
				isFloat = true;
			} // if : float result
		} // if : leftNode -> function result

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;

			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					inputNum = round(atof(defined.definedName.c_str()) * 1000) / 1000;
				} // if : define type

				else inputNum = round(atof(walkNode->leftToken->tokenName.c_str()) * 1000) / 1000;

				resultFloat = resultFloat - inputNum;

				if ( walkNode->leftToken->tokenTypeNum == FLOAT ) {
					isFloat = true;
				} // if : float result
			} // if : leftNode -> function result

		} // while: walkNode go to right node

		if ( isFloat ) {
			sstream << resultFloat;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = FLOAT;
		} // if : float result
		else {
			resultInt = (int) resultFloat;
			sstream << resultInt;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = INT;
		} // else : int result

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Minus

	void Div( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		int resultInt = 0;
		float inputNum = 0.0;
		float resultFloat = 0.0;
		bool isFloat = false;
		stringstream sstream;
		TokenTree *walkNode = currentNode;

		walkNode = walkNode->rightNode;

		if ( walkNode->leftToken != NULL ) {
			if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
				resultFloat = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define type

			else resultFloat = round(atof(walkNode->leftToken->tokenName.c_str()) * 1000) / 1000;

			if ( walkNode->leftToken->tokenTypeNum == FLOAT ) {
				isFloat = true;
			} // if : float result
		} // if : leftNode -> function result

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;

			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					inputNum = round(atof(defined.definedName.c_str()) * 1000) / 1000;
				} // if : define type

				else inputNum = round(atof(walkNode->leftToken->tokenName.c_str()) * 1000) / 1000;

				resultFloat = resultFloat / inputNum;

				if ( walkNode->leftToken->tokenTypeNum == FLOAT ) {
					isFloat = true;
				} // if : float result
			} // if : leftNode -> function result

		} // while: walkNode go to right node

		if ( isFloat ) {
			sstream << resultFloat;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = FLOAT;
		} // if : float result
		else {
			resultInt = (int) resultFloat;
			sstream << resultInt;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = INT;
		} // else : int result

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Div()

	void Mult( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		int resultInt = 0;
		float inputNum = 0.0;
		float resultFloat = 1.0;
		bool isFloat = false;
		stringstream sstream;
		TokenTree *walkNode = currentNode;

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;

			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					inputNum = round(atof(defined.definedName.c_str()) * 1000) / 1000;
				} // if : define type

				else inputNum = round(atof(walkNode->leftToken->tokenName.c_str()) * 1000) / 1000;

				resultFloat = inputNum * resultFloat;

				if ( walkNode->leftToken->tokenTypeNum == FLOAT ) isFloat = true;
			} // if : leftNode -> function result

		} // while: walkNode go to right node

		if ( isFloat ) {
			sstream << resultFloat;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = FLOAT;
		} // if : float result
		else {
			resultInt = (int) resultFloat;
			sstream << resultInt;
			string resultString = sstream.str();
			resultTokenName = resultString;
			resultTokenType = INT;
		} // else : int result

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Mult()

	void Not( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;

		if ( currentNode->rightNode->leftNode != NULL ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // if : left is a node -> check function result

		if ( currentNode->rightNode->leftToken != NULL ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				if ( defined.binding != NULL ) {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // if : definition is a binding

				else if ( defined.tokenTypeNum == NIL ) {
					resultTokenName = "#t";
					resultTokenType = T;
				} // else : definition is a token

				else {
					resultTokenName = "nil";
					resultTokenType = NIL;
				} // else : not nil result

			} // if : definition check

			else if ( currentNode->rightNode->leftToken->tokenTypeNum == NIL ) {
				resultTokenName = "#t";
				resultTokenType = T;
			} // if : 2nd token

			else {
				resultTokenName = "nil";
				resultTokenType = NIL;
			} // else
		} // if : left token has something

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Not()

	void And( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		TokenTree *walkNode = currentNode;
		bool isNil = false;
		if ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;
			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == NIL ) isNil = true;
			} // if : left token
		} // if : first one that is evaluated to nil

		if ( isNil ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
			ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
			return;
		} // if : is nil result

		else {
			while ( walkNode->rightNode != NULL ) walkNode = walkNode->rightNode;
			if ( walkNode->leftNode != NULL ) {
				ResultConnectToTree(currentNode, walkNode->leftNode, "\0", NO_TYPE, false);
			} // if : last is node
			else if ( walkNode->leftToken != NULL ) {
				resultTokenName = walkNode->leftToken->tokenName;
				resultTokenType = walkNode->leftToken->tokenTypeNum;
				ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
			} // else : last is token
		} // else : not nil result
	} // And()

	void Or( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		TokenTree *walkNode = currentNode;
		bool isT = false;
		if ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;
			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == T ) isT = true;
			} // if : left token
		} // if : first one that is evaluated to nil

		if ( isT ) {
			resultTokenName = "#t";
			resultTokenType = T;
			ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
			return;
		} // if : is nil result

		else {
			if ( walkNode->leftNode != NULL ) {
				ResultConnectToTree(currentNode, walkNode->leftNode, "\0", NO_TYPE, false);
			} // if : last is node
			else if ( walkNode->leftToken != NULL ) {
				resultTokenName = walkNode->leftToken->tokenName;
				resultTokenType = walkNode->leftToken->tokenTypeNum;
				ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
			} // else : last is token
		} // else : not nil result
	} // Or()

	void Greater( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		float compare1 = 0.0;
		float compare2 = 0.0;

		if ( currentNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				compare1 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else compare1 = round(atof(currentNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : first token

		if ( currentNode->rightNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->rightNode->leftToken->tokenName);
				compare2 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else
				compare2 = round(atof(currentNode->rightNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : second token

		if ( compare1 > compare2 ) {
			resultTokenName = "#t";
			resultTokenType = T;
		} // if : true

		else {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // else : false

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);

	} // Greater()

	void GreaterEqual( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		float compare1 = 0.0;
		float compare2 = 0.0;

		if ( currentNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				compare1 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else compare1 = round(atof(currentNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : first token

		if ( currentNode->rightNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->rightNode->leftToken->tokenName);
				compare2 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else
				compare2 = round(atof(currentNode->rightNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : second token

		if ( compare1 >= compare2 ) {
			resultTokenName = "#t";
			resultTokenType = T;
		} // if : true

		else {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // else : false

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // GreaterEqual()

	void Less( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		float compare1 = 0.0;
		float compare2 = 0.0;

		if ( currentNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				compare1 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else compare1 = round(atof(currentNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : first token

		if ( currentNode->rightNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->rightNode->leftToken->tokenName);
				compare2 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else
				compare2 = round(atof(currentNode->rightNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : second token

		if ( compare1 < compare2 ) {
			resultTokenName = "#t";
			resultTokenType = T;
		} // if : true

		else {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // else : false

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Less()

	void LessEqual( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		float compare1 = 0.0;
		float compare2 = 0.0;

		if ( currentNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				compare1 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else compare1 = round(atof(currentNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : first token

		if ( currentNode->rightNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->rightNode->leftToken->tokenName);
				compare2 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else
				compare2 = round(atof(currentNode->rightNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : second token

		if ( compare1 <= compare2 ) {
			resultTokenName = "#t";
			resultTokenType = T;
		} // if : true

		else {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // else : false

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // LessEqual()

	void Equal( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		float compare1 = 0.0;
		float compare2 = 0.0;

		if ( currentNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->leftToken->tokenName);
				compare1 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else compare1 = round(atof(currentNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : first token

		if ( currentNode->rightNode->rightNode->leftToken ) {
			if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(currentNode->rightNode->rightNode->leftToken->tokenName);
				compare2 = round(atof(defined.definedName.c_str()) * 1000) / 1000;
			} // if : define symbol

			else
				compare2 = round(atof(currentNode->rightNode->rightNode->leftToken->tokenName.c_str()) * 1000) / 1000;
		} // if : second token

		if ( compare1 == compare2 ) {
			resultTokenName = "#t";
			resultTokenType = T;
		} // if : true

		else {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // else : false

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Equal()

	void Str_Append( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = STRING;
		TokenTree *walkNode = currentNode;

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;
			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					resultTokenName = resultTokenName + defined.definedName;
				} // if : left token is symbol

				else resultTokenName = resultTokenName + walkNode->leftToken->tokenName;
			} // if : left token
		} // while : walk every node

		resultTokenName.erase(remove(resultTokenName.begin(), resultTokenName.end(), '"'), resultTokenName.end());
		resultTokenName = "\"" + resultTokenName;
		resultTokenName = resultTokenName + "\"";
		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Str_Append()

	void Is_Str_Greater( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		string compare1 = "\0";
		string compare2 = "\0";
		TokenTree *walkNode = currentNode;
		bool isNil = false;

		walkNode = walkNode->rightNode;
		if ( walkNode->leftToken != NULL ) {
			if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
				compare1 = defined.definedName;
			} // if : left token is symbol

			else compare1 = walkNode->leftToken->tokenName;
		} // if : left token

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;
			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					compare2 = defined.definedName;
				} // if : left token is symbol

				else compare2 = walkNode->leftToken->tokenName;
			} // if : left token

			if ( strcmp(compare1.c_str(), compare2.c_str()) <= 0 ) isNil = true;
			compare1 = compare2;
		} // while : walk all node

		if ( isNil ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // is false

		else {
			resultTokenName = "#t";
			resultTokenType = T;
		} // is true

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Str_Greater()

	void Is_Str_Less( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		string compare1 = "\0";
		string compare2 = "\0";
		TokenTree *walkNode = currentNode;
		bool isNil = false;

		walkNode = walkNode->rightNode;
		if ( walkNode->leftToken != NULL ) {
			if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
				compare1 = defined.definedName;
			} // if : left token is symbol

			else compare1 = walkNode->leftToken->tokenName;
		} // if : left token

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;
			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					compare2 = defined.definedName;
				} // if : left token is symbol

				else compare2 = walkNode->leftToken->tokenName;
			} // if : left token

			if ( strcmp(compare1.c_str(), compare2.c_str()) >= 0 ) isNil = true;
			compare1 = compare2;
		} // while : walk all node

		if ( isNil ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // is false

		else {
			resultTokenName = "#t";
			resultTokenType = T;
		} // is true

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Str_Less()

	void Is_Str_Equal( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
		string compare1 = "\0";
		string compare2 = "\0";
		TokenTree *walkNode = currentNode;
		bool isNil = false;

		walkNode = walkNode->rightNode;
		if ( walkNode->leftToken != NULL ) {
			if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
				DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
				compare1 = defined.definedName;
			} // if : left token is symbol

			else compare1 = walkNode->leftToken->tokenName;
		} // if : left token

		while ( walkNode->rightNode != NULL ) {
			walkNode = walkNode->rightNode;
			if ( walkNode->leftToken != NULL ) {
				if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
					DefineSymbol defined = GetDefineSymbol(walkNode->leftToken->tokenName);
					compare2 = defined.definedName;
				} // if : left token is symbol

				else compare2 = walkNode->leftToken->tokenName;
			} // if : left token

			if ( strcmp(compare1.c_str(), compare2.c_str()) != 0 ) isNil = true;
			compare1 = compare2;
		} // while : walk all node

		if ( isNil ) {
			resultTokenName = "nil";
			resultTokenType = NIL;
		} // is false

		else {
			resultTokenName = "#t";
			resultTokenType = T;
		} // is true

		ResultConnectToTree(currentNode, NULL, resultTokenName, resultTokenType, false);
	} // Is_Str_Equal()

	void Is_Eqv( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Is_Equal()

	void Is_Equal( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Is_Equal()

	void Begin( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Begin()

	void If( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // If()

	void Cond( TokenTree *currentNode ) {
		string resultTokenName = "\0";
		int resultTokenType = NO_TYPE;
	} // Cond()

	void Clean_Env( TokenTree *currentNode ) {
		gDefineSymbols.clear();
		ResultConnectToTree(currentNode, NULL, "environment cleaned", NO_TYPE, false);
	} // Clear_Env()*/

	TokenTree* FindCorrespondFunction( TokenTree *currentNode, string tokenName ) {
		if ( tokenName == "cons" ) return Cons( currentNode );
    else if ( tokenName == "quote" ) return Quote( currentNode );
		else if ( tokenName == "define" ) return Define(currentNode);
		/*else if ( tokenName == "list" ) return List(currentNode);
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
		else if ( tokenName == "eqv?" ) return Is_Eqv(currentNode);
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
          DefineSymbol * defined = GetDefineSymbol( currentNode->tokenName ) ;
          currentNode = defined->binding ;
        } // if : is in definition
        
        else {
          string errorMsg = "ERROR (unbound symbol) : " + currentNode->tokenName ;
          throw Exception( UNBOND_ERROR, errorMsg.c_str(), NULL ) ;
        } // else : throw exception
        
      } // if : if SYMBOL->check definition
    } // if : left token
    
    else {
      CheckNonList( currentNode ) ;

      if ( currentNode->needToBePrimitive == true ) {
      	if ( currentNode->leftNode->tokenType == SYMBOL ) {
					if ( CheckDefinition( currentNode->leftNode->tokenName ) ) {
						DefineSymbol * defined = GetDefineSymbol( currentNode->leftNode->tokenName ) ;
						if ( defined->binding->tokenName != "\0" ) {
							currentNode->leftNode = defined->binding ;
						} // if : define is a token

						else {
							string errorMsg = "ERROR (attempt to apply non-function) : " ;
							throw Exception( NO_APPLY_ERROR, errorMsg.c_str(), defined->binding ) ;
						} // else : define is a node-> error
					} // if : is in definition

					else {
            if ( !IsFunction( currentNode->leftNode->tokenName ) ) {
              string errorMsg = "ERROR (unbound symbol) : " + currentNode->leftNode->tokenName ;
              throw Exception( UNBOND_ERROR, errorMsg.c_str(), NULL ) ;
            } // if
					} // else : throw exception
      	} // if : if is SYMBOL　

        if ( IsFunction( currentNode->leftNode->tokenName ) ) {
          currentNode = FindCorrespondFunction( currentNode, currentNode->leftNode->tokenName ) ;
        } // if : check is Function
        
        else {
          string errorMsg = "ERROR (attempt to apply non-function) : " + currentNode->leftNode->tokenName ;
          throw Exception( NO_APPLY_ERROR, errorMsg.c_str(), NULL ) ;
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
						project.PrintEvaluateErrorMessage( e.mErrorType, e.mErrorMsg, e.mErrorNode ) ;
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
