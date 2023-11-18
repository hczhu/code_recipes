import io
import textwrap

def main() -> None:
    with open("/tmp/_test_seek0.txt", "w+") as f:
        f.write("hello\nworld\n")
        f.seek(0)
        assert f.readline().strip() == "hello"
        assert f.readline().strip() == "world"
    
    input_stream = textwrap.dedent(
        """\
        hello
        world\n""")
    with io.StringIO(input_stream) as f:
        # To move the pointer to the end of the file, you can use seek(0, 2).
        # Here, 0 is the offset, and 2 means that the offset should be calculated from the end of the file.
        f.seek(0, 2)
        f.write("again\n")
        f.seek(0)
        assert f.readline().strip() == "hello"
        assert f.readline().strip() == "world"
        assert f.readline().strip() == "again"

main()
