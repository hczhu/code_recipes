import fire
import json
import re
import logging

def add_global_variable(target):
    """ https://grafana.com/docs/grafana/latest/dashboards/variables/add-template-variables/#global-variables """
    for k, v in {
        "__dashboard": "__dashboard",
        "__from": "1594671549254",
        "__to": "1594671549254",
        "__interval": "1m",
        "__interval_ms": "1000",
        "__rate_interval": "1m",
        "__name": "__name",
        "__org": "__org",
        "__user": "__user",
        "__range": "__range",
        "__range_s": "10",
        "__range_ms": "1000",
        "__rate_interval": "5m",
        "timeFilter": "timeFilter",
        "__timeFilter": "__timeFilter",
    }.items():
        if k not in target:
            target[k] = v
    return target

def get_var_ref_res():
    """ https://grafana.com/docs/grafana/latest/dashboards/variables/variable-syntax/#advanced-variable-format-options/ """
    char = "\w" # [a-zA-Z0-9_]
    not_char = "\W" # [^a-zA-Z0-9_]
    simple_ref = r"\$([a-zA-Z_]\w*)(\W|$)"
    curly_ref = r"\${([a-zA-Z_]\w*)(:[^}]*)?}"
    return simple_ref, curly_ref

def run_tests():
    simple_ref, curly_ref = get_var_ref_res()
    for m in re.finditer(simple_ref, "sfas23$key__.dafd"):
        assert m.span(0) == (6, 13), f"{m.group(0)}"
        assert m.span(1) == (7, 12), f"{m.group(1)}"
        assert m.span(2) == (12, 13), f"{m.group(2)}"
        break
    for m in re.finditer(simple_ref, "sfas23$key__"):
        assert m.span(0) == (6, 12), f"{m.group(0)}"
        assert m.span(1) == (7, 12), f"{m.group(1)}"
        assert m.span(2) == (12, 12), f"{m.group(2)}"
        break
    for m in re.finditer(simple_ref, "[$__rate_interval]"):
        assert m.span(0) == (1, 18), f"{m.group(0)}"
        assert m.span(1) == (2, 17), f"{m.group(1)}"
        assert m.span(2) == (17, 18), f"{m.group(2)}"
        break

    for m in re.finditer(curly_ref, "sfas23${key__}asfasdJ"):
        assert m.span(0) == (6, 14), f"{m.group(0)}"
        assert m.span(1) == (8, 13), f"{m.group(1)}"
        break
    for m in re.finditer(curly_ref, "sfas23${key__:01}asfasdJ"):
        assert m.span(0) == (6, 17), f"{m.group(0)}"
        assert m.span(1) == (8, 13), f"{m.group(1)}"
        break

def process_target(target):
    lower = "abcdefghijklmnopqrstuvwxyz"
    digits = "0123456789"
    assert len(lower) == 26, lower
    start_chars = lower + lower.upper() + "_"
    chars = start_chars + digits

    expr = target["expr"]
    res = ""
    i = 0
    while i < len(expr):
        if expr[i] == "$" and i + 1 < len(expr) and (expr[i + 1] in start_chars or expr[i + 1] == "{"):
            j = i + 1 
            if expr[j] == "{": j += 1
            s = j
            while j < len(expr) and expr[j] in chars:
                j += 1
            if expr[i + 1] == "{":
                while expr[j] != "}": j += 1
                j += 1
            i = j
            var_name = expr[s:j].lower()
            if var_name not in target:
                logging.error(f"Variable {var_name} not found in taget: {target}")
            res += target[var_name]
        else:
            res += expr[i]
            i += 1
    return res

def extract_expression_from_grafana_dashboard_json(fname):
    j = json.load(open(fname, "r"))
    targets = sum([
        panel["targets"]
        for panel in j["panels"] if "targets" in panel
    ], [])
    logging.info(f"There are {len(targets)} (expressions)")
    simple_ref, curly_ref = get_var_ref_res()
    for t in targets:
        t = add_global_variable(t)
        if "expr" not in t: continue
        print(process_target(t).replace("\n", " ").replace("\r", " "))

if __name__ == "__main__":
    run_tests()
    fire.Fire(extract_expression_from_grafana_dashboard_json)


