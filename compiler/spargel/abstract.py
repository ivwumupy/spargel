"""Abstract Syntax Tree
"""

from pprint import pp

def _fn2cpp(n, fs):
    params = ', '.join([f"{ty_to_cpp(ty)}" for ty in fs.params])
    return f"{ty_to_cpp(fs.ret)} {n}({params});"

class CppCtx:
    def __init__(self):
        self.funcs = []

    def gen_decl(self):
        return '\n'.join([_fn2cpp(f[0], f[1]) for f in self.funcs])

class SourceFile:
    def __init__(self, opens, funcs):
        self.opens = opens
        self.funcs = funcs
        self.scope = None
    def resolve_names(self, mods):
        self.scope = None

        # build scopes for every import
        for o in self.opens:
            print(o)
            name = o.name
            mod = None
            for m in mods:
                if m.name == name:
                    mod = m
            if mod is None:
                raise Exception(f"cannot find module {name}")
            scope = mod.build_scope()
            scope.parent = self.scope
            self.scope = scope

        scope = NameScope()
        scope.parent = self.scope
        self.scope = scope

        self.scope.dump()

        fs = [(f.name, f.resolve_sig(self.scope)) for f in self.funcs]
        fspec = [FuncSpec(f[0], f[1][0], f[1][1]) for f in fs]

        for f in fspec:
            name = f.name
            self.scope.insert(name, f)

        self.scope.dump()

        for f in self.funcs:
            f.resolve_names(self.scope)

    def tyck(self):
        for f in self.funcs:
            f.tyck()

    def cpp(self):
        ctx = CppCtx()
        result = '\n'.join([f.cpp(ctx) for f in self.funcs])
        return ctx.gen_decl() + '\n' + result

# Toplevel

class OpenDecl:
    def __init__(self, name):
        self.name = name

    def resolve_names(self):
        print(f"==> resolving `open {self.name}`")

def ty_to_cpp(ty):
    sty = simp_ty(ty)
    match sty:
        case PrimType():
            match sty.name:
                case "$int":
                    return "int"
                case "$uint":
                    return "unsigned int"
                case "$float":
                    return "float"
                case _:
                    raise Exception("todo")
        case _:
            raise Exception("todo")

class FuncDecl:
    def __init__(self, name, params, ret, body):
        self.name = name
        self.params = params
        self.ret = ret
        self.body = body
        self.scope = None

        self.rparams = None
        self.rret = None

    def resolve_names(self, scope):
        print(f"==> resolving `func {self.name}`")
        self.scope = NameScope()
        self.scope.parent = scope

        for p in self.params:
            p.resolve_names(scope)

        for p in self.params:
            name = p.name
            self.scope.insert(name, p)

        self.scope.dump()

        self.body.resolve_names(self.scope)

    def resolve_sig(self, scope):
        print(f"==> resolving signature of `func {self.name}`")
        self.rparams = [p.type.resolve_type(scope) for p in self.params]
        self.rret = self.ret.resolve_type(scope)
        return self.rparams, self.rret

    def tyck(self):
        print(f"==> tyck func `{self.name}`")
        self.body.tyck()
        if not ty_eq(self.body.ctype, self.rret):
            raise Exception("type not match")

    def cpp(self, ctx):
        params = ', '.join([f"{ty_to_cpp(ty)} {p.name}" for p, ty in zip(self.params, self.rparams)])
        body = None
        ret = ty_to_cpp(self.rret)
        match self.body:
            case BlockExpr():
                body = " ".join([i.cpp(ctx) for i in self.body.items])
            case _:
                body = f"{ret} ret = {self.body.cpp(ctx)}; return ret;"
        return f"{ret} {self.name}({params}) {{ {body} }}"

class FuncParam:
    def __init__(self, name, type):
        self.name = name
        self.type = type
        self.rtype = None
    def __repr__(self):
        return f"FuncParam<{self.name} : {self.rtype}>"
    def resolve_names(self, scope):
        self.rtype = self.type.resolve_type(scope)

# Expr

class BlockExpr:
    def __init__(self, items):
        self.items = items
        self.ctype = None

    def resolve_names(self, scope):
        print(f"==> resolving block")
        scope.dump()

        cur_scope = scope

        for item in self.items:
            match item:
                case LetStmt():
                    item.resolve_names(cur_scope)
                    cur_scope = item.scope
                case _:
                    item.resolve_names(cur_scope)

    def tyck(self):
        for item in self.items:
            item.tyck()
        # terrible hack
        ret_ty = None
        for item in self.items:
            match item:
                case RetStmt():
                    ret_ty = item.ctype
                    break
                case _:
                    pass
        if ret_ty is not None:
            for item in self.items:
                match item:
                    case RetStmt():
                        if not ty_eq(ret_ty, item.ctype):
                            raise Exception("type not match")
                    case _:
                        pass
            self.ctype = ret_ty

class IdentExpr:
    def __init__(self, name):
        self.name = name
        self.rname = None
        self.ctype = None

    def resolve_names(self, scope):
        print(f"==> resolving ident {self.name}")
        scope.dump()

        self.rname = scope.resolve(self.name)

    def resolve_type(self, scope):
        print(f"==> resolving type `{self.name}`")
        print("current scope:")
        scope.dump()

        result = scope.resolve(self.name)
        match result:
            case AliasType():
                print(f"resolve: {self.name} --> {result}")
            case _:
                raise Exception(f"{self.name} is not a type")
        return result

    def tyck(self):
        match self.rname:
            case FuncParam():
                self.ctype = self.rname.rtype
            case LetStmt():
                self.ctype = self.rname.ctype
            case FuncSpec():
                self.ctype = self.rname
            case _:
                raise Exception("error")

    def cpp(self, ctx):
        return self.name
        

class LitExpr:
    def __init__(self, lit):
        self.lit = lit
        self.ctype = None
    def resolve_names(self, scope):
        pass
    def tyck(self):
        try:
            int(self.lit)
            self.ctype = PrimType("$int")
        except ValueError:
            try:
                float(self.lit)
                self.ctype = PrimType("$float")
            except:
                raise Exception("not int or float")
    def cpp(self, ctx):
        return self.lit

def simp_ty(ty):
    match ty:
        case AliasType():
            return ty.real
        case PrimType():
            return ty
        case _:
            raise Exception("unknown type")

def ty_eq(lhs, rhs):
    print("!!!!!!!!!!!!!!!!!!!!!!")
    print(f"CHECK {lhs} == {rhs}")
    print("!!!!!!!!!!!!!!!!!!!!!!")

    s1 = simp_ty(lhs)
    s2 = simp_ty(rhs)

    match (s1, s2):
        case (PrimType(), PrimType()):
            return s1.name == s2.name
        case _:
            return False

class CallExpr:
    def __init__(self, expr, params):
        self.expr = expr
        self.params = params
        self.ctype = None

    def resolve_names(self, scope):
        print(f"==> resolving call")
        scope.dump()

        self.expr.resolve_names(scope)
        for p in self.params:
            p.resolve_names(scope)

    def tyck(self):
        self.expr.tyck()
        for p in self.params:
            p.tyck()

        ft = self.expr.ctype

        match ft:
            case FuncSpec():
                pts = ft.params
                for p, pt in zip(self.params, pts):
                    if not ty_eq(p.ctype, pt):
                        raise Exception("type not match")
                self.ctype = ft.ret
            case _:
                raise Exception("error")

    def cpp(self, ctx):
        foo = None
        match self.expr:
            case IdentExpr():
                foo = self.expr.name
                ctx.funcs.append((self.expr.name, self.expr.ctype))
            case _:
                raise Exception("not normal form")
        params = ', '.join([p.cpp(ctx) for p in self.params])
        return f"{foo}({params})"

class RefTypeExpr:
    def __init__(self, expr):
        self.expr = expr

# Stms

class LetStmt:
    def __init__(self, name, type, expr):
        self.name = name
        self.type = type
        self.expr = expr
        self.scope = None
        self.rtype = None
        self.ctype = None

    def resolve_names(self, scope):
        print(f"==> resolving let \"{self.name}\"")
        scope.dump()

        if self.type is not None:
            self.rtype = self.type.resolve_type(scope)

        self.scope = NameScope()
        self.scope.parent = scope
        self.scope.insert(self.name, self)

        self.expr.resolve_names(self.scope)

    def tyck(self):
        print(f"===> tyck let \"{self.name}\"")

        self.expr.tyck()
        self.ctype = self.expr.ctype
        if self.rtype is not None:
            if not ty_eq(self.ctype, self.rtype):
                raise Exception("type not match")

    def cpp(self, ctx):
        return f"{ty_to_cpp(self.ctype)} {self.name} = {self.expr.cpp(ctx)};"

class RetStmt:
    def __init__(self, expr):
        self.expr = expr
        self.ctype = None
    def resolve_names(self, scope):
        print(f"==> resolving return")
        scope.dump()

        self.expr.resolve_names(scope)
    def tyck(self):
        self.expr.tyck()
        self.ctype = self.expr.ctype
    def cpp(self, ctx):
        return f"return {self.expr.cpp(ctx)};"

# Sema

class TraitSpec:
    def __init__(self, name):
        self.name = name

class ImplSpec:
    def __init__(self):
        pass

class FuncSpec:
    def __init__(self, name, params, ret):
        self.name = name
        self.params = params
        self.ret = ret
    def __repr__(self):
        return "Func<(" + ", ".join(map(str, self.params)) + ") -> " + str(self.ret)

class ModSpec:
    def __init__(self, name, types, funcs, traits, impls):
        self.name = name
        self.types = types
        self.funcs = funcs
        self.traits = traits
        self.impls = impls

    def build_scope(self):
        """
        -> NameScope
        """
        scope = NameScope()
        for t in self.types:
            scope.insert(t.name, t)
        for f in self.funcs:
            pts = [t.resolve(scope) for t in f.params]
            rt = f.ret.resolve(scope)
            scope.insert(f.name, FuncSpec(f.name, pts, rt))
        return scope

class NameScope:
    def __init__(self):
        self.parent = None
        self.names = {}

    def has(self, name):
        return name in self.names

    def insert(self, name, value):
        if name in self.names:
            raise Exception(f"name conflict: {name}")
        self.names[name] = value

    def dump(self, i = 0):
        n = len(self.names)
        print(f"NameScope #{i} [{n} names]")
        for name, value in self.names.items():
            print(f"  {name} -- {value}")
        if self.parent is not None:
            self.parent.dump(i + 1)

    def resolve(self, name):
        if name in self.names:
            return self.names[name]
        if self.parent is not None:
            return self.parent.resolve(name)
        raise Exception(f"unknown name {name}")

class AliasType:
    # name: str
    # real: Type
    def __init__(self, name, real):
        self.name = name
        self.real = real
    def __repr__(self):
        return f"AliasType<\"{self.name}\" -> {self.real}>"

class PrimType:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return f"PrimType<\"{self.name}\">"

class NamedType:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return f"NamedType<\"{self.name}\">"
    def resolve(self, scope):
        return scope.resolve(self.name)

class RefType:
    def __init__(self, inner):
        self.inner = inner
    def __repr__(self):
        return f"RefType<{self.inner}>"

## Test

std_mod = ModSpec(
    name = "std",
    types = [
        AliasType("int", PrimType("$int")),
        AliasType("uint", PrimType("$uint")),
        AliasType("float3", PrimType("$float3")),
    ],
    funcs = [
        FuncSpec("add", [NamedType("int"), NamedType("int")], NamedType("int")),
        FuncSpec("addu", [NamedType("uint"), NamedType("int")], NamedType("int")),
    ],
    traits = [],
    impls = [],
)

#demo = SourceFile(
#    opens = [
#        OpenDecl("std"),
#    ],
#    funcs = [
#        FuncDecl("vertexShader", [FuncParam("vid", IdentExpr("uint")), FuncParam("iid", IdentExpr("uint"))], IdentExpr("int"), BlockExpr([
#            LetStmt("pos", None, CallExpr(IdentExpr("addu"), [IdentExpr("vid"), LitExpr("1")])),
#            LetStmt("pos2", IdentExpr("int"), CallExpr(IdentExpr("addu"), [IdentExpr("vid"), LitExpr("1")])),
#            RetStmt(IdentExpr("pos")),
#        ])),
#        FuncDecl("fragmentShader", [FuncParam("pos", IdentExpr("float3"))], IdentExpr("float3"), BlockExpr([
#            RetStmt(IdentExpr("pos")),
#        ])),
#    ],
#)
#demo.resolve_names(mods = [std_mod])
#demo.tyck()
