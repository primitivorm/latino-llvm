include "latino/Basic/ASTNode.td"

class DeclNode<DeclNode base, string diagSpelling = "", bit abstract = 0>
    : ASTNode {
  DeclNode Base = base;
  bit Abstract = abstract;
  string DiagSpelling = diagSpelling;
}

class DeclContext {}

def Decl : DeclNode<?, "", 1>;
def TranslationUnit : DeclNode<Decl>, DeclContext;
// def PragmaComment : DeclNode<Decl>;
// def PragmaDetectMismatch : DeclNode<Decl>;
def ExternCContext : DeclNode<Decl>, DeclContext;
def Named : DeclNode<Decl, "named declarations", 1>;
  def Namespace : DeclNode<Named, "namespaces">, DeclContext;
  def UsingDirective : DeclNode<Named>;
  def NamespaceAlias : DeclNode<Named>;
  def Label : DeclNode<Named, "labels">;
  def Type : DeclNode<Named, "types", 1>;
    def TypedefName : DeclNode<Type, "typedefs", 1>;
      def Typedef : DeclNode<TypedefName>;
      def TypeAlias : DeclNode<TypedefName>;
    def UnresolvedUsingTypename : DeclNode<Type>;
    def Tag : DeclNode<Type, "tag types", 1>, DeclContext;
      def Enum : DeclNode<Tag, "enums">;
      def Record : DeclNode<Tag, "structs, unions, classes">;
        def CXXRecord : DeclNode<Record, "classes">;
  def Value : DeclNode<Named, "value declarations", 1>;
    def EnumConstant : DeclNode<Value, "enumerators">;
    def UnresolvedUsingValue : DeclNode<Value>;
    def IndirectField : DeclNode<Value>;
    def Binding : DeclNode<Value>;
    def OMPDeclareReduction : DeclNode<Value>, DeclContext;
    def OMPDeclareMapper : DeclNode<Value>, DeclContext;
    def MSGuid : DeclNode<Value>;
    def Declarator : DeclNode<Value, "declarators", 1>;
      def Field : DeclNode<Declarator, "non-static data members">;
      def MSProperty : DeclNode<Declarator>;
      def Function : DeclNode<Declarator, "functions">, DeclContext;
        def CXXDeductionGuide : DeclNode<Function>;
        def CXXMethod : DeclNode<Function>;
          def CXXConstructor : DeclNode<CXXMethod>;
          def CXXDestructor : DeclNode<CXXMethod>;
          def CXXConversion : DeclNode<CXXMethod>;
      def Var : DeclNode<Declarator, "variables">;
        def ImplicitParam : DeclNode<Var>;
        def ParmVar : DeclNode<Var, "parameters">;
        def Decomposition : DeclNode<Var>;
        def OMPCapturedExpr : DeclNode<Var>;
  def Using : DeclNode<Named>;
  def UsingPack : DeclNode<Named>;
  def UsingShadow : DeclNode<Named>;
    def ConstructorUsingShadow : DeclNode<UsingShadow>;
def LinkageSpec : DeclNode<Decl>, DeclContext;
def Export : DeclNode<Decl>, DeclContext;
def FileScopeAsm : DeclNode<Decl>;
def AccessSpec : DeclNode<Decl>;
def Friend : DeclNode<Decl>;
def StaticAssert : DeclNode<Decl>;
def Block : DeclNode<Decl, "blocks">, DeclContext;
def Captured : DeclNode<Decl>, DeclContext;
// def ClassScopeFunctionSpecialization : DeclNode<Decl>;
def Import : DeclNode<Decl>;
def OMPThreadPrivate : DeclNode<Decl>;
def OMPAllocate : DeclNode<Decl>;
def OMPRequires : DeclNode<Decl>;
def Empty : DeclNode<Decl>;
def RequiresExprBody : DeclNode<Decl>, DeclContext;
def LifetimeExtendedTemporary : DeclNode<Decl>;