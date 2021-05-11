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
}; // TokenType

struct TokenTree {
	Token *leftToken;
	Token *rightToken;
	TokenTree *leftNode;
	TokenTree *rightNode;
	TokenTree *backNode;
  bool NeedToBePrimitive;
}; // TokenType

struct DefineSymbol {
	string symbolName;
	TokenTree *binding;
}; // DefineSymbol Symbol

struct Result {
  Token resultStruct ;
  TokenTree *resultBinding;
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
	INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, LEFTPAREN, RIGHTPAREN
}; // TokenType

enum Error {
	LEFT_ERROR, RIGHT_ERROR, CLOSE_ERROR, EOF_ERROR, NO_ERROR, NOT_S_EXP_ERROR,
  PARAMETER_NUM_ERROR, PARAMETER_TYPE_ERROR, UNBOND_ERROR, NO_APPLY_ERROR,
  NO_RETURN_VAL_ERROR, DIVISION_BY_ZERO_ERROR, FORMAT_ERROR, NON_LIST_ERROR
}; // Error

vector<Token> gTokens;
vector<DefineSymbol> gDefineSymbols;
vector<Result> gResultList ;
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

// ------------------Setting Function--------------------- //

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

void SetErrorMsg( int errorType, string errorToken, string errorFunction, TokenTree* errorBinding, int line, int column ) {
	gErrorMsgType = errorType;
	gErrorMsgName = errorToken;
  gErrorFunctionName = errorFunction ;
  gErrorBinding = errorBinding ;
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
  gErrorBinding = NULL ;
	gAtomType = 0;
  gResultList.clear();
} // GlobalVariableReset()

void InitialResult( Result & result) {
  result.resultStruct.tokenName = "\0" ;
  result.resultBinding = NULL ;
} //InitialResult()


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

bool CheckDefinition( string tokenName ) {
  for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
    if ( tokenName == gDefineSymbols[i].symbolName ) return true ;
  } // for

  SetErrorMsg( UNBOND_ERROR, tokenName, "\0", NULL, 0, 0 ) ;
  return false;
}

void PushDefinitionToResultList( string tokenName ) {
  Result result ;
  InitialResult( result ) ;
  for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
    if ( tokenName == gDefineSymbols[i].symbolName ) {
      if ( gDefineSymbols[i].binding->rightNode == NULL && gDefineSymbols[i].binding->leftNode == NULL ) {
        result.resultStruct.tokenName = gDefineSymbols[i].binding->leftToken->tokenName ; // single node
        result.resultStruct.tokenTypeNum = gDefineSymbols[i].binding->leftToken->tokenTypeNum ;
      } // if
      else result.resultBinding = gDefineSymbols[i].binding ;
      gResultList.push_back( result ) ;
    } // if
  } // for
}  // PushDefinitionToResultList()




bool CheckParameter( TokenTree * currentNode, string tokenName ) {
  if ( tokenName == "cons" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL &&
         currentNode->rightNode->rightNode->rightNode == NULL ) {
      
      if ( currentNode->rightNode->rightNode->rightToken != NULL   ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0) ;
        return false;
      } // non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
						SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : not in definition
        } // if
      } // 1st node type check
      
      if ( currentNode->rightNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->rightNode->leftToken->tokenName ) ) {
						SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 );
						return false;
					} // if : not in definition
        } // if
      } // 2nd node type check
      
      return true ;
    } // if

		else {
      
      if ( currentNode->rightToken != NULL || currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
			SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
			return false;
		} // else : num of parameter error
  } // if

  else if ( tokenName == "list" ) {
    TokenTree * walkNode = currentNode ;
    bool inDefinition = false ;
    while ( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode ;
      if ( walkNode->leftToken != NULL ) {
        if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
          for ( int i = 0 ; i < gDefineSymbols.size(); i++ ) {
            if ( walkNode->leftToken->tokenName == gDefineSymbols[i].symbolName )
              inDefinition = true ;
          } // for
          
          if ( !inDefinition ) {
            SetErrorMsg( UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false;
          } // if : not in definition -> return false
          
          inDefinition = false ;
        } //if : left token is not null
        
        if ( walkNode->rightToken != NULL ) {
          SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0) ;
          return false;
        } // if : ( list 3 . 4 ) case
      } // if : check symbol in define set
      
    } // while : check right node type
    
    
    return true ;
  } // if : list
  
  else if ( tokenName == "quote" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      
      if ( currentNode->rightNode->rightToken != NULL   ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0) ;
        return false;
      } // non list check
      
      return true ;
    } // if
    else {
      if ( currentNode->rightToken != NULL   ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0) ;
        return false;
      } // non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else : num error
  } // if

  else if ( tokenName == "define" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL && currentNode->rightNode->rightNode->rightNode == NULL ) {
      
      if ( currentNode->rightNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum != SYMBOL ) {
          SetErrorMsg( PARAMETER_TYPE_ERROR, currentNode->rightNode->leftToken->tokenName, "define", NULL, 0, 0) ;
          return false;
        } // if : type error
        
        else return true ;
      } // if
      
      else {
        SetErrorMsg( PARAMETER_TYPE_ERROR, currentNode->leftToken->tokenName, "define", NULL, 0, 0) ; // qoute or dot-piar behind define
        return false ;
      } // else : type error
      
    } // if : parameter num check
    
    else {
      if ( currentNode->rightToken != NULL || currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else : Num error
  } // if : define

  else if ( tokenName == "car" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL  ) {
      
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0) ;
        return false;
      } // if : not list
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : error-> not in definition
          
          else {
            PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
            if ( gResultList.back().resultBinding == NULL ) {
              SetErrorMsg( PARAMETER_TYPE_ERROR, gResultList.back().resultStruct.tokenName, "car", NULL, 0, 0 ) ;
              return false;
            } // if  : in definition but not node
            
            gResultList.pop_back() ;
          } // else : in definition
        } // if : symbol type
        
        else {
          SetErrorMsg( PARAMETER_TYPE_ERROR, currentNode->rightNode->leftToken->tokenName, "car", NULL, 0, 0 ) ;
          return false;
        } // else : not symbol
      } // if : Check symbol in definition
      
      return true;
    } // if : num check
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else : num error
  } // if
  
  else if ( tokenName == "cdr" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL  ) {
      
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0) ;
        return false;
      } // if : not list
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : error-> not in definition
          
          else {
            PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
            if ( gResultList.back().resultBinding == NULL ) {
              SetErrorMsg( PARAMETER_TYPE_ERROR, gResultList.back().resultStruct.tokenName, "cdr", NULL, 0, 0 ) ;
              return false;
            } // if  : in definition but not node
            
            gResultList.pop_back() ;
          } // else : in definition
        } // if : symbol type
        
        else {
          SetErrorMsg( PARAMETER_TYPE_ERROR, currentNode->rightNode->leftToken->tokenName, "cdr", NULL, 0, 0 ) ;
          return false;
        } // else : not symbol
      } // if : Check symbol in definition
      
      return true;
    } // if : num check
    
    else{
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else : num error
    
  } // if

  else if ( tokenName == "atom?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( )
      
      
      return true ;
    } // if
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "pair?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "null?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "integer?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "real?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "number?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "string?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "boolean?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "symbol?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "+" ) {
    if ( currentNode->rightNode == NULL && currentNode->rightNode->rightNode != NULL )
      return false ;
    else return true ;
  } // if

  else if ( tokenName == "-" ) {
    if ( currentNode->rightNode == NULL && currentNode->rightNode->rightNode != NULL )
      return false ;
    else return true ;
  } // if

  else if ( tokenName == "*" ) {
    if ( currentNode->rightNode == NULL && currentNode->rightNode->rightNode != NULL )
      return false ;
    else return true ;
  } // if

  else if ( tokenName == "/" ) {
    if ( currentNode->rightNode == NULL && currentNode->rightNode->rightNode != NULL )
      return false ;
    else return true ;
  } // if

  else if ( tokenName == "not" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "and" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "or" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == ">" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == ">=" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "<" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "<=" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "=" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "string-append" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "string>?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "string<?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "string=?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "eqv?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "equal?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL )
      return true ;
    else return false ;
  } // if

  else if ( tokenName == "begin" ) {
    if ( currentNode->rightNode != NULL )
      return true ;
    else return false ;
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

    else return false ;
  } // if

  else if ( tokenName == "cond" ) {
    if ( currentNode->rightNode != NULL ) return true ;
    else return false ;
  } // if

  else if ( tokenName == "clear-environment" ) {
    if ( currentNode->rightNode != NULL ) return false ;
    else return true ;
  } // if

  return false ;
} // CheckParameter()


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
			SetErrorMsg(EOF_ERROR, "\"", "\0", NULL, 0, 0);
		else if ( peek == '\n' )
			SetErrorMsg(CLOSE_ERROR, "\"", "\0", NULL, gLine, gColumn + 1);
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

	if ( peek == EOF) {
		SetErrorMsg(EOF_ERROR, " ", "\0", NULL, 0, 0);
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
				SetErrorMsg(CLOSE_ERROR, "\"", "\0", NULL, 0, 0);
				return false;
			} // if
		} // else

		token.tokenTypeNum = gAtomType;
		gTokens.push_back(token);

		return true;
	} // else

} // GetToken()

// ------------------Tree Build--------------------- //


void InitialNode( TokenTree * currentNode ) {
	currentNode->leftNode = NULL;
	currentNode->leftToken = NULL;
	currentNode->rightNode = NULL;
	currentNode->rightToken = NULL;
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
        
        if ( gCurrentNode->backNode != NULL ) {
          if ( gCurrentNode->backNode->leftToken != NULL ) {
            if ( gCurrentNode->backNode->leftToken->tokenTypeNum == QUOTE )
              gCurrentNode = gCurrentNode->backNode;
            while ( gCurrentNode->rightNode != NULL )                         // find right node null
              gCurrentNode = gCurrentNode->backNode;
          } // if
        } // if
        

				if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {                 // if !dot-> create right node
					gCurrentNode->rightNode = new TokenTree;                       // and insert to left token
					gCurrentNode->rightNode->backNode = gCurrentNode;
					gCurrentNode = gCurrentNode->rightNode;
          gCurrentNode->NeedToBePrimitive = false ;
					InitialNode( gCurrentNode );
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
      
      if ( gCurrentNode->backNode != NULL ) {
        if ( gCurrentNode->backNode->leftToken != NULL ) {
          if ( gCurrentNode->backNode->leftToken->tokenTypeNum == QUOTE )
            gCurrentNode = gCurrentNode->backNode;
          while ( gCurrentNode->rightNode != NULL )                         // find right node null
            gCurrentNode = gCurrentNode->backNode;
        } // if
      } // if

			if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {                 // if !dot-> create right node
				gCurrentNode->rightNode = new TokenTree;                       // and insert to left token
				gCurrentNode->rightNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->rightNode;
        gCurrentNode->NeedToBePrimitive = false ;
				InitialNode( gCurrentNode );
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
		gCurrentNode->backNode = NULL ;
    gCurrentNode->NeedToBePrimitive = true ;
		InitialNode( gCurrentNode );
	} // if

	else {
		if ( gCurrentNode->leftToken == NULL ) {                // left token null
			if ( gCurrentNode->leftNode == NULL ) {               // left node null-> create node
				gCurrentNode->leftNode = new TokenTree;
				gCurrentNode->leftNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->leftNode;
        gCurrentNode->NeedToBePrimitive = true ;
				InitialNode( gCurrentNode );
			} // if

			else if ( gCurrentNode->leftNode != NULL ) {          // left node !null
				while ( gCurrentNode->rightNode != NULL )           // find right node null-> create node
					gCurrentNode = gCurrentNode->backNode;
        
        if ( gCurrentNode->backNode != NULL ) {
          if ( gCurrentNode->backNode->leftToken != NULL ) {
            if ( gCurrentNode->backNode->leftToken->tokenTypeNum == QUOTE )
              gCurrentNode = gCurrentNode->backNode;
            while ( gCurrentNode->rightNode != NULL )                         // find right node null
              gCurrentNode = gCurrentNode->backNode;
          } // if
        } // if

				gCurrentNode->rightNode = new TokenTree;
				gCurrentNode->rightNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->rightNode;
        gCurrentNode->NeedToBePrimitive = false ;
				InitialNode( gCurrentNode );

				if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {    // if !dot-> create left node
					gCurrentNode->leftNode = new TokenTree;
					gCurrentNode->leftNode->backNode = gCurrentNode;
					gCurrentNode = gCurrentNode->leftNode;
          gCurrentNode->NeedToBePrimitive = true ;
					InitialNode( gCurrentNode );
				} // if

			} // if
		} // if

		else if ( gCurrentNode->leftToken != NULL ) {         // left token !null
			while ( gCurrentNode->rightNode != NULL )           // find right node null-> create node
				gCurrentNode = gCurrentNode->backNode;
      
      if ( gCurrentNode->backNode != NULL ) {
        if ( gCurrentNode->backNode->leftToken != NULL ) {
          if ( gCurrentNode->backNode->leftToken->tokenTypeNum == QUOTE )
            gCurrentNode = gCurrentNode->backNode;
          while ( gCurrentNode->rightNode != NULL )        // find right node null and above quote
            gCurrentNode = gCurrentNode->backNode;
        } // if
      } // if

			gCurrentNode->rightNode = new TokenTree;                       // and create right node
			gCurrentNode->rightNode->backNode = gCurrentNode;
			gCurrentNode = gCurrentNode->rightNode;
      gCurrentNode->NeedToBePrimitive = false;
			InitialNode( gCurrentNode );

			if ( gTokens[gTokens.size() - 2].tokenTypeNum != DOT ) {                 // if !dot-> create left node
				gCurrentNode->leftNode = new TokenTree;
				gCurrentNode->leftNode->backNode = gCurrentNode;
				gCurrentNode = gCurrentNode->leftNode;
        gCurrentNode->NeedToBePrimitive = true ;
				InitialNode( gCurrentNode );
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
                          "\0", NULL, gTokens.back().tokenLine, gTokens.back().tokenColumn);
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
                    "\0", NULL, gTokens.back().tokenLine, gTokens.back().tokenColumn);
				return false;
			} // else
		} // if

		else {
			if ( gErrorMsgType == NOT_S_EXP_ERROR ) {
				SetErrorMsg(LEFT_ERROR, gTokens.back().tokenName,
                    "\0", NULL, gTokens.back().tokenLine, gTokens.back().tokenColumn);
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
                    "\0", NULL, gTokens.back().tokenLine, gTokens.back().tokenColumn);
				return false;
			} // else
		} // if

		else return false;
	} // if

	else if ( gTokens.back().tokenTypeNum == QUOTE && gTokens.back().tokenName == "quote" ) {
		// cout << "Quote " ;
		Token temp;
		InsertAtomToTree() ;

		if ( GetToken()) {
			if ( SyntaxChecker()) return true;
			else {
				SetErrorMsg(LEFT_ERROR, gTokens.back().tokenName,
                    "\0", NULL, gTokens.back().tokenLine, gTokens.back().tokenColumn);
				return false;
			} // else
		} // if

		else return false;
	} // if

	SetErrorMsg(NOT_S_EXP_ERROR, gTokens.back().tokenName,
              "\0", NULL, gTokens.back().tokenLine, gTokens.back().tokenColumn);
	return false;

} // SyntaxChecker()

// ------------------Print Function--------------------- //


void PrintSExpTree( TokenTree * currentNode, bool isRightNode, int & layer ) {
static bool lineReturn = false ;
  if ( !isRightNode ) {
    cout << "( " ;
    lineReturn = false ;
    layer ++ ;
  } // if
  
  if ( lineReturn )
    for( int i = 0; i < layer; i++ ) cout << "  " ;
  
  if ( currentNode->leftToken != NULL ) {
    cout << currentNode->leftToken->tokenName << endl ;
    lineReturn = true ;
  } // if
  
  if ( currentNode->leftNode ) PrintSExpTree( currentNode->leftNode, false, layer ) ;
  if ( currentNode->rightNode ) PrintSExpTree( currentNode->rightNode, true, layer ) ;

	if ( currentNode->rightToken != NULL && currentNode->rightToken->tokenTypeNum != NIL ) {
		for( int i = 0; i < layer; i++ ) cout << "  " ;
		cout << "." << endl ;
		for( int i = 0; i < layer; i++ ) cout << "  " ;
		cout << currentNode->rightToken->tokenName << endl ;
		lineReturn = true ;
	} // if

  if ( layer > 1  ) {
    lineReturn = true ;
    layer -- ;
    for( int i = 0; i < layer; i++ ) cout << "  " ;
    cout << ")" << endl ;
  } // if
} // PrintSExpTree()


void PrintFunctionMsg() {
  int layer = 0;
  if( gResultList.size() > 0 ) {
    if ( gResultList.back().resultStruct.tokenName != "\0" ){
      if ( gResultList.back().resultStruct.tokenTypeNum == FLOAT ) {
        cout << fixed << setprecision(3)
             << round( atof( gResultList.back().resultStruct.tokenName.c_str() ) * 1000 ) / 1000 << endl;
      } // if
      else cout << gResultList.back().resultStruct.tokenName << endl;
    } // if : Print Msg or atom
  
    else if ( gResultList.back().resultBinding != NULL ) {
      PrintSExpTree( gResultList.back().resultBinding, false, layer ) ;
      for ( int i = 0; i < layer; i++ ) cout << ")" << endl ;
    } // if
  } // if

  else {
    if ( gTokens[0].tokenTypeNum == FLOAT ) {
      cout << fixed << setprecision(3)
           << round( atof( gTokens[0].tokenName.c_str() ) * 1000 ) / 1000 << endl;
    } // if : float print
    else cout << gTokens[0].tokenName << endl;
  } // else : only one token and not symbol
} // PrintFunctionMsg()


void PrintErrorMessage() {
  int layer = 0 ;
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
		cout << "ERROR (incorrect number of arguments) : " << gErrorMsgName << endl ;
	else if ( gErrorMsgType == NO_APPLY_ERROR )
			cout << "ERROR (attempt to apply non-function) : " << gErrorMsgName << endl;
	else if ( gErrorMsgType == UNBOND_ERROR )
		cout << "ERROR (unbound symbol) : "<< gErrorMsgName << endl;
	else if ( gErrorMsgType == PARAMETER_TYPE_ERROR )
		cout << "ERROR (" << gErrorFunctionName << " with incorrect argument type) : " << gErrorMsgName << endl ;
  else if ( gErrorMsgType == NON_LIST_ERROR ) {
    cout << "ERROR (non-list) : "  ;
    PrintSExpTree( gErrorBinding, false, layer ) ;
    for ( int i = 0; i < layer; i++ ) cout << ")" << endl ;
  } // if

} // PrintErrorMessage()

// ------------------Functional Function--------------------- //

void Cons( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result );
  TokenTree* resultRootNode = NULL;
  resultRootNode = new TokenTree ;
  resultRootNode->backNode = NULL ;
  InitialNode( resultRootNode ) ;
  
  if ( currentNode->rightNode->leftNode != NULL ) {
		if ( gResultList.size() != 0 && gResultList.back().resultBinding != NULL ) {
			resultRootNode->leftNode = gResultList.back().resultBinding;
			gResultList.pop_back();
		} // if : function result
    
    else if ( gResultList.back().resultStruct.tokenName != "\0" ){
      resultRootNode->leftToken = new Token  ;
      resultRootNode->leftToken->tokenName = gResultList.back().resultStruct.tokenName ;
      resultRootNode->leftToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
      gResultList.pop_back();
    } // function result is a token
	} // if : connect left node

	else if ( currentNode->rightNode->leftToken != NULL ) {
		if ( CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
			PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName );
			if ( gResultList.back().resultBinding != NULL ) {
				resultRootNode->leftNode = gResultList.back().resultBinding;
			} // if : definition is a node

			else {
				resultRootNode->leftToken = new Token ;
				resultRootNode->leftToken->tokenName = gResultList.back().resultStruct.tokenName ;
        resultRootNode->leftToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
			} // if : definition is a token
			gResultList.pop_back();
		} // if : check if is in definition
		else resultRootNode->leftToken = currentNode->rightNode->leftToken ;
	} // else : only left token

	if ( currentNode->rightNode->rightNode->leftNode != NULL ) {
		if ( gResultList.size() != 0 && gResultList.back().resultBinding != NULL ) {
			resultRootNode->rightNode = gResultList.back().resultBinding;
			gResultList.pop_back();
		} // if : function result
    
    else if ( gResultList.back().resultStruct.tokenName != "\0" ){
      resultRootNode->rightToken = new Token  ;
      resultRootNode->rightToken->tokenName = gResultList.back().resultStruct.tokenName ;
      resultRootNode->rightToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
      gResultList.pop_back();
    } // function result is a token
	} // if : connect right node

	else if ( currentNode->rightNode->rightNode->leftToken != NULL ) {
		if ( CheckDefinition(currentNode->rightNode->rightNode->leftToken->tokenName) ) {
			PushDefinitionToResultList( currentNode->rightNode->rightNode->leftToken->tokenName ) ;
			if ( gResultList.back().resultBinding != NULL ) {
				resultRootNode->rightNode = gResultList.back().resultBinding;
			} // if : definition is a node

			else {
				resultRootNode->rightToken = new Token ;
				resultRootNode->rightToken->tokenName = gResultList.back().resultStruct.tokenName ;
        resultRootNode->rightToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
			} // if : definition is a token
			gResultList.pop_back();
		} // if : check if is in definition
		else resultRootNode->rightToken = currentNode->rightNode->rightNode->leftToken ;
	} // else : only right token

	result.resultBinding = resultRootNode ;
	gResultList.push_back( result ) ;
  
}  // Cons()

void List( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  TokenTree* resultRootNode = NULL;
  TokenTree* resultWalkNode = NULL ;
  TokenTree* walkNode = currentNode ;
  
  if ( walkNode->rightNode != NULL ) {                        // root node
    resultRootNode = new TokenTree ;
    resultRootNode->backNode = NULL ;
    InitialNode( resultRootNode ) ;
    resultWalkNode = resultRootNode ;
    
    walkNode = walkNode->rightNode ;
    if ( walkNode->leftToken != NULL ) {
      if ( CheckDefinition( walkNode->leftToken->tokenName ) ) {
        PushDefinitionToResultList( walkNode->leftToken->tokenName ) ;
        if ( gResultList.back().resultBinding != NULL )     // definition is a node
          resultWalkNode->leftNode = gResultList.back().resultBinding ;
        else {
          resultWalkNode->leftToken = new Token ;
          resultWalkNode->leftToken->tokenName = gResultList.back().resultStruct.tokenName ;
          resultWalkNode->leftToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
        } // else : definition is a token
        gResultList.pop_back() ;
      } // if : find if in definition
      
      else resultWalkNode->leftToken = walkNode->leftToken ;
    } // if : get token and check definition
    
    if ( walkNode->leftNode != NULL ) {
      if ( gResultList.back().resultBinding != NULL )               // connect node
        resultWalkNode->leftNode = gResultList.back().resultBinding ;
      else if ( gResultList.back().resultStruct.tokenName != "\0" ) {
        resultWalkNode->leftToken = new Token ;
        resultWalkNode->leftToken->tokenName = gResultList.back().resultStruct.tokenName ;
        resultWalkNode->leftToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
      } // if : the result of a fucntion is a token
      
      gResultList.pop_back() ;
    } // if : get node
  } // if
  
  while ( walkNode->rightNode != NULL ){                      // right node
    walkNode = walkNode->rightNode ;
    
    if ( walkNode->leftToken != NULL ) {
      resultWalkNode->rightNode = new TokenTree ;
      resultWalkNode = resultWalkNode->rightNode ;
      InitialNode( resultWalkNode ) ;
      
      if ( CheckDefinition( walkNode->leftToken->tokenName ) ) {
        PushDefinitionToResultList( walkNode->leftToken->tokenName ) ;
        if ( gResultList.back().resultBinding != NULL )     // definition is a node
          resultWalkNode->leftNode = gResultList.back().resultBinding ;
        else {
          resultWalkNode->leftToken = new Token ;
          resultWalkNode->leftToken->tokenName = gResultList.back().resultStruct.tokenName ;
          resultWalkNode->leftToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
        } // else : definition is a token
        gResultList.pop_back() ;
      } // if : find if in definition
      else resultWalkNode->leftToken = walkNode->leftToken ;
    } // if : get token and check definition
    
    if ( walkNode->leftNode != NULL ) {
      resultWalkNode->rightNode = new TokenTree ;
      resultWalkNode = resultWalkNode->rightNode ;
      InitialNode( resultWalkNode ) ;
      if ( gResultList.back().resultBinding != NULL )
        resultWalkNode->leftNode = gResultList.back().resultBinding ;
      
      else if ( gResultList.back().resultStruct.tokenName != "\0" ) {
        resultWalkNode->leftToken = new Token ;
        resultWalkNode->leftToken->tokenName = gResultList.back().resultStruct.tokenName ;
        resultWalkNode->leftToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
      } // if : the result of a fucntion is a token
      
      gResultList.pop_back() ;
    } // if : get node
    
  } // while
  
  result.resultBinding = resultRootNode ;
  gResultList.push_back( result ) ;
} // List()

void Quote( TokenTree* currentNode ) {
  TokenTree * notQuoteNode = currentNode ;
  Result result ;
  InitialResult( result ) ;
  //if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) return ;
  
  if ( notQuoteNode->leftToken != NULL && notQuoteNode->leftToken->tokenName == "quote" ) {
    if ( notQuoteNode->rightNode->leftNode != NULL ) {
      
      if ( notQuoteNode->rightNode->leftNode->leftToken != NULL ) {
        if ( IsFunction( notQuoteNode->rightNode->leftNode->leftToken->tokenName ) ) return ;
      } // if : if is Function -> result list has result -> return
			
      notQuoteNode = notQuoteNode->rightNode->leftNode;
			result.resultBinding = notQuoteNode ;
		} // if : node

		else if ( notQuoteNode->rightNode->leftToken != NULL ) {
      if ( CheckDefinition( notQuoteNode->rightNode->leftToken->tokenName ) ) {
        PushDefinitionToResultList( notQuoteNode->rightNode->leftToken->tokenName ) ;
        return;
      } // if : chech if same symbol
			result.resultStruct.tokenName = notQuoteNode->rightNode->leftToken->tokenName ;
      result.resultStruct.tokenTypeNum = notQuoteNode->rightNode->leftToken->tokenTypeNum;
		} // else : only token
  } // while : Find not quote node

  gResultList.push_back( result ) ;

} // Quote

void Define( TokenTree* currentNode ) {
	DefineSymbol temp ;
  Result result ;
  InitialResult( result ) ;
	temp.symbolName = currentNode->rightNode->leftToken->tokenName ;
  
  for ( int i = 0; i < gDefineSymbols.size(); i++ ) {           // find repeat symbol
    if ( temp.symbolName == gDefineSymbols[i].symbolName ) {
      if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {   // another funcion result
        gDefineSymbols[i].binding =  gResultList.back().resultBinding;
        gResultList.pop_back() ;
      } // if
      
      else gDefineSymbols[i].binding = currentNode->rightNode->rightNode ;     // no funcion result
      result.resultStruct.tokenName =  temp.symbolName + " defined" ;
      gResultList.push_back( result ) ;
      return ;
    } // if
  } // for : find same definition
  
  if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {   // another funcion result
    temp.binding =  gResultList.back().resultBinding;
    gResultList.pop_back() ;
  } // if
  
  else temp.binding = currentNode->rightNode->rightNode ;       // no funcion result
  
	gDefineSymbols.push_back( temp ) ;
	result.resultStruct.tokenName =  temp.symbolName + " defined" ;
  gResultList.push_back( result ) ;
} // Define()

void Car( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  TokenTree* walkNode ;
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      walkNode = gResultList.back().resultBinding ;
      gResultList.pop_back() ;
      if ( walkNode == NULL ) {
        return ;
      } // if : definition is a token
      
      else if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
        if ( walkNode->leftToken != NULL ) {
          result.resultStruct.tokenName = walkNode->leftToken->tokenName ;
          result.resultStruct.tokenTypeNum = walkNode->leftToken->tokenTypeNum ;
        } // if : left side token
        else if ( walkNode->leftNode != NULL )
          result.resultBinding = walkNode->leftNode ;
      } // if : right side has something -> get left side
      
      else if ( walkNode->rightToken == NULL && walkNode->rightNode == NULL ) {
        while ( walkNode->leftNode != NULL )
          walkNode = walkNode->leftNode ;
        if ( walkNode->leftToken != NULL ) {
          result.resultStruct.tokenName = walkNode->leftToken->tokenName ;
          result.resultStruct.tokenTypeNum = walkNode->leftToken->tokenTypeNum ;
        } // if : find left token
      } // if : right side has nothing -> get first token of left side
    } // if : one token but symbol -> find definition
  } // if : only one token
  
  else if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      walkNode = gResultList.back().resultBinding ;
      gResultList.pop_back() ;
      if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
        if ( walkNode->leftToken != NULL ) {
          result.resultStruct.tokenName = walkNode->leftToken->tokenName ;
          result.resultStruct.tokenTypeNum = walkNode->leftToken->tokenTypeNum ;
        } // if : get left side tioken
        else if ( walkNode->leftNode != NULL )
          result.resultBinding = walkNode->leftNode ;
      } // if : right side has something -> get left side
      
      else if ( walkNode->rightToken == NULL && walkNode->rightNode == NULL ) {
        while ( walkNode->leftNode != NULL )
          walkNode = walkNode->leftNode ;
        if ( walkNode->leftToken != NULL ) {
          result.resultStruct.tokenName = walkNode->leftToken->tokenName ;
          result.resultStruct.tokenTypeNum = walkNode->leftToken->tokenTypeNum ;
        } // if : find left token
      } // if : right side has nothing -> get first token of left side
    } // if : check gResultList's binding
    
    else {
      return ;
    } // else : function result is a token -> return the token
    
  } // if : leftnode -> funtion result in resultList
  
  
  gResultList.push_back( result ) ;
} // Car()

void Cdr( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  TokenTree* walkNode ;
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      walkNode = gResultList.back().resultBinding ;
      gResultList.pop_back() ;
      if ( walkNode == NULL ) {
        return ;
      } // if : definition is a token
      
      else if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
        if ( walkNode->rightToken != NULL ) {
          result.resultStruct.tokenName = walkNode->rightToken->tokenName ;
          result.resultStruct.tokenTypeNum = walkNode->rightToken->tokenTypeNum ;
        } // if : get right token in left side
        else if ( walkNode->rightNode != NULL )
          result.resultBinding = walkNode->rightNode ;
      } // if : right side has something -> get right side
    } // if : right token is a symbol -> check definition
    
  } // if : check left token
  
  else if ( currentNode->rightNode->leftNode != NULL ) {
    walkNode = gResultList.back().resultBinding ;
    gResultList.pop_back() ;
    if ( walkNode == NULL ) {
      return ;
    } // if : resultList is a token
    
    else if ( walkNode->rightToken != NULL || walkNode->rightNode != NULL ) {
      if ( walkNode->rightToken != NULL ) {
        result.resultStruct.tokenName = walkNode->rightToken->tokenName ;
        result.resultStruct.tokenTypeNum = walkNode->rightToken->tokenTypeNum ;
      } // if : get right token in left side
      else if ( walkNode->rightNode != NULL )
        result.resultBinding = walkNode->rightNode ;
    } // if : right side has something -> get right side
  } // if : check right node if left side
  
  gResultList.push_back( result ) ;
} // Cdr()

void Is_Atom( TokenTree* currentNode ) {
  
} // Is_Atom

bool Is_Pair( TokenTree* currentNode ) {
  return true ;
} // Is_Pair()

bool Is_List( TokenTree* currentNode ) {
  return true ;
} // Is_List

bool Is_Null( TokenTree* currentNode ) {
  return true ;
} // Is_Null

bool Is_Int( TokenTree* currentNode ) {
  return true ;
} // Is_Int()

bool Is_Real( TokenTree* currentNode ) {
  return true ;
} // Is_Real()

bool Is_Num( TokenTree* currentNode ) {
  return true ;
} // Is_Num()

bool Is_Str( TokenTree* currentNode ) {
  return true ;
} // Is_Str()

bool Is_Bool( TokenTree* currentNode ) {
  return true ;
} // Is_Bool

bool Is_Symbol( TokenTree* currentNode ) {
  return true ;
} // Is_Symbol()

bool Plus( TokenTree* currentNode ) {
  return true ;
} // Plus()

bool Minus( TokenTree* currentNode ) {
  return true ;
} // Minus

bool Div( TokenTree* currentNode ){
  return true ;
} // Div()

bool Mult( TokenTree* currentNode ) {
  return true ;
} // Mult()

bool Not( TokenTree* currentNode ) {
  return true ;
} // Not()

bool And( TokenTree* currentNode ) {
  return true ;
} // And()

bool Or( TokenTree* currentNode ) {
  return true ;
} // Or()

bool Greater( TokenTree* currentNode ) {
  return true ;
} // Greater()

bool GreaterEqual( TokenTree* currentNode ) {
  return true ;
} // GreaterEqual()

bool Less( TokenTree* currentNode ) {
  return true ;
} // Less()

bool LessEqual( TokenTree* currentNode ) {
  return true ;
} // LessEqual()

bool Equal( TokenTree* currentNode ) {
  return true ;
} // Equal()

bool Str_Append( TokenTree* currentNode ) {
  return true ;
} // Str_Append()

bool Is_Str_Greater(  TokenTree* currentNode) {
  return true ;
} // Is_Str_Greater()

bool Is_Str_Less( TokenTree* currentNode ) {
  return true ;
} // Is_Str_Less()

bool Is_Str_Equal( TokenTree* currentNode ) {
  return true ;
} // Is_Str_Equal()

bool Is_Eqv( TokenTree* currentNode ) {
  return true ;
} // Is_Equal()

bool Is_Equal( TokenTree* currentNode ) {
  return true ;
} // Is_Equal()

bool Begin( TokenTree* currentNode ) {
  return true ;
} // Begin()

bool If(  TokenTree* currentNode) {
  return true ;
} // If()

bool Cond( TokenTree* currentNode ) {
  return true ;
} // Cond()

bool Clear_Env( ){
  gDefineSymbols.clear() ;
  return true;
} // Clear_Env()

void FindCorrespondFunction( TokenTree* currentNode, string tokenName  ) {
  if ( tokenName == "cons" ) Cons( currentNode ) ;
  else if ( tokenName == "list" ) List( currentNode ) ;
  else if ( tokenName == "quote" ) Quote( currentNode ) ;
  else if ( tokenName == "define" ) Define(currentNode ) ;
  else if ( tokenName == "car" ) Car( currentNode ) ;
  else if ( tokenName == "cdr" ) Cdr( currentNode ) ;
  else if ( tokenName == "atom?" ) Is_Atom( currentNode ) ;
  else if ( tokenName == "pair?" ) Is_Pair( currentNode ) ;
  else if ( tokenName == "list?" ) Is_List( currentNode ) ;
  else if ( tokenName == "null?" ) Is_Null( currentNode ) ;
  else if ( tokenName == "integer?" ) Is_Int( currentNode ) ;
  else if ( tokenName == "real?" ) Is_Real( currentNode ) ;
  else if ( tokenName == "number?" ) Is_Num( currentNode ) ;
  else if ( tokenName == "string?" ) Is_Str( currentNode ) ;
  else if ( tokenName == "boolean?" ) Is_Bool( currentNode ) ;
  else if ( tokenName == "symbol?" ) Is_Symbol( currentNode ) ;
  else if ( tokenName == "+" ) Plus( currentNode ) ;
  else if ( tokenName == "-" ) Minus( currentNode ) ;
  else if ( tokenName == "*" ) Mult( currentNode ) ;
  else if ( tokenName == "/" ) Div( currentNode ) ;
  else if ( tokenName == "not" ) Not( currentNode ) ;
  else if ( tokenName == "and" ) And( currentNode ) ;
  else if ( tokenName == "or" ) Or( currentNode ) ;
  else if ( tokenName == ">" ) Greater( currentNode ) ;
  else if ( tokenName == ">=" ) GreaterEqual( currentNode ) ;
  else if ( tokenName == "<" ) Less( currentNode ) ;
  else if ( tokenName == "<=" ) LessEqual( currentNode ) ;
  else if ( tokenName == "=" ) Equal( currentNode ) ;
  else if ( tokenName == "string-append" ) Str_Append( currentNode ) ;
  else if ( tokenName == "string>?" ) Is_Str_Greater( currentNode ) ;
  else if ( tokenName == "string<?" ) Is_Str_Less( currentNode ) ;
  else if ( tokenName == "string=?" ) Is_Str_Equal( currentNode ) ;
  else if ( tokenName == "eqv?" ) Is_Eqv( currentNode ) ;
  else if ( tokenName == "equal?" ) Is_Equal( currentNode ) ;
  else if ( tokenName == "begin" ) Begin( currentNode ) ;
  else if ( tokenName == "if" ) If( currentNode ) ;
  else if ( tokenName == "cond" ) Cond( currentNode ) ;
  else if ( tokenName == "clear-environment" ) Clear_Env() ;
} // FindCorrespondFunction()



// ------------------Evaluate Function--------------------- //


bool TraversalTreeAndCheck( TokenTree * currentNode, bool quoteScope ) {
	if ( currentNode != NULL) {
    
    if ( currentNode->leftToken != NULL ){
      if ( currentNode->leftToken->tokenTypeNum == QUOTE ) quoteScope = true ;
      else if ( IsFunction( currentNode->leftToken->tokenName ) ) quoteScope = false   ;
    } // if
    
    if( !TraversalTreeAndCheck( currentNode->rightNode, quoteScope ) ) return false;
    if( !TraversalTreeAndCheck( currentNode->leftNode, quoteScope ) ) return false;
		
    if ( currentNode->leftToken != NULL ) {
			if ( IsFunction( currentNode->leftToken->tokenName ) ) {
        if ( CheckParameter( currentNode, currentNode->leftToken->tokenName )  ){
          FindCorrespondFunction( currentNode, currentNode->leftToken->tokenName ) ;
					// cout << currentNode->leftToken->tokenName << endl;
        } // if
        
				else return false;
			} // if : is Function Check
      
      else if ( currentNode->NeedToBePrimitive == true && !quoteScope ) {
        if ( currentNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->leftToken->tokenName ) )
            SetErrorMsg( UNBOND_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
          else {
						PushDefinitionToResultList( currentNode->leftToken->tokenName ) ;
						if ( gResultList.back().resultBinding == NULL )
							SetErrorMsg( NO_APPLY_ERROR, gResultList.back().resultStruct.tokenName, "\0", NULL, 0, 0  ) ;
						else
							SetErrorMsg( NO_APPLY_ERROR, gResultList.back().resultBinding->leftToken->tokenName, "\0", NULL, 0, 0  ) ;
						gResultList.pop_back() ;
          } // else : if definition but not function
        } // if
              
        else
          SetErrorMsg( NO_APPLY_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
        return false;
      } // if : not function but has to be
		} // if
	} // if

	return true;

} // TraversalTreeAndCheck()

bool EvaluateSExp(){
  gCurrentNode = gTreeRoot ;
  bool quoteScope = false ;
  if ( gCurrentNode == NULL ) {                   // find definition symbol
    if ( gTokens[0].tokenTypeNum == SYMBOL ) {
    	if ( CheckDefinition( gTokens[0].tokenName ) ) {
				PushDefinitionToResultList(gTokens[0].tokenName);
				return true;
			} // if : symbol in definition
			else return false ;
    } // if  : symbol but not in definition
  	else return true ; // else : not symbol
  } // if
  
  else if ( TraversalTreeAndCheck( gCurrentNode, quoteScope ) ) return true;
  else return false ;
} // EvaluateSExp()



// ------------------Main Function--------------------- //

int main() {
	cout << "Welcome to OurScheme!" << endl << endl;
	char uTestNum = '\0';
	while ( uTestNum != '\n' ) {
		uTestNum = cin.get();
	} // while
  
  GlobalVariableInitial() ;

	do {
		cout << "> ";
		if ( GetToken()) {
			if ( SyntaxChecker()) {
				if ( !ExitDetect()) {
          if ( EvaluateSExp() ) PrintFunctionMsg() ;
          else PrintErrorMessage();
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

		GlobalVariableInitial();
	} while ( !gIsEnd );


	cout << endl << "Thanks for using OurScheme!" << endl;
	return 0;

} // main()
