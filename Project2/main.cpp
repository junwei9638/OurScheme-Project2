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
  string definedName;
  int definedType ;
	TokenTree * binding;
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
	INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, LEFTPAREN, RIGHTPAREN, NO_TYPE
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

void InitialDefineStruct( DefineSymbol & define ) {
  define.definedName = "\0" ;
  define.symbolName = "\0" ;
  define.definedType = NO_TYPE ;
  define.binding = NULL ;
} //InitialResult()


void PushDefinitionToResultList( string tokenName ) {
  Result result ;
  InitialResult( result ) ;
  for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
    if ( tokenName == gDefineSymbols[i].symbolName ) {
      if ( gDefineSymbols[i].definedName != "\0" ) {
        result.resultStruct.tokenName = gDefineSymbols[i].definedName ; // single node
        result.resultStruct.tokenTypeNum = gDefineSymbols[i].definedType ;
      } // if : only a token
      
      else result.resultBinding = gDefineSymbols[i].binding ;
      gResultList.push_back( result ) ;
    } // if
  } // for
}  // PushDefinitionToResultList()

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
} // CheckDefinition()

bool FindRightToken( TokenTree * currentNode ) {
  
  if( currentNode != NULL ) {
    if ( currentNode->rightToken != NULL ) return true ;
    
    if ( FindRightToken( currentNode->leftNode ) ) return true ;
    if ( FindRightToken( currentNode->rightNode ) ) return true ;
  } // if : current node judge
  
  return false ;
}  // FindRightToken()

bool CheckParameterOfMathSymbol( TokenTree * currentNode ) {
  int functionResultIndex = (int)gResultList.size() - 1 ;
  if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL ) {
    TokenTree* walkNode = currentNode ;
    while( walkNode->rightNode != NULL ) {
      walkNode = walkNode->rightNode ;
      if( walkNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( walkNode->leftNode != NULL ) {
        if ( gResultList[functionResultIndex].resultBinding != NULL ) {
          SetErrorMsg( PARAMETER_TYPE_ERROR, "\0", "+", gResultList.back().resultBinding, 0, 0 ) ;
          return false;
        } // if : function result is a node
        
        else if ( gResultList[functionResultIndex].resultStruct.tokenTypeNum != INT &&
                  gResultList[functionResultIndex].resultStruct.tokenTypeNum != FLOAT ) {
          SetErrorMsg( PARAMETER_TYPE_ERROR, gResultList[functionResultIndex].resultStruct.tokenName, "+", NULL, 0, 0 ) ;
          return false;
        } // if : function result is a token -> int or float
        
        functionResultIndex -- ;
      } // if : leftNode is a function result
      
      if ( walkNode->leftToken != NULL ){
        if ( walkNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( walkNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, walkNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : not in definition
             
          else {
            PushDefinitionToResultList( walkNode->leftToken->tokenName ) ;
            if ( gResultList.back().resultBinding != NULL ) {
              SetErrorMsg( PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, "+", NULL, 0, 0 ) ;
              return false;
            } // if : function result is a node
            
            else if ( gResultList.back().resultStruct.tokenTypeNum != INT &&
                      gResultList.back().resultStruct.tokenTypeNum != FLOAT ) {
              SetErrorMsg( PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, "+", NULL, 0, 0 ) ;
              return false;
            } // if : function result is a token -> int or float
            
            gResultList.pop_back() ;
          } // else : check definition is a node or token
        } // if : Check token in definition
        
        else {
          if ( walkNode->leftToken->tokenTypeNum != INT && walkNode->leftToken->tokenTypeNum != FLOAT ) {
            SetErrorMsg( PARAMETER_TYPE_ERROR, walkNode->leftToken->tokenName, "+", NULL, 0, 0 ) ;
            return false;
          } // if : function result is a token -> int or float
        } // else : left token -> int or float
        
      } // if : leftside is a Token
      
    } // while : check every right node
    
    return true;
  } // if : num check
  
  else {
    if ( currentNode->rightToken != NULL ) {
      SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
      return false ;
    } // if : non list check
    
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightToken != NULL ) {
      SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
      return false ;
    } // if : non list check
    
    SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
    return false ;
  } // else : num check false
} // CheckParameterOfMathSymbol()



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
    } // if : num check

		else {
      
      if ( currentNode->rightToken != NULL  ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode && currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
			SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0);
			return false;
		} // else : num of parameter error
  } // if : cons

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
    } // if : num check
    
    else {
      if ( currentNode->rightToken != NULL   ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0) ;
        return false;
      } // non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else : num error
  } // if : quote

  else if ( tokenName == "define" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode != NULL && currentNode->rightNode->rightNode->rightNode == NULL ) {
      
      if ( currentNode->rightNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum != SYMBOL ) {
          SetErrorMsg( FORMAT_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
          return false;
        } // if : type error
        
        else if ( IsFunction( currentNode->rightNode->leftToken->tokenName ) ) {
          SetErrorMsg( FORMAT_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
          return false;
        } // else : Symbol is a function
      } // if
      
      else if ( currentNode->rightNode->leftNode != NULL ) {
        SetErrorMsg( FORMAT_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // else : type error -> qoute or dot-piar behind define
       
      if ( currentNode->rightNode->rightNode->leftToken != NULL ) {
        if ( IsFunction( currentNode->rightNode->rightNode->leftToken->tokenName ) ) {
          SetErrorMsg( FORMAT_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
          return false;
        } // else : 2nd Symbol is a function
      } // else : check 2nd parameter
      
      return true ;
    } // if : parameter num check
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode && currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( FORMAT_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
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
      
      if ( currentNode->rightNode->leftNode != NULL ) {
        if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenName != "\0" ) {
          SetErrorMsg( PARAMETER_TYPE_ERROR, gResultList.back().resultStruct.tokenName, "car", NULL, 0, 0 ) ;
          return false;
        } // if : result list is not able to be one token
      } // if : function result
      
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
  } // if : car
  
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
      
      
      if ( currentNode->rightNode->leftNode != NULL ) {
        if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenName != "\0" ) {
          SetErrorMsg( PARAMETER_TYPE_ERROR, gResultList.back().resultStruct.tokenName, "car", NULL, 0, 0 ) ;
          return false;
        } // if : result list is not able to be one token
      } // if : function result
      
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
    
  } // if : cdr

  else if ( tokenName == "atom?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if : atom?

  else if ( tokenName == "pair?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if
  
  else if ( tokenName == "list?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "null?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "integer?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "real?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "number?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "string?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "boolean?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "symbol?" ) {
    if ( currentNode->rightNode != NULL && currentNode->rightNode->rightNode == NULL ) {
      if ( currentNode->rightNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      if ( currentNode->rightNode->leftToken != NULL ) {
        if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->rightNode->leftToken->tokenName ) ) {
            SetErrorMsg( UNBOND_ERROR, currentNode->rightNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
            return false ;
          } // if : Unbond error
        } // if : symbol->check definition
      } // if : check token type
      
      return true ;
    } // if : num check
    
    
    else {
      if ( currentNode->rightToken != NULL ) {
        SetErrorMsg( NON_LIST_ERROR, "\0", "\0", currentNode, 0, 0 ) ;
        return false ;
      } // if : non list check
      
      SetErrorMsg( PARAMETER_NUM_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0) ;
      return false ;
    } // else
  } // if

  else if ( tokenName == "+" ) {
    if ( CheckParameterOfMathSymbol( currentNode ) ) return true;
    else return false ;
  } // if : +

  else if ( tokenName == "-" ) {
    if ( CheckParameterOfMathSymbol( currentNode ) ) return true;
    else return false ;
  } // if : -

  else if ( tokenName == "*" ) {
    if ( CheckParameterOfMathSymbol( currentNode ) ) return true;
    else return false ;
  } // if : *

  else if ( tokenName == "/" ) {
    if ( CheckParameterOfMathSymbol( currentNode ) ) return true;
    else return false ;
  } // if : \

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
            if ( gCurrentNode->backNode->NeedToBePrimitive == true )
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
          if ( gCurrentNode->backNode->NeedToBePrimitive == true )
            gCurrentNode = gCurrentNode->backNode;
          while ( gCurrentNode->rightNode != NULL )        // find right node null and above function
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
	if ( AtomJudge(gTokens.back().tokenTypeNum)) {
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
        gCurrentNode = gCurrentNode->backNode ;
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
  if ( !isRightNode && currentNode->leftToken->tokenTypeNum != QUOTE ) {
    cout << "( " ;
    lineReturn = false ;
    layer ++ ;
  } // if
  
  if ( currentNode->leftToken != NULL ) {
    if ( currentNode->leftToken->tokenTypeNum == QUOTE )
      lineReturn = false ;
  } // if

  if ( lineReturn ) {
    for( int i = 0; i < layer; i++ ) cout << "  " ;
  } // if
  
  if ( currentNode->leftToken != NULL && currentNode->leftToken->tokenTypeNum != QUOTE ) {
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
  else if ( gErrorMsgType == NO_APPLY_ERROR ) {
    if ( gErrorBinding == NULL )
      cout << "ERROR (attempt to apply non-function) : " << gErrorMsgName << endl;
    else {
      cout << "ERROR (attempt to apply non-function) : " ;
      PrintSExpTree( gErrorBinding, false, layer ) ;
      for ( int i = 0; i < layer; i++ ) cout << ")" << endl ;
    } // else : not null error
  } // if
	else if ( gErrorMsgType == UNBOND_ERROR )
		cout << "ERROR (unbound symbol) : "<< gErrorMsgName << endl;
	else if ( gErrorMsgType == PARAMETER_TYPE_ERROR )
		cout << "ERROR (" << gErrorFunctionName << " with incorrect argument type) : " << gErrorMsgName << endl ;
  else if ( gErrorMsgType == NON_LIST_ERROR ) {
    cout << "ERROR (non-list) : "  ;
    PrintSExpTree( gErrorBinding, false, layer ) ;
    for ( int i = 0; i < layer; i++ ) cout << ")" << endl ;
  } // if
  
  else if ( gErrorMsgType == FORMAT_ERROR ) {
    cout << "ERROR (DEFINE format) : " ;
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
		} // if : function result
    
    else if ( gResultList.back().resultStruct.tokenName != "\0" ){
      resultRootNode->leftToken = new Token  ;
      resultRootNode->leftToken->tokenName = gResultList.back().resultStruct.tokenName ;
      resultRootNode->leftToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
    } // function result is a token
    
    gResultList.pop_back();
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
		} // if : function result
    
    else if ( gResultList.back().resultStruct.tokenName != "\0" ){
      resultRootNode->rightToken = new Token  ;
      resultRootNode->rightToken->tokenName = gResultList.back().resultStruct.tokenName ;
      resultRootNode->rightToken->tokenTypeNum = gResultList.back().resultStruct.tokenTypeNum ;
    } // function result is a token
    
    gResultList.pop_back();
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
  
  if ( notQuoteNode->leftToken != NULL && notQuoteNode->leftToken->tokenName == "quote" ) {
    if ( notQuoteNode->rightNode->leftNode != NULL ) {
      
      if ( notQuoteNode->rightNode->leftNode->leftToken != NULL ) {
        if ( IsFunction( notQuoteNode->rightNode->leftNode->leftToken->tokenName ) ) return ;
      } // if : if is Function -> result list has result -> return
			
      notQuoteNode = notQuoteNode->rightNode->leftNode ; // not quote node == root
			
      result.resultBinding = notQuoteNode ;
		} // if : node

		else if ( notQuoteNode->rightNode->leftToken != NULL ) {
			result.resultStruct.tokenName = notQuoteNode->rightNode->leftToken->tokenName ;
      result.resultStruct.tokenTypeNum = notQuoteNode->rightNode->leftToken->tokenTypeNum;
		} // else : only token
  } // if : Find not quote node

  gResultList.push_back( result ) ;

} // Quote

void Define( TokenTree* currentNode ) {
	DefineSymbol firstToken ;
  DefineSymbol secondToken ;
  Result secondDefineResult ;
  Result result ;
  InitialResult( secondDefineResult ) ;
  InitialResult( result ) ;
  InitialDefineStruct( firstToken ) ;
  InitialDefineStruct( secondToken ) ;

	firstToken.symbolName = currentNode->rightNode->leftToken->tokenName ;
  
  //--------------double definition -> push second token's value to result list--------------//
  //-------------------------make second token as a function---------------------------------//
  if ( currentNode->rightNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      secondToken.symbolName = currentNode->rightNode->rightNode->leftToken->tokenName ;
    
      for ( int i = 0; i < gDefineSymbols.size(); i++ ) {
        if ( secondToken.symbolName == gDefineSymbols[i].symbolName ) {
          if ( gDefineSymbols[i].binding != NULL ) {
            secondDefineResult.resultBinding = gDefineSymbols[i].binding ;
          } // if : second token is a binding -> push to result list
          
          else {
            secondDefineResult.resultStruct.tokenName = gDefineSymbols[i].definedName ;
            secondDefineResult.resultStruct.tokenTypeNum = gDefineSymbols[i].definedType ;
          } // else : second token is a token -> push to result list
          
          gResultList.push_back( secondDefineResult ) ;
        } // if : find second token in definition
      } // for : find second token in definition
      
    } // if : second token is a symbol
  } // if : if 2nd token is a symbol
  
  
  //--------------find same definition-------------//
  for ( int i = 0; i < gDefineSymbols.size(); i++ ) {           // find repeat symbol
    if ( firstToken.symbolName == gDefineSymbols[i].symbolName ) {
      if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
        gDefineSymbols[i].binding =  gResultList.back().resultBinding;
        gDefineSymbols[i].definedName = "\0" ;
        gDefineSymbols[i].definedType = NO_TYPE ;
        gResultList.pop_back() ;
      } // if : another funcion result has binding
      
      else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenName != "\0" ) {
        gDefineSymbols[i].definedName = gResultList.back().resultStruct.tokenName ;
        gDefineSymbols[i].definedType = gResultList.back().resultStruct.tokenTypeNum ;
        gDefineSymbols[i].binding = NULL ;
        gResultList.pop_back() ;
      } // if : another function result is a token
      
      else if ( currentNode->rightNode->rightNode->leftToken != NULL ) {
        gDefineSymbols[i].definedName = currentNode->rightNode->rightNode->leftToken->tokenName ;
        gDefineSymbols[i].definedType = currentNode->rightNode->rightNode->leftToken->tokenTypeNum ;
        gDefineSymbols[i].binding = NULL ;
      } // if : no function and second parameter is a token
      
      result.resultStruct.tokenName =  firstToken.symbolName + " defined" ;
      gResultList.push_back( result ) ;
      return ;
    } // if : find same definition
  } // for : find same definition
  
  
  //------------------no define ever-------------//
  if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) { // another funcion result
    firstToken.binding =  gResultList.back().resultBinding;
    gResultList.pop_back() ;
  } // if : same definition has binding
  
  else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenName != "\0" ) {
    firstToken.definedName = gResultList.back().resultStruct.tokenName ;
    firstToken.definedType = gResultList.back().resultStruct.tokenTypeNum ;
    gResultList.pop_back() ;
  } // if : another function result is a token
  
  else if ( currentNode->rightNode->rightNode->leftToken->tokenName != "\0" ) {
    firstToken.definedName = currentNode->rightNode->rightNode->leftToken->tokenName ;
    firstToken.definedType = currentNode->rightNode->rightNode->leftToken->tokenTypeNum ;
    firstToken.binding = NULL ;
  } // if : no function and second parameter is a token
  
	gDefineSymbols.push_back( firstToken ) ;
	result.resultStruct.tokenName =  firstToken.symbolName + " defined" ;
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
      
      gResultList.pop_back() ;
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
      
      gResultList.pop_back() ;
    } // if : right token is a symbol -> check definition
  } // if : check left token
  
  else if ( currentNode->rightNode->leftNode != NULL ) {
    walkNode = gResultList.back().resultBinding ;
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
    
    gResultList.pop_back() ;
  } // if : check right node if left side
  
  
  gResultList.push_back( result ) ;
} // Cdr()

void Is_Atom( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> nil
    
    else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenName != "\0" ) {
      if ( AtomJudge( gResultList.back().resultStruct.tokenTypeNum ) ) {
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // if : Check if is atom
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else
    } // if : result is a token -> return the token
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else {
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      gResultList.pop_back() ;
    } // if : left token is a symbol
    
    else if ( AtomJudge( currentNode->rightNode->leftToken->tokenTypeNum ) ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : Check if is atom
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not atom
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Atom

void Is_Pair( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a pair -> true
    
    else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenName != "\0" ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a token -> Nil
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // if : definition is a binding
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : definition is a token
      
      gResultList.pop_back() ;
      
    } // if : definition check
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol -> not definition
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Pair()

void Is_List( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      if ( FindRightToken( gResultList.back().resultBinding ) ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum = NIL ;
      } // if : find right token
      
      else {
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : no right token
    } // if : result is a pair -> true
    
    else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenName != "\0" ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a token -> Nil
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        if ( FindRightToken( gResultList.back().resultBinding ) ) {
          result.resultStruct.tokenName = "nil" ;
          result.resultStruct.tokenTypeNum = NIL ;
        } // if : find right token
        
        else {
          result.resultStruct.tokenName = "#t" ;
          result.resultStruct.tokenTypeNum = T ;
        } // else : no right token
      } // if : definition is a binding
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : definition is a token
      
      gResultList.pop_back() ;
      
    } // if : definition check
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol -> not definition
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_List

void Is_Null( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> false
    
    else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenTypeNum == NIL ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a nil -> t
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not nil result
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else if ( gResultList.back().resultStruct.tokenTypeNum == NIL ){
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : not nil result
      
      gResultList.pop_back() ;
      
    } // if : definition check
    
    else if ( currentNode->rightNode->leftToken->tokenTypeNum == NIL ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : 2nd token is nil
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol & not nil
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Null

void Is_Int( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> false
    
    else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenTypeNum == INT ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a int
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not nil result
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else if ( gResultList.back().resultStruct.tokenTypeNum == INT ){
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : not nil result
      
      gResultList.pop_back() ;
      
    } // if : definition check
    
    else if ( currentNode->rightNode->leftToken->tokenTypeNum == INT ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : 2nd token is INT
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol & not int
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Int()

void Is_Real( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> false
    
    else if ( gResultList.size() > 0 && ( gResultList.back().resultStruct.tokenTypeNum == INT ||
              gResultList.back().resultStruct.tokenTypeNum == FLOAT ) ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a int or float
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not nil result
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else if ( gResultList.back().resultStruct.tokenTypeNum == INT || gResultList.back().resultStruct.tokenTypeNum == FLOAT ){
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : not nil result
      
      gResultList.pop_back() ;
      
    } // if : definition check
    
    else if ( currentNode->rightNode->leftToken->tokenTypeNum == INT ||
              currentNode->rightNode->leftToken->tokenTypeNum == FLOAT ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : 2nd token is INT or float
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol & not int or float
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Real()

void Is_Num( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> false
    
    else if ( gResultList.size() > 0 && ( gResultList.back().resultStruct.tokenTypeNum == INT ||
              gResultList.back().resultStruct.tokenTypeNum == FLOAT ) ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a int or float
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not nil result
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else if ( gResultList.back().resultStruct.tokenTypeNum == INT || gResultList.back().resultStruct.tokenTypeNum == FLOAT ){
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : not nil result
      
      gResultList.pop_back() ;
      
    } // if : definition check
    
    else if ( currentNode->rightNode->leftToken->tokenTypeNum == INT ||
              currentNode->rightNode->leftToken->tokenTypeNum == FLOAT ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : 2nd token is INT
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol & not int or float
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Num()

void Is_Str( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> false
    
    else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenTypeNum == STRING  ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a String
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not nil result
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else if ( gResultList.back().resultStruct.tokenTypeNum == STRING ){
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : not nil result
      
      gResultList.pop_back() ;
    } // if : definition check
    
    else if ( currentNode->rightNode->leftToken->tokenTypeNum == STRING ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : 2nd token is String
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol & not String
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Str()

void Is_Bool( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> false
    
    else if ( gResultList.size() > 0 && ( gResultList.back().resultStruct.tokenTypeNum == NIL ||
              gResultList.back().resultStruct.tokenTypeNum == T ) ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a T or NIL
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not nil result
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else if ( gResultList.back().resultStruct.tokenTypeNum == NIL ||
               gResultList.back().resultStruct.tokenTypeNum == T ){
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      else {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // else : not nil result
      
      gResultList.pop_back() ;
    } // if : definition check
    
    else if (  currentNode->rightNode->leftToken->tokenTypeNum == NIL ||
               currentNode->rightNode->leftToken->tokenTypeNum == T ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : 2nd token is NIL or T
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol & not NIL or t
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Bool

void Is_Symbol( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  if ( currentNode->rightNode->leftNode != NULL ) {
    if ( gResultList.size() > 0 && gResultList.back().resultBinding != NULL ) {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // if : result is a pair -> false
    
    else if ( gResultList.size() > 0 && gResultList.back().resultStruct.tokenTypeNum == SYMBOL ) {
      result.resultStruct.tokenName = "#t" ;
      result.resultStruct.tokenTypeNum = T ;
    } // if : result is a T or NIL
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not nil result
    
    gResultList.pop_back() ;
  } // if : left is a node -> check function result
  
  if ( currentNode->rightNode->leftToken != NULL ) {
    if ( currentNode->rightNode->leftToken->tokenTypeNum == SYMBOL ) {
      PushDefinitionToResultList( currentNode->rightNode->leftToken->tokenName ) ;
      if ( gResultList.back().resultBinding != NULL ) {
        result.resultStruct.tokenName = "nil" ;
        result.resultStruct.tokenTypeNum= NIL ;
      } // if : definition is a binding
      
      else {
        result.resultStruct.tokenName = "#t" ;
        result.resultStruct.tokenTypeNum = T ;
      } // else : definition is a token
      
      gResultList.pop_back() ;
    } // if : definition check
    
    else {
      result.resultStruct.tokenName = "nil" ;
      result.resultStruct.tokenTypeNum= NIL ;
    } // else : not symbol & not NIL or t
  } // if : left token has something
  
  gResultList.push_back( result ) ;
} // Is_Symbol()

void Plus( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  int resultInt  = 0 ;
  float inputNum = 0.0 ;
  float resultFloat  = 0.0 ;
  bool isFloat = false;
  ostringstream ss;
  TokenTree * walkNode = currentNode ;
  
  while( walkNode->rightNode != NULL ) {
    walkNode = walkNode->rightNode ;
    if ( walkNode->leftNode != NULL ) {
      inputNum = round( atof( gResultList.back().resultStruct.tokenName.c_str() ) * 1000 ) / 1000 ;
      resultFloat = inputNum + resultFloat ;
      
      if ( gResultList.back().resultStruct.tokenTypeNum == FLOAT ) {
        isFloat = true ;
      } // if : float result
      gResultList.pop_back() ;
    } // if : leftNode -> function result
    
    if ( walkNode->leftToken != NULL ) {
      inputNum = round( atof( walkNode->leftToken->tokenName.c_str() ) * 1000 ) / 1000 ;
      resultFloat = inputNum + resultFloat ;
      
      if ( walkNode->leftToken->tokenTypeNum == FLOAT ) {
        isFloat = true ;
      } // if : float result
    } // if : leftNode -> function result
    
  } // while: walkNode go to right node
  
  if ( isFloat ) {
    ss  << resultFloat  ;
    string resultString(ss.str());
    result.resultStruct.tokenName = resultString ;
    result.resultStruct.tokenTypeNum = FLOAT ;
  } // if : float result
  else {
    resultInt = (int) resultFloat ;
    ss  << resultInt  ;
    string resultString(ss.str());
    result.resultStruct.tokenName = resultString ;
    result.resultStruct.tokenTypeNum = INT ;
  } // else : int result
  
  gResultList.push_back( result ) ;
} // Plus()

void Minus( TokenTree* currentNode ) {
  Result result ;
  InitialResult( result ) ;
  int resultInt  = 0 ;
  float inputNum = 0.0 ;
  float resultFloat  = 0.0 ;
  bool isFloat = false ;
  ostringstream ss;
  TokenTree * walkNode = currentNode ;
  
  walkNode = walkNode->rightNode ;
  if ( walkNode->leftNode != NULL ) {
    resultFloat = round( atof( gResultList.back().resultStruct.tokenName.c_str() ) * 1000 ) / 1000 ;
    
    if ( gResultList.back().resultStruct.tokenTypeNum == FLOAT ) {
      isFloat = true ;
    } // if : float result
    gResultList.pop_back() ;
  } // if : leftNode -> function result
  
  if ( walkNode->leftToken != NULL ) {
    resultFloat = round( atof( walkNode->leftToken->tokenName.c_str() ) * 1000 ) / 1000 ;
    
    if ( walkNode->leftToken->tokenTypeNum == FLOAT ) {
      isFloat = true ;
    } // if : float result
  } // if : leftNode -> function result
  
  while( walkNode->rightNode != NULL ) {
    walkNode = walkNode->rightNode ;
    if ( walkNode->leftNode != NULL ) {
      inputNum = round( atof( gResultList.back().resultStruct.tokenName.c_str() ) * 1000 ) / 1000 ;
      resultFloat = resultFloat - inputNum ;
      
      if ( gResultList.back().resultStruct.tokenTypeNum == FLOAT ) {
        isFloat = true ;
      } // if : float result
      gResultList.pop_back() ;
    } // if : leftNode -> function result
    
    if ( walkNode->leftToken != NULL ) {
      inputNum = round( atof( walkNode->leftToken->tokenName.c_str() ) * 1000 ) / 1000 ;
      resultFloat = resultFloat - inputNum ;
      
      if ( walkNode->leftToken->tokenTypeNum == FLOAT ) {
        isFloat = true ;
      } // if : float result
    } // if : leftNode -> function result
    
  } // while: walkNode go to right node
  
  if ( isFloat ) {
    ss  << resultFloat  ;
    string resultString(ss.str());
    result.resultStruct.tokenName = resultString ;
    result.resultStruct.tokenTypeNum = FLOAT ;
  } // if : float result
  else {
    resultInt = (int) resultFloat ;
    ss  << resultInt  ;
    string resultString(ss.str());
    result.resultStruct.tokenName = resultString ;
    result.resultStruct.tokenTypeNum = INT ;
  } // else : int result
  
  gResultList.push_back( result ) ;
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

void Clear_Env( ){
  gDefineSymbols.clear() ;
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
		
    if ( currentNode->NeedToBePrimitive == true && currentNode->leftToken != NULL ) {
			if ( IsFunction( currentNode->leftToken->tokenName ) ) {
        if ( CheckParameter( currentNode, currentNode->leftToken->tokenName )  ){
          FindCorrespondFunction( currentNode, currentNode->leftToken->tokenName ) ;
					// cout << currentNode->leftToken->tokenName << endl;
        } // if : find correspond function
        
				else return false;
			} // if : is Function Check
      
      else if ( !quoteScope ) {
        if ( currentNode->leftToken->tokenTypeNum == SYMBOL ) {
          if ( !CheckDefinition( currentNode->leftToken->tokenName ) )
            SetErrorMsg( UNBOND_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
          else {
						PushDefinitionToResultList( currentNode->leftToken->tokenName ) ;
						if ( gResultList.back().resultBinding == NULL )
							SetErrorMsg( NO_APPLY_ERROR, gResultList.back().resultStruct.tokenName, "\0", NULL, 0, 0  ) ;
						else
							SetErrorMsg( NO_APPLY_ERROR, "\0", "\0", gResultList.back().resultBinding, 0, 0  ) ;
						gResultList.pop_back() ;
          } // else : if definition but not function
        } // if
              
        else
          SetErrorMsg( NO_APPLY_ERROR, currentNode->leftToken->tokenName, "\0", NULL, 0, 0 ) ;
        return false;
      } // if : not function but has to be
		} // if : need to be primitive
	} // if : current node != null

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
