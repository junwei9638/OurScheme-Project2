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


enum TokenType {
  INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, LEFTPAREN, RIGHTPAREN, NO_TYPE
}; // TokenType

enum Error {
  LEFT_ERROR, RIGHT_ERROR, CLOSE_ERROR, EOF_ERROR, NO_ERROR, NOT_S_EXP_ERROR,
  PARAMETER_NUM_ERROR, PARAMETER_TYPE_ERROR, UNBOND_ERROR, NO_APPLY_ERROR,
  NO_RETURN_VAL_ERROR, DIVISION_BY_ZERO_ERROR, FORMAT_ERROR, NON_LIST_ERROR,
  LEVEL_ERROR
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

  Exception( int type ) {
    mErrorType = type ;
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
      while ( peek != '\n' && peek != EOF ) {
        cin.get();
        peek = cin.peek();
      } // while

      if ( peek == '\n' ) cin.get();
      InitialLineColumn();                  // get endl
    } // if

  } // ClearSpaceAndOneLine()

  void ClearInput() {
    char peek = cin.peek();
    while ( peek != '\n' && peek != EOF ) {
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
  } // GlobalVariableInitial()


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


  void SetFromQuote( TokenTree* currentNode  ) {
    currentNode->fromQuote = true ;
    if ( currentNode->leftNode ) SetFromQuote( currentNode->leftNode ) ;
    if ( currentNode->rightNode ) SetFromQuote( currentNode->rightNode ) ;
  } // SetFromQuote()

  // ------------------JudgeMent Function--------------------- //
  bool ExitDetect() {
    int nilExit = -1;
    int exitNil = -1;
    string tokenString = "\0";

    for ( int i = 0 ; i < gTokens.size() ; i++ )
      tokenString += gTokens[i].tokenName;

    nilExit = ( int ) tokenString.find( "(nil.exit)" );
    exitNil = ( int ) tokenString.find( "(exit.nil)" );

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
  } // AtomJudge()

  bool IsFunction( string tokenName ) {
    if ( tokenName == "cons" || tokenName == "list" || tokenName == "quote" || tokenName == "define" ||
         tokenName == "car" || tokenName == "cdr" || tokenName == "atom?" || tokenName == "pair?" ||
         tokenName == "list?" || tokenName == "null?" ||
         tokenName == "integer?" || tokenName == "real?" ||
         tokenName == "number?" || tokenName == "string?" || tokenName == "boolean?" ||
         tokenName == "symbol?" ||
         tokenName == "+" || tokenName == "-" || tokenName == "*" ||
         tokenName == "/" || tokenName == "not" ||
         tokenName == "and" || tokenName == "or" || tokenName == ">" || tokenName == ">=" ||
         tokenName == "<" ||
         tokenName == "<=" || tokenName == "=" || tokenName == "string-append" ||
         tokenName == "string>?" || tokenName == "string<?" || tokenName == "string=?" ||
         tokenName == "eqv?" || tokenName == "equal?" ||
         tokenName == "begin" || tokenName == "if" || tokenName == "cond" ||
         tokenName == "clean-environment" || tokenName == "exit" )
      return true;
    else return false;
  } // IsFunction()

  bool CheckDefinition( string tokenName ) {
    for ( int i = 0 ; i < gDefineSymbols.size() ; i++ ) {
      if ( tokenName == gDefineSymbols[i].symbolName ) return true;
    } // for

    return false;
  } // CheckDefinition()


  bool CheckNonList( TokenTree* currentNode ) {
    TokenTree* walkNode = currentNode ;
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      if ( walkNode->tokenName != "\0" && walkNode->tokenName != "nil" ) {
        return false ;
      } // if : nonlist
    } // while : check right token

    return true ;
    
  } // CheckNonList()
  
  void CheckParameterNum( TokenTree* currentNode, int needNum, string functionName ) {
    int num = 0 ;
    TokenTree* walkNode = currentNode ;
    while ( walkNode->rightNode && walkNode->rightNode->tokenName == "\0"  ) {
      walkNode = walkNode->rightNode ;
      num++ ;
    } // while
    
    if ( num != needNum ) {
      if ( functionName == "define" ) {
        cout << "ERROR (DEFINE format) : " ;
        PrintEvaluateErrorTree( currentNode, true ) ;
        throw Exception( PARAMETER_NUM_ERROR ) ;
      } // if : define format
      
      else if ( functionName == "+" || functionName == "-" || functionName == "*" || functionName == "/" ||
                functionName == "and" || functionName == "or" ||
                functionName == ">" || functionName == ">=" ||
                functionName == "<" || functionName == "<=" || functionName == "=" ||
                functionName == "string-append" || functionName == "string>?" ||
                functionName == "string<?" || functionName == "string=?" || functionName == "begin" ) {
        if ( num < needNum ) {
          cout << "ERROR (incorrect number of arguments) : " + functionName << endl << endl ;
          throw Exception( PARAMETER_NUM_ERROR ) ;
        } // if : not >= needNum
      } // if : function

      else if ( functionName == "cond" ) {
        if ( num < needNum ) {
          cout << "ERROR (COND format) : " ;
          PrintEvaluateErrorTree( currentNode, true ) ;
          throw Exception( FORMAT_ERROR ) ;
        } // if : not >= needNum
      } // if : cond format

      else if ( functionName == "if" ) {
        if ( num != 2 && num != 3 ) {
          cout << "ERROR (incorrect number of arguments) : " + functionName << endl << endl;
          throw Exception( PARAMETER_NUM_ERROR ) ;
        } // if : not 2 or 3
      } // if : if situation
      
      else {
        cout << "ERROR (incorrect number of arguments) : " + functionName << endl << endl;
        throw Exception( PARAMETER_NUM_ERROR ) ;
      } // else
    } // if : throw exception
    
  } // CheckParameterNum()

  bool CompareTwoTrees( TokenTree* compare1, TokenTree* compare2, bool& isSame ) {
    if ( strcmp( compare1->tokenName.c_str(), compare2->tokenName.c_str() ) != 0 )
      isSame = false ;

    if ( ( compare1->leftNode && compare2->leftNode ) )
      CompareTwoTrees( compare1->leftNode, compare2->leftNode, isSame ) ;
    else if ( ( compare1->leftNode && !compare2->leftNode ) ||
              ( !compare1->leftNode && compare2->leftNode ) ) {
      if ( compare1->leftNode && compare1->leftNode->tokenType != NIL )
        isSame = false;
      if ( compare2->leftNode && compare2->leftNode->tokenType != NIL )
        isSame = false;
    } // if

    if ( ( compare1->rightNode && compare2->rightNode ) )
      CompareTwoTrees( compare1->rightNode, compare2->rightNode, isSame ) ;
    else if ( ( compare1->rightNode && !compare2->rightNode ) ||
              ( !compare1->rightNode && compare2->rightNode ) ) {
      if ( compare1->rightNode && compare1->rightNode->tokenType != NIL )
        isSame = false;
      if ( compare2->rightNode && compare2->rightNode->tokenType != NIL )
        isSame = false;
    } // if

    return isSame ;
  } // CompareTwoTrees()

  // ------------------Get Token--------------------- //

  string StringProcess() {
    string inputString = "\0";
    bool closeQuote = false;
    char ch = cin.get();
    gColumn++;
    inputString += ch;
    char peek = cin.peek();
    gAtomType = STRING;

    while ( closeQuote == false && peek != '\n' && peek != EOF ) {

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
        SetErrorMsg( EOF_ERROR, "\"",  0, 0 );
      else if ( peek == '\n' )
        SetErrorMsg( CLOSE_ERROR, "\"",  gLine, gColumn + 1 );
      return "\0";
    } // if                                        // no closing quote

    return inputString;
  } // StringProcess()

  string NumProcess( string atomExp ) {

    if ( atomExp[0] == '+' ) atomExp = atomExp.substr( 1, atomExp.length() - 1 );  // '+' case
    if ( atomExp[0] == '.' ) atomExp = "0" + atomExp;                             // .xxx case
    if ( atomExp[0] == '-' && atomExp[1] == '.' ) atomExp.insert( 1, "0" );        // -.xxx case

    int dot = ( int ) atomExp.find( '.' );
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
        if ( ( ( int ) atomExp[i] >= 48 && ( int ) atomExp[i] <= 57 ) ||
             atomExp[i] == '+' || atomExp[i] == '-' || atomExp[i] == '.' ) {
          if ( atomExp[i] == '+' || atomExp[i] == '-' ) signBit++;
          if ( ( int ) atomExp[i] >= 48 && ( int ) atomExp[i] <= 57 ) digitBit++;
          if ( atomExp[i] == '.' ) dotBit++;
          isNum = true;
        } // if

        else isSymbol = true;
      } // for                                                           // Num Judge

      if ( signBit > 1 || digitBit == 0 || dotBit > 1 ) isNum = false;

      if ( signBit == 1 ) {
        if ( atomExp[0] != '+' && atomExp[0] != '-' ) isNum = false;
      } // if

      gAtomType = SYMBOL;
      if ( isNum && !isSymbol ) atomExp = NumProcess( atomExp );
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

    atomExp = AtomAnalyze( atomExp );

    return atomExp;
  } // GetAtom()

  char GetChar() {
    char peek = '\0';
    peek = cin.peek();
    while ( peek == ' ' || peek == '\t' || peek == '\n' || peek == ';' ) {
      if ( peek == ';' ) {
        while ( peek != '\n' && peek != EOF ) {
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
      SetErrorMsg( EOF_ERROR, "\0",  0, 0 );
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
        int notSymbol = ( int ) token.tokenName.find( '"' );
        if ( notSymbol != token.tokenName.npos ) {
          SetErrorMsg( CLOSE_ERROR, "\"", 0, 0 );
          return false;
        } // if
      } // else

      token.tokenTypeNum = gAtomType;
      gTokens.push_back( token );

      return true;
    } // else

  } // GetToken()

  // ------------------ReadSExp--------------------- //


  bool SyntaxChecker() {
    if ( AtomJudge( gTokens.back().tokenTypeNum ) || gTokens.back().tokenName == "quote" ) {
      return true;
    } // if

    else if ( gTokens.back().tokenTypeNum == LEFTPAREN ) {

      if ( !GetToken() ) return false;

      if ( SyntaxChecker() ) {
        if ( !GetToken() ) return false;

        while ( SyntaxChecker() ) {
          if ( !GetToken() ) return false;
        } // while

        if ( gErrorMsgType != NOT_S_EXP_ERROR ) return false;

        if ( gTokens.back().tokenTypeNum == DOT ) {
          // cout << "Dot " ;
          if ( GetToken() ) {
            if ( SyntaxChecker() ) {
              if ( !GetToken() ) return false;
            } // if  syntax checker

            else {
              if ( gErrorMsgType == NOT_S_EXP_ERROR ) {
                SetErrorMsg( LEFT_ERROR, gTokens.back().tokenName,
                             gTokens.back().tokenLine, gTokens.back().tokenColumn );
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
          SetErrorMsg( RIGHT_ERROR, gTokens.back().tokenName,
                       gTokens.back().tokenLine, gTokens.back().tokenColumn );
          return false;
        } // else
      } // if

      else {
        if ( gErrorMsgType == NOT_S_EXP_ERROR ) {
          SetErrorMsg( LEFT_ERROR, gTokens.back().tokenName,
                       gTokens.back().tokenLine, gTokens.back().tokenColumn );
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
      gTokens.push_back( temp );
      temp.tokenName = "quote";
      temp.tokenTypeNum = QUOTE;
      gTokens.push_back( temp );

      if ( GetToken() ) {
        if ( SyntaxChecker() ) {
          temp.tokenName = ")";
          temp.tokenTypeNum = RIGHTPAREN;
          gTokens.push_back( temp );
          return true;
        } // if :push right paren
        else {

          if ( gErrorMsgType == NOT_S_EXP_ERROR )
            SetErrorMsg( LEFT_ERROR, gTokens.back().tokenName,
                         gTokens.back().tokenLine, gTokens.back().tokenColumn );

          return false;
        } // else
      } // if

      else return false;
    } // if

    SetErrorMsg( NOT_S_EXP_ERROR, gTokens.back().tokenName,
                 gTokens.back().tokenLine, gTokens.back().tokenColumn );
    return false;

  } // SyntaxChecker()


  void CreateTree( TokenTree *currentNode, vector<Token> &Tokens, bool isRight ) {
    if ( AtomJudge( Tokens.front().tokenTypeNum ) || Tokens.front().tokenTypeNum == QUOTE ) {
      if ( isRight ) {
        currentNode->rightNode = new TokenTree;
        InitialNode( currentNode->rightNode );
        currentNode->rightNode->tokenName = Tokens.front().tokenName;
        currentNode->rightNode->tokenType = Tokens.front().tokenTypeNum;
        Tokens.erase( Tokens.begin() );
      } // if : isRight

      else {
        currentNode->leftNode = new TokenTree;
        InitialNode( currentNode->leftNode );
        currentNode->leftNode->tokenName = Tokens.front().tokenName;
        currentNode->leftNode->tokenType = Tokens.front().tokenTypeNum;
        Tokens.erase( Tokens.begin() );
      } // else : isLeft
    } // if


    else if ( Tokens.front().tokenTypeNum == LEFTPAREN ) {
      if ( isRight ) {
        currentNode->rightNode = new TokenTree;
        InitialNode( currentNode->rightNode );
        Tokens.erase( Tokens.begin() );
        CreateTree( currentNode->rightNode, Tokens, false );
        currentNode = currentNode->rightNode;
      } // if : isRight

      else {
        currentNode->leftNode = new TokenTree;
        currentNode->leftNode->needToBePrimitive = true;
        InitialNode( currentNode->leftNode );
        Tokens.erase( Tokens.begin() ) ;
        CreateTree( currentNode->leftNode, Tokens, false );
        currentNode = currentNode->leftNode;
      } // else : is left


      while ( Tokens.front().tokenTypeNum == LEFTPAREN || Tokens.front().tokenTypeNum == QUOTE ||
              AtomJudge( Tokens.front().tokenTypeNum ) ) {
        currentNode->rightNode = new TokenTree;
        InitialNode( currentNode->rightNode );
        CreateTree( currentNode->rightNode, Tokens, false );
        currentNode = currentNode->rightNode;
      } // while


      if ( Tokens.front().tokenTypeNum == DOT ) {
        Tokens.erase( Tokens.begin() );
        CreateTree( currentNode, Tokens, true );
      } // if : Dot

      if ( Tokens.front().tokenTypeNum == RIGHTPAREN ) {
        Tokens.erase( Tokens.begin() );
        return;
      } // if : right paren

    } // if : left paren


  } // CreateTree()


  bool ReadSExp() {
    if ( SyntaxChecker() ) {
      vector<Token> tokens = gTokens;
      gTreeRoot = new TokenTree;
      gCurrentNode = gTreeRoot;
      InitialNode( gTreeRoot );

      CreateTree( gTreeRoot, tokens, false ) ;
      
      gTreeRoot = gTreeRoot->leftNode ;
      gCurrentNode = gTreeRoot;

      return true;
    } // if

    else return false;

  } // ReadSExp()


  // ------------------Print Function--------------------- //


  void PrintSExpTree( TokenTree *currentNode, bool isRightNode, int &layer, bool isError ) {
    static bool lineReturn = false;
    if ( !isRightNode && currentNode->tokenName == "\0" ) {
      cout << "( ";
      lineReturn = false;
      layer++;
    } // if


    if ( lineReturn && currentNode->tokenName == "\0" ) {
      for ( int i = 0 ; i < layer ; i++ )
        cout << "  " ;
    } // if

    if ( currentNode->tokenName != "\0" ) {
      if ( !isRightNode ) {
        if ( IsFunction( currentNode->tokenName ) && !isError &&
             !currentNode->fromQuote ) {
          cout << "#<procedure " ;
          cout << currentNode->tokenName << ">" << endl;
        } // if : is Function

        else if ( currentNode->tokenType == FLOAT ) {
          cout << fixed << setprecision( 3 )
               << round( atof( currentNode->tokenName.c_str() ) * 1000 ) / 1000 << endl;
        } // if : float print

        else cout << currentNode->tokenName << endl;
        lineReturn = true;
      } // if : leftNode
      
      else  {
        if ( currentNode->tokenType != NIL ) {
          for ( int i = 0 ; i < layer ; i++ )
            cout << "  ";
          cout << "." << endl;
          for ( int i = 0 ; i < layer ; i++ )
            cout << "  ";
          if ( IsFunction( currentNode->tokenName ) && !isError ) {
            cout << "#<procedure ";
            cout << currentNode->tokenName << ">" << endl;
          } // if : is Function

          else if ( currentNode->tokenType == FLOAT ) {
            cout << fixed << setprecision( 3 )
                 << round( atof( currentNode->tokenName.c_str() ) * 1000 ) / 1000 << endl;
          } // if : float print

          else cout << currentNode->tokenName << endl;
        } // if : not nil print

      } // else : . node case
    } // if

    if ( currentNode->leftNode )
      PrintSExpTree( currentNode->leftNode, false, layer, isError );

    if ( currentNode->rightNode )
      PrintSExpTree( currentNode->rightNode, true, layer, isError );

    if ( layer > 1 && currentNode->tokenName == "\0" &&
         ( currentNode->rightNode == NULL  ||
           ( currentNode->rightNode && currentNode->rightNode->tokenName != "\0" ) ) ) {
      lineReturn = true;
      layer--;
      for ( int i = 0 ; i < layer ; i++ )
        cout << "  " ;
      cout << ")" << endl;
    } // if : print right paren
    
  } // PrintSExpTree()


  void PrintFunctionMsg( ) {
    int layer = 0;
    if ( gTreeRoot->tokenName != "\0" ) {
      if ( gTreeRoot->tokenType == FLOAT ) {
        cout << fixed << setprecision( 3 )
             << round( atof( gTreeRoot->tokenName.c_str() ) * 1000 ) / 1000 << endl;
      } // if : float print
      else {
        if ( IsFunction( gTreeRoot->tokenName )  ) {
          cout << "#<procedure " ;
          cout << gTreeRoot->tokenName << ">" << endl;
        } // if : is Function
        else cout << gTreeRoot->tokenName << endl;
      } // else
    } // if : only one result

    else if ( !gTreeRoot->leftNode && !gTreeRoot->rightNode ) {
      cout << "nil" << endl;
    } // if : only one node and has no tokenName
    
    else if ( gTreeRoot != NULL ) {
      PrintSExpTree( gTreeRoot, false, layer, false );
      while ( layer > 0  ) {
        layer--;
        for ( int i = 0 ; i < layer ; i++ )
          cout << "  " ;
        cout << ")" << endl;
      } // while : print indent and layer
    } // if

    cout << endl ;
  } // PrintFunctionMsg()

  void PrintNode( TokenTree* currentNode ) {
    if ( currentNode->tokenType == FLOAT ) {
      cout << fixed << setprecision( 3 )
           << round( atof( currentNode->tokenName.c_str() ) * 1000 ) / 1000 ;
    } // if : float print

    else cout << currentNode->tokenName ;

  } // PrintNode()


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

  void PrintEvaluateErrorTree( TokenTree* errorNode, bool isError ) {
    int layer = 0 ;
    PrintSExpTree( errorNode, false, layer, isError );
    while ( layer > 0  ) {
      layer--;
      for ( int i = 0 ; i < layer ; i++ )
        cout << "  " ;
      cout << ")" << endl;
    } // while : print indent and layer

    cout << endl  ;
  } // PrintEvaluateErrorTree()
  
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
    SetFromQuote( currentNode->rightNode->leftNode ) ;
    return currentNode->rightNode->leftNode;
  } // Quote()

  TokenTree* Define( TokenTree* currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    DefineSymbol defined  ;
    InitialNode( resultNode ) ;
    
    CheckParameterNum( currentNode, 2, "define" ) ;
    if ( currentNode->rightNode->leftNode->tokenType != SYMBOL ||
         IsFunction( currentNode->rightNode->leftNode->tokenName ) ) {
      cout << "ERROR (DEFINE format) : " ;
      PrintEvaluateErrorTree( currentNode, true ) ;
      throw Exception( FORMAT_ERROR ) ;
    } // if : first token is not symbol or is a function

    if ( currentNode != gTreeRoot ) {
      cout <<  "ERROR (level of DEFINE)" << endl << endl ;
      throw Exception( LEVEL_ERROR ) ;
    } // if : level of define

    defined.symbolName = currentNode->rightNode->leftNode->tokenName ;
    defined.binding = EvaluateSExp( currentNode->rightNode->rightNode->leftNode ) ;

    gDefineSymbols.push_back( defined ) ;

    resultNode->tokenName = currentNode->rightNode->leftNode->tokenName + " defined" ;
    return resultNode ;
  } // Define()

  TokenTree* List( TokenTree* currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* resultWalkNode = resultNode ;
    InitialNode( resultNode ) ;
    
    if ( currentNode->rightNode ) {
      currentNode = currentNode->rightNode ;
      resultWalkNode->leftNode = EvaluateSExp( currentNode->leftNode ) ;
    } // if : connect to left node
    
    while ( currentNode->rightNode ) {
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
      cout << "ERROR (car with incorrect argument type) : " ;
      PrintNode( walkNode ) ;
      cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // if : parameter type

    resultNode = walkNode->leftNode  ;
    return resultNode ;
  } // Car()

  TokenTree* Cdr( TokenTree *currentNode ) {
    TokenTree * resultNode = NULL;
    TokenTree * walkNode = NULL;
    CheckParameterNum( currentNode, 1, "cdr" ) ;
    
    walkNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    if ( walkNode->tokenName != "\0" ) {
      cout << "ERROR (cdr with incorrect argument type) : " ;
      PrintNode( walkNode ) ;
      cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // if : parameter type
    
    if ( walkNode->rightNode )
      resultNode = walkNode->rightNode  ;
    
    else {
      resultNode = new TokenTree ;
      InitialNode( resultNode ) ;
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
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
  } // Is_Atom()

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
  } // Is_List()

  TokenTree* Is_Null( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "null?" ) ;
    
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    
    if ( judgeNode->tokenType == NIL ) {
      resultNode->tokenName = "#t" ;
      resultNode->tokenType = T ;
    } // if : is atom
    
    else {
      resultNode->tokenName = "nil" ;
      resultNode->tokenType = NIL ;
    } // else : not atom
    
    return resultNode ;
  } // Is_Null()

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
    
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) &&
         !judgeNode->fromQuote ) {
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
  } // Is_Bool()

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
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) && !judgeNode->fromQuote  ) {
        inputNum = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
        resultFloat = inputNum + resultFloat;
        if ( judgeNode->tokenType == FLOAT ) isFloat = true;
      } // if : int or float
      
      else {
        cout << "ERROR (+ with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw Exception
      

    } // while: walkNode go to right node

    if ( isFloat ) {
      sstream << resultFloat;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
    } // if : float result
    else {
      resultInt = ( int ) resultFloat;
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
          resultFloat = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
          firstNum = false ;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // if : first num
          
        else {
          inputNum = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
          resultFloat = resultFloat - inputNum;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // else : not first num
      } // if : int or float
      
      else {
        cout << "ERROR (- with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw Exception
      
    } // while: walkNode go to right node

    if ( isFloat ) {
      sstream << resultFloat;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
    } // if : float result
    else {
      resultInt = ( int ) resultFloat;
      sstream << resultInt;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = INT ;
    } // else : int result

    return resultNode ;
  } // Minus()

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
          resultFloat = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
          firstNum = false ;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // if : first num
          
        else {
          inputNum = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
          
          if ( inputNum == 0.0 ) {
            cout << "ERROR (division by zero) : /" << endl << endl ;
            throw Exception( DIVISION_BY_ZERO_ERROR ) ;
          } // if : division zero
          
          resultFloat = resultFloat / inputNum;
          if ( judgeNode->tokenType == FLOAT ) isFloat = true;
        } // else : not first num
      } // if : int or float
      
      else {
        cout << "ERROR (/ with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw Exception
      
    } // while: walkNode go to right node

    if ( isFloat ) {
      sstream << resultFloat;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
    } // if : float result
    else {
      resultInt = ( int ) resultFloat;
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
        inputNum = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
        resultFloat = inputNum * resultFloat;
        if ( judgeNode->tokenType == FLOAT ) isFloat = true;
      } // if
      
      else {
        cout << "ERROR (* with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw Exception
      

    } // while: walkNode go to right node

    if ( isFloat ) {
      sstream << resultFloat;
      string resultString = sstream.str();
      resultNode->tokenName = resultString ;
      resultNode->tokenType = FLOAT ;
    } // if : float result
    else {
      resultInt = ( int ) resultFloat;
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
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT )  ) {
      compare1 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (> with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) ) {
        compare2 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
        if ( compare1 <= compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (> with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT )  ) {
      compare1 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (>= with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) ) {
        compare2 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
        if ( compare1 < compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (>= with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) ) {
      compare1 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (< with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) ) {
        compare2 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
        if ( compare1 >= compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (< with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) ) {
      compare1 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (<= with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) ) {
        compare2 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
        if ( compare1 > compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (<= with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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
    if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT )  ) {
      compare1 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
    } // if : int or float
    
    else {
      cout << "ERROR (= with incorrect argument type) : " + judgeNode->tokenName ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( ( judgeNode->tokenType == INT || judgeNode->tokenType == FLOAT ) ) {
        compare2 = round( atof( judgeNode->tokenName.c_str() ) * 1000 ) / 1000;
        if ( compare1 != compare2 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (= with incorrect argument type) : " + judgeNode->tokenName ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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
        judgeNode->tokenName.erase( judgeNode->tokenName.begin() );
        judgeNode->tokenName.resize( judgeNode->tokenName.size()-1 );
        resultTokenName = resultTokenName + judgeNode->tokenName ;
      } // if : string
      
      else {
        cout << "ERROR (string-append with incorrect argument type) : " ;
        PrintNode( judgeNode ) ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
    } // while : walk every node

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
    if ( judgeNode->tokenType == STRING ) {
      compare1 = judgeNode->tokenName ;
    } // if : string
    
    else {
      cout << "ERROR (string>? with incorrect argument type) : "  ;
      PrintNode( judgeNode ) ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == STRING ) {
        compare2 = judgeNode->tokenName ;
        if ( strcmp( compare1.c_str(), compare2.c_str() ) <= 0 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : string
      
      else {
        cout << "ERROR (string>? with incorrect argument type) : " ;
        PrintNode( judgeNode ) ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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
    if ( judgeNode->tokenType == STRING ) {
      compare1 = judgeNode->tokenName ;
    } // if : string
    
    else {
      cout << "ERROR (string<? with incorrect argument type) : " ;
      PrintNode( judgeNode ) ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == STRING ) {
        compare2 = judgeNode->tokenName ;
        if ( strcmp( compare1.c_str(), compare2.c_str() ) >= 0 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (string<? with incorrect argument type) : " ;
        PrintNode( judgeNode ) ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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
    if ( judgeNode->tokenType == STRING ) {
      compare1 = judgeNode->tokenName ;
    } // if : string
    
    else {
      cout << "ERROR (string=? with incorrect argument type) : " ;
      PrintNode( judgeNode ) ;
      if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
      else cout << endl << endl ;
      throw Exception( PARAMETER_TYPE_ERROR ) ;
    } // else : throw exception
    
    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      judgeNode = EvaluateSExp( walkNode->leftNode ) ;
      if ( judgeNode->tokenType == STRING ) {
        compare2 = judgeNode->tokenName ;
        if ( strcmp( compare1.c_str(), compare2.c_str() ) != 0 ) isNil  = true ;
        compare1 = compare2 ;
      } // if : int or float
      
      else {
        cout << "ERROR (string=? with incorrect argument type) : " ;
        PrintNode( judgeNode ) ;
        if ( judgeNode->tokenName == "\0" ) PrintEvaluateErrorTree( judgeNode, true ) ;
        else cout << endl << endl ;
        throw Exception( PARAMETER_TYPE_ERROR ) ;
      } // else : throw exception
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

  TokenTree* Is_Eqv( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* compare1 = NULL ;
    TokenTree* compare2 = NULL ;
    InitialNode( resultNode ) ;
    CheckParameterNum( currentNode, 2, "eqv?" ) ;

    compare1 = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    compare2 = EvaluateSExp( currentNode->rightNode->rightNode->leftNode ) ;

    if ( compare1->tokenName != "\0" && compare2->tokenName != "\0" ) {
      if ( AtomJudge( compare1->tokenType ) && AtomJudge( compare2->tokenType ) &&
           compare1->tokenType != STRING && compare2->tokenType != STRING &&
           !compare1->fromQuote && !compare2->fromQuote &&
           compare1->tokenName == compare2->tokenName  ) {
        resultNode->tokenName = "#t" ;
        resultNode->tokenType = T ;
      } // if : same token ,not quote string

      else  {
        resultNode->tokenName = "nil" ;
        resultNode->tokenType = NIL ;
      } // else not same token
    } // if : token compare

    else {
      if ( compare1 == compare2 ) {
        resultNode->tokenName = "#t" ;
        resultNode->tokenType = T ;
      } // if : same node

      else  {
        resultNode->tokenName = "nil" ;
        resultNode->tokenType = NIL ;
      } // else not same node
    } // else : node compare

    return  resultNode ;
  } // Is_Eqv()


  TokenTree* Is_Equal( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    TokenTree* compare1 = NULL ;
    TokenTree* compare2 = NULL ;
    bool isSame = true ;
    InitialNode( resultNode ) ;
    CheckParameterNum( currentNode, 2, "equal?" ) ;

    compare1 = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    compare2 = EvaluateSExp( currentNode->rightNode->rightNode->leftNode ) ;

    if ( compare1->tokenName != "\0" && compare2->tokenName != "\0" ) {
      if ( compare1->tokenName == compare2->tokenName  ) {
        if ( compare1->fromQuote ) {
          if ( compare2->fromQuote ) {
            resultNode->tokenName = "#t";
            resultNode->tokenType = T;
          } // if

          else {
            resultNode->tokenName = "nil" ;
            resultNode->tokenType = NIL ;
          } // else
        } // if

        else if ( !compare1->fromQuote ) {
          if ( !compare2->fromQuote ) {
            resultNode->tokenName = "#t";
            resultNode->tokenType = T;
          } // if

          else {
            resultNode->tokenName = "nil" ;
            resultNode->tokenType = NIL ;
          } // else
        } // if

      } // if : same token ,not quote string

      else  {
        resultNode->tokenName = "nil" ;
        resultNode->tokenType = NIL ;
      } // else not same token
    } // if : token compare

    else {
      isSame = CompareTwoTrees( compare1, compare2, isSame ) ;
      if ( isSame || compare1 == compare2 ) {
        resultNode->tokenName = "#t" ;
        resultNode->tokenType = T ;
      } // if : same node

      else  {
        resultNode->tokenName = "nil" ;
        resultNode->tokenType = NIL ;
      } // else not same node
    } // else : node compare

    return  resultNode ;
  } // Is_Equal()

  TokenTree* Begin( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    TokenTree* walkNode = currentNode ;
    CheckParameterNum( currentNode, 1, "begin" ) ;

    while ( walkNode->rightNode ) {
      walkNode = walkNode->rightNode ;
      resultNode = EvaluateSExp( walkNode->leftNode );
    } // while : walk every parameter

    return  resultNode ;

  } // Begin()

  TokenTree* If( TokenTree *currentNode ) {
    TokenTree* resultNode = NULL ;
    TokenTree* judgeNode = NULL ;

    CheckParameterNum( currentNode, 2, "if" ) ;
    judgeNode = EvaluateSExp( currentNode->rightNode->leftNode ) ;
    if ( judgeNode->tokenType != NIL ) {
      resultNode = EvaluateSExp( currentNode->rightNode->rightNode->leftNode ) ;
    } // if : return first parameter

    else {
      if ( currentNode->rightNode->rightNode->rightNode )
        resultNode = EvaluateSExp( currentNode->rightNode->rightNode->rightNode->leftNode ) ;
      else {
        cout << "ERROR (no return value) : " ;
        PrintEvaluateErrorTree( currentNode, true ) ;
        throw Exception( NO_RETURN_VAL_ERROR ) ;
      } // else : nothing can return -> throw
    } // else : return second parameter

    return  resultNode ;
  } // If()

  TokenTree* Cond( TokenTree *currentNode ) {
    int num = 0 ;
    TokenTree* walkNode = currentNode ;
    TokenTree* walkParameterNode = NULL ;
    TokenTree* resultNode = NULL ;
    TokenTree* judgeNode = NULL ;
    CheckParameterNum( currentNode, 1, "cond" ) ;


    while ( walkNode->rightNode  ) {
      walkNode = walkNode->rightNode ;
      if ( walkNode->leftNode->tokenName != "\0" ) {
        cout << "ERROR (COND format) : " ;
        PrintEvaluateErrorTree( currentNode, true ) ;
        throw Exception( FORMAT_ERROR ) ;
      } // if : parameter is not node

      if ( !walkNode->leftNode->rightNode || !CheckNonList( walkNode->leftNode ) ) {
        cout << "ERROR (COND format) : " ;
        PrintEvaluateErrorTree( currentNode, true ) ;
        throw Exception( FORMAT_ERROR ) ;
      } // if : nothing can return : or parameter non list

      num++ ;
    } // while : check parameter type

    walkNode = currentNode->rightNode ;
    for ( int i = 0; i < num - 1 ; i++ ) {
      judgeNode = EvaluateSExp( walkNode->leftNode->leftNode ) ;
      if ( judgeNode->tokenType != NIL ) {
        walkParameterNode = walkNode->leftNode ;
        while ( walkParameterNode->rightNode ) {
          walkParameterNode = walkParameterNode->rightNode ;
          resultNode = EvaluateSExp( walkParameterNode->leftNode );
        } // while : walk every parameter

        return  resultNode ;
      } // if

      walkNode = walkNode->rightNode ;
    } // for : check num-1 parameter can return

    if ( walkNode->leftNode->leftNode->tokenName == "else" ) {
      walkParameterNode = walkNode->leftNode ;
      while ( walkParameterNode->rightNode ) {
        walkParameterNode = walkParameterNode->rightNode ;
        resultNode = EvaluateSExp( walkParameterNode->leftNode );
      } // while : walk every parameter

      return  resultNode ;
    } // if : return

    else {
      judgeNode = EvaluateSExp( walkNode->leftNode->leftNode ) ;
      if ( judgeNode->tokenType != NIL ) {
        walkParameterNode = walkNode->leftNode ;
        while ( walkParameterNode->rightNode ) {
          walkParameterNode = walkParameterNode->rightNode ;
          resultNode = EvaluateSExp( walkParameterNode->leftNode );
        } // while : walk every parameter

        return  resultNode ;
      } // if : cond true : return result

      else {
        cout << "ERROR (no return value) : " ;
        PrintEvaluateErrorTree( currentNode, true ) ;
        throw Exception( FORMAT_ERROR ) ;
      } // else : no return value

    } // else

  } // Cond()


  TokenTree* Clean_Env( TokenTree *currentNode ) {
    TokenTree* resultNode = new TokenTree ;
    InitialNode( resultNode ) ;
    if ( currentNode != gTreeRoot ) {
      cout <<  "ERROR (level of CLEAN-ENVIRONMENT)" << endl << endl ;
      throw Exception( LEVEL_ERROR ) ;
    } // if : level of define

    resultNode->tokenName = "environment cleaned" ;
    gDefineSymbols.clear();

    return  resultNode ;
  } // Clean_Env()

  TokenTree* Exit( TokenTree *currentNode ) {
    if ( currentNode != gTreeRoot ) {
      cout <<  "ERROR (level of EXIT)" << endl << endl ;
      throw Exception( LEVEL_ERROR ) ;
    } // if : level of define

    CheckParameterNum( currentNode, 0, "exit" ) ;

    return NULL ;
  } // Exit()*/

  TokenTree* FindCorrespondFunction( TokenTree *currentNode, string tokenName ) {
    if ( tokenName == "cons" ) return Cons( currentNode );
    else if ( tokenName == "quote" ) return Quote( currentNode );
    else if ( tokenName == "define" ) return Define( currentNode );
    else if ( tokenName == "list" ) return List( currentNode );
    else if ( tokenName == "car" ) return Car( currentNode );
    else if ( tokenName == "cdr" ) return Cdr( currentNode );
    else if ( tokenName == "atom?" ) return Is_Atom( currentNode );
    else if ( tokenName == "pair?" ) return Is_Pair( currentNode );
    else if ( tokenName == "list?" ) return Is_List( currentNode );
    else if ( tokenName == "null?" ) return Is_Null( currentNode );
    else if ( tokenName == "integer?" ) return Is_Int( currentNode );
    else if ( tokenName == "real?" ) return Is_Real( currentNode );
    else if ( tokenName == "number?" ) return Is_Num( currentNode );
    else if ( tokenName == "string?" ) return Is_Str( currentNode );
    else if ( tokenName == "boolean?" ) return Is_Bool( currentNode );
    else if ( tokenName == "symbol?" ) return Is_Symbol( currentNode );
    else if ( tokenName == "+" ) return Plus( currentNode );
    else if ( tokenName == "-" ) return Minus( currentNode );
    else if ( tokenName == "*" ) return Mult( currentNode );
    else if ( tokenName == "/" ) return Div( currentNode );
    else if ( tokenName == "not" ) return Not( currentNode );
    else if ( tokenName == "and" ) return And( currentNode );
    else if ( tokenName == "or" ) return Or( currentNode );
    else if ( tokenName == ">" ) return Greater( currentNode );
    else if ( tokenName == ">=" ) return GreaterEqual( currentNode );
    else if ( tokenName == "<" ) return Less( currentNode );
    else if ( tokenName == "<=" ) return LessEqual( currentNode );
    else if ( tokenName == "=" ) return Equal( currentNode );
    else if ( tokenName == "string-append" ) return Str_Append( currentNode );
    else if ( tokenName == "string>?" ) return Is_Str_Greater( currentNode );
    else if ( tokenName == "string<?" ) return Is_Str_Less( currentNode );
    else if ( tokenName == "string=?" ) return Is_Str_Equal( currentNode );
    else if ( tokenName == "eqv?" ) return Is_Eqv( currentNode );
    else if ( tokenName == "equal?" ) return Is_Equal( currentNode );
    else if ( tokenName == "begin" ) return Begin( currentNode );
    else if ( tokenName == "if" ) return If( currentNode );
    else if ( tokenName == "cond" ) return Cond( currentNode );
    else if ( tokenName == "clean-environment" ) return Clean_Env( currentNode );
    else if ( tokenName == "exit" ) return Exit( currentNode );
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
          if ( !IsFunction( currentNode->tokenName ) ) {
            cout << "ERROR (unbound symbol) : " + currentNode->tokenName << endl << endl ;
            throw Exception( UNBOND_ERROR );
          } // if : not function
        } // else : throw exception
        
      } // if : if SYMBOL->check definition
    } // if : left token
    
    else {
      if ( currentNode->leftNode->tokenName == "\0" ) {
        TokenTree* judgeNode = NULL ;
        judgeNode = EvaluateSExp( currentNode->leftNode ) ;
        
        currentNode->leftNode = judgeNode ;
        if ( currentNode->leftNode->rightNode == NULL ) return judgeNode;
      } // if : duoble paren

      if ( !CheckNonList( currentNode ) ) {
        cout << "ERROR (non-list) : " ;
        PrintEvaluateErrorTree( currentNode, true ) ;
        throw Exception( NON_LIST_ERROR ) ;
      } // if : Non list check

      if ( currentNode->needToBePrimitive == true ) {
        if ( currentNode->leftNode->tokenType == SYMBOL ) {
          if ( CheckDefinition( currentNode->leftNode->tokenName ) ) {
            DefineSymbol defined = GetDefineSymbol( currentNode->leftNode->tokenName ) ;
            if ( defined.binding->tokenName != "\0" ) {
              currentNode->leftNode = defined.binding ;
            } // if : define is a token

            else {
              cout << "ERROR (attempt to apply non-function) : " ;
              PrintEvaluateErrorTree( defined.binding, true ) ;
              throw Exception( NO_APPLY_ERROR ) ;
            } // else : define is a node-> error
          } // if : is in definition

          else {
            if ( !IsFunction( currentNode->leftNode->tokenName ) ) {
              cout << "ERROR (unbound symbol) : " + currentNode->leftNode->tokenName << endl << endl  ;
              throw Exception( UNBOND_ERROR ) ;
            } // if
          } // else : throw exception
        } // if : if is SYMBOL　

        if ( IsFunction( currentNode->leftNode->tokenName ) ) {
          currentNode = FindCorrespondFunction( currentNode, currentNode->leftNode->tokenName ) ;
        } // if : check is Function
        
        else {
          /*
          if ( currentNode->leftNode->tokenName == "\0" && currentNode->leftNode->leftNode ) {
            cout << "ERROR (attempt to apply non-function) : " ;
            PrintEvaluateErrorTree( currentNode->leftNode, false ) ;
          } // if : left node apply non function

          else cout << "ERROR (attempt to apply non-function) : " + currentNode->leftNode->tokenName
               << endl << endl ;
          throw Exception( NO_APPLY_ERROR ) ;
          */
          cout << "ERROR (attempt to apply non-function) : " + currentNode->leftNode->tokenName
               << endl << endl ;
          throw Exception( NO_APPLY_ERROR ) ;
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
    if ( project.GetToken() ) {
      if ( project.ReadSExp() ) {
        if ( !project.ExitDetect() ) {
          
          try {
            gTreeRoot = project.EvaluateSExp( gTreeRoot ) ;
            project.PrintFunctionMsg() ;
          } // try
          catch( Exception e ) {
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
