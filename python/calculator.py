import operator
import unittest
from dataclasses import dataclass
from typing import Generator, List
from enum import Enum


class TokenType(Enum):
    NUM = 1
    LEFT = 2
    RIGHT = 3
    PLUS = 4
    MINUS = 5


@dataclass
class Token:
    t: TokenType = TokenType.LEFT
    v: int = 0
    s: str = ""


TokenTypeMap = {
    '(': TokenType.LEFT,
    ')': TokenType.RIGHT,
    '+': TokenType.PLUS,
    '-': TokenType.MINUS,
}

_OPS = {TokenType.PLUS: operator.add, TokenType.MINUS: operator.sub}


def gen_tokens(s: str) -> Generator[Token, None, None]:
    p = 0
    while p < len(s):
        while p < len(s) and s[p] == ' ':
            p += 1
        if p < len(s):
            e = p
            while e < len(s) and s[e].isdigit():
                e += 1
            if e > p:
                yield Token(t=TokenType.NUM, v=int(s[p:e]), s=s[p:e])
                p = e
            else:
                yield Token(t=TokenTypeMap[s[p]], s=s[p])
                p += 1


class Calculator:
    def push_num(self, stack: List[Token], numToken: Token):
        if not stack or stack[-1].t in {TokenType.LEFT}:
            stack.append(numToken)
            return
        # must have '+' or '-'
        assert stack[-1].t in {TokenType.PLUS, TokenType.MINUS}, stack[-1]
        op = _OPS[stack[-1].t]
        stack.pop()
        left_op = 0
        if stack and stack[-1].t == TokenType.NUM:
            left_op = stack[-1].v
            stack.pop()
        stack.append(Token(t=TokenType.NUM, v=op(left_op, numToken.v)))

    def push_right(self, stack: List[Token]):
        assert len(stack) >= 2, len(stack)
        assert stack[-1].t == TokenType.NUM, [t.t for t in stack]
        assert stack[-2].t == TokenType.LEFT, [t.t for t in stack]

        numToken = stack.pop()
        stack.pop()
        self.push_num(stack, numToken)

    # s consists of digits, '+', '-', '(', ')', and ' '.
    #  s represents a valid expression.
    #  '+' is not used as a unary operation (i.e., "+1" and "+(2 + 3)" is invalid).
    #  '-' could be used as a unary operation (i.e., "-1" and "-(2 + 3)" is valid).
    # There will be no two consecutive operators in the input.
    # Every number and running calculation will fit in a signed 32-bit integer.
    def calculate(self, s: str) -> int:
        stack = []
        for t in gen_tokens(s):
            if t.t in {TokenType.LEFT, TokenType.PLUS, TokenType.MINUS}:
                stack.append(t)
            elif t.t == TokenType.RIGHT:
                self.push_right(stack)
            else:
                self.push_num(stack, t)
        assert len(stack) == 1, len(stack)
        assert stack[-1].t == TokenType.NUM
        return stack[-1].v


class TestCalculator(unittest.TestCase):
    def setUp(self):
        self.calc = Calculator()

    def test_single_number(self):
        self.assertEqual(self.calc.calculate("3"), 3)

    def test_simple_addition(self):
        self.assertEqual(self.calc.calculate("1 + 1"), 2)

    def test_simple_subtraction(self):
        self.assertEqual(self.calc.calculate("5 - 3"), 2)

    def test_addition_and_subtraction(self):
        self.assertEqual(self.calc.calculate("2-1 + 2"), 3)

    def test_parentheses(self):
        self.assertEqual(self.calc.calculate("(1+(4+5+2)-3)+(6+8)"), 23)

    def test_unary_minus(self):
        self.assertEqual(self.calc.calculate("-2 + 1"), -1)

    def test_unary_minus_with_parens(self):
        self.assertEqual(self.calc.calculate("-(2 + 3)"), -5)

    def test_nested_parentheses(self):
        self.assertEqual(self.calc.calculate("((2+3))"), 5)

    def test_spaces(self):
        self.assertEqual(self.calc.calculate("  3 + 5  "), 8)

    def test_subtraction_result_negative(self):
        self.assertEqual(self.calc.calculate("1 - 5"), -4)

    def test_complex_expression(self):
        self.assertEqual(self.calc.calculate("(1+2)-(3-4)"), 4)


if __name__ == "__main__":
    unittest.main()
