from dataclasses import dataclass

kLower = "#"
kUpper = "$"
kDigit = "@"

kSpecials = kLower + kUpper + kDigit

@dataclass(frozen=True, eq=True)
class State:
    has_lower: bool = False
    has_upper: bool = False
    has_digit: bool = False
    length: int = 0
    consumed_chars: int = 0
    last_char: chr = ""
    last2_char: chr = ""
    

cache = {}

def appendChar(st, ch, consumed=0):
    # print(st, ch)
    if ch not in kSpecials and ch == st.last_char and ch == st.last2_char:
        return None
    return State(
        has_lower=st.has_lower or ch.islower() or ch == kLower,
        has_upper=st.has_upper or ch.isupper() or ch == kUpper,
        has_digit=st.has_digit or ch.isdigit() or ch == kDigit,
        length=st.length + 1,
        consumed_chars=st.consumed_chars + consumed,
        last_char=ch,
        last2_char=st.last_char,
    )

def search(st, orig_pass):
    if st is None: return 1e10
    if st.length > 20: return 1e10
    res = 1e10
    
    missing_chars = ""
    
    if not st.has_lower: missing_chars += kLower
    if not st.has_upper: missing_chars += kUpper
    if not st.has_digit: missing_chars += kDigit

    if missing_chars == "" and st.length >= 6:
        # Delete all remaining chars
        res = len(orig_pass) - st.consumed_chars
        if res == 0:
            return 0
    if st in cache:
        return cache[st]
    

    final_len = st.length + len(orig_pass) - st.consumed_chars
    lower_res = 0
    if final_len > 20:
        lower_res = final_len - 20
    elif final_len < 6:
        lower_res = 6 - final_len


    chars_to_insert_or_replace = missing_chars
    if chars_to_insert_or_replace == "":
        chars_to_insert_or_replace = kLower

    # consume one
    if len(orig_pass) > st.consumed_chars:
        consumed_one_st = appendChar(st, orig_pass[st.consumed_chars], 1)
        if consumed_one_st is None:
            # Insert only when the next char is the same as the previous two
            # and the remaining length + current length < 6
            if st.length + len(orig_pass) - st.consumed_chars < 6:
                for ch in chars_to_insert_or_replace:
                    res = min(res,
                        search(
                         appendChar(st, ch),
                          orig_pass
                        ) + 1
                    )   
                    if res == lower_res:
                        cache[st] = res
                        return res
        else:
            res = min(res, search(
                consumed_one_st,
                orig_pass)
            )
            if res == lower_res:
                cache[st] = res
                return res
        # replace next one
        for ch in chars_to_insert_or_replace:
            res = min(res,
                search(
                    appendChar(st, ch, 1),
                    orig_pass
                ) + 1
            )
            if res == lower_res:
                cache[st] = res
                return res
        # Delete next one
        res = min(res,
            search(
                State(
                    has_lower=st.has_lower,
                    has_upper=st.has_upper,
                    has_digit=st.has_digit,
                    length=st.length,
                    consumed_chars=st.consumed_chars + 1,
                    last_char=st.last_char,
                    last2_char=st.last2_char,
                ),
                orig_pass
            ) + 1
        )
        if res == lower_res:
            cache[st] = res
            return res
    else:
        # Append more chars after having consumed all chars
        for ch in chars_to_insert_or_replace:
            res = min(res,
                search(
                    appendChar(st, ch, 0),
                    orig_pass
                ) + 1
            )
            if res == lower_res:
                cache[st] = res
                return res
    cache[st] = res
    # print(st, res)
    return res
    

class Solution(object):
    def strongPasswordChecker(self, password):
        """
        :type password: str
        :rtype: int
        """
        cache.clear()
        return search(
            State(), password
        )


sol = Solution()

assert sol.strongPasswordChecker("aA1") == 3

assert sol.strongPasswordChecker("1337C0d3") == 0


assert sol.strongPasswordChecker("a") == 5

assert sol.strongPasswordChecker("hoAISJDBVWD09232UHJEPODKNLADU1") == 10

assert sol.strongPasswordChecker("ababababababababaaaaa") == 3   

assert sol.strongPasswordChecker("1234567890123456Baaaaa") == 3