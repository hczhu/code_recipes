
class Type:
    def __init__(self, v: str|list["Type"]):
        self.v = self.l = None
        if isinstance(v, str):
            self.v = v
        else:
            self.l = v
    
    def __str__(self) -> str:
        return self.v if self.v is not None else (
            "(" + ", ".join([ str(l) for l in self.l ]) + ")"
        )
    
    def __repr__(self) -> str:
        return str(self)

    def __eq__(self, other: "Type") -> bool:
        # print(other, type(other))
        return self.v == other.v and self.l == other.l

    def __hash__(self) -> int:
        return hash(str(self))

    def is_type_var(self) -> bool:
        return self.v is not None and len(self.v) == 1 and self.v.isupper()

class Func:
    def __init__(self, params: list[Type], output: Type):
        self.params = params
        self.output = output
    
    def __str__(self) -> str:
        return f"func{str(Type(self.params))}->{str(self.output)}"


class TypeMisMatch(Exception):
    pass

def get_return_type(f: Func, args: list[Type]) -> Type:
    q = [(f.params, args)]
    type_map = {}
    while len(q) > 0:
        params, args = q[0]
        q = q[1:]
        if isinstance(params, list):
            if len(params) != len(args):
                raise TypeMisMatch()
            q += [(x, y) for x, y in zip(params, args)]
        else:
            if params.v != args.v:
                if not params.is_type_var():
                    raise TypeMisMatch()
                if params not in type_map:
                    # print(params, "-->", args)
                    type_map[params] = args
                elif type_map[params] != args:
                    raise TypeMisMatch()
            else:
                if params.v is None:
                    if len(params.l) != len(args.l):
                        raise TypeMisMatch()
                    q += [
                        (x, y) for x, y in zip(params.l, args.l)
                    ]
    
    # print(type_map)
    root = Type([f.output])
    q =[(f.output, root, 0)]
    while len(q) > 0:
        t, parent, idx = q[0]
        q = q[1:]
        if t.is_type_var():
            if t not in type_map:
                raise TypeMisMatch()
            else:
                parent.l[idx] = type_map[t]
        elif t.l is not None:
            q += [
                (c, t, i) for i, c in enumerate(t.l)
            ]
    return root.l[0]



if __name__ == "__main__":
    assert Type("int") == Type("int")
    assert Type([Type("int"), Type("str")]) != Type("int")
    assert Type([Type("int"), Type("str")]) != Type([Type("int")])
    assert Type([Type("int"), Type("str")]) == Type([Type("int"), Type("str")])

    i = Type("int")
    s = Type("str")
    T = Type("T")
    X = Type("X")
    x = Type("x")

    assert str(i) == "int"
    assert str(Type([i, s, i, Type([s, i, T])])) == "(int, str, int, (str, int, T))"
    assert str(Type([i, s, i, Type([s, i, Type([s, T, i]), T])])) == "(int, str, int, (str, int, (str, T, int), T))"

    f = Func([i, s, T, Type([s, i, T]), X], T)
    assert str(f) == "func(int, str, T, (str, int, T), X)->T", str(f)

    assert get_return_type(f, [i, s, Type([s, s]), Type([s, i, Type([s, s])]), x]) == Type([s, s])
    try:
        get_return_type(f, [i, s, Type([s, s]), Type([s, i, Type([s, s, X])]), x])
    except TypeMisMatch:
        pass
    else:
        assert False, "Expect a TypeMisMatch exception"

    f = Func([i, s, T, Type([s, i, T]), X], Type([T, X, T]))
    assert get_return_type(f, [i, s, Type([s, s]), Type([s, i, Type([s, s])]), x]) == Type([Type([s, s]), x, Type([s, s])])