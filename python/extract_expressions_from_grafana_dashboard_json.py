import fire
import json
import re
import logging

def extract_expression_from_grafana_dashboard_json(fname):
    j = json.load(open(fname, "r"))
    targets = sum([
        panel["targets"]
        for panel in j["panels"] if "targets" in panel
    ], [])
    logging.info(f"There are {len(targets)} (expressions)")
    var_refs_re = r"\$([a-zA-Z_][a-zA-Z_0-9]+)([^a-zA-Z_0-9]|$)"
    for t in targets:
        expr = t["expr"]
        for m in re.finditer(var_refs_re, expr):
            var = m.group(1).lower()
            if var not in t:
                logging.error(f"Can't find variable {var} in target: {t}")
                continue
            expr = expr[:m.span(0)[0]] + t[var] + expr[m.span(1)[1]:]
        print(expr)

if __name__ == "__main__":
    fire.Fire(extract_expression_from_grafana_dashboard_json)


