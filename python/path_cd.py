from typing import Tuple


class Path:
    def __init__(self, p: str) -> None:
        if len(p) == 0:
            raise RuntimeError("An empty path")
        if p[0] == "~":
            p = "/home" + p[1:]
        # An empty self.segs means "/"
        self.segs = list(filter(lambda s: len(s) > 0, p.split("/")))
        self.is_abs = p[0] == "/"

    def cd(self, p: "Path") -> "Path":
        joined_segs = (
            self.segs if not p.is_abs else []
        ) + p.segs
        stack = []
        for seg in joined_segs:
            match seg:
                case ".":
                    pass
                case "..":
                    if len(stack) > 0:
                        stack.pop()
                case _:
                    stack.append(seg)
        self.segs = stack
        self.is_abs = self.is_abs or p.is_abs
        return self

    def __str__(self) -> str:
        return ("/" if self.is_abs else "") + "/".join(self.segs)

    def resolve_links(self, raw_links: dict[str, str]) -> str:
        assert self.is_abs
        links: dict[str, Path] = {}
        for k, v in raw_links.items():
            k_path = Path(k)
            v_path = Path(v)
            if not (k_path.is_abs and v_path.is_abs):
                raise RuntimeError("Links should only have absolute paths")
            links[str(k_path)] = v_path

        visited: set[str] = set({str(self)})
        current = Path(str(self))

        def find_matched_link(segs: list[str]) -> Tuple[list[str], bool]:
            for l in range(len(segs), -1, -1):
                current.segs = segs[0:l]
                link_to = links.get(str(current), None)
                if link_to is not None:
                    return link_to.segs + segs[l:], True
            return segs, False

        while True:
            current.segs, matched = find_matched_link(current.segs)
            if not matched:
                break
            if str(current) in visited:
                raise RuntimeError(
                    f"A cycle is detected in the links {current}")
            visited.add(str(current))
            # print(current)
        return str(current)


if __name__ == "__main__":
    a = Path("/")
    b = Path("foo/bar/../../baz")
    assert a.is_abs
    assert str(a.cd(b)) == "/baz"
    assert str(Path("/foo/../").cd(Path("./baz"))) == "/baz"
    assert str(Path("/foo/bar/").cd(Path("baz/"))) == "/foo/bar/baz"
    assert Path("/foo/bar/").cd(Path("baz/")).resolve_links(
        {
            "/foo/bar": "/abc",
        },
    ) == "/abc/baz"
    assert Path("/foo/bar/").cd(Path("baz/")).resolve_links(
        {
            "/foo/bar": "/abc",
            "/abc": "/bcd",
            "/bcd/baz": "/xyz",
        },
    ) == "/xyz"
    assert Path("/foo/bar/").cd(Path("baz/")).resolve_links(
        {
            "/foo/bar": "/abc",
            "/foo/bar/baz": "/xyz",
        },
    ) == "/xyz"
    assert Path("/foo/bar/").cd(Path("baz/")).resolve_links(
        {
            "/foo/bar": "/abc",
            "/abc": "/bcd",
            "/bcd/baz": "/xyz",
        },
    ) == "/xyz"
