#include <clang-c/Index.h>
#include <stdio.h>

enum CXChildVisitResult visitor(CXCursor, CXCursor, CXClientData); 
enum CXChildVisitResult enum_child_visitor(CXCursor, CXCursor, CXClientData); 
enum CXChildVisitResult struct_child_visitor(CXCursor, CXCursor, CXClientData); 
void enum_handler(CXCursor);
void struct_handler(CXCursor);


void print_CXString(CXString s)
{
  printf("%s", clang_getCString(s));
  clang_disposeString(s);
}

void print_cursor_type(CXCursor cursor)
{
  CXType cursor_type = clang_getCursorType(cursor);
  print_CXString(clang_getTypeSpelling(cursor_type));
}

void print_cursor_spelling(CXCursor cursor)
{
  CXString cursor_spelling = clang_getCursorSpelling(cursor);
  print_CXString(cursor_spelling);
}

// void print_cursor_kind(CXCursor cursor){
//   enum CXCursorKind cursor_kind = clang_getCursorKind(cursor);
//   print_CXString(clang_getCursorKindSpelling(cursor_kind));
// }

void print_parent(CXCursor cursor)
{
   CXCursor parent = clang_getCursorLexicalParent(cursor);
   print_CXString(clang_getCursorDisplayName(parent));
}

void enum_handler(CXCursor cursor)
{
  printf("EnumDecl\t\"");
  print_cursor_spelling(cursor);
  printf("\"\t\tParent = \"");
  print_parent(cursor);
  printf("\"\n");
  clang_visitChildren(cursor, enum_child_visitor, 0);
}

void struct_handler(CXCursor cursor)
{
  printf("StructDecl\t\"");
  print_cursor_spelling(cursor);
  printf("\"\t\tParent = \"");
  print_parent(cursor);
  printf("\"\n");
  clang_visitChildren(cursor, struct_child_visitor, 0);
}

enum CXChildVisitResult enum_child_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) 
{
  enum CXCursorKind cursor_kind = clang_getCursorKind(cursor);
  switch(cursor_kind)
  {
    case CXCursor_IntegerLiteral:
    {
      long long enum_const_value = clang_getEnumConstantDeclValue(parent);
      printf("IntegerLiteral\t\t value = \"%lld\" \t\tParent = \"", enum_const_value);
      print_cursor_spelling(parent);
      printf("\"\n");
      break;
    }
    
    case CXCursor_EnumConstantDecl:
      printf("EnumConstantDecl\t\"");
      print_cursor_spelling(cursor);
      printf("\"\t\tParent = \"");
      print_cursor_spelling(parent);
      printf("\"\n");
      break;

    default:
      printf("Unaddressed - ");
      print_CXString(clang_getCursorKindSpelling(cursor_kind));
      printf("\n");
  }
  return CXChildVisit_Recurse;
}

enum CXChildVisitResult struct_child_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  enum CXCursorKind cursor_kind = clang_getCursorKind(cursor); 
  switch(cursor_kind)
  {
    case CXCursor_FieldDecl:
      printf("FieldDecl\t\"");
      print_cursor_spelling(cursor);
      printf("\"\t\ttype = \"");
      print_cursor_type(cursor);
      printf("\"");
      int is_bit_field = clang_Cursor_isBitField(cursor);
      if(is_bit_field){
        printf("\t\t(bitfield = %d)", clang_getFieldDeclBitWidth(cursor));
      }
      printf("\t\tParent = \"");
      print_cursor_spelling(parent);
      printf("\"\n");
      
      break;
    
    case CXCursor_StructDecl:
      struct_handler(cursor);
      break;

    case CXCursor_EnumDecl:
      enum_handler(cursor);
      break;
    
    default:
      printf("Unaddressed - ");
      print_CXString(clang_getCursorKindSpelling(cursor_kind));
      printf("\n");  
  }
  return CXChildVisit_Continue;
}

enum CXChildVisitResult toplevel_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
  enum CXCursorKind cursor_kind = clang_getCursorKind(cursor); 
  switch(cursor_kind)
  {
    case CXCursor_EnumDecl:
      enum_handler(cursor);
      break;

    case CXCursor_StructDecl:
      struct_handler(cursor);
      break;

    default:
      printf("Unaddressed - ");
      print_CXString(clang_getCursorKindSpelling(cursor_kind));
      printf("\n");
  }
  return CXChildVisit_Continue;
}

int main(int argc, char **argv) {
  CXIndex index = clang_createIndex(1, 0);
  CXTranslationUnit TU = clang_parseTranslationUnit(index, 0, argv, argc, 0, 0, CXTranslationUnit_None);
  CXCursor cursor = clang_getTranslationUnitCursor(TU);
  clang_visitChildren(cursor, toplevel_visitor, 0);
  clang_disposeTranslationUnit(TU);
  clang_disposeIndex(index);
  return 0;
}
